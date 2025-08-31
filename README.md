# **P**ca**PP**lus**P**lus**P**layer (5p)

Please be aware that this software is still in the construction phase. As such, it may contain bugs, incomplete features, and other issues. Additionally, the master branch may undergo significant changes. However, each commit should still deliver its promised functionality. Please proceed with caution.

This project is open-source, but it is your responsibility to verify and comply with any potential license issues.
We recommend thoroughly reviewing the licenses of any dependencies or third-party code used in this project.

## Table of Contents

- [About](#about)
- [Compiling](#compiling)
- [Quick Dive](#quick-dive)
- [Packet Reassembly](#packet-reassembly)
- [ToDos](#todos)


---
<a name="about"></a>
## About

This software ''replays'' pcap(ng) files, i.e., reads their content and forwards the data via TCP or UDP packets.
The dependencies for the main functionality are pcapplusplus and boost.

1. On Windows, I am not aware of any suitable tools available for this task (especially with a free commercial license).
    On Linux, there is tcpreplay, which, however, did not work well with Boost sockets (at least for me).
    As far as I know, this issue is related to the different network layers on which the techniques operate.

    5p is supposed to read pcap(ng) files and forward the data via UDP/TCP for testing.

    However, there are some limitations for Windows usage:

    On Windows, the wcap.dll and packet.dll required by the pcapplusplus library (to run the program) are part of the npcap SDK and require a license for commercial usage.
    Compiling the software, however, does not require these dependencies.
    This repository contains a devcontainer (Linux) in which the software can be built and uses libpcap (BSD license).
    However, please note that a commercial license for Docker might be necessary.
    To clarify this issue, please refer to the following graphic which visualizes the data flow:

    ![dataflow](./docs/assets/dataflow.svg)


2. On Linux, you should be able to run the software without this limitation.

3. For Docker usage, please ensure there is enough memory for compilation tasks; >= 4GB is recommended.
If you compile natively on Windows, you might require the v142 toolset for Visual Studio (2019); otherwise, there might be unresolved symbol errors by the Boost library.

---
<a name="compiling"></a>
## Compiling

The project uses the Conan package manager (version 2): [Conan Documentation](https://docs.conan.io/2/tutorial.html). For the following steps, a Python 3 installation with pip is required.

### Install conan

```shell
python3 -m pip install conan
conan profile detect
```

If the latter commands do not succeed, some build tools (e.g., g++ on Linux and MSVC on Windows) might be missing.

### Compile 5p

By default the conan.io remote should be available:

```shell
conan remote list
```

should yield

conancenter: https://center.conan.io [Verify SSL: True, Enabled: True].

In more recent conan 2 versions it might be https://center2.conan.io.
In the root directory of 5p (after e.g. `git clone https://github.com/NotAvailable-EUR/5p.git && cd 5p`) execute

```shell
conan install . --build=missing
conan build . --build=missing
```

The first command fetches all dependencies, and if they are not available, it tries to compile them. For this task, a CMake installation is required (check potential error logs).

**IMPORTANT NOTE** if you create a solution and compile it manually by using an IDE of your choice make sure to use "Release" build type as some dependencies might not be available as Debug build type.


---
<a name="quick-dive"></a>
## Quick Dive

After compilation you can run the program via

```
5p /path/to/test/trace.pcapng
```

which reads the packets from the trace and forwards them to `127.0.0.1` with the original destination port and protocol (TCP/UDP) of the packets.

The parameters can be adjusted in the following way


Adjust the ip so that all packets will be forwarde to that ip, still with original protocol and port
```
5p /path/to/test/trace.pcapng --ip 192.168.178.123
```

Adjust the port, the packets will be send to default ip `127.0.0.1` but all will use port `49999` with the packet original protocol (TCP/UDP).
```
5p /path/to/test/trace.pcapng --port 49999
```

Change the protocol, `2 --> UDP; 1 --> TCP`, i.e. all packets will be forwarde to `127.0.0.1` on their default port via UDP.
```
5p /path/to/test/trace.pcapng --protocol 2
```

Of course you can combine this parameters, e.g. to forward all packets via UDP on port `49999` you can use
```
5p /path/to/test/trace.pcapng --protocol 2 -- port 49999
```


Other parameters are
`--level` for the log level (logging to file always enabled on debug level)

`--filter` to apply a filter in BPF format

`--skip` to skip a certain number of packets

`--sleep` to specify delay between packet send events


For more details check
```
5p --help
```



### Using the devcontainer on Windows

It is possible to send the data from devcontainer to windows host and thus to not use any licensed dll files.
The ip address to receive the data on the host (Windows) you can extract via

```shell
ping host.docker.internal
```

which returns the ip address to use:

PING host.docker.internal (192.168.65.254) 56(84) bytes of data.

64 bytes from 192.168.65.254 (192.168.65.254): icmp_seq=1 ttl=63 time=1.38 ms

64 bytes from 192.168.65.254 (192.168.65.254): icmp_seq=2 ttl=63 time=0.491 ms
...

```
5p /path/to/test/trace.pcapng --ip 192.168.65.254
```

Note that for other ports you might need to forward that ports; cf. .devcontainer/devcontainer.json



### Test Data flow

In the root directory’s scripts folder, there are basic scripts for testing the receiving and sending UDP/TCP packets.



---
<a name="Packet reassembly"></a>
## Packet Reassembly

Sometimes packets might be fragmented. These packets will be shown in Wireshark (usually) as IPv4 packets with the fragmented flag being true. In this case, it is important to choose a suitable filter since, for example, `--filter "udp"` will throw these packets away and thus the reassembly will fail.

In such cases, a filter as follows might be more suitable (in this example for UDP packets):


```
5p path/to/test/trace.pcapng --filter "ip host x.x.x.x and ip dst x.x.x.x and ((ip[6:2] & 0x1fff != 0) or (ip proto \udp))"
```


This filter first checks for specific host and destination ip addresses and also for either complete UDP packets (non-fragmented) or fragmented packets via `(ip[6:2] & 0x1fff != 0)` which basically checks for the fragmented flag being true in the IP header (TODO is this 100% fail-safe?)



---
<a name="todos"></a>
## ToDos

- Test usage on other setups (so far tested on Windows with Docker)
- Add (more) unit tests, especially concerning packet reassembly
- Use consistent code style
- Is it possible to exclude Packet.dll and wpcap.dll from linking?
- Add pipeline for version ranges e.g. new boost versions
