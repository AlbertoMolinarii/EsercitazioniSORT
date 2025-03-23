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
   	char tipo_client[10] = "";
   	char nomef[10] = "";
   	char nomec[10] = "";
   	
   	int quantita_vaccini = 0;
   	int quantita_minima = 0;
   	int numero_pazienti = 0;
   	
   	LIST listaFornitori;
   	listaFornitori = NewList();
   	LIST listaCentri;
   	listaCentri = NewList();
   	
   	ItemType fornitore;
   	fornitore.tipo = FORNITORE;
   	ItemType centro;
   	centro.tipo = CENTRO;

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
		
		if (sscanf(buf, "%s %i %i", nomef, &quantita_vaccini, &quantita_minima) == 3) {
			strcpy(tipo_client, "fornitore");
			printf("%s: %s %i %i\n", tipo_client, nomef, quantita_vaccini, quantita_minima);
			
			strcpy(fornitore.itemf.nome, nomef);
			fornitore.itemf.quantita_vaccini = quantita_vaccini;
			fornitore.itemf.quantita_minima = quantita_minima;
			fornitore.itemf.sockfd = newsockfd;
			
		} else if (sscanf(buf, "%s %i", nomec, &numero_pazienti) == 2) {
			strcpy(tipo_client, "centro");
			printf("%s: %s %i\n",tipo_client, nomec, numero_pazienti);
			
			strcpy(centro.itemc.nome, nomec);
			centro.itemc.numero_pazienti = numero_pazienti;
			centro.itemc.sockfd = newsockfd;
		}
		
		bzero(buf, BUF_SIZE);
		
		if (strcmp(tipo_client, "fornitore") == 0) {
			
			if (isEmpty(listaCentri)) {
				listaFornitori = EnqueueLast(listaFornitori, fornitore);
			
				printf("Lista dei fornitori in attesa:\n");
				PrintList(listaFornitori);
			} else {
				LIST centriServiti = NewList();
				int somma_pazienti = 0;
				LIST tmp = NULL;
				
				ItemType *best_centro = NULL;
				tmp = listaCentri;
			
				while (tmp != NULL) {
					ItemType *it = &tmp->item;
					
					if (it->tipo == CENTRO && it->itemc.numero_pazienti <= quantita_vaccini && it->itemc.numero_pazienti >= quantita_minima) {
						if (best_centro == NULL || it->itemc.numero_pazienti > best_centro->itemc.numero_pazienti) {
							best_centro = it;
						}
					}
					tmp = tmp->next;
				}
				
				if (best_centro != NULL && best_centro->itemc.numero_pazienti >= quantita_minima) {
					somma_pazienti += best_centro->itemc.numero_pazienti;
					centriServiti = EnqueueLast(centriServiti, *best_centro);
					listaCentri = Dequeue(listaCentri, *best_centro);
				} else {
					listaFornitori = EnqueueLast(listaFornitori, fornitore);
				
					printf("Lista dei fornitori in attesa:\n");
					PrintList(listaFornitori);
				}
				
				tmp = listaCentri;
				while (tmp != NULL) {
					ItemType *it = &tmp->item;
					//LIST next = tmp->next;
					
					if (it->tipo == CENTRO && it->itemc.numero_pazienti <= (quantita_vaccini - somma_pazienti) && (it->itemc.numero_pazienti + somma_pazienti) >= quantita_minima) {
						somma_pazienti += it->itemc.numero_pazienti;
						centriServiti = EnqueueLast(centriServiti, *it);
						listaCentri = Dequeue(listaCentri, *it);
						printf("Altro centro trovato\n");
					}
					tmp = tmp->next;
				}
				
				if (centriServiti == NULL) {
					printf("Nessun centro servito\n");
				} else {
					
					tmp = centriServiti;
					char mex[100] = "";
					
					while (tmp != NULL) {
						if (strlen(mex) > 0) {
							strcat(mex, ", ");
						}
						strcat(mex, tmp->item.itemc.nome);
						
						strcpy(buf, fornitore.itemf.nome);
						if ( send(tmp->item.itemc.sockfd, buf, strlen(buf)+1, 0) == -1 ) 
						{
							perror("Error on send\n");
							exit(1);
						}
						fornitore.itemf.quantita_vaccini -= tmp->item.itemc.numero_pazienti;
						
						tmp = tmp->next;
					}
					listaFornitori = EnqueueLast(listaFornitori, fornitore);
					
					strcpy(buf, mex);
					if ( send(fornitore.itemf.sockfd, buf, strlen(buf)+1, 0) == -1 ) 
					{
						perror("Error on send\n");
						exit(1);
					}
				}
			}
		}
		
		if (strcmp(tipo_client, "centro") == 0) {
			
			LIST tmp = listaFornitori;
			int ftrovato = 0;
			
			while (tmp != NULL) {
				ItemType *it = &tmp->item;
				if (it->tipo == FORNITORE && numero_pazienti >= it->itemf.quantita_minima && numero_pazienti <= it->itemf.quantita_vaccini) {
					it->itemf.quantita_vaccini -= numero_pazienti;
					
					strcpy(buf, nomec);
					if ( send(it->itemf.sockfd, buf, strlen(buf)+1, 0) == -1 ) 
					{
						perror("Error on send\n");
						exit(1);
					}
					strcpy(buf, it->itemf.nome);
					if ( send(centro.itemc.sockfd, buf, strlen(buf)+1, 0) == -1 ) 
					{
						perror("Error on send\n");
						exit(1);
					}
					
					listaCentri = Dequeue(listaCentri, centro);
					ftrovato = 1;
					
					if (it->itemf.quantita_vaccini == 0) {
						listaFornitori = Dequeue(listaFornitori, *it);
					}
					
					break;
				}
				tmp = tmp->next;
			}
			if (ftrovato == 0) {
				listaCentri = EnqueueLast(listaCentri, centro);
			
				printf("Lista dei centri in attesa:\n");
				PrintList(listaCentri);
			}
		}
			
		//close(newsockfd);
	}

	close(sockfd);
	return 0;
}



