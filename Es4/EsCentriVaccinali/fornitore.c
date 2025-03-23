#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <netdb.h>


#define BUF_SIZE 1000


char *host_name = "127.0.0.1"; /* local host */
int port = 8000;


int main(int argc, char *argv[]) 
{
	char nome[30] = "";
	int quantita_vaccini = 0;
	int quantita_minima = 0;
	
	if (argc < 4) { 
		printf("Usage: %s <nome> <quantita_vaccini> <quantita_minima>\n", argv[0]);
		exit(1);
        }
    
        strcpy(nome, argv[1]);
        quantita_vaccini = atoi(argv[2]);
        quantita_minima = atoi(argv[3]);
	
	struct sockaddr_in serv_addr;
 	struct hostent* server;	
	
	if ( ( server = gethostbyname(host_name) ) == 0 ) 
	{
		perror("Error resolving local host\n");
		exit(1);
	}

	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = ((struct in_addr *)(server->h_addr))->s_addr;
	serv_addr.sin_port = htons(port);
	
	int sockfd = socket( PF_INET, SOCK_STREAM, 0 );
	if ( sockfd == -1 ) 
	{
		perror("Error opening socket\n");
		exit(1);
	}    

	if ( connect(sockfd, (void*)&serv_addr, sizeof(serv_addr) ) == -1 ) 
	{
		perror("Error connecting to socket\n");
		exit(1);
	}
	
	char buf[BUF_SIZE];	
	buf[0]='\0';
	
	snprintf(buf, sizeof(buf), "%s %i %i", nome, quantita_vaccini, quantita_minima);

	printf("Sending message \"%s\" to server...\n", buf);

	if ( send(sockfd, buf, strlen(buf)+1, 0) == -1 ) 
	{
		perror("Error on send\n");
		exit(1);
	}

	printf("Message sent. Waiting for response...\n");

	if ( recv(sockfd, buf, BUF_SIZE, 0) == -1 ) 
	{
		perror("Error in receiving response from server\n");
		exit(1);
	}

	printf("\nResponse from server: \"%s\"\n", buf);

	close(sockfd);

	return 0;
}



