obj-m+=query.o
INCLUDES = -I./build/SimpleLogger -I./build/inih -I./build/uthash/src
CCFLAGS = -g -O2 -fpic
CCC = gcc
LIB = -lprovenance -lpthread -lz -lrt
.SUFFIXES: .c

prepare:
	mkdir -p build
	cd ./build && git clone https://github.com/troydhanson/uthash.git

build_kernel:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	sudo $(MAKE) -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules_install

build_service:
	$(CCC) $(INCLUDES) query.c $(LIB) -o service.o $(CCFLAGS)

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean

load:
	sudo modprobe query

unload:
	sudo rmmod query

install_uthash:
	cd ./build/uthash/include && sudo cp --force ./utarray.h /usr/include/utarray.h
	cd ./build/uthash/include && sudo cp --force ./uthash.h /usr/include/uthash.h
	cd ./build/uthash/include && sudo cp --force ./utlist.h /usr/include/utlist.h
	cd ./build/uthash/include && sudo cp --force ./utringbuffer.h /usr/include/utringbuffer.h
	cd ./build/uthash/include && sudo cp --force ./utstring.h /usr/include/utstring.h

install: install_uthash
	cd ./include && sudo $(MAKE) install
