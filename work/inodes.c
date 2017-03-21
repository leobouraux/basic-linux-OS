#include "inode.h"
#include "sector.h"
#include <string.h>
#include <inttypes.h>

/**
 * @brief read all inodes from disk and print out their content to
 *        stdout according to the assignment
 * @param u the filesystem
 * @return 0 on success; < 0 on error.
 */
int inode_scan_print(const struct unix_filesystem *u){
    uint16_t start = u->s.s_inode_start;
    int i_count = 0;
    uint16_t size = u->s.s_isize;
    for(uint16_t inc = 0;inc < size; ++inc){
        struct inode inodes[SECTOR_SIZE];
        sector_read(u->f, start+inc, inodes);
        for (int i = 0; i < INODES_PER_SECTOR; ++i) {
            struct inode inode = inodes[i];
            if (inode.i_mode & IALLOC){
                printf("inode %d ", ++i_count);
                if (inode.i_mode & IFDIR){
                    printf("(" SHORT_DIR_NAME ")");
                }else{
                    printf("(" SHORT_FIL_NAME ")");
                }
                printf(" len  %d\n", inode_getsize(&inode));
            }
        }
        ++inc;
    }
    return 0;
}
