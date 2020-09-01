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

#define BUFFER_SIZE 1000
#define MAX_CONN 200

int main(int argc, char *argv[])
{
  // Take command line arguments
  char myport[] = "8181", servport[] = "8080", servip[16] = "172.16.2.30";
  if (argc == 4)
  {
    strcpy(myport, argv[1]);
    strcpy(servip, argv[2]);
    strcpy(servport, argv[3]);
  }

  int sockfd;
  int cli_connfd[MAX_CONN];
  int ser_connfd[MAX_CONN];
  struct sockaddr_in myaddr, servaddr, cliaddr;
  char buffer[BUFFER_SIZE];
  char strexit[10];

  // Create socket file descriptor
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
  {
    printf("\nSocket creation failed.\n");
    exit(EXIT_FAILURE);
  }

  // Make the socket file descriptor non-blocking
  if(fcntl(sockfd, F_SETFL, O_NONBLOCK) != 0)
  {
    printf("\nNon-blocking socket failed.\n");
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
    printf("\nSocket bind failed.\n");
    exit(EXIT_FAILURE);
  }

  // Start listening to the TCP socket
  if ((listen(sockfd, 5)) != 0)
  {
    printf("\nListen failed.\n");
    exit(EXIT_FAILURE);
  }

  printf("Proxy running on port %s. Forwarding all connections to %s:%s\n", myport, servip, servport);

  fd_set readfd;
  int i = 0, nfds = sockfd + 2;

  while (1)
  {
    // Wait for connection request
    FD_ZERO(&readfd);
    FD_SET(sockfd, &readfd);
    FD_SET(STDIN_FILENO, &readfd);
    for(int j = 0; j < i ;j++)
    {
      FD_SET(cli_connfd[j], &readfd);
      FD_SET(ser_connfd[j], &readfd);
    }

    select(nfds, &readfd, NULL, NULL, NULL);

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
          printf("\nTCP server accept failed.\n");
          exit(EXIT_FAILURE);
        }
      }
      else
      {
        printf("Connection accepted from %s:%d\n", inet_ntoa(cliaddr.sin_addr),ntohs(cliaddr.sin_port));

        // Make the connection non-blocking
        if(fcntl(cli_connfd[i], F_SETFL, O_NONBLOCK) != 0)
        {
          printf("\nNon-blocking client connection socket failed.\n");
          exit(EXIT_FAILURE);
        }

        // Create socket file descriptor for the connection with the server
        ser_connfd[i] = socket(AF_INET, SOCK_STREAM, 0);
        if (ser_connfd < 0)
        {
          printf("\nTCP socket creation failed.\n");
          exit(0);
        }

        // Server information
        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = inet_addr(servip);
        servaddr.sin_port = htons(atoi(servport));

        // Send connection request
        if (connect(ser_connfd[i], (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0)
        {
          printf("\nTCP connect failed.\n");
          exit(EXIT_FAILURE);
        }

        // Make connection non-blocking
        if(fcntl(ser_connfd[i], F_SETFL, O_NONBLOCK) != 0)
        {
          printf("\nNon-blocking client connection socket failed.\n");
          exit(EXIT_FAILURE);
        }

        nfds = ser_connfd[i] + 2;
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

      if (FD_ISSET(cli_connfd[j], &readfd))
      {
        int k = 0;
        bzero(buffer, BUFFER_SIZE);

        // Receive data from the client and forward it to the server
        k = recv(cli_connfd[j], buffer, BUFFER_SIZE-1, 0);
        while (k > 0)
        {
          send(ser_connfd[j], (const char*) buffer, k, 0);
          bzero(buffer, BUFFER_SIZE);
          k = recv(cli_connfd[j], buffer, BUFFER_SIZE-1, 0);
        }

        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
          continue;
        }
        else
        {
          printf("\nReceive failed.\n");
          exit(EXIT_FAILURE);
        }
      }

      if (FD_ISSET(ser_connfd[j], &readfd))
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
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
          continue;
        }
        else
        {
          printf("\nReceive failed.\n");
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
  }
  close(sockfd);

  return 0;
}
