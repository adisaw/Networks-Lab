#include "rsocket.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BSIZE 10

const unsigned int port1 = 70114, port2 = 70115;

int main()
{
  int M2;
  struct sockaddr_in addr1, addr2;
  int size = 100;
  char str[size];

  // Create socket file descriptor
  M2 = r_socket(AF_INET, SOCK_MRP, 0);
  if (M2 < 0)
  {
      perror("\nSocket creation failed ");
      exit(EXIT_FAILURE);
  }

  bzero(&addr1, sizeof(addr1));
  bzero(&addr2, sizeof(addr2));

  addr1.sin_family = AF_INET;
  addr1.sin_addr.s_addr = INADDR_ANY;
  addr1.sin_port = htons(port1);

  addr2.sin_family = AF_INET;
  addr2.sin_addr.s_addr = INADDR_ANY;
  addr2.sin_port = htons(port2);

  // Bind the socket
  if (r_bind(M2, (const struct sockaddr *)&addr2, sizeof(addr2)) < 0)
  {
      perror("\nBind failed ");
      r_close(M2);
      exit(EXIT_FAILURE);
  }

  printf("Waiting to receive characters...\n");

  int n, i = 0;
  socklen_t len;
  char buffer[BSIZE];

  // Receive characters
  bzero(buffer, BSIZE);
  n = r_recvfrom(M2, (char *)buffer, BSIZE, 0, (struct sockaddr *)&addr1, &len);
  while (1)
  {
    printf("%d -- %s\n", ++i, buffer);
    bzero(buffer, BSIZE);
    n = r_recvfrom(M2, (char *)buffer, 10, 0, (struct sockaddr *)&addr1, &len);
  }

  printf("\nAll characters received.\n");
  r_close(M2);

  return 0;
}
