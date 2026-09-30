# Project context

- 5p replays pcap/pcapng traffic over TCP or UDP. PcapPlusPlus reads and reassembles packets; Boost.Asio sends them. CLI11 parses options.
- Public headers live in `includes/5p/`; implementations live in `src/`; GoogleTest tests live in `tests/`. Keep interface and implementation changes together.
- Use existing Conan 2 workflow: `conan install . --build=missing`, then `conan build . --build=missing`. Build runs CTest and cppcheck; CMake registers `5p_tests`.
- Keep packet parsing, reassembly, sender selection, and socket I/O in their owning modules. Preserve TCP/UDP behavior, packet byte lengths, and fragment handling; avoid ad hoc parsing and broad rewrites.
- Prefer small functions with one responsibility. Add focused GoogleTest coverage for behavior changes; keep comments short and useful.
- Windows runtime capture dependencies (Npcap DLLs) have licensing constraints; see `README.md` before changing platform integration.