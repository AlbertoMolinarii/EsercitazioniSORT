all:	client server 

client:	client.c list.o
	gcc -g -Wall -o client client.c list.o

server:	server.c list.o
	gcc -Wall -o server server.c list.o
	
.c.o:
	gcc -g -Wall -c $<

clean:
	rm -f client server *.o *~
