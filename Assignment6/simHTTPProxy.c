// Name: Soumen Dutta
// Roll number: 17CS10057

// Name: Aditya Sawant
// Roll number: 17CS10060

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>

#define BUFFER_SIZE 1000
#define MAX_CONN 200
#define HEADER_LEN 500

void tokenize_address(char *str, char *type, char *host, char *port, char *path, char *vers)
{
  char *arr1, *arr2, *arr3;
  arr1 = strtok(str, " ");
  arr2 = strtok(NULL, " ");
  arr3 = strtok(NULL, " ");
  strcpy(type, arr1);
  strcpy(vers,arr3);
  str = arr2;

  arr1 = strtok(str, "/");
  if(arr1[strlen(arr1)-1] == ':')
  {
    arr2 = strtok(NULL, "/");
    arr3 = strtok(NULL, " ");
    str = arr2;
    strcpy(path, "/");
    if (arr3 != NULL)
      strcat(path, arr3);
  }
  else
  {
    arr2 = strtok(NULL, " ");
    str = arr1;
    strcpy(path, "/");
    if (arr2 != NULL)
      strcat(path, arr2);
  }

  arr1 = strtok(str, ":");
  arr2 = strtok(NULL, ":");
  strcpy(host, arr1);
  if (arr2 == NULL)
    strcpy(port, "80");
  else
    strcpy(port, arr2);
}

int main(int argc, char *argv[])
{
  signal(SIGPIPE, SIG_IGN);
  // Take command line arguments
  char myport[] = "8080", servport[] = "8080", servip[16] = "172.16.2.30";
  if (argc == 2)
  {
    strcpy(myport, argv[1]);
  }

  int sockfd;
  int cli_connfd[MAX_CONN];
  int ser_connfd[MAX_CONN];
  int status[MAX_CONN];
  int conntype[MAX_CONN];
  char address[MAX_CONN][HEADER_LEN];
  struct sockaddr_in myaddr, servaddr, cliaddr;
  char buffer[BUFFER_SIZE];
  struct hostent* tmp;
  struct in_addr addr;
  char tokenize[HEADER_LEN];
  char strexit[10];
  char port[200],path[200],host[200],type[200],vers[200];
  char end[]="\r\n\r\n";

  // Create socket file descriptor
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
  {
    perror("\nSocket creation failed ");
    exit(EXIT_FAILURE);
  }

  // Make the socket file descriptor non-blocking
  if(fcntl(sockfd, F_SETFL, O_NONBLOCK) != 0)
  {
    perror("\nNon-blocking socket failed ");
    exit(EXIT_FAILURE);
  }

  // Proxy server information
  bzero(&myaddr, sizeof(myaddr));
  myaddr.sin_family = AF_INET;
  myaddr.sin_addr.s_addr = htons(INADDR_ANY);
  myaddr.sin_port = htons(atoi(myport));

  // Bind the TCP socket with the server address
  if (bind(sockfd, (struct sockaddr*)&myaddr, sizeof(myaddr)) != 0)
  {
    perror("\nSocket bind failed ");
    exit(EXIT_FAILURE);
  }

  // Start listening to the TCP socket
  if ((listen(sockfd, 5)) != 0)
  {
    perror("\nListen failed ");
    exit(EXIT_FAILURE);
  }

  printf("Proxy running on port %s.\n", myport);

  fd_set readfd;
  int i = 0, nfds = sockfd + 2;
  char temp[BUFFER_SIZE];
  char temp2[BUFFER_SIZE];
  while (i < MAX_CONN)
  {
    // Wait for connection request
    FD_ZERO(&readfd);
    FD_SET(sockfd, &readfd);
    FD_SET(STDIN_FILENO, &readfd);
    for(int j = 0; j < i ;j++)
    {
      //if (status[j] != -1)
        FD_SET(cli_connfd[j], &readfd);
        if(cli_connfd[j]>nfds)
        	nfds=cli_connfd[j];
      if (status[j] == 1)
      {
        FD_SET(ser_connfd[j], &readfd);
        if(ser_connfd[j]>nfds)
        	nfds=ser_connfd[j];
       }

    }

    select(nfds+1, &readfd, NULL, NULL, NULL);

    if (FD_ISSET(sockfd, &readfd))
    {
      socklen_t len;
      bzero(&cliaddr, sizeof(cliaddr));
      len = sizeof(cliaddr);

      // Accept the TCP connection request
      cli_connfd[i] = accept(sockfd, (struct sockaddr*)&cliaddr, &len);

      if (cli_connfd[i] < 0)
      {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
          continue;
        }
        else
        {
          perror("\nTCP server accept failed ");
          exit(EXIT_FAILURE);
        }
      }
      else
      {
        printf("Connection accepted from %s:%d\n", inet_ntoa(cliaddr.sin_addr),ntohs(cliaddr.sin_port));

        // Make the connection non-blocking
        if(fcntl(cli_connfd[i], F_SETFL, O_NONBLOCK) != 0)
        {
          perror("\nNon-blocking client connection socket failed ");
          exit(EXIT_FAILURE);
        }

       // nfds = cli_connfd[i] + 2;
        status[i] = 0;
        bzero(address[i], HEADER_LEN);
        i++;
      }
    }

    else if(FD_ISSET(STDIN_FILENO,&readfd))
    {
    	scanf("%s",strexit);
    	if(strcmp(strexit,"exit")==0)
    	{
    		break;
    	}
    }

    for(int j = 0; j < i; j++)
    {

      if (status[j] != 0 && FD_ISSET(ser_connfd[j], &readfd))
      {
        int k = 0;
        bzero(buffer, BUFFER_SIZE);

        // Receive data from the server and forward it to the client
        k = recv(ser_connfd[j], buffer, BUFFER_SIZE-1, 0);
        while (k > 0)
        {
          send(cli_connfd[j], (const char*) buffer, k, 0);
          bzero(buffer, BUFFER_SIZE);
          k = recv(ser_connfd[j], buffer, BUFFER_SIZE-1, 0);
        }
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno <= 0)
        {
          continue;
        }
        else
        {
          perror("\nReceive failed ");
          exit(EXIT_FAILURE);
        }
      }

      if (FD_ISSET(cli_connfd[j], &readfd))
      {
        int k = 0,dtemp=0;
        int p=0;
        bzero(buffer, BUFFER_SIZE);

        // Receive data from the client and forward it to the server
        k = recv(cli_connfd[j], buffer, BUFFER_SIZE-1, 0);

        while (k > 0)
        {
          if (status[j] == 0)
          {
            buffer[k] = '\0';
            //printf("buffer :%s\n", buffer);
            int x = 0, y = 0;
            char arr1[BUFFER_SIZE], arr2[BUFFER_SIZE];
            bzero(arr1, BUFFER_SIZE);
            bzero(arr2, BUFFER_SIZE);
            while(x < k && buffer[x] != '\n')
            {
              arr1[x] = buffer[x];
              x++;
            }
            while(x + y < k)
            {
              arr2[y] = buffer[x+y];
              y++;
            }
            strcat(address[j], arr1);
            if (strlen(arr2) == 0)
            {
              bzero(buffer, BUFFER_SIZE);
              k = recv(cli_connfd[j], buffer, BUFFER_SIZE-1, 0);
              continue;
            }
            else
            {
              k = strlen(arr2);

              bzero(buffer, BUFFER_SIZE);
              strcpy(buffer, arr2);
              status[j] = 1;
            }
            strcpy(tokenize, address[j]);
            tokenize_address(tokenize,type,host,port,path,vers);
            if(strcmp(type,"GET")==0 || strcmp(type,"POST")==0)
            {
              printf("%s\n", address[j]);
            	printf("Host: %s, Port: %s, Path: %s \n\n", host, port, path);
            }

            // Create socket file descriptor for the connection with the server
            ser_connfd[j] = socket(AF_INET, SOCK_STREAM, 0);
            if (ser_connfd[j] < 0)
            {
              perror("\nTCP socket creation failed ");
              exit(EXIT_FAILURE);
            }

            bzero(&servaddr, sizeof(servaddr));
            servaddr.sin_family = AF_INET;
            char nnnn[100];
            tmp = gethostbyname(host);
            if (tmp != NULL)
            {
              bcopy(*tmp->h_addr_list, (char *) &addr, sizeof(addr));
              sprintf(nnnn, "%s", inet_ntoa(addr));
              //printf("\nNN:#:%s\n", nnnn);
              if (nnnn[0] != '1' || nnnn[1] != '0' || nnnn[2] != '.')
              {
                tmp = NULL;
                conntype[j]=1;
              }
              else
              {
              	conntype[j]=2;
              }
            }

            if ((strcmp(type, "GET") != 0 && strcmp(type, "POST") != 0) || tmp == NULL)
            {
              //printf("\nNULL\n");
              servaddr.sin_addr.s_addr = inet_addr(servip);
              servaddr.sin_port = htons(atoi(servport));
            }
            else
            {
              //printf("\n%s\n", nnnn);
              bcopy(*tmp->h_addr_list, (char *) &addr, sizeof(addr));
              servaddr.sin_addr.s_addr = addr.s_addr;
              servaddr.sin_port = htons(atoi(port));
              bzero(address[j],HEADER_LEN);
              strcat(address[j],type);
              strcat(address[j]," ");
              strcat(address[j],path);
              strcat(address[j]," ");
              strcat(address[j],vers);
              //////////
              ///////////
              //////////

            }

            // Send connection request
            if (connect(ser_connfd[j], (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0)
            {
              perror("\nTCP connect failed ");
              exit(EXIT_FAILURE);
            }

           // Make connection non-blocking
            if(fcntl(ser_connfd[j], F_SETFL, O_NONBLOCK) != 0)
            {
              perror("\nNon-blocking client connection socket failed ");
              exit(EXIT_FAILURE);
            }

            //nfds = ser_connfd[j] + 2;
            //printf("%s", address[j]);
            send(ser_connfd[j], (const char*)address[j], strlen(address[j]), 0);
          }
          else if(conntype[j]==2)
          {
          	 int pos=0,backsl=0;
          	 //printf("\n%s\n",buffer);
          	 //printf("conntype : %d",conntype[j] );
          	 if(dtemp==0)
          	 {
	          	 for(int z=0;z<k-2;z++)
	          	 {
	          	 	if (buffer[z]=='G' && buffer[z+1]=='E' && buffer[z+2]=='T')
	          	 	{
	          	 		pos=z;
	          	 		dtemp=1;

	          	 		break;
	          	 	}
	          	 }
	          	 for(int z=0;z<k-3;z++)
	          	 {
	          	 	if (buffer[z]=='P' && buffer[z+1]=='O' && buffer[z+2]=='S' && buffer[z+3]=='T')
	          	 	{
	          	 		pos=z;
	          	 		dtemp=1;

	          	 		break;
	          	 	}
	          	 }
	          	if(pos!=0)
          	 	{
          	 		 char val[BUFFER_SIZE];
                 for(int m=0;m<pos;m++)
                 {
                  val[m]=buffer[m];
                 }
                 val[pos]='\0';
          	 		send(ser_connfd[j],(const char*)val,strlen(val),0);
          	 	}
          	 	else if(dtemp==0)
          	 	{
          	 		send(ser_connfd[j],(const char*)buffer,k,0);
          	 	}
	         }
          	 if(dtemp==1 && pos!=0)
          	 {
          	 	int z;
          	 	for(z=pos;z<k;z++)
          	 	{
          	 		if(buffer[z]!='\n')
          	 		{
          	 			temp[p]=buffer[z];
          	 			p++;

          	 		}
          	 		else
          	 		{
          	 			backsl=1;
          	 			dtemp=0;
          	 			break;
          	 		}
          	 	}
          	 	if(backsl==1)
          	 	{
          	 		bzero(temp2,BUFFER_SIZE);
                int num=0;
          	 		for(;z<k;z++)
          	 		{
          	 			temp2[num]=buffer[z];
          	 			num++;
          	 		}
          	 	}
          	 }
          	 else if(dtemp==1 && pos==0)
          	 {
          	 	int z;
          	 	for(z=0;z<k;z++)
          	 	{
          	 		if(buffer[z]!='\n')
          	 		{
          	 			temp[p]=buffer[z];
          	 			p++;

          	 		}
          	 		else
          	 		{
          	 			backsl=1;
          	 			dtemp=0;
          	 			break;
          	 		}
          	 	}
          	 	if(backsl==1)
          	 	{
          	 		bzero(temp2,BUFFER_SIZE);
                int num=0;
          	 		for(;z<k;z++)
          	 		{
          	 			temp2[num]=buffer[z];
          	 			num++;
          	 		}
          	 		temp2[num]='\0';
          	 	}
          	 }
          	 if(backsl==1)
          	 {
          	 	printf("%s\n",temp);
              tokenize_address(temp,type,host,port,path,vers);
          	 	if(strcmp(type,"GET")==0 || strcmp(type,"POST")==0)
            	printf("Host: %s, Port: %s, Path: %s \n\n",host, port, path);
              bzero(temp,BUFFER_SIZE);
              p=0;
            	char sender[1000];
            	strcat(sender,type);
              	strcat(sender," ");
              	strcat(sender,path);
              	strcat(sender," ");
              	strcat(sender,vers);
              	strcat(sender,temp2);
              	send(ser_connfd[j],(const char*)sender,strlen(sender),0);
                bzero(temp2,BUFFER_SIZE);

          	 }



			 //send(ser_connfd[j],(const char*)buffer,k,0);


          	 bzero(buffer,BUFFER_SIZE);
          	 k=recv(cli_connfd[j],buffer,BUFFER_SIZE-1,0);


          }
          else
          {
          	send(ser_connfd[j],(const char*)buffer,k,0);
          	bzero(buffer,BUFFER_SIZE);
          	k=recv(cli_connfd[j],buffer,BUFFER_SIZE-1,0);
          }
        }
        if (errno <= 0 || errno == EAGAIN || errno == EWOULDBLOCK)
        {
          continue;
        }
        else
        {
          perror("\nReceive failed ");
          exit(EXIT_FAILURE);
        }
      }

    }
  }

  // Close all the connections
  for (int j = 0; j < i; j++)
  {
    close(ser_connfd[j]);
    close(cli_connfd[j]);
    //printf("%d : %s\n", j, address[j]);
  }
  close(sockfd);

  return 0;
}
