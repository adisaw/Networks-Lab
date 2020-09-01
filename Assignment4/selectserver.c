// Name: Soumen Dutta
// Roll number: 17CS10057

// Name: Aditya Sawant
// Rolll number: 17CS10060

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

#define BUFFER_SIZE 50
#define PORT 8081

int main()
{
  int udp_sockfd, tcp_sockfd;
  struct sockaddr_in udp_servaddr, tcp_servaddr;

  // Create UDP socket file descriptor
  udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if ( udp_sockfd < 0 )
  {
      perror("\nUDP socket creation failed.\n");
      exit(EXIT_FAILURE);
  }

  bzero(&udp_servaddr, sizeof(udp_servaddr));
  udp_servaddr.sin_family = AF_INET;
  udp_servaddr.sin_addr.s_addr = INADDR_ANY;
  udp_servaddr.sin_port = htons(PORT);

  // Bind the UDP socket with the server address
  if ( bind(udp_sockfd, (const struct sockaddr *)&udp_servaddr, sizeof(udp_servaddr)) < 0 )
  {
      perror("\nUDP socket bind failed.\n");
      exit(EXIT_FAILURE);
  }

  // Create TCP socket file descriptor
  tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (tcp_sockfd < 0)
  {
    perror("\nTCP socket creation failed.\n");
    exit(EXIT_FAILURE);
  }

  bzero(&tcp_servaddr, sizeof(tcp_servaddr));
  tcp_servaddr.sin_family = AF_INET;
  tcp_servaddr.sin_addr.s_addr = htons(INADDR_ANY);
  tcp_servaddr.sin_port = htons(PORT);

  // Bind the TCP socket with the server address
  if ((bind(tcp_sockfd, (struct sockaddr*)&tcp_servaddr, sizeof(tcp_servaddr))) != 0)
  {
    perror("\nTCP socket bind failed.\n");
    exit(EXIT_FAILURE);
  }

  // Start listening to the TCP socket
  if ((listen(tcp_sockfd, 5)) != 0)
  {
    printf("\nTCP listen failed.\n");
    exit(EXIT_FAILURE);
  }

  printf("\nServer Running...\n");
  fd_set readfd;

  while (1)
  {
    FD_ZERO(&readfd);
    FD_SET(udp_sockfd, &readfd);
    FD_SET(tcp_sockfd, &readfd);
    select(tcp_sockfd+1, &readfd, NULL, NULL, NULL);
    if (FD_ISSET(udp_sockfd, &readfd))
    {
      struct sockaddr_in udp_cliaddr;
      int n;
      socklen_t len;
      char buffer[BUFFER_SIZE];
      struct hostent *tmp = NULL;
      struct in_addr addr;

      bzero(&udp_cliaddr, sizeof(udp_cliaddr));
      bzero(buffer, BUFFER_SIZE);
      len = sizeof(udp_cliaddr);

      n = recvfrom(udp_sockfd, (char *)buffer, BUFFER_SIZE - 1, 0, (struct sockaddr *)&udp_cliaddr, &len);
      buffer[n] = '\0';

      tmp = gethostbyname(buffer);
      bzero(buffer, BUFFER_SIZE);

      if (tmp != NULL)
      {
        bcopy(*tmp->h_addr_list, (char *) &addr, sizeof(addr));
        sprintf(buffer,"%s", inet_ntoa(addr));
      }

      sendto(udp_sockfd, (const char *)buffer, strlen(buffer), 0, (const struct sockaddr *) &udp_cliaddr, len);
    }

    else if (FD_ISSET(tcp_sockfd, &readfd))
    {
      struct sockaddr_in tcp_cliaddr;
      socklen_t len;
      int tcp_connfd;

      bzero(&tcp_cliaddr, sizeof(tcp_cliaddr));
      len = sizeof(tcp_cliaddr);
      tcp_connfd = accept(tcp_sockfd, (struct sockaddr*)&tcp_cliaddr, &len);
      if (tcp_connfd < 0)
      {
        perror("\nTCP server accept failed.\n");
        exit(EXIT_FAILURE);
      }
      int pid = fork();
      if (pid == 0)
      {
        char buffer[BUFFER_SIZE];
      	char filename[26];
      	int i;
        DIR *dp;
        struct dirent *ep;

        bzero(buffer, BUFFER_SIZE);
      	i = recv(tcp_connfd, buffer, BUFFER_SIZE-1, 0);
      	buffer[BUFFER_SIZE-1] = '\0';
      	sprintf(filename, "%s", buffer);

      	while(filename[strlen(filename)-1] != '$')
      	{
        	bzero(buffer, BUFFER_SIZE);
          i = recv(tcp_connfd, buffer, BUFFER_SIZE-1, 0);
          buffer[BUFFER_SIZE-1] = '\0';
          strcat(filename, buffer);
      	}
      	filename[strlen(filename)-1] = '\0';

      	char endimg[30]="img";
      	char path[30]="image/";
      	char imagefile[40];
      	strcat(path,filename);
      	dp = opendir (path);
      	char end[10]="END";

      	if (dp != NULL)
        {
          while (ep = readdir (dp))
     	  	{
          	sprintf(imagefile,"%s/%s",path,ep->d_name);
          	int fp = open(imagefile, O_RDONLY);
  			    if(fp < 0)
  			    {
              break;
  			    }
      			do
      			{
      				bzero(buffer, BUFFER_SIZE);
      				i = read(fp,buffer,BUFFER_SIZE-1);
      				send(tcp_connfd, (const char*) buffer, strlen(buffer), 0);
      			} while(i > 0);
      			send(tcp_connfd, (const char*)endimg, strlen(endimg), 0);
      			close(fp);
          }
  		    (void) closedir(dp);
        }
    	  send(tcp_connfd,(const char *)end, strlen(end),0);
    	  close(tcp_connfd);
        close(tcp_sockfd);
        exit(0);
      }
      close(tcp_connfd);
    }
  }
  return 0;
}
