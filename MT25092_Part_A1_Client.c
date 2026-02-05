#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

int main(int argc, char *argv[]) {
    if (argc < 5) {
        printf("Usage: %s <ip> <port> <msg_size> <duration>\n", argv[0]);
        return 1;
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);
    size_t msg_size = atoi(argv[3]);
    int duration = atoi(argv[4]);

    char *buffer = malloc(msg_size);
    size_t total_bytes = 0;
    long msg_count = 0;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server = {
        .sin_family = AF_INET,
        .sin_port = htons(port)
    };
    inet_pton(AF_INET, ip, &server.sin_addr);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Connect failed");
        return 1;
    }

    // High Precision Timer Setup
    struct timespec ts_start, ts_end;
    clock_gettime(CLOCK_MONOTONIC, &ts_start);

    time_t start_loop = time(NULL);
    while (time(NULL) - start_loop < duration) {
        // MSG_WAITALL ensures we get the full message size
        ssize_t n = recv(sock, buffer, msg_size, MSG_WAITALL);
        if (n <= 0) break;
        total_bytes += n;
        msg_count++;
    }
    
    clock_gettime(CLOCK_MONOTONIC, &ts_end);

    // Calculate time in microseconds
    double time_taken_us = (ts_end.tv_sec - ts_start.tv_sec) * 1e6 + 
                           (ts_end.tv_nsec - ts_start.tv_nsec) / 1e3;

    // Average Latency per message
    double latency = (msg_count > 0) ? (time_taken_us / msg_count) : 0;

    printf("BYTES=%zu\n", total_bytes);
    printf("LATENCY=%.6f\n", latency);

    free(buffer);
    close(sock);
    return 0;
}