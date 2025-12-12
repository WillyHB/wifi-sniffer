obj-m += sniffer.o
BUILDDIR := build
#sniffer-objs := sniffer_main.o sniffer_netlink.o
sniffer-y := sniffer_main.o sniffer_netlink.o

all: 
	mkdir -p $(BUILDDIR)
	make -C /lib/modules/$(shell uname -r)/build/ M=$(CURDIR) MO="$(CURDIR)/$(BUILDDIR)" modules

clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(CURDIR) MO="$(CURDIR)/$(BUILDDIR)" clean
	rm -fr build/
