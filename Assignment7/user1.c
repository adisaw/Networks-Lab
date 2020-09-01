#include "rsocket.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

const unsigned int port1 = 70114, port2 = 70115;

int main()
{
  int M1;
  struct sockaddr_in addr1, addr2;
  size_t size = 100;
  char *str = (char *)malloc(size);

  // Create socket file descriptor
  M1 = r_socket(AF_INET, SOCK_MRP, 0);
  if (M1 < 0)
  {
      perror("\nSocket creation failed ");
      exit(EXIT_FAILURE);
  }

  bzero(&addr1, sizeof(addr2));
  bzero(&addr2, sizeof(addr2));

  addr1.sin_family = AF_INET;
  addr1.sin_addr.s_addr = INADDR_ANY;
  addr1.sin_port = htons(port1);

  addr2.sin_family = AF_INET;
  addr2.sin_addr.s_addr = INADDR_ANY;
  addr2.sin_port = htons(port2);

  // Bind the socket
  if (r_bind(M1, (const struct sockaddr *)&addr1, sizeof(addr1)) < 0)
  {
      perror("\nBind failed ");
      r_close(M1);
      exit(EXIT_FAILURE);
  }

  // Read a string from keyboard
  printf("Enter a string:\n");
  if (getline(&str, &size, stdin) < 0)
  {
    perror("\nGetline failed ");
    r_close(M1);
    exit(EXIT_FAILURE);
  }

  // Send each chracter of the string in a different r_sendto call
  int temp = strlen(str) - 1;
  for (int i = 0; i < temp; i++)
  {
    r_sendto(M1, (const char *)(str+i), 1, 0, (const struct sockaddr *) &addr2, sizeof(addr2));
  }

  // Wait for all retransmissions to occur.
  for (int i = 0; i < 15000; i++)
    usleep(1000);

  printf("All characters sent.\n");
  printf("Number of characters sent: %d \n", temp);

  free(str);
  r_close(M1);

  return 0;
}
