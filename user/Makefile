srcs := $(wildcard *.c)
build_dir := ./build
objs :=  $(srcs:%.c=$(build_dir)/%.o)
target := user
CPPFLAGS := -I/usr/include/libnl3
LDFLAGS := -lnl-3 -lnl-genl-3

all: $(objs)
	gcc $^ -o $(target) $(LDFLAGS)

$(build_dir)/%.o: %.c
	gcc $(CPPFLAGS) -c $^ -o $@ #For each .c file, compile into .o file with .o name

.PHONY: clean
clean:
	rm $(objs)
	rm $(target)
	rm $(build_dir)/$(wildcard *.o)
	rm compile_commands.json
