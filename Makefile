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

load_5:
	for number in {1..4}; \
	do \
		echo $$number;\
		cp -r . ../query$$number; \
		cd ../query$$number && mv query.c query$$number.c;\
		cd ../query$$number && sed -i -e "s/obj-m+=query.o/obj-m+=query$$number.o/g" Makefile;\
		cd ../query$$number && sed -i -e "s/sudo modprobe query/sudo modprobe query$$number/g" Makefile;\
		cd ../query$$number && $(MAKE) all;\
		cd ../query$$number && $(MAKE) load;\
		cd ../camflow-query;\
	done
	sudo modprobe query

load_10:
	for number in {1..9}; \
	do \
		echo $$number;\
		cp -r . ../query$$number; \
		cd ../query$$number && mv query.c query$$number.c;\
		cd ../query$$number && sed -i -e "s/obj-m+=query.o/obj-m+=query$$number.o/g" Makefile;\
		cd ../query$$number && sed -i -e "s/sudo modprobe query/sudo modprobe query$$number/g" Makefile;\
		cd ../query$$number && $(MAKE) all;\
		cd ../query$$number && $(MAKE) load;\
		cd ../camflow-query;\
	done
	sudo modprobe query

load_100:
	for number in {1..99}; \
	do \
		echo $$number;\
		cp -r . ../query$$number; \
		cd ../query$$number && mv query.c query$$number.c;\
		cd ../query$$number && sed -i -e "s/obj-m+=query.o/obj-m+=query$$number.o/g" Makefile;\
		cd ../query$$number && sed -i -e "s/sudo modprobe query/sudo modprobe query$$number/g" Makefile;\
		cd ../query$$number && $(MAKE) all;\
		cd ../query$$number && $(MAKE) load;\
		cd ../camflow-query;\
	done
	sudo modprobe query

load_200:
	for number in {1..199}; \
	do \
		echo $$number;\
		cp -r . ../query$$number; \
		cd ../query$$number && mv query.c query$$number.c;\
		cd ../query$$number && sed -i -e "s/obj-m+=query.o/obj-m+=query$$number.o/g" Makefile;\
		cd ../query$$number && sed -i -e "s/sudo modprobe query/sudo modprobe query$$number/g" Makefile;\
		cd ../query$$number && $(MAKE) all;\
		cd ../query$$number && $(MAKE) load;\
		cd ../camflow-query;\
	done
	sudo modprobe query

unload:
	sudo rmmod query
