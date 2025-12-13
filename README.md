# Wifi Sniffer
## Overview
A linux kernel module + userspace program which captures and parses 802.11 Wifi Frames, displaying parsed data with ncurses in a simple dashboard. 

## Screenshots
<img width="2880" height="1800" alt="sniffer" src="https://github.com/user-attachments/assets/54a724d0-0abe-4a07-8076-f790f823f4a5" />

<img width="2880" height="1800" alt="sniffer2" src="https://github.com/user-attachments/assets/c2ff5541-6b62-45d4-ae29-8fd7ab53280f" />

<img width="2880" height="1800" alt="sniffer3" src="https://github.com/user-attachments/assets/b5f88a34-c201-4078-8746-1f0d45ba1ebf" />

## Features
- Captures and parses radiotap + 802.11 MAC frames via an RX handler
- Parses frame types & subtypes, mac addresses, ToDs/FromDs, retries and management frame ies

## How it Works TLDR
Kernel Module Intercepts rx packets through a virtual monitor interface, parses and sends radiotap header + 80211 header + 80211 frame body to a userspace program
Userspace program parses header and displays packets and APs in a nice format.

## Technical Program Flow
When you first run `run.sh` the first thing it does is call the `sniffer_start.sh` script. This script firstly kills wpa_supplicant and stops NetworkManager. Afterwards, the `wlo1` wifi interface (The name on my machine) is brought down. We stop NetworkManager and kill wpa_supplicant so that they don't bring `wlo1` up again after we bring it down. Once that's down, we add a new monitor type interface to the phy and phy0 (physical wifi card) so we can monitor all wifi frames, not just the ones that are meant for our machine. We bring this interface up and then load the kernel module.

After that, the `run.sh` script cds into the user directory and runs the userspace program.

### Kernel Module
The main job of the kernel module is to intercept packets, parse them, then multicast the parsed data through a netlink socket as a generic netlink message. 
When it initialises, it firstly searches for the `mon0` virtual wifi interface/device by name. Using dev_by_name, the function returns a `struct netdevice` which represents the interface. Afterward, it registers our `sniffer_rx_handler` as an rx_handler for the interface - when it receives packets, it sends them to our handler as an `skb`. It also initialises our generic netlink family which we will use to send data to our userspace.

#### sniffer_rx_handler
The first thing the rx_handler does it get the radiotap header and parses the data through my custom function, which iterates using the ieee80211_radiotap_iterator struct and functions. It returns a custom `struct rt_info` which holds rssi info, channel freq and flags.

Afterwards, it gets the 80211 header by adding the radiotap length to the skb data, and parses various fields such as the frame type, subtype, whether is a retry frame, tods/fromds and lastly uses tods/fromds to correctly set src_mac and dst_mac in my custom `struct hdr_info`.

Finally we store the body of the frame and the length of the body in a u8* and int. I remove 4 from the length to account for the NCS, however I'm not sure about the correctness of this...
Finally all the data is sent to my `send_to_userspace` function.

#### send_to_userspace
This functions purpose is to allocate and send the generic netlink message. It allocates an `skb`, allocates the generic netlink message, attaches the generic netlink header and then attaches our three attributes: the radiotap header, 802.11 header and 802.11 frame body. We call `genlmsg_end` to finalise the creation of the genl message, and then multicast it through our `SNIFFER_MCGRP_FRAMES` multicast group. 


### Userspace Program
The userspace program is where the main parsing of the 802.11 frame occurs + displaying the data to the user. In the main function we firstly initialise the ncurses ui, creates a `struct ctx` which holds context information for our program like an AP buffer, Packet Buffer and general settings - and then initialises the netlink socket we will use to get the information from the kernel module multicast. For the netlink handling I use the libnl library. I chose libnl since it's a professional and stable library which eased the handling of the netlink socket. My initialisation function is simple: It allocates space for the socket, connects it as a general netlink socket, gets the multicast group id from the family name + multicast group name, and then adds a `membership` to this multicast group to the socket.It then adds my frame_handler as a callback with the `struct ctx` as an argument for when the socket receives data, and then disables sequence checking and makes the receive function nonblocking, so it doesn't block my ui and input handling.

In the main loop my program checks for input and modifies local variables accordingly, draws the ncurses ui with my own custom `draw_ui` function, checks all currently registered APs for their last_seen time, removing them if they haven't been seen for a while, calls the `nlrecvmsgs_default` function to receive from the socket and call my `frame_handler` function, and finally decreases the hop timer, and calls `channel_hop` which changes the channel my `mon0` virtual interface is monitoring.

#### frame_handler
My frame handler is simple. It receives `struct nl_msg` and `void *arg`, which holds the ctx, from the libnl library. It gets the netlink header, then generic netlink header and then parses the attributes using `nla_parse` into an array of `struct nlattr`. We then store the data for each attribute in structs: `struct hdr_info` for `SNIFFER_ATTR_HEADER`, `struct rt_info` for `SNIFFER_ATTR_RADIOTAP` and a `uint8_t *body` for `SNIFFER_ATTR_BODY`. For the body we also get the len using the `nla_len` function. It then passes all of this data to my handle_frame_info function.

#### handle_frame_info
This is the main parsing function. It takes incoming packets and extracts APs and stores them in our `struct ap` array of APs, and general packets in our `struct packet_info` packet_buf array. It increments the packet count, increments rx,tx and retries for each AP currently in the AP array and performes packet frame-type-specific parsing: 
If it's a `BEACON` or `PROBE_RESPONSE` for example we iterate the frame body for IEs which hold various data - the only data we're interested in is the APs SSID (WiFi name). 
If it's just a `BEACON` we increase the APs beacon count and take note of the frequency the beacon was sent at, so we know what channel the AP is on.
If it's an `ACTION` or `ACTION_NO_ACK` frame, we set the data in the `struct action` in the packet_info struct with the category and code of the action frame. A lot of action frames will come up as `UNKNOWN` in the sniffer - I didn't implement most action frames, mainly since I was unsure of which to add, and a lot of them are vendor specific and I couldn't find solid resources on it.
 with the category and code of the action frame. A lot of action frames will come up as `UNKNOWN` in the sniffer - I didn't implement most action frames, mainly since I was unsure of which to add, and a lot of them are vendor specific and I couldn't find solid resources on it.

Finally, if it's not a beacon packet (Since they'd crowd the packet window) we add the packet to the packet_buffer which gets displayed to the user.

#### channel_hop
This function was a shell script until right at the end of the development, when I decided to put it in the main c program. It uses the `nl80211` netlink interface, sending a generic netlink message to the nl80211 family with the `SET_WIPHY` command. As attributes we send firstly the frequencies we jump to, which are converted from an array of channels I define that the program jumps between using my `channel_to_freq` function. Then it also send the index of the interface where we want to switch the channel, which we get with if_nametoindex (`mon0` since that's our monitoring interface). We wait to receive an ACK from nl80211 before we free the message and return. 

## How to run/build
1. Clone the git repo
2. Run `make` in the project root (builds the kernel module + userspace program)
3. Run `sudo ./run.sh` it will load the kernel module, takes down `wlo1` (That's what my main wifi interface is called, if you want to use the code yourself rename it in the `sniffer_start.sh` and `sniffer_stop.sh` files), brings up a new `mon0` monitor virtual interface and then opens the userspace program
4. When you close the userspace program, it automatically unloads the kernel module and brings your system back to normal
5. Run `make clean` in the project root to clean up all build files

## Possible Improvements
- Limited data available to sniff on my machine - I believe my laptop wificard removes rssi data and a lot of data packets don't show up. I wanted to test with a usb wifi antenna but never got around to getting one.
- Uses my laptops main physical wifi card - would be nice to work with a second one so I don't have to bring down wifi connectivity while the program runs
- Program is missing a lot of detail in the analysis - not really a limitation since this was more an experiement than actually building a professional program.
- The channel_hopping code should probably be in another thread since nl80211 could take some time, however I have limited experience with multithreading in C and didn't feel this was the right time to experiment.

## What I learnt
This project taught me a lot about various topics I've never really touched before. This was my first time learning in depth about the lower parts of networking, and it was really interesting to learn about 802.11 wifi frames and how they're received and parsed/handled in linux. Also this was  my first time writing at a kernel level in linux, and writing a kernel module was very informative to me. What started out as a small project just to get familiar with the 802.11 protocol became so much more, and a project I'm really proud to show off. I plan to possibly extend the sniffer in the future - if I buy a usb wifi receiver maybe I could receive and parse even more data than I can now! It'd be interesting to see the RSSI of the frames for example.

## Resources

#### Generic Netlink Introduction
Archive: *https://web.archive.org/save/https://www.yaroslavps.com/weblog/genl-intro/*

#### 802.11 Frame Types
Archive: *https://web.archive.org/web/20251213173113/https://howiwifi.com/2020/07/13/802-11-frame-types-and-formats/*

#### 802.11 Frame Addressing
Archive: *https://web.archive.org/web/20251213173352/https://www.networkacademy.io/ccna/wireless/802-11-frame-addressing*

#### 802.11 Frame Format
Archive: *https://web.archive.org/web/20251213173231/https://www.networkacademy.io/ccna/wireless/802-11-frame-format*

#### 802.11 Management Action Frames
Archive: *https://web.archive.org/web/20251213173451/https://mrncciew.com/2014/10/09/802-11-mgmt-action-frames/*

#### Libnl Docs
Archive: *https://web.archive.org/web/20251213175021/https://www.infradead.org/~tgr/libnl/doc/core.html#_message_format*


