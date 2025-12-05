obj-m += sniffer.o
#sniffer-objs := sniffer_main.o sniffer_netlink.o
sniffer-y := sniffer_main.o sniffer_netlink.o

all: 
	mkdir -p build
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) MO="$(PWD)/build" modules

clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) MO="$(PWD)/build" clean
	rm -r build/
