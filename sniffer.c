#include "linux/kernel.h"
#include "linux/module.h"

int sniffer_init(void);
void sniffer_clean(void);

int sniffer_init() {
	printk(KERN_INFO "Sniffer loaded!\n");
	return 0;
}

void sniffer_clean() {

}

module_init(sniffer_init);
module_exit(sniffer_clean);

MODULE_AUTHOR("William Hansen-Baird");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A small project experimenting with WiFi packet sniffing :)");
