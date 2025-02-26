// Lab 3: Exp3c Asymmetric Key RSA (Server)
// Run this on WSL before starting the Client on Windows
// Install dependencies: sudo apt-get update && sudo apt-get install libssl-dev
// Compile using: gcc Lab3_Exp3c_Asym_Server_WSL.c -lssl -lcrypto -o asym_server -Wno-deprecated-declarations
// Run: ./asym_server

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/err.h>

#define PORT 65432
#define BUFFER_SIZE 512

// Load RSA private key from file
RSA *load_private_key(const char *filename)
{
  FILE *fp = fopen(filename, "r");
  if (!fp)
  {
    perror("fopen private.pem");
    exit(EXIT_FAILURE);
  }
  RSA *rsa = PEM_read_RSAPrivateKey(fp, NULL, NULL, NULL);
  fclose(fp);
  if (!rsa)
  {
    ERR_print_errors_fp(stderr);
    exit(EXIT_FAILURE);
  }
  return rsa;
}

// Load RSA public key from file
RSA *load_public_key(const char *filename)
{
  FILE *fp = fopen(filename, "r");
  if (!fp)
  {
    perror("fopen public.pem");
    exit(EXIT_FAILURE);
  }
  RSA *rsa = PEM_read_RSA_PUBKEY(fp, NULL, NULL, NULL);
  fclose(fp);
  if (!rsa)
  {
    ERR_print_errors_fp(stderr);
    exit(EXIT_FAILURE);
  }
  return rsa;
}

// Function to send an additional encrypted message to the client
void SendAdditionalMessage(int socket, RSA *rsa_pub, char *msg)
{
  unsigned char encrypted[BUFFER_SIZE];
  int encrypted_len = RSA_public_encrypt(strlen(msg), (unsigned char *)msg, encrypted, rsa_pub, RSA_PKCS1_PADDING);
  if (encrypted_len == -1)
  {
    ERR_print_errors_fp(stderr);
    exit(EXIT_FAILURE);
  }

  write(socket, encrypted, encrypted_len);
}

int main()
{
  int server_fd, new_socket;
  struct sockaddr_in address;
  int addr_len = sizeof(address);
  unsigned char buffer[BUFFER_SIZE];
  int bytes_read;

  OpenSSL_add_all_algorithms();
  ERR_load_crypto_strings();

  // Create TCP socket
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0)
  {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
  {
    perror("bind");
    exit(EXIT_FAILURE);
  }
  if (listen(server_fd, 5) < 0)
  {
    perror("listen");
    exit(EXIT_FAILURE);
  }
  printf("Asymmetric server listening on port %d...\n", PORT);

  new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addr_len);
  if (new_socket < 0)
  {
    perror("accept");
    exit(EXIT_FAILURE);
  }

  bytes_read = read(new_socket, buffer, BUFFER_SIZE);

  RSA *rsa = load_private_key("private.pem");
  unsigned char decrypted[BUFFER_SIZE];
  int decrypted_len = RSA_private_decrypt(bytes_read, buffer, decrypted, rsa, RSA_PKCS1_PADDING);
  if (decrypted_len == -1)
  {
    ERR_print_errors_fp(stderr);
    exit(EXIT_FAILURE);
  }
  decrypted[decrypted_len] = '\0';
  printf("Decrypted message from client: %s\n", decrypted);

  // Load public key to encrypt response
  RSA *rsa_pub = load_public_key("public.pem");

  // First response message
  char *response = "Hello from asymmetric server";
  unsigned char encrypted[BUFFER_SIZE];
  int encrypted_len = RSA_public_encrypt(strlen(response), (unsigned char *)response, encrypted, rsa_pub, RSA_PKCS1_PADDING);
  if (encrypted_len == -1)
  {
    ERR_print_errors_fp(stderr);
    exit(EXIT_FAILURE);
  }

  write(new_socket, encrypted, encrypted_len);

  // Additional message
  char *additional_message = "Exp3c: Additional message from Server\n";
  SendAdditionalMessage(new_socket, rsa_pub, additional_message);

  RSA_free(rsa);
  RSA_free(rsa_pub);
  close(new_socket);
  close(server_fd);
  EVP_cleanup();
  ERR_free_strings();

  return 0;
}
