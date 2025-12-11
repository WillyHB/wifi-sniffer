build_dir := ./build
src_dir := ./src

srcs := $(wildcard $(src_dir)/*.c)
objs :=  $(srcs:$(src_dir)/%.c=$(build_dir)/%.o)
target := user
CPPFLAGS := -I/usr/include/libnl3
CFLAGS := -g -o0 -Wall -Wextra
LDFLAGS := -lnl-3 -lnl-genl-3 -lncurses

all: $(objs)
	gcc $^ -o $(target) $(LDFLAGS)

$(build_dir)/%.o: $(src_dir)/%.c
	gcc $(CFLAGS) $(CPPFLAGS) -c $^ -o $@ #For each .c file, compile into .o file with .o name

.PHONY: clean
clean:
	rm $(objs)
	rm $(target)
	rm compile_commands.json
