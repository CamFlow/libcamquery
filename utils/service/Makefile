INCLUDES = -I../build/SimpleLogger -I../build/inih -I../../build/uthash/src
CCFLAGS = -g -O2 -fpic
CCC = gcc
LIB = -lprovenance -lpthread -lz -lrt
.SUFFIXES: .c

all:
	$(CCC) $(INCLUDES) query.c $(LIB) -o service.o $(CCFLAGS)

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
	sudo camflow -a true

clean:
	rm -f service.o
