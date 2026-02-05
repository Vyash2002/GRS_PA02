#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/uio.h>
#include <string.h>
// #include "MT25092_Common.h"
#define NUM_FIELDS 8  // Must match your Client definition

typedef struct {
    int sockfd;
    size_t msg_size;
} client_arg_t;

void *client_handler(void *arg) {
    client_arg_t *carg = (client_arg_t *)arg;
    int sock = carg->sockfd;
    size_t total_size = carg->msg_size;
    free(carg); // Free struct immediately

    // Calculate individual field size based on input argument
    size_t field_size = total_size / NUM_FIELDS;

    // Prepare IOVEC (Scatter/Gather structure)
    struct iovec iov[NUM_FIELDS];
    char *buffers[NUM_FIELDS];

    // Allocate memory for each field
    for (int i = 0; i < NUM_FIELDS; i++) {
        buffers[i] = malloc(field_size);
        memset(buffers[i], 'B', field_size); // Fill with dummy data
        iov[i].iov_base = buffers[i];
        iov[i].iov_len = field_size;
    }

    struct msghdr hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.msg_iov = iov;
    hdr.msg_iovlen = NUM_FIELDS;

    // High-performance loop (No sleep)
    while (1) {
        ssize_t n = sendmsg(sock, &hdr, 0);
        if (n <= 0) break; // Client disconnected
    }

    // Cleanup
    for (int i = 0; i < NUM_FIELDS; i++) {
        free(buffers[i]);
    }
    close(sock);
    return NULL;
}

int main(int argc, char *argv[]) {
    // 1. Correct Argument Parsing
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <port> <msg_size> [threads]\n", argv[0]);
        exit(1);
    }

    int port = atoi(argv[1]);
    size_t msg_size = atoi(argv[2]);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }

    // 2. Allow Port Reuse (Crucial for benchmarking scripts)
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

    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) continue;

        client_arg_t *arg = malloc(sizeof(client_arg_t));
        arg->sockfd = client_fd;
        arg->msg_size = msg_size; // Pass the size from command line

        pthread_t tid;
        if (pthread_create(&tid, NULL, client_handler, arg) == 0) {
            pthread_detach(tid);
        } else {
            close(client_fd);
            free(arg);
        }
    }
    return 0;
}