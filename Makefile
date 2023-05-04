CC=gcc
CFLAGS=-c -Wall -pthread

# Server targets
server: main_server.o client_handler.o authentication.o file_operations.o log.o
	$(CC) main_server.o client_handler.o authentication.o file_operations.o log.o -o server -pthread

main_server.o: main_server.c
	$(CC) $(CFLAGS) main_server.c

client_handler.o: client_handler.c client_handler.h
	$(CC) $(CFLAGS) client_handler.c

authentication.o: authentication.c authentication.h
	$(CC) $(CFLAGS) authentication.c

file_operations.o: file_operations.c file_operations.h
	$(CC) $(CFLAGS) file_operations.c

log.o: log.c log.h
	$(CC) $(CFLAGS) log.c

# Client targets
client: main_client.o client_operations.o
	$(CC) main_client.o client_operations.o -o client

main_client.o: main_client.c
	$(CC) $(CFLAGS) main_client.c

client_operations.o: client_operations.c client_operations.h
	$(CC) $(CFLAGS) client_operations.c

# Clean target
clean:
	rm -rf *.o server client