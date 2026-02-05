# # Compiler and flags
# CC = gcc
# CFLAGS = -O2 -Wall -pthread

# # Targets
# A1_SERVER = A1_Server
# A1_CLIENT = A1_Client

# A2_SERVER = A2_Server
# A2_CLIENT = A2_Client

# A3_SERVER = A3_Server
# A3_CLIENT = A3_Client

# # Default target
# all: A1 A2 A3

# ################################
# # Part A1
# ################################
# A1:
# 	$(CC) $(CFLAGS) MT25092_Part_A1_Server.c -o $(A1_SERVER)
# 	$(CC) $(CFLAGS) MT25092_Part_A1_Client.c -o $(A1_CLIENT)

# ################################
# # Part A2
# ################################
# A2:
# 	$(CC) $(CFLAGS) MT25092_Part_A2_Server.c -o $(A2_SERVER)
# 	$(CC) $(CFLAGS) MT25092_Part_A2_Client.c -o $(A2_CLIENT)

# ################################
# # Part A3
# ################################
# A3:
# 	$(CC) $(CFLAGS) MT25092_Part_A3_Server.c -o $(A3_SERVER)
# 	$(CC) $(CFLAGS) MT25092_Part_A3_Client.c -o $(A3_CLIENT)

# ################################
# # Clean
# ################################
# clean:
# 	rm -f $(A1_SERVER) $(A1_CLIENT) \
# 	      $(A2_SERVER) $(A2_CLIENT) \
# 	      $(A3_SERVER) $(A3_CLIENT)
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
