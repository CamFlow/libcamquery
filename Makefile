obj-m+=query.o
INCLUDES = -I./build/SimpleLogger -I./build/inih -I./build/uthash/src
CCFLAGS = -g -O2 -fpic
CCC = gcc
LIB = -lprovenance -lpthread -lz -lrt
.SUFFIXES: .c

build_kernel:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	sudo $(MAKE) -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules_install

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean

load:
	sudo modprobe query

unload:
	sudo rmmod query

install:
	cd ./include && sudo $(MAKE) install
