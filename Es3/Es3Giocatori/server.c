#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <netdb.h>
#include "list.h"

#define BUF_SIZE 1000
#define N 4


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
   
   	LIST players;
   	players = NewList();
   	LIST players_con;
   	players_con = NewList();
   	LIST vincitori;
   	vincitori = NewList();

	int count_players = 0;
	int client_sockets[N];

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
		
		client_sockets[count_players] = newsockfd;
		count_players += 1; 
		
		bzero(buf, BUF_SIZE);
		
		// Message reception
		if ( recv( newsockfd, buf, BUF_SIZE, 0 ) == -1) 
		{
			perror("Error on receive");
			exit(1);
		}
		
		ItemType p;
		
		strcpy(p.nome, buf);
	   	bzero(buf, BUF_SIZE);
	   	
	   	char* itemTrovato = FindNome(players, p);
	   	if (itemTrovato == NULL) {
	   		p.partite_giocate = 0;
	   		p.punteggio_totale = 0;
	   		
	   		players = EnqueueLast(players, p);
	   		printf("Giocatori iscritti:\n");
	   		PrintList(players);
	   		
	   		players_con = EnqueueLast(players_con, p);
	   		printf("Giocatori connessi:\n");
    			PrintNomi(players_con);
	  	}
	  	else {
	  		printf("Giocatori iscritti:\n");
    			PrintList(players);
    			players_con = EnqueueLast(players_con, p);
    			printf("Giocatori connessi:\n");
    			PrintNomi(players_con);
	  	}
	   	
	   	if (count_players == N)
	   	{
			printf("Simulazione del gioco in corso...\n");
				
			//Simulazione del gioco
			
			srand(time(NULL));
			
			int indiciVincitori[3];
			int indiciEstratti[N];
			
			for (int i = 0; i < N; i++) {
				indiciEstratti[i] = 0;
			}
			
			for (int i = 0; i < N; i++) {
				int n;
				do {
					n = rand() % N;
				} while (indiciEstratti[n]);
				
				indiciEstratti[n] = 1;
				indiciVincitori[i] = n;
			}
			
			LIST tmp = players_con;
			while (tmp != NULL) {
				ItemType* item = Find(players, tmp->item);
				
				if (item != NULL) {
					item->partite_giocate += 1;
				}
				tmp = tmp->next;
			}
			
			for (int i = 0; i < 3; i++) {
				int indice = indiciVincitori[i];
				
				ItemType* item = getItemAtIndex(players_con, indice);
				ItemType* item2 = Find(players, *item);
				if (item != NULL) {
					if (i == 0) {item2->punteggio_totale += 3;}
					if (i == 1) {item2->punteggio_totale += 2;}
					if (i == 2) {item2->punteggio_totale += 1;}
					vincitori = EnqueueLast(vincitori, *item2);
				}
			}
			
			printf("Simulazione terminata!\n");
			
			bzero(buf, BUF_SIZE);
			int pos = 1;
			while (vincitori != NULL) {
			char tmp[BUF_SIZE];
				snprintf(tmp, sizeof(tmp), "%d°" "%s", pos, vincitori->item.nome);
				
				if (strlen(buf)+strlen(tmp)+2 < BUF_SIZE) {
					if (pos > 1) {
						strcat(buf, ", ");
					}
					strcat(buf, tmp);
				}
				
				vincitori = vincitori->next;
				pos++;
			}
			
			for (int i = 0; i < N; i++) {
				if ( send(client_sockets[i], buf, strlen(buf)+1, 0) == -1) {
					perror("Error on send");
					exit(1);
				}
				close(client_sockets[i]);
			}
			count_players = 0;
			tmp = DeleteList(tmp);
			players_con = DeleteList(players_con);
			vincitori = DeleteList(vincitori);
		}
	}

	close(sockfd);
	return 0;
}



