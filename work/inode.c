#include "inode.h"
#include "sector.h"
#include "error.h"
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
            struct inode inod = inodes[i];
            if (inod.i_mode & IALLOC){
                printf("inode %3d ", ++i_count);
                if (inod.i_mode & IFDIR){
                    printf("(" SHORT_DIR_NAME ")");
                }else{
                    printf("(" SHORT_FIL_NAME ")");
                }
                printf(" len %4d\n", inode_getsize(&inod));
            }
        }
    }
    return 0;
}

void inode_print(const struct inode *inode){
    printf("**********FS INODE START**********\n");
    if(inode == NULL){
        printf("NULL ptr");
    }else{
        printf("i_mode: %" PRIu16"\n", inode->i_mode);
        printf("i_nlink: %" PRIu8"\n", inode->i_nlink);
        printf("i_uid: %" PRIu8"\n", inode->i_uid);
        printf("i_gid: %" PRIu8"\n", inode->i_gid);
        printf("i_size0: %" PRIu8"\n", inode->i_size0);
        printf("i_size1: %" PRIu16"\n", inode->i_size1);
        printf("size: %" PRIu16"\n", inode_getsize(inode));
    }
    printf("**********FS INODE END**********\n");
}

int inode_read(const struct unix_filesystem *u, uint16_t inr, struct inode *inode){
    if(inr < 0 || inr > u->s.s_isize) {
        return ERR_INODE_OUTOF_RANGE;
    }
    uint16_t start = u->s.s_inode_start;
    uint16_t size = u->s.s_isize;
    uint16_t block_offset = inr / INODES_PER_SECTOR;
    struct inode inodes[SECTOR_SIZE];
    int err = 0;
    err = sector_read(u->f, start+block_offset, inodes);
    *inode = inodes[inr % INODES_PER_SECTOR];
    if (! (inode->i_mode & IALLOC)){
        return ERR_UNALLOCATED_INODE;
    }
    return err;
}

int inode_findsector(const struct unix_filesystem *u, const struct inode *i, int32_t file_sec_off){
    if(file_sec_off == NULL){
        file_sec_off = 0;
    }
    if(file_sec_off < 0 || file_sec_off > inode_getsize(i)){
        return ERR_OFFSET_OUT_OF_RANGE;
    }
    if (! (i->i_mode & IALLOC)){
        return ERR_UNALLOCATED_INODE;
    }
    if(inode_getsize(i) > 7 * ADDRESSES_PER_SECTOR * SECTOR_SIZE){
        return ERR_FILE_TOO_LARGE;
    }
    if(inode_getsize(i) <= 8 * SECTOR_SIZE){
        return i->i_addr[file_sec_off];
    }else{
        uint16_t data[SECTOR_SIZE];
        sector_read(u->f, i->i_addr[file_sec_off / ADDRESSES_PER_SECTOR], data);
        return data[file_sec_off % ADDRESSES_PER_SECTOR];
    }
}




