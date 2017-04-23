#include "inode.h"
#include "sector.h"
#include "error.h"
#include <inttypes.h>
#include <stdlib.h>

/**
 * week05 
 * 
 * @brief read all inodes from disk and print out their content to
 *        stdout according to the assignment
 * @param u the filesystem
 * @return 0 on success; < 0 on error.
 */
int inode_scan_print(const struct unix_filesystem *u){
    M_REQUIRE_NON_NULL(u);

    uint16_t start = u->s.s_inode_start;
    int i_count = 0;
    uint16_t size = u->s.s_isize;

    //read all the inode sectors
    for(uint16_t inc = 0; inc < size; ++inc){
        struct inode inodes[SECTOR_SIZE]; 
        int j = sector_read(u->f, (uint32_t)(start+inc), inodes);
        if(j == ERR_BAD_PARAMETER || j == ERR_IO){
            return j;
        }
        //print each inode's specs
        for (unsigned int i = 0; i < INODES_PER_SECTOR; ++i) {
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


/**
 * week05
 * 
 * @brief prints the content of an inode structure
 * @param inode the inode structure to be displayed
 */
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


/**
 * week05
 * 
 * @brief read the content of an inode from disk
 * @param u the filesystem (IN)
 * @param inr the inode number of the inode to read (IN)
 * @param inode the inode structure, read from disk (OUT)
 * @return 0 on success; <0 on error
 */
int inode_read(const struct unix_filesystem *u, uint16_t inr, struct inode *inode){
    M_REQUIRE_NON_NULL(u);
    M_REQUIRE_NON_NULL(inode);

    //return if inr bigger than total inr
    long unsigned int size = u->s.s_isize*INODES_PER_SECTOR;
    if(inr > size) {
        return ERR_INODE_OUTOF_RANGE;
    }

    //read the corresponding sector to inr
    uint16_t start = u->s.s_inode_start;
    uint16_t block_offset = inr / INODES_PER_SECTOR;
    struct inode inodes[SECTOR_SIZE];
    int err = sector_read(u->f,  (uint32_t)(start+block_offset), inodes);

    //instanciate the inode from the array
    *inode = inodes[inr % INODES_PER_SECTOR];
    if (! (inode->i_mode & IALLOC)){
        return ERR_UNALLOCATED_INODE;
    }
    return err;
}


/**
 * week05
 * 
 * @brief identify the sector that corresponds to a given portion of a file
 * @param u the filesystem (IN)
 * @param inode the inode (IN)
 * @param file_sec_off the offset within the file (in sector-size units)
 * @return >0: the sector on disk;  0: unallocated;  <0 error
 */
int inode_findsector(const struct unix_filesystem *u, const struct inode *i, int32_t file_sec_off){
    M_REQUIRE_NON_NULL(u);
    M_REQUIRE_NON_NULL(i);

    //handle errors
    int relativeSize = (inode_getsize(i)-1)/SECTOR_SIZE+1;
    if(file_sec_off >=relativeSize){
        return ERR_OFFSET_OUT_OF_RANGE;
    }
    if (! (i->i_mode & IALLOC)){
        return ERR_UNALLOCATED_INODE;
    }
    if(relativeSize > (ADDR_SMALL_LENGTH-1)*ADDRESSES_PER_SECTOR){
        return ERR_FILE_TOO_LARGE;
    }

    //find sector
    if(relativeSize <=ADDR_SMALL_LENGTH){
        return i->i_addr[file_sec_off];
    }else {
        //big file, need second level addressing
        uint16_t data[SECTOR_SIZE];
        sector_read(u->f, i->i_addr[file_sec_off / (ADDRESSES_PER_SECTOR)], data);
        return data[file_sec_off % ADDRESSES_PER_SECTOR];
    }
}
