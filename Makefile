obj-m+=query.o

prepare:
	mkdir -p build
	cd ./build && git clone https://github.com/troydhanson/uthash.git

all:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	sudo $(MAKE) -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules_install

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean

load:
	sudo modprobe query

unload:
	sudo rmmod query
