CC = gcc
output: client.o server.o sort.o
	${CC} client.o -o client 
	${CC} server.o -o server -g -lpthread -lrt
	${CC} sort.o -o sort -lpthread
	rm *.o
client.o: client.c
	${CC} -c client.c -lpthread
server.o: server.c
	${CC} -c server.c -g
sort.o: sort.c
	${CC} -c sort.c -lpthread
clean:
	rm *.o

