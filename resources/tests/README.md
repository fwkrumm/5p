The pcap files in this directory are exclusively for testing purposes.

## License Information

These captures were obtained from the [Wireshark Sample Captures](https://wiki.wireshark.org/SampleCaptures) catalogue:

| File | Original download | Integration test coverage |
| --- | --- | --- |
| `hart_ip.pcap` | [HART-IP](https://wiki.wireshark.org/uploads/__moin_import__/attachments/SampleCaptures/hart_ip.pcap) | TCP and UDP on port 5094 |
| `chargen-udp.pcap` | [Chargen UDP](https://wiki.wireshark.org/uploads/__moin_import__/attachments/SampleCaptures/chargen-udp.pcap) | Two UDP datagrams; classic pcap format |
| `dns.cap` | [DNS](https://wiki.wireshark.org/uploads/__moin_import__/attachments/SampleCaptures/dns.cap) | Nineteen UDP queries on port 53 |
| `tcp-ethereal-file1.trace` | [HTTP POST](https://wiki.wireshark.org/uploads/__moin_import__/attachments/SampleCaptures/tcp-ethereal-file1.trace) | TCP payload across 130 segments |

The catalogue does not state a license for these individual contributed captures. Wireshark's GPL license for its software does not by itself establish the license of the attachments. `LICENSE.txt` contains GPLv3 text but should not be taken as proof of permission to redistribute these captures; confirm the terms with the original contributors before redistributing them.

Despite its `.pcap` suffix, `hart_ip.pcap` has a pcapng header. These fixtures exercise socket replay; none substitutes for a dedicated fragmentation or raw-interface replay fixture.
