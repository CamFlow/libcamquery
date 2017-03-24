INCLUDES = -I../build/SimpleLogger -I../build/inih
CCFLAGS = -g -O2 -fpic
CCC = gcc
LIB = /usr/local/lib/provenancelib.a -lpthread -lz
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

clean:
	rm -f service.o
