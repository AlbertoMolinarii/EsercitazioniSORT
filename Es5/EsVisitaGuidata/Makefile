all:	guida visitatori server 

guida:	guida.c
	gcc -Wall -o guida guida.c

visitatori:	visitatori.c
	gcc -Wall -o visitatori visitatori.c

server:	server.c list.o
	gcc -Wall -o server server.c list.o

.c.o:
	gcc -g -Wall -c $<

clean:
	rm -f guida visitatori server *.o *~

