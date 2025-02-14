// Lab2_exp1b_server_WSL.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>         // For close()
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>     // For sockaddr_in
#include <arpa/inet.h>      // For inet_ntoa()

#define PORT 65431          // Port to listen on
#define BUFFER_SIZE 1024    // Buffer size for receiving messages

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    socklen_t addr_len = sizeof(address);
    char buffer[BUFFER_SIZE];
    int opt = 1;
    ssize_t bytes_read, bytes_sent;

    // 1. Create socket file descriptor (IPv4, TCP)
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 2. Attach socket to the port 65431
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt))) {
        perror("setsockopt failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 3. Define the server address
    address.sin_family = AF_INET;          // IPv4
    address.sin_addr.s_addr = INADDR_ANY;  // Listen on all interfaces
    address.sin_port = htons(PORT);        // Convert to network byte order

    // 4. Bind the socket to the address
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // 5. Listen for incoming connections
    if (listen(server_fd, 5) < 0) { // backlog of 5
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    while (1) { // Server runs indefinitely
        // 6. Accept a new connection
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                                 &addr_len)) < 0) {
            perror("accept failed");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        printf("Connected to client: %s:%d\n",
               inet_ntoa(address.sin_addr), ntohs(address.sin_port));

        // 7. Communicate with the connected client
        while (1) {
            memset(buffer, 0, BUFFER_SIZE); // Clear the buffer

            // Receive data from the client
            bytes_read = recv(new_socket, buffer, BUFFER_SIZE - 1, 0);
            if (bytes_read < 0) {
                perror("recv failed");
                break;
            } else if (bytes_read == 0) {
                printf("Client disconnected: %s:%d\n",
                       inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                break; // Client disconnected
            }

            buffer[bytes_read] = '\0'; // Null-terminate the received data
            printf("Received from client: %s\n", buffer);

            // Append " Server reply" to the received message
            strncat(buffer, " Server reply", BUFFER_SIZE - bytes_read - 1);

            // Send the modified message back to the client
            bytes_sent = send(new_socket, buffer, strlen(buffer), 0);
            if (bytes_sent < 0) {
                perror("send failed");
                break;
            }

            printf("Sent to client: %s\n", buffer);
        }

        // 8. before closing the connection send one last message to the client before closing
        // This is to demonstrate half-close feature of TCP
        if(bytes_read == 0){ // if the client has closed on its side of the connection
            strcpy(buffer, "Server is sending a last message before closing from my side"); // dest ptr, src ptr
            bytes_sent = send(new_socket, buffer, strlen(buffer), 0);
            if (bytes_sent < 0) {
                perror("send failed");
                break;
            }            
            sleep(2); // just to give time for the client by delaying 2 secs before closing
            close(new_socket);
        }

    }

    // 9. Close the server socket (unreachable code in this example)
    close(server_fd);
    return 0;
}

