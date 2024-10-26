from scapy.all import *
from scapy.all import IP, UDP

# define the payload and fragment size
payload = b"This is a test payload for fragmented UDP packets."
FRAGMENT_SIZE = 8  # size of each fragment
RUNS = 10  # number of times to send the payload

# create the IP and UDP headers
ip = IP(dst="127.0.0.1")  #  the destination IP
udp = UDP(sport=12345, dport=80)  # source and destination ports

# fragment the payload
fragments = [payload[i:i+FRAGMENT_SIZE] for i in range(0, len(payload), FRAGMENT_SIZE)]

# send each fragment
for _ in range(RUNS):
    for i, fragment in enumerate(fragments):
        ip.frag = i
        ip.flags = "MF" if i < len(fragments) - 1 else 0
        packet = ip / udp / fragment
        send(packet)

print("fragmented UDP packets sent of payload size %d" % len(payload))
