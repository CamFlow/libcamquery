obj-m+=query.o
INCLUDES = -I./build/SimpleLogger -I./build/inih -I./build/uthash/src
CCFLAGS = -g -O2 -fpic
CCC = gcc
LIB = -lprovenance -lpthread -lz -lrt
.SUFFIXES: .c

all:
	$(CCC) $(INCLUDES) integrity.c $(LIB) -o integrity.o $(CCFLAGS)

install:
	sudo cp --force ./integrity.o /usr/bin/integrityd
	sudo cp --force ./integrityd.service /etc/systemd/system/integrityd.service
	sudo systemctl enable integrityd.service

restart:
	sudo systemctl restart integrityd.service

clean:
	rm *.o
