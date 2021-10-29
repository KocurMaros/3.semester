CC = gcc
output: client.o server.o
	${CC} client.o -o client -lpthread
	${CC} server.o -o server -lpthread
	rm *.o
	
client.o: client.c
	${CC} -c client.c -lpthread
server.o: server.c
	${CC} -c server.c -lpthread
clean:
	rm *.o

