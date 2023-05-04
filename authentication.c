#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "authentication.h"
#include "log.h"

int authenticate(int client_fd) {
    FILE *auth_file = fopen("auth.txt", "r");
    if (auth_file == NULL) {
        perror("Error opening auth.txt");
        return 0;
    }

    char username[64];
    char password[64];
    char received_username[64];
    char received_password[64];

    while (1) {
        send(client_fd, "Enter username: ", 16, 0);
        recv(client_fd, received_username, sizeof(received_username), 0);
        strtok(received_username, "\n");

        send(client_fd, "Enter password: ", 16, 0);
        recv(client_fd, received_password, sizeof(received_password), 0);
        strtok(received_password, "\n");

        int auth_success = 0;
        while (fscanf(auth_file, "%63s %63s", username, password) == 2) {
            if (strcmp(received_username, username) == 0 && strcmp(received_password, password) == 0) {
                auth_success = 1;
                break;
            }
        }

        if (auth_success) {
            fclose(auth_file);
            log_message("Successful login attempt - User: %s", received_username);
            send(client_fd, "Client authenticated\n", 21, 0);
            return 1;
        } else {
            log_message("Failed login attempt - User: %s", received_username);
            send(client_fd, "Client authentication failed\n", 28, 0);
            fseek(auth_file, 0, SEEK_SET);
        }
    }
}