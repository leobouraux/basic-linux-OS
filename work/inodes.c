#include "inode.h"
#include "sector.h"
#include <string.h>

/**
 * @brief read all inodes from disk and print out their content to
 *        stdout according to the assignment
 * @param u the filesystem
 * @return 0 on success; < 0 on error.
 */
int inode_scan_print(const struct unix_filesystem *u){
    uint16_t start = u->s.s_inode_start;
    uint16_t inc = 0;
    uint16_t size = u->s.s_isize;
    struct inode inode;
    memset(&inode, 0, sizeof(&inode));
    while (inc < size){
        sector_read(u->f, start+inc, &inode);
        if (inode.i_mode & IALLOC){
            printf("inode %d", inc);
            if (inode.i_mode & IFDIR){
                printf("(" SHORT_DIR_NAME ")");
            }else{
                printf("(" SHORT_FIL_NAME ")");
            }
            printf("len %d\n", inode_getsize(&inode));
        }
        ++inc;
    }
}
