

cfiles := $(wildcard *.c)

cobjs := $(patsubst %.c, %.out, $(cfiles))
cflags := -g -Wall

all: $(cobjs)
	echo $(cobjs)

.PHONY: clean

clean:
	rm $(cobjs)

%.out:%.c 
	gcc $(cflags) $< -o $@
