#!/bin/sh

echo "================================================================================"
echo
./mangle-paxflags bad-mmap
./bad-mmap
echo
echo "========================================"
echo
./mangle-paxflags -p bad-mmap
./mangle-paxflags bad-mmap
./bad-mmap
echo
echo "========================================"
echo
./mangle-paxflags -e bad-mmap
./mangle-paxflags bad-mmap
./bad-mmap
echo
echo "================================================================================"
