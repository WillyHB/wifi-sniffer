obj-m += sniffer.o
BUILD_DIR := build
#sniffer-objs := sniffer_main.o sniffer_netlink.o
sniffer-y := sniffer_main.o sniffer_netlink.o

all: 
	mkdir -p $(BUILD_DIR)
	make -C /lib/modules/$(shell uname -r)/build/ M=$(CURDIR) MO="$(CURDI)/build" modules

clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(CURDIR) MO="$(CURDIR)/build" clean
	rm -r build/
