#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/uio.h>
#include <string.h>
#include <errno.h>
#include <linux/errqueue.h>
// #include "MT25092_Common.h"
#define NUM_FIELDS 8  // Must match your Client definition

typedef struct {
    int sockfd;
    size_t msg_size;
} client_arg_t;

// Function to clean up the error queue (Required for MSG_ZEROCOPY)
static void drain_error_queue(int sockfd) {
    char cbuf[256];
    struct msghdr msg = {0};
    struct iovec iov;
    char dummy;

    iov.iov_base = &dummy;
    iov.iov_len = sizeof(dummy);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = cbuf;
    msg.msg_controllen = sizeof(cbuf);

    // Drain as many notifications as possible without blocking
    while (recvmsg(sockfd, &msg, MSG_ERRQUEUE | MSG_DONTWAIT) > 0);
}

void *client_handler(void *arg) {
    client_arg_t *carg = (client_arg_t *)arg;
    int sockfd = carg->sockfd;
    size_t total_size = carg->msg_size;
    free(carg); // Free struct immediately

    // Calculate individual field size based on input argument
    size_t field_size = total_size / NUM_FIELDS;

    // Prepare IOVEC (Scatter/Gather structure)
    struct iovec iov[NUM_FIELDS];
    char *buffers[NUM_FIELDS];

    // Allocate aligned memory for ZeroCopy (preferred, though not strictly required for TCP)
    for (int i = 0; i < NUM_FIELDS; i++) {
        // We use calloc/malloc. Ideally posix_memalign for pure zerocopy, 
        // but standard malloc is fine for this benchmark.
        buffers[i] = malloc(field_size);
        memset(buffers[i], 'C', field_size); // Fill with dummy data
        iov[i].iov_base = buffers[i];
        iov[i].iov_len  = field_size;
    }

    struct msghdr hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.msg_iov = iov;
    hdr.msg_iovlen = NUM_FIELDS;

    while (1) {
        // Send with MSG_ZEROCOPY
        ssize_t n = sendmsg(sockfd, &hdr, MSG_ZEROCOPY);
        
        if (n < 0) {
            if (errno == EAGAIN || errno == ENOBUFS) {
                // Socket buffer full, try to drain queue and retry
                drain_error_queue(sockfd);
                continue; 
            }
            break; // Fatal error or disconnect
        }

        // IMPORTANT: We must periodically drain the error queue to prevent
        // the kernel from disabling ZeroCopy due to "too many pending completions".
        // In a tight loop, doing this every time is safer.
        drain_error_queue(sockfd);
        
        // REMOVED: usleep(100); -> This was killing your throughput!
    }

    // Cleanup
    for (int i = 0; i < NUM_FIELDS; i++) {
        free(buffers[i]);
    }
    close(sockfd);
    return NULL;
}

int main(int argc, char *argv[]) {
    // 1. Correct Argument Parsing (must accept msg_size)
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

    // 2. Allow Port Reuse (Vital for script restart)
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {0};
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

        // 3. Enable ZeroCopy on the new socket
        int enable = 1;
        if (setsockopt(client_fd, SOL_SOCKET, SO_ZEROCOPY, &enable, sizeof(enable)) < 0) {
            // If ZeroCopy isn't supported by OS/Hardware, fallback warn
            // perror("setsockopt SO_ZEROCOPY failed");
        }

        client_arg_t *arg = malloc(sizeof(client_arg_t));
        arg->sockfd = client_fd;
        arg->msg_size = msg_size; // 4. Use the correct message size from ARGV

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