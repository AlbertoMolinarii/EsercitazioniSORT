#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <unistd.h>
#include <netdb.h>


#define BUF_SIZE 1000

char *host_name = "127.0.0.1";

int port = 8080;
int portServer = 8000;

int main(int argc, char *argv[]) 
{
	struct sockaddr_in serv_addr;
	struct sockaddr_in cli_addr;
	int ret;
	int J = 0;
	
	if (argc < 2) {
			printf("Usage: <int>");
			exit(1);
	}
	
	J = atoi(argv[1]);

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
	int count = 0;
	char listam[J][BUF_SIZE];
	char messaggio[BUF_SIZE];

	while(1) 
	{
		printf("\n---\npid %d: waiting connections\n", getpid());
		
		// New connection acceptance		
		int newsockfd = accept( sockfd, (struct sockaddr *)&cli_addr, &address_size );      
		if (newsockfd == -1) 
		{
			perror("Error on accept");
			exit(1);
		}
		
		ret = fork();
		count += 1;
		
		if (ret == 0) {
		  printf("pid %d child of %d: serving active connection\n", getpid(), getppid());
		  // Message reception
		  if ( recv( newsockfd, buf, BUF_SIZE, 0 ) == -1) 
		  {
				perror("Error on receive");
				exit(1);
		  }
		  printf("pid %d: received %s\n", getpid(), buf);
		  
		  printf("AAAAA: %i\n", count);
		  
		  strcpy(listam[count], buf);
		  
		  //Invio al Server centrale
		  
		  if (count == J) {
		  
				struct sockaddr_in server_addr;
				struct hostent* serverS;	
				
				if ( ( serverS = gethostbyname(host_name) ) == 0 ) 
				{
					perror("Error resolving local host\n");
					exit(1);
				}

				bzero(&server_addr, sizeof(server_addr));
				server_addr.sin_family = AF_INET;
				server_addr.sin_addr.s_addr = ((struct in_addr *)(serverS->h_addr))->s_addr;
				server_addr.sin_port = htons(portServer);
				
				int sockfd = socket( PF_INET, SOCK_STREAM, 0 );
				if ( sockfd == -1 ) 
				{
					perror("Error opening socket\n");
					exit(1);
				}    

				if ( connect(sockfd, (void*)&server_addr, sizeof(server_addr) ) == -1 ) 
				{
					perror("Error connecting to socket\n");
					exit(1);
				}
				
				for (int i = 0; i < J; i++) {
					
					printf("Sending message \"%s\" to server...\n", listam[i]);

					if ( send(sockfd, listam[i], strlen(listam[i])+1, 0) == -1 ) 
					{
						perror("Error on send\n");
						exit(1);
					}
				}

				printf("Message sent. Waiting for response...\n");

				if ( recv(sockfd, buf, BUF_SIZE, 0) == -1 ) 
				{
					perror("Error in receiving response from server\n");
					exit(1);
				}

				printf("\nResponse from server: \"%s\"\n", buf);

				close(sockfd);
			}
		  
		  //Stop codice al Server
		
		  /*if (send(newsockfd, &num, sizeof(num), 0 ) == -1) {
			perror("Error on send");
			exit(1);
			  }*/
		  printf("pid %d: close the connections and exit\n",getpid());
		  close(newsockfd);
		  exit(0);
		}
		else {
		  printf("pid %d: generated the process %d: continue executions serving other clients\n", getpid(), ret);
		  close(newsockfd);
		}
	}

	close(sockfd);
	return 0;
}



