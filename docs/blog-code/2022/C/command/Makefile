
 

cfiles = $(wildcard *.c)
cppfiles = $(wildcard *.cpp)
cobjs = $(patsubst %.c, %.out, $(cfiles))
cppobjs = $(patsubst %.cpp, %.out, $(cppfiles))
CC=gcc
CXX=g++
Cflags=-g
main: $(cobjs) $(cppobjs)

%.out:%.c
	$(CC) $(Cflags) $< -o $@

%.out:%.cpp
	$(CXX) $(Cflags) $< -o $@

.PHONY:clean

clean:
	rm $(cobjs) $(cppobjs)

