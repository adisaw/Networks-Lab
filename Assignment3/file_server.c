// Name: Soumen Dutta
// Roll number: 17CS10057

// Name: Aditya Sawant
// Rolll number: 17CS10060

#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX 80
#define PORT 8081


int main()
{
	int sockfd, connfd, len;
	struct sockaddr_in serveraddr, clientaddr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
  {
		printf("Socket Creation Failed!\n");
		exit(EXIT_FAILURE);
	}
	else
  {
		printf("Socket Creation Successful!\n");
  }

	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htons(INADDR_ANY);
	serveraddr.sin_port = htons(PORT);

	if ((bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr))) != 0)
	{
		printf("Bind Failed!\n");
		exit(EXIT_FAILURE);
	}
	else
  	{
		printf("Bind Successful!\n");
  	}

	if ((listen(sockfd, 5)) != 0)
	{
		printf("Listen Failed!\n");
		exit(EXIT_FAILURE);
	}
	else
  	{
		printf("Server Listening!\n");
  	}

	len = sizeof(clientaddr);
	connfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
	if (connfd < 0)
  	{
		printf("Server Accept Failed!\n");
		exit(EXIT_FAILURE);
	}
	else
  	{
		printf("Server Accept Successful!\n");
  	}

  	char buffer[MAX];
  	char filename[26];
  	int i;

	bzero(buffer, MAX);
  	i = recv(connfd, buffer, MAX-1, 0);
  	buffer[MAX-1] = '\0';
  	sprintf(filename, "%s", buffer);

  	while(filename[strlen(filename)-1] != '$')
  	{
    	bzero(buffer, MAX);
      	i = recv(connfd, buffer, MAX-1, 0);
      	buffer[MAX-1] = '\0';
      	strcat(filename, buffer);
  	}
  	filename[strlen(filename)-1] = '\0';
	printf("Filename Received!\n");
	printf("Filename: %s\n", filename);

	int fd = open(filename, O_RDONLY);
	if(fd < 0)
	{
		bzero(buffer, MAX);
		send(connfd, (const char*) buffer, strlen(buffer), 0);
		printf("File Not Found!\n");
		close(sockfd);
		exit(0);
	}
	printf("File Open Successful!\n");

	do
	{
		bzero(buffer, MAX);
		i = read(fd, buffer, MAX-1);
		send(connfd, (const char*) buffer, strlen(buffer), 0);
	} while(i > 0);
	printf("File Reading Complete!\n");

	

  close(fd);
	close(connfd);
	close(sockfd);
	printf("Connection Closed!\n");
	return 0;
}
