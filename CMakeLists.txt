cmake_minimum_required(VERSION 3.6)
project(pps17-team-olympique-de-marseille)

set(CMAKE_C_STANDARD 99)

set(INODE_FILES work/test-inodes.c work/test-core.c work/error.c work/mount.c work/sector.c work/inode.c work/filev6.c work/bmblock.c)

set(FILE_FILES work/test-file.c work/test-core.c work/error.c work/mount.c work/sector.c work/inode.c work/filev6.c work/bmblock.c work/sha.c)

set(DIRENT_FILES work/test-dirent.c work/test-core.c work/error.c work/mount.c work/sector.c work/inode.c work/filev6.c work/bmblock.c work/direntv6.c work/sha.c)

set(SHELL_FILES work/shell.c work/error.c work/mount.c work/sector.c work/inode.c work/filev6.c work/bmblock.c work/direntv6.c work/sha.c)

set(BITMAP_FILES work/test-bitmap.c work/bmblock.c)

add_executable(test-inodes ${INODE_FILES})

add_executable(test-file ${FILE_FILES})

add_executable(test-dirent ${DIRENT_FILES})

add_executable(shell ${SHELL_FILES})

add_executable(test-bitmap ${BITMAP_FILES})

target_link_libraries(test-dirent crypto)

target_link_libraries(shell crypto)