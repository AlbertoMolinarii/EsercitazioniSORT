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
	char str[10] = "";
	char operazione[15] = "";
	float Tgoal = 0.0;
	int lunghezzaLista = argc - 4;
	char **listaSensori = NULL;
	char buf[BUF_SIZE];	
	buf[0]='\0';
	
	if (argc < 3) {
		printf("Usage: inserire l'operazione da effettuare\n");
	}
	
	strcpy(operazione, argv[1]);
	strcpy(str, argv[2]);
	
	if (strcmp(operazione, "Iscrizione") == 0) {
		Tgoal = atof(argv[3]);
		
		if (lunghezzaLista > 0) {
				listaSensori = malloc(lunghezzaLista * sizeof(char *));
				if (listaSensori == NULL) {
						perror("Errore allocazione memoria array\n");
						exit(1);
				}
				
				for (int i = 0; i < lunghezzaLista; i++) {
					listaSensori[i] = strdup(argv[i + 4]);
				}
		}
		
		printf("Lista sensori:\n");
		for (int i = 0; i < lunghezzaLista; i++) {
				printf("- %s\n", listaSensori[i]);
		}
		
		strcat(buf, operazione);
		strcat(buf, " ");
		strcat(buf, str);
		strcat(buf, " ");
		
		for (int i = 0; i < lunghezzaLista; i++) {
				strcat(buf, listaSensori[i]);
				if (i != lunghezzaLista - 1) {
						strcat(buf, ";");
				}
		}
	} else if (strcmp(operazione, "Disiscrizione") == 0) {
		strcat(buf, operazione);
		strcat(buf, " ");
		strcat(buf, str);
		
		printf("Hai selezionato di volere eliminare: %s\n", str);
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

	printf("Sending message \"%s\" to server...\n", buf);

	/* This sends the string plus the string terminator '\0' */
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



