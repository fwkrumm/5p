import socket

def udp_sender():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    #server_address = ("192.168.65.254", 49999) # ip via ping host.docker.internal; for testing udp -> host system
    server_address = ("127.0.0.1", 49999)
    message = b'This is the message. It will be sent via UDP.'
    message = bytearray(list(range(0, 256)))

    try:
        print(f'Sending {message}')
        sent = sock.sendto(message, server_address)
        print(f'Sent {sent} bytes')
    finally:
        print('Closing socket')
        sock.close()

if __name__ == '__main__':
    udp_sender()