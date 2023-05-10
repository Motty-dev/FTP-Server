#ifndef CLIENT_OPERATIONS_H
#define CLIENT_OPERATIONS_H

void authenticate_and_run_client_operations(int client_fd);
void show_menu_and_handle_commands(int client_fd);
void receive_directory_listing(int client_fd);
void send_file(int client_fd, char* filename);
void receive_file(int client_fd, char* filename);

#endif