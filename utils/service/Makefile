INCLUDES = -I../build/SimpleLogger -I../build/inih -I../../build/uthash/src
CCFLAGS = -g -O2 -fpic
CCC = gcc
LIB = /usr/local/lib/provenancelib.a -lpthread -lz -lrt
.SUFFIXES: .c

all:
	$(CCC) $(INCLUDES) query.c $(LIB) -o service.o $(CCFLAGS)

debug:
	$(CCC) $(INCLUDES) query.c $(LIB) -o service.o $(CCFLAGS) -g

install:
	echo $(uname -r)
	sudo cp --force ./service.o /usr/bin/camflow-provenance
	sudo mkdir -p /var/relay
	sudo cp --force ./camflow-provenance.service /etc/systemd/system/camflow-provenance.service
	sudo systemctl enable camflow-provenance.service

restart:
	sudo systemctl restart camflow-provenance.service

analyze:
	./script.sh > rst.txt &
	#sudo camflow --track-file Makefile propagate
	sudo camflow -a true

run_kernel:
	phoronix-test-suite benchmark pts/build-linux-kernel-1.7.0
	sudo camflow -a false

run_unpack:
	phoronix-test-suite benchmark pts/unpack-linux-1.0.0
	sudo camflow -a false

prepare_postmark:
	@echo "Preparing postmark"
	mkdir -p build/postmark
	cp -f src/postmark-1_5.c build/postmark/postmark-1_5.c
	cd build/postmark && cc -o postmark postmark-1_5.c
	@echo "Please ignore warning"

run_postmark:
	@echo "set size 4096 102400"
	@echo "set subdirectories 10"
	@echo "set number 4500"
	@echo "set transactions 1500000"
	cd build/postmark && ./postmark
	sudo camflow -a false

clean:
	rm -f service.o
