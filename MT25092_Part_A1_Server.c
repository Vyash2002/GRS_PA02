#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>
// #include "MT25092_Common.h"
// Structure to pass arguments to the thread
typedef struct {
    int sockfd;
    size_t msg_size;
} client_arg_t;

// Thread function to handle sending data
void *client_handler(void *arg) {
    client_arg_t *carg = (client_arg_t *)arg;
    int sock = carg->sockfd;
    size_t size = carg->msg_size;
    free(carg); // Free the struct immediately

    // Allocate a buffer of the requested message size
    char *buffer = malloc(size);
    // Fill with dummy data (e.g., 'A')
    memset(buffer, 'A', size);

    // Keep sending data as fast as possible until client disconnects
    while (1) {
        ssize_t n = send(sock, buffer, size, 0);
        if (n <= 0) {
            break; // Client closed connection or error
        }
    }

    free(buffer);
    close(sock);
    return NULL;
}

int main(int argc, char *argv[]) {
    // 1. Updated Argument Parsing (Port, MsgSize, ThreadCount)
    // Even if A1 doesn't use the thread count arg for logic, the script passes it.
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <port> <msg_size> [threads]\n", argv[0]);
        exit(1);
    }

    int port = atoi(argv[1]);
    size_t msg_size = atoi(argv[2]);
    // threads arg (argv[3]) is available but standard accept/thread model 
    // spawns threads dynamically based on connections, so we might not need to enforce it strictly here.

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }

    // Allow port reuse immediately after crash/restart
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(1);
    }

    listen(server_fd, 128);
    
    // printf("A1 Server listening on port %d with msg_size %zu\n", port, msg_size);

    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) continue;

        client_arg_t *arg = malloc(sizeof(client_arg_t));
        arg->sockfd = client_fd;
        arg->msg_size = msg_size; // 2. Pass the correct size from args

        pthread_t tid;
        pthread_create(&tid, NULL, client_handler, arg);
        pthread_detach(tid);
    }
    
    return 0;
}