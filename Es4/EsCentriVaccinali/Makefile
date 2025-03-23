all:	centro fornitore server 

centro:	centro.c
	gcc -g -Wall -o centro centro.c
	
fornitore: fornitore.c
	gcc -g -Wall -o fornitore fornitore.c

server:	server.c list.o
	gcc -Wall -o server server.c list.o
	
.c.o:
	gcc -g -Wall -c $<

clean:
	rm -f centro fornitore server *.o *~
