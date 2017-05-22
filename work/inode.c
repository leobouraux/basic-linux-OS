#include "inode.h"
#include "sector.h"
#include "error.h"
#include <inttypes.h>
#include <stdlib.h>


int inode_scan_print(const struct unix_filesystem *u) {
    M_REQUIRE_NON_NULL(u);

    uint16_t start = u->s.s_inode_start;
    int i_count = 0;
    uint16_t size = u->s.s_isize;

    //read all the inode sectors
    for (uint16_t inc = 0; inc < size; ++inc) {
        struct inode inodes[SECTOR_SIZE];
        int j = sector_read(u->f, (uint32_t) (start + inc), inodes);
        if (j == ERR_BAD_PARAMETER || j == ERR_IO) {
            return j;
        }
        //print each inode's specs
        for (unsigned int i = 0; i < INODES_PER_SECTOR; ++i) {
            struct inode inod = inodes[i];
            if (inod.i_mode & IALLOC) {
                printf("inode %3d ", ++i_count);
                if (inod.i_mode & IFDIR) {
                    printf("(" SHORT_DIR_NAME ")");
                } else {
                    printf("(" SHORT_FIL_NAME ")");
                }
                printf(" len %4d\n", inode_getsize(&inod));
            }
        }
    }
    return 0;
}


void inode_print(const struct inode *inode) {
    printf("**********FS INODE START**********\n");
    if (inode == NULL) {
        printf("NULL ptr");
    } else {
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


int inode_read(const struct unix_filesystem *u, uint16_t inr, struct inode *inode) {
    M_REQUIRE_NON_NULL(u);
    M_REQUIRE_NON_NULL(inode);

    //return if inr bigger than total inr
    long unsigned int size = u->s.s_isize * INODES_PER_SECTOR;
    if (inr > size) {
        return ERR_INODE_OUTOF_RANGE;
    }
    //read the corresponding sector to inr
    uint16_t start = u->s.s_inode_start;
    uint16_t block_offset = inr / INODES_PER_SECTOR;
    struct inode inodes[SECTOR_SIZE];
    int err = sector_read(u->f, (uint32_t) (start + block_offset), inodes);
    //instanciate the inode from the array
    *inode = inodes[inr % INODES_PER_SECTOR];
    printf("f_inode_read1 ");
    if (!(inode->i_mode & IALLOC)) {
        return ERR_UNALLOCATED_INODE;
    }
    return err;
}


int inode_findsector(const struct unix_filesystem *u, const struct inode *i, int32_t file_sec_off) {
    M_REQUIRE_NON_NULL(u);
    M_REQUIRE_NON_NULL(i);

    //handle errors
    int relativeSize = (inode_getsize(i) - 1) / SECTOR_SIZE + 1;
    if (file_sec_off >= relativeSize) {
        return ERR_OFFSET_OUT_OF_RANGE;
    }
    if (!(i->i_mode & IALLOC)) {
        return ERR_UNALLOCATED_INODE;
    }
    if (relativeSize > (ADDR_SMALL_LENGTH - 1) * ADDRESSES_PER_SECTOR) {
        return ERR_FILE_TOO_LARGE;
    }

    //find sector
    if (relativeSize <= ADDR_SMALL_LENGTH) {
        return i->i_addr[file_sec_off];
    } else {
        //big file, need second level addressing
        uint16_t data[SECTOR_SIZE];
        int err = sector_read(u->f, i->i_addr[file_sec_off / (ADDRESSES_PER_SECTOR)], data);
        if (err < 0) {
            return err;
        }
        return data[file_sec_off % ADDRESSES_PER_SECTOR];
    }
}

int inode_write(struct unix_filesystem *u, uint16_t inr, struct inode *inode) {
    M_REQUIRE_NON_NULL(u);
    M_REQUIRE_NON_NULL(inode);

    uint16_t start = u->s.s_inode_start;
    uint16_t block_offset = inr / INODES_PER_SECTOR;
    struct inode inodes[SECTOR_SIZE];
    int err = sector_read(u->f, (uint32_t) (start + block_offset), inodes);
    if(err < 0){
        return err;
    }
    inodes[inr % INODES_PER_SECTOR] = *inode;
    return sector_write(u->f, (uint32_t) (start + block_offset), inodes);
}

int inode_alloc(struct unix_filesystem *u){
    int inr = bm_find_next(u->ibm);
    if(inr < 0){
        return ERR_NOMEM;
    }
    bm_set(u->ibm, (uint64_t)inr);
    return inr;
}

int inode_setsize(struct inode *inode, int new_size) {
    if (new_size < 0) {
        return ERR_NOMEM;
    }
    inode->i_size0 = (uint8_t)(new_size >> 16);
    inode->i_size1 = (uint16_t)(new_size & 0xFFFF);
    return 0;
}