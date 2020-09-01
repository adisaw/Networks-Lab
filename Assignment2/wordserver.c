// A Simple UDP Server that sends a HELLO message
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
  
#define MAXLINE 1024 
  
int main() 
{ 
    int sockfd; 
    struct sockaddr_in servaddr, cliaddr; 
      
    // Create socket file descriptor 
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if ( sockfd < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
      
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
      
    servaddr.sin_family    = AF_INET; 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(8181); 
      
    // Bind the socket with the server address 
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,  
            sizeof(servaddr)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    
    printf("\nServer Running....\n");
  
    int n; 
    socklen_t len;
    char buffer[MAXLINE]; 
 
    len = sizeof(cliaddr);
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0, 
			( struct sockaddr *) &cliaddr, &len); 
    buffer[n] = '\0'; 
    printf("%s\n", buffer);
    FILE *fp=fopen(buffer,"r");

    if(fp==NULL)
    {
        char message[200]="NOTFOUND ";
        
        strcat(message,buffer);
        //printf("%s\n",message);
        sendto(sockfd, (const char *)message, strlen(message), 0, 
            (const struct sockaddr *) &cliaddr, sizeof(cliaddr)); 
       
    }
    else
    {
        char start[MAXLINE];
        fgets(start,MAXLINE,fp);
        sendto(sockfd,(const char *)start, strlen(start), 0,
        (const struct sockaddr *) &cliaddr, sizeof(cliaddr));

        char words[10];
        int i=1;
        while(strcmp(start,"END\n")!=0)
        {
            
            n=recvfrom(sockfd, (char *)buffer, MAXLINE, 0,
                (struct sockaddr *) &cliaddr,&len);
            buffer[n]='\0';
            char num[4];
            sprintf(words,"WORD%d",i);
          
           // printf("%d",strcmp(words,buffer));
            if(strcmp(words,buffer)==0)
            {
                fgets(start,MAXLINE,fp);
                printf("%s",start);
                sendto(sockfd, (const char *)start, strlen(start),
                0, (const struct sockaddr *)&cliaddr,sizeof(cliaddr));
                i++;
            }
            
         


        }  
           fclose(fp);      
    } 

    
    
      
    return 0; 
} 
