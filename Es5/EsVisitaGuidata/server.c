#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "list.h"
#include <unistd.h>
#include <netdb.h>


#define BUF_SIZE 1000
#define N 30


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
	char tipoclient[10];
	char nome[N];
	int numminimo;
	int nummassimo;
	
	LIST listaGuide;
	listaGuide = NewList();
	int numpersone;
	ItemType guida;
	LIST listaVisit;
	listaVisit = NewList();
	ItemType visit;
	int numtotale = 0;

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
		
		if (sscanf(buf,"%s %i %i", nome, &numminimo, &nummassimo) == 3) {
				strcpy(tipoclient, "guida");
				strcpy(guida.nome, nome);
				guida.minimo = numminimo;
				guida.massimo = nummassimo;
				guida.sockfd = newsockfd;
				//guida.posti = 0;
				
				listaGuide = EnqueueLast(listaGuide, guida);
				
				printf("Guide in attesa:\n");
				PrintList(listaGuide);
		}
		else if (sscanf(buf, "%i", &numpersone) == 1) {
				strcpy(tipoclient, "visita");
				visit.numpersone = numpersone;
				visit.sockfd = newsockfd;
				listaVisit = EnqueueLast(listaVisit, visit);
		}

	    if (strcmp(tipoclient, "visita") == 0) {
			numtotale += numpersone;
			ItemType *migliore = trovaMigliore(numtotale, listaGuide);
			if (migliore != NULL) {
				
				migliore->posti += numtotale;

				if (numtotale >= migliore->minimo) {

					sprintf(buf, "Guida assegnata: %s", migliore->nome);
					LIST temp = listaVisit;
					while (temp != NULL) {
						
						if (send(temp->item.sockfd, buf, strlen(buf) + 1, 0) == -1) {
							perror("Error on send\n");
							exit(1);
						}
						close(temp->item.sockfd);
						temp = temp->next;
					}

					snprintf(buf, sizeof(buf), "%i", numtotale);
					if (send(migliore->sockfd, buf, strlen(buf) + 1, 0) == -1) {
						perror("Error on send\n");
						exit(1);
					}
					
					listaGuide = Dequeue(listaGuide, *migliore);
					close(migliore->sockfd);
					listaVisit = NewList();
					numtotale = 0;
				}
			} 
			
			int count = 0;
			LIST tmp = listaGuide;
			while (tmp != NULL) {
				if (tmp->item.massimo < numtotale) {
					count += 1;
				}
				tmp = tmp->next;
			}
			if (count == getLength(listaGuide)) {
				sprintf(buf, "Nessuna guida trovata per il numero richiesto di persone.");
				if (send(newsockfd, buf, strlen(buf) + 1, 0) == -1) {
					perror("Error on send\n");
					exit(1);
				}
				close(newsockfd);
			}
		}
	}

	close(sockfd);
	return 0;
}



