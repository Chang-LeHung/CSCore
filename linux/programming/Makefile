

cfiles := $(wildcard *.c)

cobjs := $(patsubst %.c, %.out, $(cfiles))
cflags := -g -Wall

all: $(cobjs)


.PHONY: clean

clean:
	rm $(cobjs)

%.out:%.c 
	gcc $(cflags) $< -o $@
