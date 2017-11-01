obj-m+=query.o
INCLUDES = -I./build/SimpleLogger -I./build/inih -I./build/uthash/src
CCFLAGS = -g -O2 -fpic
CCC = gcc
LIB = -lprovenance -lpthread -lz -lrt
.SUFFIXES: .c

all:
	$(CCC) $(INCLUDES) unicorn.c $(LIB) -o unicorn.o $(CCFLAGS)

install:
	sudo cp --force ./unicorn.o /usr/bin/unicornd
	sudo cp --force ./unicornd.service /etc/systemd/system/unicornd.service
	sudo systemctl enable unicornd.service

clean:
	rm *.o
