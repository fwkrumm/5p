import socket

def tcp_sender(host, port, message):
    # Create a TCP/IP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        # Connect the socket to the server
        sock.connect((host, port))
        print(f"Connected to {host}:{port}")

        # Send the message
        sock.sendall(message.encode('utf-8'))
        print(f"Sent: {message}")

        # Receive response (optional)
        #response = sock.recv(1024)
        #print(f"Received: {response.decode('utf-8')}")

    finally:
        # Close the socket
        sock.close()
        print("Connection closed")

# Example usage
host = '127.0.0.1'
port = 49999
message = 'Hello, TCP Server!'
tcp_sender(host, port, message)
