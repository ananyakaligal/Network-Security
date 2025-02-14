#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  // For close()
#include <pthread.h> // For threads
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> // For sockaddr_in
#include <arpa/inet.h>  // For inet_ntoa()

#define PORT 65431
#define BUFFER_SIZE 1024
#define MAX_THREADS 2

int server_fd;

// Function to handle client connections
void *accept_new_connection(void *arg)
{
  struct sockaddr_in client_addr;
  socklen_t addr_len = sizeof(client_addr);
  int serial_number = 1;

  while (1)
  {
    // Accept a new client connection
    int client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
    if (client_socket < 0)
    {
      perror("Accept failed");
      continue;
    }

    printf("Connected to client: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    // Handle communication with the client
    while (1)
    {
      char buffer[BUFFER_SIZE];
      memset(buffer, 0, BUFFER_SIZE);

      // Receive message from the client
      ssize_t bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
      if (bytes_read <= 0)
      {
        printf("Client disconnected: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        break;
      }

      buffer[bytes_read] = '\0'; // Null-terminate received message
      printf("Received from client: %s\n", buffer);

      // Append serial number and reply
      char reply[BUFFER_SIZE];
      snprintf(reply, BUFFER_SIZE, "%d: %.1000s Server reply", serial_number++, buffer);

      // Send reply to client
      ssize_t bytes_sent = send(client_socket, reply, strlen(reply), 0);
      if (bytes_sent < 0)
      {
        perror("Send failed");
        break;
      }

      printf("Sent to client: %s\n", reply);
    }

    // Close client socket
    close(client_socket);
  }

  return NULL;
}

int main()
{
  struct sockaddr_in server_addr;
  int opt = 1;

  // 1. Create socket
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("Socket failed");
    exit(EXIT_FAILURE);
  }

  // 2. Set socket options
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
  {
    perror("setsockopt failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  // 3. Define server address
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(PORT);

  // 4. Bind the socket
  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
  {
    perror("Bind failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  printf("Server listening on port %d...\n", PORT);

  // 5. Listen for incoming connections
  if (listen(server_fd, 5) < 0)
  {
    perror("Listen failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  // 6. Create threads to handle multiple clients
  pthread_t threads[MAX_THREADS];
  for (int i = 0; i < MAX_THREADS; i++)
  {
    pthread_create(&threads[i], NULL, accept_new_connection, NULL);
  }

  // 7. Keep threads running
  for (int i = 0; i < MAX_THREADS; i++)
  {
    pthread_join(threads[i], NULL);
  }

  // 8. Close server socket
  close(server_fd);
  return 0;
}
