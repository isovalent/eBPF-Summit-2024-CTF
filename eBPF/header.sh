#!/bin/bash

echo "Generating eBPF Header"
mkdir ./header/
bpftool btf dump file /sys/kernel/btf/vmlinux format c > ./header/vmlinux.h

