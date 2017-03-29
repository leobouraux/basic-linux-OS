#include "mount.h"
#include "inode.h"
#include "filev6.h"
#include <stdio.h>
#include <string.h>
#include "sha.h"
#include "error.h"
#include "sector.h"

void helper(struct unix_filesystem *u, struct filev6 *fs, int i){
    int err = filev6_open(u, i, fs);
    if (err < 0) {
        printf("filev6_open failed for inode #%d\n", i);
    } else {
        printf("Printing inode #%d:\n", i);
        inode_print(&fs->i_node);
        if (fs->i_node.i_mode & IFDIR) {
            printf("It's a directory");
        } else {
            printf("The first sector of data of which contains :\n");
        }
        char b[SECTOR_SIZE + 1];
        filev6_readblock(fs, &b);
        b[SECTOR_SIZE] = '\0';
        printf("%s", b);
    }
}

int test(struct unix_filesystem *u) {
    struct filev6 fs;
    memset(&fs, 255, sizeof(fs));
    helper(u, &fs, 3);
    memset(&fs, 255, sizeof(fs));
    helper(u, &fs, 5);

    printf("Listing inodes SHA:\n");
    int i_count = 0;
    uint16_t size = u->s.s_isize;
    for (uint16_t inc = 0; inc < size; ++inc) {
        struct inode inodes[SECTOR_SIZE];
        int j = sector_read(u->f, u->s.s_inode_start + inc, inodes);
        if (j == ERR_BAD_PARAMETER || j == ERR_IO) {
            return j;
        }
        for (int i = 0; i < INODES_PER_SECTOR; ++i) {
            struct inode inod = inodes[i];
            print_sha_inode(u, inod, i_count++);
        }
    }
    return 0;
}
