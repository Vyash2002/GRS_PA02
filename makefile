#MT25092
CC=gcc
CFLAGS=-O2 -pthread

all: A1 A2 A3

A1:
	$(CC) $(CFLAGS) MT25092_Part_A1_Server.c -o MT25092_Part_A1_Server
	$(CC) $(CFLAGS) MT25092_Part_A1_Client.c -o MT25092_Part_A1_Client

A2:
	$(CC) $(CFLAGS) MT25092_Part_A2_Server.c -o MT25092_Part_A2_Server
	$(CC) $(CFLAGS) MT25092_Part_A2_Client.c -o MT25092_Part_A2_Client

A3:
	$(CC) $(CFLAGS) MT25092_Part_A3_Server.c -o MT25092_Part_A3_Server
	$(CC) $(CFLAGS) MT25092_Part_A3_Client.c -o MT25092_Part_A3_Client

clean:
	rm -f MT25092_Part_A?_Server MT25092_Part_A?_Client

