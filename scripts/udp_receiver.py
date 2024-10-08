import socket

def udp_receiver():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    server_address = ('', 49999)
    sock.bind(server_address)

    print('Waiting to receive message')
    while True:
        data, address = sock.recvfrom(4096)
        print(f'Received {data} from {address}')
        break

if __name__ == '__main__':
    udp_receiver()