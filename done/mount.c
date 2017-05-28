#include <string.h>
#include "mount.h"
#include "error.h"
#include "sector.h"
#include <inttypes.h>
#include "inode.h"

void fill_fbm(struct unix_filesystem *u){
    uint16_t start = u->s.s_inode_start;
    uint16_t size = u->s.s_isize;

    //read all the inode sectors
    for(uint16_t inc = 0; inc < size; ++inc){
        struct inode inodes[SECTOR_SIZE];
        int j = sector_read(u->f, (uint32_t)(start+inc), inodes);
        if(j != ERR_BAD_PARAMETER && j != ERR_IO){
            for (unsigned int i = 0; i < INODES_PER_SECTOR; ++i) {
                struct inode inod = inodes[i];
                if (inod.i_mode & IALLOC){
                    int sector = 0;
                    int32_t offset = 0;
                    int32_t fsize = inode_getsize(&inod);
                    //handle redirections when file too big
                    if(fsize > ADDR_SMALL_LENGTH*SECTOR_SIZE){
                        for (int k = 0; k <= (fsize/SECTOR_SIZE)/ADDRESSES_PER_SECTOR; ++k) {
                            bm_set(u->fbm, inod.i_addr[k]);
                        }
                    }
                    //find sectors for current inode
                    while (fsize > offset){
                        sector = inode_findsector(u, &inod, offset/SECTOR_SIZE);
                        if(sector > 0){
                            offset += SECTOR_SIZE;
                            bm_set(u->fbm, (uint64_t)sector);
                        }else{
                            offset = fsize;
                        }
                    }
                }
            }
        }
    }
}

void fill_ibm(struct unix_filesystem *u){
    uint16_t start = u->s.s_inode_start;
    uint16_t size = u->s.s_isize;

    //read all the inode sectors
    for(uint16_t inc = 0; inc < size; ++inc){
        struct inode inodes[SECTOR_SIZE];
        int j = sector_read(u->f, (uint32_t)(start+inc), inodes);
        if(j == ERR_BAD_PARAMETER || j == ERR_IO){
           for (unsigned int i = 0; i < INODES_PER_SECTOR; ++i) {
               bm_set(u->ibm, i+inc*INODES_PER_SECTOR);
            }
        }else{
            for (unsigned int i = 0; i < INODES_PER_SECTOR; ++i) {
                struct inode inod = inodes[i];
                if (inod.i_mode & IALLOC){
                    bm_set(u->ibm, i+inc*INODES_PER_SECTOR);
                }
            }
        }
    }
}

int mountv6(const char *filename, struct unix_filesystem *u){
    M_REQUIRE_NON_NULL(filename);
    M_REQUIRE_NON_NULL(u);

    //initiate the filesystem
    memset(u, 0, sizeof(*u));
    u->fbm = NULL;
    u->ibm = NULL;
    u->f = fopen(filename, "r+b");
    if(u->f == NULL){
        return ERR_IO;
    }

    //read boot block to check consistence of disk
    uint8_t content[SECTOR_SIZE];
    int err = sector_read(u->f, BOOTBLOCK_SECTOR, content);
    if(err == ERR_IO || err == ERR_BAD_PARAMETER){
        return err;
    }
    if(content[BOOTBLOCK_MAGIC_NUM_OFFSET] != BOOTBLOCK_MAGIC_NUM){
        return ERR_BADBOOTSECTOR;
    }
    //read super block
    err = sector_read(u->f, SUPERBLOCK_SECTOR, &u->s);
    if(err >= 0){
        u->ibm = bm_alloc(u->s.s_inode_start, (uint64_t)(u->s.s_isize * INODES_PER_SECTOR));
        u->s.s_ibmsize = (uint16_t)u->ibm->length;
        fill_ibm(u);
        u->fbm = bm_alloc((uint16_t)(u->s.s_block_start+1), (uint64_t)(u->s.s_fsize));
        u->s.s_fbmsize = (uint16_t)u->fbm->length;
        fill_fbm(u);
    }
    return err;
}

int umountv6(struct unix_filesystem *u){
    M_REQUIRE_NON_NULL(u);
    
    int j = 0;
    j = fclose(u->f);
    if(j == EOF){
        return ERR_IO;
    }
    return 0;
}

void mountv6_print_superblock(const struct unix_filesystem *u){	
    printf("**********FS SUPERBLOCK START**********\n");
    printf("s_isize       : %" PRIu16"\n", u->s.s_isize);
    printf("s_fsize       : %" PRIu16"\n", u->s.s_fsize);
    printf("s_fbmsize     : %" PRIu16"\n", u->s.s_fbmsize);
    printf("s_ibmsize     : %" PRIu16"\n", u->s.s_ibmsize);
    printf("s_inode_start : %" PRIu16"\n", u->s.s_inode_start);
    printf("s_block_start : %" PRIu16"\n", u->s.s_block_start);
    printf("s_fbm_start   : %" PRIu16"\n", u->s.s_fbm_start);
    printf("s_ibm_start   : %" PRIu8"\n", u->s.s_ibm_start);
    printf("s_flock       : %" PRIu8"\n", u->s.s_flock);
    printf("s_ilock       : %" PRIu8"\n", u->s.s_ilock);
    printf("s_fmod        : %" PRIu8"\n", u->s.s_fmod);
    printf("s_ronly       : %" PRIu16"\n", u->s.s_ronly);
    printf("s_time        : [%" PRIu16"] %" PRIu16"\n", u->s.s_time[0], u->s.s_time[1]);
    printf("**********FS SUPERBLOCK END************\n");
}


int mountv6_mkfs(const char *filename, uint16_t num_blocks, uint16_t num_inodes) {//struct unix_filesystem *u en param ?
    M_REQUIRE_NON_NULL(filename);

    //1.
    //comment representer un super block qui est un secteur different des secteurs avec inode
    struct superblock spb = {0};
    spb.s_isize =  num_inodes % INODES_PER_SECTOR == 0 ? num_inodes / INODES_PER_SECTOR : num_inodes / INODES_PER_SECTOR + 1;
    spb.s_fsize = num_blocks;
    if(spb.s_fsize < spb.s_isize)
        return ERR_NOT_ENOUGH_BLOCS;
    spb.s_inode_start = SUPERBLOCK_SECTOR+1;  //les blocks de bitmaps ne sont pas stockés sur le disk mais calculé au démarage
    spb.s_block_start = spb.s_inode_start + spb.s_isize;

    //create a binary file
    FILE* f = fopen(filename, "wb");

    //create abd write the bootblock sector
    uint8_t bootblock[SECTOR_SIZE];
    bootblock[BOOTBLOCK_MAGIC_NUM_OFFSET] = BOOTBLOCK_MAGIC_NUM;
    int err = sector_write(f, BOOTBLOCK_SECTOR, bootblock);
    if(err < 0){
        return err;
    }

    //write the superblock
    err = sector_write(f, SUPERBLOCK_SECTOR, &spb);
    if(err < 0){
        return err;
    }

    //set and write all inodes sectors to 0 between     s_inode_start+1  and  s_block_start-1
    struct inode sectorOfInodes[INODES_PER_SECTOR] = {0};
    sectorOfInodes[1].i_mode = IFDIR | IALLOC;        //TODO  sectorOfInodes[0] ou 1 ?
    sectorOfInodes[1].i_size0 = 0;
    sectorOfInodes[1].i_size1 = 0;
    sectorOfInodes[1].i_addr[0] =  (uint16_t) (spb.s_block_start + 1);

    for (uint32_t i = spb.s_inode_start; i < (uint32_t)(spb.s_block_start-1); ++i) {
        err = sector_write(f, i, sectorOfInodes);
        if(err < 0){
            return err;
        }
    }

    fclose(f);

    return 0;
}
