CFLAGS += -Wall -pedantic -g -std=c99 -Wextra -Wfloat-equal -Wshadow -Wpointer-arith -Wbad-function-cast -Wcast-qual -Wcast-align -Wwrite-strings -Wconversion -Wunreachable-code
LDLIBS += -lm -lcrypto
CC = gcc

all: test-inodes test-file test-dirent
test-inodes: test-core.o error.o test-inodes.o mount.o sector.o inode.o filev6.o

test-file: test-core.o error.o test-file.o mount.o sector.o inode.o filev6.o sha.o

test-dirent: test-core.o test-dirent.o error.o mount.o sector.o inode.o sha.o filev6.o direntv6.o

clean:
	rm *.o
