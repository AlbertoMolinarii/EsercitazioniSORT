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
	char nome_libro[30] = "A default test string";
	int numero_copie = 0;
	char tipo_client[10] = "";
	
	if (argc == 2) {
		strcpy(tipo_client, "lettore");
		strcpy(nome_libro, argv[1] );
	}
	else if (argc == 3) {
		strcpy(tipo_client, "editore");
		strcpy(nome_libro, argv[1] );
		numero_copie = atoi(argv[2]);
	}
	
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

	if (strcmp(tipo_client, "lettore") == 0)
	{
		printf("Sending message \"%s\" to server...\n", nome_libro);

		if ( send(sockfd, nome_libro, strlen(nome_libro)+1, 0) == -1 ) 
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
		
		if (strcmp(buf, "Libro disponibile, grazie dell'acquisto") == 0) {
			close(sockfd);
			return 0;
		}
		
			
		if ( recv(sockfd, buf, BUF_SIZE, 0) == -1 ) 
		{
			perror("Error in receiving response from server\n");
			exit(1);
		}

		printf("\nResponse from server: \"%s\"\n", buf);
	}
	
	if (strcmp(tipo_client, "editore") == 0)
	{
		printf("Sending message \"%s\" and \"%i\" to server...\n", nome_libro, numero_copie);
		
		snprintf(buf, sizeof(buf), "%s %d", nome_libro, numero_copie);
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
	}
	close(sockfd);

	return 0;
}



