#ifndef FILE_OPERATIONS_H
#define FILE_OPERATIONS_H

void handle_client_commands(int client_fd);
void send_directory_listing(int client_fd);
void receive_file(int client_fd, const char *filename, long int file_size);
void send_file(int client_fd, const char *filename);

#endif
