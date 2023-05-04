#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include "file_operations.h"
#include "log.h"

#define MAX_BUF_SIZE 2048

void handle_client_commands(int client_fd) {
    char buffer[MAX_BUF_SIZE];
    ssize_t bytes_received;

    while ((bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';

        if (strcmp(buffer, "list\n") == 0) {
            send_directory_listing(client_fd);
        } else if (strncmp(buffer, "upload ", 7) == 0) {
            char *filename = strtok(buffer + 7, "\n");
            if (filename) {
                receive_file(client_fd, filename);
            }
        } else if (strncmp(buffer, "download ", 9) == 0) {
            char *filename = strtok(buffer + 9, "\n");
            if (filename) {
                send_file(client_fd, filename);
            }
        } else {
            printf("Unknown command: %s\n", buffer);
        }
    }
}

void send_directory_listing(int client_fd) {
    DIR *dir;
    struct dirent *entry;
    char buffer[MAX_BUF_SIZE];

    dir = opendir("server_files");
    if (dir == NULL) {
        perror("Error opening directory");
        return;
    }

    int file_count = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            snprintf(buffer, sizeof(buffer), "%s\n", entry->d_name);
            send(client_fd, buffer, strlen(buffer), 0);
            file_count++;
        }
    }

    if (file_count == 0) {
        snprintf(buffer, sizeof(buffer), "No files yet.\n");
        send(client_fd, buffer, strlen(buffer), 0);
    }

    closedir(dir);
    snprintf(buffer, sizeof(buffer), "\n"); // Send an extra newline character
    send(client_fd, buffer, strlen(buffer), 0);

    char confirmation[MAX_BUF_SIZE];
    recv(client_fd, confirmation, sizeof(confirmation) - 1, 0);
}

void receive_file(int client_fd, const char *filename) {
    int file_fd;
    char buffer[MAX_BUF_SIZE];
    ssize_t bytes_received;

    char server_file_path[MAX_BUF_SIZE];
    snprintf(server_file_path, sizeof(server_file_path), "server_files/%s", filename);

    file_fd = open(server_file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_fd == -1) {
        perror("Error opening file for upload");
        return;
    }

    // Receive the file size
    long int file_size;
    recv(client_fd, &file_size, sizeof(long int), 0);

    while (file_size > 0 && (bytes_received = recv(client_fd, buffer, MAX_BUF_SIZE, 0)) > 0) {
        write(file_fd, buffer, bytes_received);
        file_size -= bytes_received;
    }

    close(file_fd);
    log_message("File uploaded: %s", filename);
}

void send_file(int client_fd, const char *filename) {
    int file_fd;
    struct stat file_stat;
    char buffer[MAX_BUF_SIZE];
    ssize_t bytes_read, bytes_written;

    char server_file_path[MAX_BUF_SIZE];
    snprintf(server_file_path, sizeof(server_file_path), "server_files/%s", filename);

    file_fd = open(server_file_path, O_RDONLY);
    if (file_fd == -1) {
        perror("Error opening file for download");
        return;
    }

    fstat(file_fd, &file_stat);

    // Send the file size
    long int file_size = file_stat.st_size;
    send(client_fd, &file_size, sizeof(long int), 0);

    while ((bytes_read = read(file_fd, buffer, sizeof(buffer))) > 0) {
        bytes_written = write(client_fd, buffer, bytes_read);
        if (bytes_written == -1) {
            perror("Error sending file");
            break;
        }
    }

    close(file_fd);
    log_message("File downloaded: %s", filename);
}