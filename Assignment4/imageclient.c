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


#define BUFFER_SIZE 10
#define PORT 8081

int main()
{
	int imgcount=0;
	int sockfd;

	struct sockaddr_in serveraddr;
	char dirname[50] ;
	char buffer[BUFFER_SIZE];
	printf("Enter subdirectory name: ");
	scanf("%s",dirname);
	strcat(dirname,"$");

	// create socket file descriptor.
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
  {
		perror("\nTCP socket creation failed.\n");
		exit(0);
	}

	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htons(INADDR_ANY);
	serveraddr.sin_port = htons(PORT);

	// Send Connection Request
	if (connect(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) != 0)
  {
		perror("\nTCP connect failed.\n");
		exit(0);
	}

	// Send subdirectory name.
  send(sockfd, (const char*) dirname, strlen(dirname), 0);
	printf("\nSubdirectory name sent.\n");
	int i=recv(sockfd,buffer,BUFFER_SIZE-1,0);
	buffer[i]='\0';

	char img[]="img";
	char end[]="END";
	int j=0,k=0;
	int flag=0;

 	// Read the incomming data while checking for delimiters
	while(1)
  {
    for(int i=0;i<strlen(buffer);i++)
    {
    	if(buffer[i]==img[j])
    	{
    		j++;
    		if (j == 3)
    		{
    			imgcount++;
  				j = 0;
  			}
			}
    	else
    	{
  			j = 0;
  		}
  		if(buffer[i] == end[k])
  		{
  			k++;
  			if(k == 3)
  			{
  				flag = 1;
  			}
  		}
  		else
  		{
  			k = 0;
  		}
  	}
  	if(flag == 1)
  	{
  		break;
  	}

		bzero(buffer, BUFFER_SIZE);
    i = recv(sockfd, buffer, BUFFER_SIZE-1, 0);
		buffer[i] = '\0';
	}

	// If directory exitsfinal value is number of images + 2.
	
	if (imgcount == 0)
	{
		printf("\nSubdirectory does not exit.\n");
	}
	else
	{
		printf("\nNumber of images received is %d.\n", imgcount-2);
	}
	close(sockfd);
	return 0;
}
