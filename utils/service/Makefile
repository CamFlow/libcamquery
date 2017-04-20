INCLUDES = -I../build/SimpleLogger -I../build/inih -I../../build/uthash/src
CCFLAGS = -g -O2 -fpic
CCC = gcc
LIB = /usr/local/lib/provenancelib.a -lpthread -lz -lrt
.SUFFIXES: .c

all:
	$(CCC) $(INCLUDES) query.c $(LIB) -o service.o $(CCFLAGS)
	$(CCC) $(INCLUDES) test.c $(LIB) -o test.o $(CCFLAGS)

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

prepare_single:
	cd ../../../camflow-install/build/camflow-patches/v0.3.0/build/linux-4.10.10/security && touch security.c

prepare_ten:
	cd ../../../camflow-install/build/camflow-patches/v0.3.0/build/linux-4.10.10/security && touch security.c && touch commoncap.c && touch device_cgroup.c && touch inode.c && touch lsm_audit.c && touch min_addr.c && cd ../lib/ && touch argv_split.c && touch atomic64.c && touch audit.c && touch bcd.c

prepare_hundred:
	cd ../../../camflow-install/build/camflow-patches/v0.3.0/build/linux-4.10.10/security && touch security.c && touch commoncap.c && touch device_cgroup.c && touch inode.c && touch lsm_audit.c && touch min_addr.c && cd ../lib/ && touch argv_split.c && touch atomic64.c && touch audit.c && touch bcd.c && touch bch.c && touch bitmap.c && touch bitrev.c && touch bsearch.c && touch btree.c && touch bug.c && touch bust_spinlocks.c && touch chacha20.c && touch check_signature.c && touch checksum.c && touch clz_ctz.c && touch clz_tab.c && touch cmdline.c && touch compat_audit.c && touch cordic.c && touch cpumask.c && touch cpu_rmap.c && touch crc16.c && touch crc32.c && touch crc7.c && touch crc8.c && touch crc-ccitt.c && touch crc-itu-t.c && touch crc-t10dif.c && touch ctype.c && touch debug_info.c && touch debug_locks.c && touch debugobjects.c && touch dec_and_lock.c && touch decompress_bunzip2.c && touch kobject.c && touch kobject_uevent.c && touch kstrtox.c && touch lcm.c && touch libcrc32c.c && touch libcrc32c.mod.c && touch list_debug.c && touch list_sort.c && touch llist.c && touch locking-selftest.c && cd ../fs/ && touch no-block.c && touch userfaultfd.c && touch utimes.c && touch coredump.c && touch char_dev.c && touch filesystems.c && touch namei.c && touch mpage.c && touch splice.c && touch binfmt_elf.c && touch libfs.c && touch readdir.c && touch direct-io.c && touch anon_inodes.c && touch binfmt_elf.c && cd ../firmware/ && touch ihex2fw.c && cd ../crypto/ && touch testmgr.c && touch xcbc.c && touch memneq.c && touch crc32c_generic.c && touch shash.c && touch cast6_generic.c && touch sha256_generic.c && touch blkcipher.c && touch seed.c && touch fips.c && touch salsa20_generic.c && touch ansi_cprng.c && touch rsaprivkey-asn1.c && touch algif_aead.c && cd ../net/ && touch socket.c && touch sysctl_net.c && touch compat.c && cd ../mm/ && touch slab_common.c && touch zswap.c && touch interval_tree.c && touch process_vm_access.c && touch highmem.c && touch vmalloc.c && touch memory_hotplug.c && touch page_idle.c && touch page_counter.c && touch debug.c && touch oom_kill.c && touch cma.c && touch slub.c && touch mprotect.c && touch khugepaged.c && touch backing-dev.c && touch mmzone.c

run_build:
	cd ../../../camflow-install/build/camflow-patches/v0.3.0/build/linux-4.10.10 && make -j4
	sudo camflow -a false

clean:
	rm -f service.o
