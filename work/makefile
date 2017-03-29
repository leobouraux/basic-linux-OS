all: test-inodes test-file
error.o: error.c error.h
mount.o: mount.c mount.h unixv6fs.h bmblock.h error.h sector.h
sector.o: sector.c mount.h unixv6fs.h bmblock.h error.h sector.h
test-core.o: test-core.c mount.h unixv6fs.h bmblock.h error.h
test-inodes.o: test-inodes.c mount.o unixv6fs.h bmblock.h inode.h
inode.o: inode.c inode.h unixv6fs.h mount.h bmblock.h sector.h sha.h
filev6.o: filev6.c filev6.h unixv6fs.h mount.h bmblock.h inode.h sector.h error.h
sha.o: sha.c error.h mount.h unixv6fs.h bmblock.h filev6.h
test-file.o: test-file.c mount.h unixv6fs.h bmblock.h inode.h filev6.h sha.h
test-inodes: test-core.o error.o test-inodes.o mount.o sector.o inode.o
test-file: test-core.o error.o test-file.o mount.o sector.o inode.o filev6.o sha.o
	gcc -std=c99 -o test-file test-core.o error.o test-file.o mount.o sector.o inode.o filev6.o sha.o -lcrypto