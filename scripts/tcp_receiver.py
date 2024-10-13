import socket

def start_tcp_receiver(ip, port):
    # Create a TCP/IP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # Bind the socket to the address and port
    server_address = (ip, port)
    print(f'Starting TCP receiver on {ip}:{port}')
    sock.bind(server_address)

    # Listen for incoming connections
    sock.listen(1)

    print('Waiting for a connection...')
    connection, client_address = sock.accept()
    try:
        print(f'Connection from {client_address}')

        # Receive the data in chunks and print it
        while True:
            data = connection.recv(1024)
            if data:
                print(f'Received: {data}')
            # break # use break here if you want to receive data only once
    finally:
        # Clean up the connection
        connection.close()

if __name__ == "__main__":
    ip = "127.0.0.1"  # Example IP
    port = 49999      # Example port
    start_tcp_receiver(ip, port)