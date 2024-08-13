# ebpf Summit 2024 Capture the Flag ⛳️🐝 

This is the repository containing all the code/files for the CTF for the upcoming (2024) eBPF summit.

## Building

Create the required ebpf headers `cd ebpf` & `./header.sh`


The `/emperium` folder contains the code for the **Emperium** system, this is the security system for the Empire's Tie Fighter systems. Access to this system would give the rebels the upper hand when it comes to X-Wing to Tie fighter combat. 

To build the stolen copy of the `emperium` system:

`cd emperium` & `go build`

## Running

In order to run the **Emperium** system, it must be started with `sudo` in order to have the privileges to interact with 🐝 eBPF, including programs and maps.

## Notes:

When running `emperium` it will create 1000s of maps in order to make it more difficult to find the correct maps (more to do on that)