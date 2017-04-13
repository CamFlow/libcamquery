INCLUDES = -I../build/SimpleLogger -I../build/inih -I../../build/uthash/src
CCFLAGS = -g -O2 -fpic
CCC = gcc
LIB = /usr/local/lib/provenancelib.a -lpthread -lz -lrt
.SUFFIXES: .c

all:
	$(CCC) $(INCLUDES) service.c $(LIB) -o service.o $(CCFLAGS)

debug:
	$(CCC) $(INCLUDES) service.c $(LIB) -o service.o $(CCFLAGS) -g

install:
	cp --force ./service.o /usr/bin/camflow-provenance
	mkdir -p /var/relay
	cp --force ./camflow-provenance.service /etc/systemd/system/camflow-provenance.service
	systemctl enable camflow-provenance.service

restart:
	systemctl restart camflow-provenance.service

analyze:
	sudo camflow --track-file Makefile propagate
	./script.sh > rst.txt &

benchmark:
	phoronix-test-suite benchmark pts/build-linux-kernel-1.7.0

clean:
	rm -f service.o
