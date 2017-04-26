CFLAGS += -Wall -pedantic -g -std=c99 -Wextra -Wfloat-equal -Wshadow -Wpointer-arith -Wbad-function-cast -Wcast-qual -Wcast-align -Wwrite-strings -Wconversion -Wunreachable-code
LDLIBS += -lm -lcrypto
CC = gcc

all: test-inodes test-file test-dirent shell fs test-bitmap
test-inodes: test-core.o error.o test-inodes.o mount.o sector.o inode.o filev6.o

test-file: test-core.o error.o test-file.o mount.o sector.o inode.o filev6.o sha.o

test-dirent: test-core.o test-dirent.o error.o mount.o sector.o inode.o sha.o filev6.o direntv6.o

shell: shell.o filev6.o direntv6.o error.o sector.o inode.o sha.o mount.o

test-bitmap: test-bitmap.o bmblock.o

clean:
	rm *.o

fs.o: fs.c
	$(COMPILE.c) -D_DEFAULT_SOURCE $$(pkg-config fuse --cflags) -o $@ -c $<

fs: fs.o mount.o sector.o inode.o filev6.o direntv6.o error.o
	$(LINK.c) -o $@ $^ $(LDLIBS) $$(pkg-config fuse --libs)
