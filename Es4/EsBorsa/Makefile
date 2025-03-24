all:	investitore agente server 

investitore:	investitore.c
	gcc -Wall -o investitore investitore.c
	
agente:	agente.c
	gcc -Wall -o agente agente.c

server:	server.c list.o
	gcc -Wall -o server server.c list.o

.c.o:
	gcc -g -Wall -c $<

clean:
	rm -f investitore agente server *.o *~

