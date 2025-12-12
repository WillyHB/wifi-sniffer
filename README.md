# Wifi Sniffer
### Overview
A linux kernel module + userspace program which captures and parses 802.11 Wifi Frames, displaying parsed data with ncurses in a simple dashboard. 

### Features
- Captures and parses radiotap + 802.11 MAC frames via an RX handler
- Parses frame types & subtypes, mac addresses, ToDs/FromDs, retries and management frame ies

### Diagram

### How It Works
When you first run `run.sh` the first thing it does is call the `sniffer_start.sh` script. This script firstly kills wpa_supplicant and stops NetworkManager. Afterwards, the `wlo1` wifi interface (The name on my machine) is brought down. We stop NetworkManager and kill wpa_supplicant so that they don't bring `wlo1` up again after we bring it down. Once that's down, we add a new monitor type interface to the phy and phy0 (physical wifi card) so we can monitor all wifi frames, not just the ones that are meant for our machine. We bring this interface up and then load the kernel module.

After that, the `run.sh` script cds into the user directory and runs the userspace program.


### How to run/build
1. Clone the git repo
2. Run `make` in the project root (builds the kernel module + userspace program)
3. Run `sudo ./run.sh` it will load the kernel module, takes down `wlo1` (That's what my main wifi interface is called, if you want to use the code yourself rename it in the `sniffer_start.sh` and `sniffer_stop.sh` files), brings up a new `mon0` monitor virtual interface and then opens the userspace program
4. When you close the userspace program, it automatically unloads the kernel module and brings your system back to normal
5. Run `make clean` in the project root to clean up all build files
### Limitations
- Limited data available to sniff on my machine - I believe my laptop wificard removes rssi data and a lot of data packets don't show up. I wanted to test with a usb wifi antenna but never got around to getting one.
- Uses my laptops main physical wifi card - would be nice to work with a second one so I don't have to bring down wifi connectivity while the program runs
- Program is missing a lot of detail in the analysis - not really a limitation since this was more an experiement than actually building a professional program.
### What I learnt
I learnt a lot about networking and linux kernel / driver development. What started out as a small project just to get familiar with 802.11 and networking taught me so much

This was my first time working at such a low level, so getting to understand how kernel modules work, creating my own wifi interface and intercepting wifi frames and learning everything about the 802.11 frame headers and how they're built was really informative to me.


