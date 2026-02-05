#define _GNU_SOURCE
#define _POSIX_C_SOURCE 199309L // Required for clock_gettime
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/uio.h>
#include <string.h>
#include <time.h>

#define NUM_FIELDS 8

int main(int argc, char *argv[]) {
    if (argc < 5) {
        printf("Usage: %s <server_ip> <port> <msg_size> <duration>\n", argv[0]);
        return -1;
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);
    size_t msg_size = atoi(argv[3]);
    int duration = atoi(argv[4]);

    size_t field_size = msg_size / NUM_FIELDS;
    size_t total_bytes = 0;
    long msg_count = 0; // 1. Added message counter

    struct iovec iov[NUM_FIELDS];
    for (int i = 0; i < NUM_FIELDS; i++) {
        iov[i].iov_base = malloc(field_size);
        iov[i].iov_len = field_size;
        memset(iov[i].iov_base, 0, field_size);
    }

    struct msghdr msg = {0};
    msg.msg_iov = iov;
    msg.msg_iovlen = NUM_FIELDS;

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server = {0};
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    inet_pton(AF_INET, ip, &server.sin_addr);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Connect failed");
        return 1;
    }

    // 2. High Precision Timer Start
    struct timespec ts_start, ts_end;
    clock_gettime(CLOCK_MONOTONIC, &ts_start);

    time_t start_loop = time(NULL);
    while (time(NULL) - start_loop < duration) {
        ssize_t n = recvmsg(sock, &msg, MSG_WAITALL);
        if (n <= 0) break;
        total_bytes += n;
        msg_count++; // 3. Increment counter
    }

    // 4. High Precision Timer End
    clock_gettime(CLOCK_MONOTONIC, &ts_end);

    // 5. Calculate Time in Microseconds
    double time_taken_us = (ts_end.tv_sec - ts_start.tv_sec) * 1e6 + 
                           (ts_end.tv_nsec - ts_start.tv_nsec) / 1e3;

    // 6. Calculate Average Latency
    double latency = (msg_count > 0) ? (time_taken_us / msg_count) : 0;

    printf("BYTES=%zu\n", total_bytes);
    printf("LATENCY=%.6f\n", latency); // 7. Print Latency for script to read

    for (int i = 0; i < NUM_FIELDS; i++)
        free(iov[i].iov_base);

    close(sock);
    return 0;
}