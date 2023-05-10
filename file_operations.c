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
            char *filename = strtok(buffer + 7, " ");
            if (filename) {
                long int file_size;
                char *file_size_str = strtok(NULL, " ");
                if (file_size_str) {
                    file_size = strtol(file_size_str, NULL, 10);
                    receive_file(client_fd, filename, file_size);
                }
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
    char data[MAX_BUF_SIZE];
    const char *directory_path = "server_files"; // Set the directory path

    memset(data, 0, sizeof(data));

    dir = opendir(directory_path);
    if (dir != NULL) {
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_REG) {
                strncat(data, entry->d_name, sizeof(data) - strlen(data) - 1);
                strncat(data, "\n", sizeof(data) - strlen(data) - 1);
            }
        }
        closedir(dir);
    }

    if (strlen(data) == 0) {
        strncpy(data, "No files yet.\n", sizeof(data) - 1);
    }
    
    strncat(data, "END\n", sizeof(data) - strlen(data) - 1);
    send(client_fd, data, strlen(data), 0);
}

void receive_file(int client_fd, const char *filename, long int file_size) {
    char buffer[MAX_BUF_SIZE];
    FILE *file;
    size_t bytes_received;
    char file_path[MAX_BUF_SIZE];
    long int bytes_total = 0;

    snprintf(file_path, sizeof(file_path), "server_files/%s", filename); // Save the file in the server_files directory

    file = fopen(file_path, "wb");
    if (file != NULL) {
        while (bytes_total < file_size) {
            memset(buffer, 0, sizeof(buffer));
            bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);
            if (bytes_received > 0) {
                fwrite(buffer, 1, bytes_received, file);
                bytes_total += bytes_received;
            }
        }
        fclose(file);
    } else {
        perror("Error opening file for writing");
    }

    // Send a confirmation message after the file is uploaded
    snprintf(buffer, sizeof(buffer), "File uploaded: %s\n", filename);
    send(client_fd, buffer, strlen(buffer), 0);
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