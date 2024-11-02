"""
This script creates a simple UDP packet with a payload and writes it to a pcap file.
Will be used for test purposes.
"""

from scapy.all import IP, UDP, Raw, wrpcap

# Create a simple UDP packet with a payload
payload = "This is a test payload"
packet = IP(dst="192.168.1.1")/UDP(dport=12345, sport=54321)/Raw(load=payload)

# Create a list of packets
packets = [packet] * 10  # Create 10 identical packets

# Write the created packets to a PCAP file
wrpcap('test.pcap', packets)
