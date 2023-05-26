`extras/scripts`
================
This directory contains helpful shell scripts when working with CAN.

### How-to-`SocketCAN`
```bash
sudo ./setup_scan.sh
```
Display received CAN frames via [`candump`](https://manpages.ubuntu.com/manpages/jammy/man1/candump.1.html):
```bash
candump can0
```
Transmit CAN frames via [`cansend`](https://manpages.ubuntu.com/manpages/jammy/man1/cansend.1.html):
```bash
cansend can0 00001234#DEADBEEF
```
