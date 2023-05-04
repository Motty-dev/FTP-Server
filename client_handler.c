#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include "authentication.h"
#include "client_handler.h"
#include "file_operations.h"
#include "log.h"

void *client_thread(void *arg) {
    int client_fd = *(int *)arg;
    pthread_detach(pthread_self());

    if (authenticate(client_fd)) {
        printf("Client authenticated\n");
        log_message("Client authenticated");
        handle_client_commands(client_fd);
    } else {
        printf("Client authentication failed\n");
        log_message("Client authentication failed");
    }

    close(client_fd);
    free(arg);

    int err = errno;
    if (err == ECONNRESET) {
        printf("Client disconnected\n");
        log_message("Client disconnected");
    }

    return NULL;
}