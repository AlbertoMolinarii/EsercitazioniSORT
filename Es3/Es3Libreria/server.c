#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <unistd.h>
#include <netdb.h>
#include "list.h"

#define BUF_SIZE 1000
#define N 5


int port = 8000;


int main() 
{
	struct sockaddr_in serv_addr;
	struct sockaddr_in cli_addr;

	// Socket opening
	int sockfd = socket( PF_INET, SOCK_STREAM, 0 );  
	if ( sockfd == -1 ) 
	{
		perror("Error opening socket");
		exit(1);
	}
	
	int options = 1;
	if(setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, &options, sizeof (options)) < 0) {
		perror("Error on setsockopt");
		exit(1);
	}

	bzero( &serv_addr, sizeof(serv_addr) );
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);

	// Address bindind to socket
	if ( bind( sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr) ) == -1 ) 
	{
		perror("Error on binding");
		exit(1);
	}
	
	// Maximum number of connection kept in the socket queue
	if ( listen( sockfd, 20 ) == -1 ) 
	{
		perror("Error on listen");
		exit(1);
	}


	socklen_t address_size = sizeof( cli_addr );	
	
   	char buf[BUF_SIZE];
   	LIST libri;
   	libri = NewList();
   	char tipo_client[10] = "";
   	char nome_libro[20] = "";
   	int numero_copie = 0;
   	ItemType l;
   	
   	int count = 0;
   	char client_socket[N][20];

	while(1) 
	{
		printf("\nWaiting for a new connection...\n");
		
		// New connection acceptance		
		int newsockfd = accept( sockfd, (struct sockaddr *)&cli_addr, &address_size );      
		if (newsockfd == -1) 
		{
			perror("Error on accept");
			exit(1);
		}
		
		bzero(buf, BUF_SIZE);
		
		// Message reception
		if ( recv( newsockfd, buf, BUF_SIZE, 0 ) == -1) 
		{
			perror("Error on receive");
			exit(1);
		}

		printf("Received from client: \"%s\"\n", buf);
		
		if (sscanf(buf, "%s %d", nome_libro, &numero_copie) == 2) {
			strcpy(tipo_client, "editore");
		} else {
			strcpy(tipo_client, "lettore");
			strcpy(nome_libro, buf);
		}
		
		if (strcmp(tipo_client, "editore") == 0) {
			strcpy(l.titolo, nome_libro);
			l.numero_copie = numero_copie;
			
			char* itemTrovato = FindNome(libri, l);
			if (itemTrovato == NULL) {
				libri = EnqueueLast(libri, l);
				PrintList(libri);
				strcpy(buf, "Libro inserito correttamente");
				if ( send( newsockfd, buf, strlen(buf) + 1, 0 ) == -1) 
				{
					perror("Error on send");
					exit(1);
				}
				
				
				for (int i = 0; i < count && numero_copie > 0; i++) {
					char nomelibro2[20] = "";
					int sockfd;
					
					if (sscanf(client_socket[i], "%s %d", nomelibro2, &sockfd) == 2) {
						if (strcmp(nome_libro, nomelibro2) == 0) {
						
							strcpy(buf, "Libro ora disponibile, grazie dell'acquisto");
							if ( send( sockfd, buf, strlen(buf) + 1, 0 ) == -1) 
							{
								perror("Error on send");
								exit(1);
							}
							
							for (int j=i; j < count-1; j++) {
								strcpy(client_socket[i], client_socket[j+1]);
							}
							
							close(newsockfd);
							numero_copie--;
							count--;
							i--;
						}
					}	
				}
				
				ItemType* tp = Find(libri, l);
				tp->numero_copie--;
				
				if (tp != NULL) {
					tp->numero_copie = numero_copie;
					if (tp->numero_copie == 0) {
						libri = Dequeue(libri, *tp);
					}
				}
				
				printf("Lista aggiornata:\n");
				PrintList(libri);
			}
			else {
				printf("Libro gia' esistente");
				strcpy(buf, "Libro gia' esistente");
				if ( send( newsockfd, buf, strlen(buf) + 1, 0 ) == -1) 
				{
					perror("Error on send");
					exit(1);
				}
			}
		}
		
		if (strcmp(tipo_client, "lettore") == 0) {
			strcpy(l.titolo, nome_libro);
			l.numero_copie = 0;
			ItemType* itemTrovato = Find(libri, l);
			
			if (itemTrovato != NULL && itemTrovato->numero_copie > 0) {
				strcpy(buf, "Libro disponibile, grazie dell'acquisto");
				if ( send( newsockfd, buf, strlen(buf) + 1, 0 ) == -1) 
				{
					perror("Error on send");
					exit(1);
				}
				itemTrovato->numero_copie--;
				
				if (itemTrovato->numero_copie == 0) {
					libri = Dequeue(libri, *itemTrovato);
				}
				close(newsockfd);
				
				printf("Lista aggiornata:\n");
				PrintList(libri);
			}
			else {
				strcpy(buf, "Libro non disponibile, si prega di attendere");
				if ( send( newsockfd, buf, strlen(buf) + 1, 0 ) == -1) 
				{
					perror("Error on send");
					exit(1);
				}
				char tmp[30] = ""; 
				snprintf(tmp, sizeof(tmp), "%s %d", nome_libro, newsockfd);
				strcpy(client_socket[count], tmp);
				count += 1;
			}
		}
	}

	close(sockfd);
	return 0;
}



