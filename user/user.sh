#!/bin/sh

cd ../
make clean
make
echo "\n\n"
echo "-------------------"
echo "[!] Doing whoami..."
whoami
echo "[+] Loading the LKM"
make load
echo "[!] Doing whoami after loading LKM..."
whoami
