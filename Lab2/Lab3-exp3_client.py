import socket
import time

# Server address (replace with your WSL IP from ifconfig)
HOST = '172.20.49.57'  # Change based on WSL IP
PORT = 65431           # Same port as the server


def get_network_details():
    hostname = socket.gethostname()
    ip_addr = socket.gethostbyname(hostname)
    fqdn = socket.getfqdn()
    
    print("Host name: ", hostname)
    print("IP address: ", ip_addr)
    print("FQDN: ", fqdn)


print("Lab2: Exp3: Client running on Windows: Welcome !!!")
get_network_details()

# Create and connect socket
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    print(f"Connected to server at {HOST}:{PORT}")

    while True:
        # Send message to server
        message = "Hello, server!"
        s.sendall(message.encode())
        print(f"Sent: {message}")

        # Receive response
        data = s.recv(1024)
        if not data:
            print("Server closed connection.")
            break

        print(f"Received: {data.decode()}")

        # Wait 2 seconds before sending the next message
        time.sleep(2)
