# Lab 3: Exp3b Asymmetric Key RSA (Client)
# Run this on Windows after starting the Server on WSL
# Install dependencies: pip install cryptography

import socket
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import padding

HOST = '172.20.49.57'  # Change it to the WSL IP address using `ifconfig`
PORT = 65432

# Load the server's public key (used to encrypt messages to the server)
with open("public.pem", "rb") as key_file:
    public_key = serialization.load_pem_public_key(key_file.read())

# Load the private key (used to decrypt messages from the server)
with open("private.pem", "rb") as key_file:
    private_key = serialization.load_pem_private_key(key_file.read(), password=None)

# Encrypt the message with the server's public key
message = b"Hello from asymmetric client"
ciphertext = public_key.encrypt(
    message,
    padding.PKCS1v15()
)

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    s.sendall(ciphertext)
    
    # Receive encrypted response from server
    data = s.recv(512)
    print("Received encrypted data from server:", data.hex())

    # Decrypt the received data using private.pem
    decrypted_message = private_key.decrypt(
        data,
        padding.PKCS1v15()
    )
    
    print("Decrypted message from server:", decrypted_message.decode())
