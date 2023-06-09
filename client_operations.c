#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include "client_operations.h"

#define MAX_BUF_SIZE 2048

static void remove_newline(char *str) {
    char *newline = strchr(str, '\n');
    if (newline) {
        *newline = '\0';
    }
}

void authenticate_and_run_client_operations(int client_fd) {
    char buffer[MAX_BUF_SIZE];
    ssize_t bytes_received;

    while (1) {
        bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            perror("Error receiving data from server");
            break;
        }

        buffer[bytes_received] = '\0';
        remove_newline(buffer);

        if (strstr(buffer, "Enter username:") != NULL || strstr(buffer, "Enter password:") != NULL) {
            printf("%s", buffer);
            char input[MAX_BUF_SIZE];
            fgets(input, sizeof(input), stdin);
            send(client_fd, input, strlen(input), 0);
        } else if (strcmp(buffer, "Client authenticated") == 0) {
            printf("%s\n", buffer);
            show_menu_and_handle_commands(client_fd);
            break;
        } else if (strcmp(buffer, "Client authentication failed") == 0) {
            printf("%s\n", buffer);
            printf("\n");
        } else {
            printf("Unknown message from server: %s\n", buffer);
        }
    }
}

void show_menu_and_handle_commands(int client_fd) {
    int option;
    char buffer[MAX_BUF_SIZE];

    do {
        printf("\nMenu:\n");
        printf("1. List files on the server\n");
        printf("2. Upload a file to the server\n");
        printf("3. Download a file from the server\n");
        printf("4. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &option);
        getchar(); // to consume the newline character

        switch (option) {
            case 1:
                send(client_fd, "list\n", 5, 0);
                receive_directory_listing(client_fd);
                break;
            case 2:
                printf("Enter the name of the file to upload: ");
                fgets(buffer, sizeof(buffer), stdin);
                remove_newline(buffer);
                send(client_fd, "upload ", 7, 0);
                send(client_fd, buffer, strlen(buffer), 0);
                send(client_fd, "\n", 1, 0);
                send_file(client_fd, buffer);
                break;
            case 3:
                printf("Enter the name of the file to download: ");
                fgets(buffer, sizeof(buffer), stdin);
                remove_newline(buffer);
                send(client_fd, "download ", 9, 0);
                send(client_fd, buffer, strlen(buffer), 0);
                send(client_fd, "\n", 1, 0);
                receive_file(client_fd, buffer);
                break;
            case 4:
                printf("Exiting...\n");
                break;
            default:
                printf("Invalid option. Please try again.\n");
                break;
        }
        fflush(stdin);
    } while (option != 4);
}

void receive_directory_listing(int client_fd) {
    char data[MAX_BUF_SIZE];
    ssize_t bytes_received;
    int file_count = 0;

    printf("\nFiles on the server:\n");

    // Receive the file list
    bytes_received = recv(client_fd, data, sizeof(data) - 1, 0);
    if (bytes_received > 0) {
        data[bytes_received] = '\0';
        char *file_name = strtok(data, "\n");

        while (file_name != NULL) {
            if (strcmp(file_name, "END") != 0) {
                printf("%s\n", file_name);
                file_count++;
            } else {
                break;
            }
            file_name = strtok(NULL, "\n");
        }
    }

    if (file_count == 0) {
        printf("No files yet.\n");
    }
}

void send_file(int client_fd, char* filename) {
    int file_fd;
    struct stat file_stat;
    char buffer[MAX_BUF_SIZE];
    ssize_t bytes_read, bytes_written;

    file_fd = open(filename, O_RDONLY);
    if (file_fd == -1) {
        perror("Error opening file for upload");
        return;
    }

    fstat(file_fd, &file_stat);

    // Send the "upload" command, filename, and file size
    snprintf(buffer, sizeof(buffer), "upload %s %lld", filename, file_stat.st_size);
    send(client_fd, buffer, strlen(buffer), 0);

    while ((bytes_read = read(file_fd, buffer, sizeof(buffer))) > 0) {
        bytes_written = write(client_fd, buffer, bytes_read);
        if (bytes_written == -1) {
            perror("Error sending file");
            break;
        }
    }

    close(file_fd);

    // Receive confirmation message
    char confirmation[MAX_BUF_SIZE];
    ssize_t bytes_received;
    bytes_received = recv(client_fd, confirmation, sizeof(confirmation) - 1, 0);
    if (bytes_received > 0) {
        confirmation[bytes_received] = '\0';
        printf("%s", confirmation);
    }
}

void receive_file(int client_fd, char* filename) {
    int file_fd;
    char buffer[MAX_BUF_SIZE];
    ssize_t bytes_read, bytes_written;

    // Receive file size as long int
    long int file_size;
    recv(client_fd, &file_size, sizeof(long int), 0);

    char file_path[MAX_BUF_SIZE];
    snprintf(file_path, sizeof(file_path), "downloded_files/%s", filename);
    file_fd = open(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (file_fd == -1) {
        perror("Error opening file for download");
        return;
    }

    while (file_size > 0) {
        bytes_read = read(client_fd, buffer, sizeof(buffer));
        if (bytes_read == -1) {
            perror("Error receiving file");
            break;
        }
        bytes_written = write(file_fd, buffer, bytes_read);
        if (bytes_written == -1) {
            perror("Error writing file");
            break;
        }
        file_size -= bytes_read;
    }

    close(file_fd);
    printf("File downloaded: %s\n", filename);
}