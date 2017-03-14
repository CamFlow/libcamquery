obj-m+=query.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean

load:
	sudo insmod query.ko

unload:
	sudo rmmod query.ko
