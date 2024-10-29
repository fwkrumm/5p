"""

to test the reassembly of fragmented UDP packets use

5p.exe fragmented2.pcapng --port 49999 --filter "src host 127.0.0.1 and dst host 127.0.0.1 and not tcp" --protocol 2

this additionally forwards the packets to the port 49999 via udp (protocol == 2)
important however is to not use "udp" or any specific protocol in the filter
because the fragments packets are of type ipv4

"""


from scapy.all import *
from scapy.all import IP, UDP, fragment

# define the payload and fragment size
payload = b"This is a test payload for fragmented UDP packets."
FRAGMENT_SIZE = 8  # size of each fragment
RUNS = 10  # number of times to send the payload

# create the IP and UDP headers
ip = IP(dst="127.0.0.1")  # the destination IP
udp = UDP(sport=12345, dport=80)  # source and destination ports

# create the initial packet with headers
packet = ip / udp / payload

# fragment the packet
fragments = fragment(packet, FRAGMENT_SIZE)

# send each fragment
for _ in range(RUNS):
    for fragment in fragments:
        send(fragment)

print("fragmented UDP packets sent with payload size %d" % len(payload))
