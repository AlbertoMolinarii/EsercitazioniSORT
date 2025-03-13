#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#include <sys/wait.h>

char *host_name = "127.0.0.1";
int port = 8000;

void generate_request()
{
	struct sockaddr_in serv_addr;
	struct hostent* server;
	int n, answer;
	
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
	if ( sockfd == -1)
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
	n = rand();
	
	printf("Send the number %d\n", n);
	
	if ( send(sockfd, &n, sizeof(n), 0) == -1 )
	{
		perror("Error on send\n");
		exit(1);
	}
	
	printf("Waiting response \n");
	
	if ( recv (sockfd, &answer, sizeof(answer), 0 ) == -1 )
	{
		perror("Error in receiving response form server\n");
		exit(1);
	}
	
	printf("response: %d\n", answer);
	
	close(sockfd);
	exit(0);
}

int main()
{
	for (int i = 0; i < 5; i++)
	{
		int ret = fork();
		if (ret < 0)
		{
			perror("Fork error");
			exit(1);
		}
		if (ret == 0)
		{
			generate_request();
		}
	}
	
	for (int i = 0; i < 5; i++)
	{
		wait(NULL);
	}
		
	return 0;
}






