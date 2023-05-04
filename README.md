# FTP Server

This is a simple FTP server implemented using TCP socket programming in C. It allows clients to connect and transfer files to and from the server.

## Requirements

1. The server should be able to accept multiple client connections simultaneously.
2. Clients should be able to authenticate with the server using a username and password.
3. Clients should be able to list the contents of the server's directory, upload files to the server, and download files from the server.
4. The server should maintain a log of all connections, login attempts, and file transfers.
5. The server should handle errors gracefully, such as when a client tries to upload a file that already exists on the server.
6. The UI must be in the terminal.

## Usage

To compile the server, run:

make server


To compile the client, run:

make client


To start the server, run:

./server


To start the client, run:

./client

Once you have started the client,
first youll need to auth yourself. 

username: user1
password: password1

in auth.txt you can add another username and passwords.
you can use the following commands:

- `1`: list the contents of the server's directory.
- `2`: download a file from the server.
- `3`: upload a file to the server.
- `4`: exit the client.

## Logging

The server maintains a log of all connections, login attempts, and file transfers in the `server.log` file.

## Cleaning up

To clean up the compiled files, run:

make clean
