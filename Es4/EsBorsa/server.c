#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "list.h"
#include <unistd.h>
#include <netdb.h>


#define BUF_SIZE 1000


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
   	char tipoclient[15] = "";
   	char nome[10] = "";
   	
   	int quantita = 0;
   	int prezzounitario = 0;
   	int prezzominimo = 0;
   	
   	ItemType agente;
   	
   	LIST listaAgenti;
   	listaAgenti = NewList();

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

	    	if (sscanf(buf, "%s %i %i %i", nome, &quantita, &prezzounitario, &prezzominimo) == 4) {
	    		strcpy(tipoclient, "agente");
	    		printf("Agente: %s %i %i %i\n", nome, quantita, prezzounitario, prezzominimo);
	    		
	    		strcpy(agente.nome, nome);
	    		agente.quantita = quantita;
	    		agente.prezzounitario = prezzounitario;
	    		agente.prezzominimo = prezzominimo;
	    		agente.sockfd = newsockfd;
	    		agente.ricavo = 0;
	    		
	    		listaAgenti = EnqueueLast(listaAgenti, agente);
	    		PrintList(listaAgenti);
	    		
	    	} else if (sscanf(buf, "%s", tipoclient) == 1) {
	    		strcpy(tipoclient, "investitore");
	    		printf("Investitore\n");
	    	}
		
		bzero(buf, BUF_SIZE);
		
		if (strcmp(tipoclient, "investitore") == 0) {
			
			char out[100] = "";
			int len = 0;

			LIST tmp = listaAgenti;
			while (tmp != NULL) {
			    ItemType *it = &tmp->item;    

			    int added = snprintf(out + len, 100 - len, "%s %i %i\n", it->nome, it->quantita, it->prezzounitario);

			    if (added < 0 || len + added >= BUF_SIZE) {
				printf("Errore: buffer troppo piccolo.\n");
				break;
			    }

			    len += added;

			    tmp = tmp->next;
			}
			
			strcpy(buf,out);
			
			if ( send( newsockfd, buf, strlen(buf) + 1, 0 ) == -1) 
			{
				perror("Error on send");
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
			
			ItemType tr;
			strcpy(tr.nome, buf);
			
			ItemType* trovato = Find(listaAgenti, tr);
			
			if (trovato != NULL) {
			
				trovato->quantita -= 1;
				int prezzo = trovato->prezzounitario;
				trovato->prezzounitario += 1;
				trovato->ricavo += prezzo;
				printf("ricavo: %i", prezzo);
				
				if (trovato->quantita == 0) {
					listaAgenti = Dequeue(listaAgenti, *trovato);
					
					bzero(buf, BUF_SIZE);
					snprintf(buf, sizeof(buf), "Ricavo tot: %i", trovato->ricavo);
					
					if ( send( trovato->sockfd, buf, strlen(buf) + 1, 0 ) == -1) 
					{
						perror("Error on send");
						exit(1);
					}
				}
				
				LIST tmp = listaAgenti;
				while (tmp != NULL) {
					ItemType* it = &(tmp->item);
					
					if (strcmp(it->nome, trovato->nome) != 0) {
						it->prezzounitario -= 1;
					}
		
					tmp = tmp->next;
				}
				
			} else {
				printf("Agente: %s non trovato\n", buf);
			}
			
			tmp = listaAgenti;
			while (tmp != NULL) {
				ItemType* it = &(tmp->item);
			
				if (it->prezzounitario < it->prezzominimo) {
					listaAgenti = Dequeue(listaAgenti, *it);
					
					bzero(buf, BUF_SIZE);
					snprintf(buf, sizeof(buf), "Ricavo tot: %i", it->ricavo);
					
					if ( send( it->sockfd, buf, strlen(buf) + 1, 0 ) == -1) 
					{
						perror("Error on send");
						exit(1);
					}
				}
				tmp = tmp->next;
			}
		}
		//close(newsockfd);
	}

	close(sockfd);
	return 0;
}



