#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>

#define MAXLEN 1024
//"server_file.txt" is the file that should be present in same directory as wordserver.c
int main()
{
	char *req_filename = "server_file.txt";
	char *my_filename = "client_file.txt";
	char buffer[MAXLEN];

	int sockfd=socket(AF_INET, SOCK_DGRAM,0);
	if(sockfd<0)
	{
		perror("Socket Creation Failed!!");
		exit(EXIT_FAILURE);
	}
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0,sizeof(serveraddr));
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port=htons(8181);
	serveraddr.sin_addr.s_addr=INADDR_ANY;

	int n;
	socklen_t len;

	sendto(sockfd,(const char *)req_filename,strlen(req_filename),0,(const struct sockaddr *)& serveraddr, sizeof(serveraddr));
	printf("file_name sent\n");

	n=recvfrom(sockfd, (char *)buffer, MAXLEN,0,(struct sockaddr *)&serveraddr,&len);
	buffer[n]='\0';
	//printf("%d",strcmp(buffer,"HELLO\n"));
	//printf("\n%s",buffer);

	if(strcmp(buffer,"HELLO\n")==0)
	{
		int i=1;
		char words[10];
		char inputfile[MAXLEN];
		FILE *fp=fopen(my_filename,"w");
		do
		{
			
			sprintf(words,"WORD%d",i);
			sendto(sockfd,(const char *)words,strlen(words),0,(const struct sockaddr *)&serveraddr,sizeof(serveraddr));
			n=recvfrom(sockfd,(char *)inputfile,MAXLEN,0,(struct sockaddr *)&serveraddr,&len);
			inputfile[n]='\0';
			//printf("%s",inputfile);
			//printf("%d\n",strcmp(buffer,"END"));
			//printf("%d",strcmp(buffer,"END"));
			if(strcmp(inputfile,"END\n")!=0)
			{
				fprintf(fp,"%s",inputfile);
			}
			i++;



		}while(strcmp(inputfile,"END\n")!=0);
		fclose(fp);
	}
	//printf("%d",strcmp(buffer,"NOTFOUND server_file.txt"));
	else
	{
		printf("File %s Not Found\n",req_filename);
	}
	close(sockfd);
	return 0;

}