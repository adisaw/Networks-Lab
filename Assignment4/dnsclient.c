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
#include <arpa/inet.h>
#include <netinet/in.h>

#define BUFFER_SIZE 50
#define PORT 8081

int main()
{
  int sockfd;
  struct sockaddr_in servaddr;

  // Creating socket file descriptor
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if ( sockfd < 0 )
  {
      perror("\nUDP socket creation failed.\n");
      exit(EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr));

  // Server information
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(PORT);
  servaddr.sin_addr.s_addr = INADDR_ANY;

  int n;
  socklen_t len;
  char buffer[BUFFER_SIZE];

  printf("\nEnter domain name: ");
  scanf("%s", buffer);

  // Send the domain name to the server.
  sendto(sockfd, (const char *)buffer, strlen(buffer), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
  bzero(buffer, BUFFER_SIZE);

  // Receive server's response.
  n = recvfrom(sockfd, (char *)buffer, BUFFER_SIZE - 1, 0, ( struct sockaddr *)&servaddr, &len);
  buffer[n] = '\0';

  // If response in empty, domain name was invalid.
  if (strlen(buffer) == 0)
  {
    printf("\nInvaild Domain name.\n");
  }
  else
  {
    printf("\nIP Address: %s\n", buffer);
  }

  // close socket.
  close(sockfd);
  return 0;
}
