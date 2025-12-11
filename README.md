# Wifi Sniffer
---
### Overview
A linux kernel module + userspace program which captures and parses 802.11 Wifi Frames, displaying parsed data with ncurses in a simple dashboard. 

### Features
- Captures and parses radiotap + 802.11 MAC frames via an RX handler
- Parses frame types & subtypes, mac addresses, ToDs/FromDs, retries and management frame ies

### Diagram

### How It Works

### How to run/build
1. Clone the git repo
2. Run sudo ./run.sh
3. It builds the kernel module, loads it, builds the userspace program, and then runs it.
4. When you close the userspace program it cleans itself up afterwards (not the fastest, but it's nice to keep everything clean)
### Limitations
- Limited data on my machine - I believe my laptop card removes rssi data and a lot of data packets don't show up, although I see them with tcpdump.
- Uses main physical wifi card - would be nice to work with a second one so I don't have to bring down wifi connectivity while the program runs
- Program is missing a lot of detail in the analysis - not really a limitation since this was more an experiement than actually building a professional program.
### What I learnt
I learnt a lot about networking and linux kernel / driver development. This was my first time working at such a low level, so getting to understand how kernel modules work, creating my own wifi interface and intercepting wifi frames and learning everything about the 802.11 frame headers and how they're built was really informative to me.
