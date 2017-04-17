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

benchmark:
	phoronix-test-suite benchmark pts/build-linux-kernel-1.7.0
	sudo camflow -a false

clean:
	rm -f service.o
