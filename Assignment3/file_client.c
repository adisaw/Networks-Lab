// Name: Soumen Dutta
// Roll number: 17CS10057

// Name: Aditya Sawant
// Rolll number: 17CS10060

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX 5
#define PORT 8081

int main()
{
	int bytecount=0;
	int wordcount=0;
	int sockfd;
	struct sockaddr_in serveraddr;
	char filename[50] ;
	printf("Enter filename: ");
	scanf("%s",filename);
	strcat(filename,"$");


	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
  {
		printf("Socket Creation Failed!\n");
		exit(0);
	}
	else
  {
		printf("Socket Creation Successful!\n");
  }

	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htons(INADDR_ANY);
	serveraddr.sin_port = htons(PORT);

	if (connect(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) != 0)
  {
		printf("Connect Failed!\n");
		exit(0);
	}
	else
  {
		printf("Connect Successful!\n");
  }

  send(sockfd, (const char*) filename, strlen(filename), 0);
	printf("Filename Sent!\n");

  char buffer[MAX];
  int i, ctr;
	bzero(buffer, MAX);
	i = recv(sockfd, buffer, MAX-1, 0);
	if (i == 0)
	{
		printf("File Not Found!\n");
		close(sockfd);
		exit(0);
	}

	int fd = open("client_file.txt", O_CREAT | O_WRONLY | O_TRUNC, 0666);
	if (fd < 0)
	{
		printf("File Creation Failed!\n");
		close(sockfd);
		exit(0);
	}

	ctr = 1;
  while(i != 0)
  {
		for(int k=0;k<strlen(buffer);k++)
		{
				if(buffer[k] == ' ' | buffer[k] == '\t' | buffer[k] == '\n')
				{
					if (ctr == 0)
					{
						ctr = 1;
						wordcount = wordcount+1;
					}
				}
				else
				{
					ctr = 0;
				}
		}
		bytecount = bytecount+i;
		//printf("!%s! %d\n", buffer, wordcount);
    write(fd, buffer, strlen(buffer));
		bzero(buffer, MAX);
    i = recv(sockfd, buffer, MAX-1, 0);
  }
	if (ctr == 0)
		wordcount = wordcount + 1;
  printf("The file transfer is successful.\nSize of the file = %d bytes, no of words = %d\n"
  	,bytecount, wordcount);
	close(fd);
	close(sockfd);
	return 0;
}
