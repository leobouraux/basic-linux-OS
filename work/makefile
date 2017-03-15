all: test-inodes
error.o: error.c
mount.o: mount.c mount.h unixv6fs.h bmblock.h error.h sector.h
test-core.o: test-core.c mount.h unixv6fs.h bmblock.h error.h
test-inodes.o: test-inodes.c mount.o unixv6fs.h bmblock.h
test-inodes: test-core.o error.o test-inodes.o mount.o