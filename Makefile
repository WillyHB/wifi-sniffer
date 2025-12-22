SUBDIRS := kern user

.PHONY: all clean

all:
	for d in $(SUBDIRS); \
		do $(MAKE) -C $$d; \
	done
clean:
	rm compile_commands.json
	for d in $(SUBDIRS); \
		do $(MAKE) -C $$d clean; \
	done
