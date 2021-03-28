#!/bin/sh

cd ../
make clean
make
echo "\n\n"
echo "-------------------"
echo "[!] Doing whoami..."
whoami
echo "[!] Creating file to hide and doing ls..."
touch lzrs_keywordFOO
ls
echo "[+] Loading the LKM"
make load
echo "[!] Doing whoami after loading LKM..."
whoami
echo "[!] Doing ls after loading LKM..."
ls
