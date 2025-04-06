#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <netdb.h>


#define BUF_SIZE 1000


char *host_name = "127.0.0.1"; /* local host */
int port = 8080;

float floatCasuale(float min, float max) {
		return min + ((float) rand() / RAND_MAX) * (max - min);
}

void connectServer(char str[30])
{
	char messaggio[100];
	
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

	srand(time(NULL) + getpid());
	int intervallo = rand() % (6 + 1);
	sleep(intervallo);
	printf("Intervallo di tempo: %i\n", intervallo);
	
	float temperatura = floatCasuale(-5.0, 35.0);
	
	sprintf(messaggio, "%s %.2f", str, temperatura);

	printf("Sending message \"%s\" to server...\n", messaggio);

	if ( send(sockfd, messaggio, strlen(messaggio)+1, 0) == -1 ) 
	{
		perror("Error on send\n");
		exit(1);
	}

	printf("Message sent. Waiting for response...\n");
	
	char buf[BUF_SIZE];	
	buf[0]='\0';

	if ( recv(sockfd, buf, BUF_SIZE, 0) == -1 ) 
	{
		perror("Error in receiving response from server\n");
		exit(1);
	}

	printf("\nResponse from server: \"%s\"\n", buf);

	close(sockfd);
}

int main(int argc, char *argv[]) {
	
	char str[30] = "";
	int Nt = 0;
	
	if (argc < 3) {
		printf("Usage: <string> <int>\n");
		exit(1);
	} else {
		strcpy(str, argv[1] );
		Nt = atoi(argv[2]);
	}
	
	for (int i = 0; i < Nt; i++) {
			int ret = fork();
			if (ret < 0) {
					perror("Errore nella fork\n");
					exit(1);
			} else if (ret == 0) {
				connectServer(str);
				exit(0);
			}
	}
	for (int i = 0; i < Nt; i++) {
			wait(NULL);
	}
	
	return 0;
}

