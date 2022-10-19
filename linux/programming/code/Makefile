

cfiles := $(wildcard *.c)

cobjs := $(patsubst %.c, %.out, $(cfiles))
cflags := -g -Wall

all: $(cobjs)

%.out:%.c 
	gcc $(cflags) $< -o $@

.PHONY: clean

clean:
	rm $(cobjs)
