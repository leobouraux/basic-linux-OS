#include <string.h>
#include "mount.h"
#include "error.h"
#include "sector.h"
#include <inttypes.h>


/**
 * week04
 * 
 * @brief  mount a unix v6 filesystem
 * @param filename name of the unixv6 filesystem on the underlying disk (IN)
 * @param u the filesystem (OUT)
 * @return 0 on success; <0 on error
 */
int mountv6(const char *filename, struct unix_filesystem *u){
    M_REQUIRE_NON_NULL(filename);
    M_REQUIRE_NON_NULL(u);
    
    memset(u, 0, sizeof(&u));
    u->fbm = NULL;
    u->ibm = NULL;
    u->f = fopen(filename, "rb");
    if(u->f == NULL){
        return ERR_IO;
    }
    uint8_t content[SECTOR_SIZE];
    sector_read(u->f, BOOTBLOCK_SECTOR, content);
    if(content != BOOTBLOCK_MAGIC_NUM_OFFSET){
        return ERR_BADBOOTSECTOR;
    }
    uint8_t superblock[SECTOR_SIZE];
    sector_read(u->f, SUPERBLOCK_SECTOR, superblock);

}

/**
 * week04
 * 
 * @brief umount the given filesystem
 * @param u - the mounted filesytem
 * @return 0 on success; <0 on error
 */
int umountv6(struct unix_filesystem *u){
    M_REQUIRE_NON_NULL(u);
    
    int j = 0;
    j = fclose(u->f);
    if(j == EOF){
        return ERR_IO;
    }
    return 0;
}

/**
 * week04
 * 
 * @brief print to stdout the content of the superblock
 * @param u - the mounted filesytem
 */
void mountv6_print_superblock(const struct unix_filesystem *u){	
    printf("**********FS SUPERBLOCK START**********");
    printf("s_isize : %" PRIu16"\n", u->s.s_isize);
    printf("s_fsize : %" PRIu16"\n", u->s.s_fsize);
    printf("s_fbmsize : %" PRIu16"\n", u->s.s_fbmsize);
    printf("s_ibmsize : %" PRIu16"\n", u->s.s_ibmsize);
    printf("s_inode_start : %" PRIu16"\n", u->s.s_inode_start);
    printf("s_block_start : %" PRIu16"\n", u->s.s_block_start);
    printf("s_fbm_start : %" PRIu16"\n", u->s.s_fbm_start);
    printf("s_ibm_start : %" PRIu8"\n", u->s.s_ibm_start);
    printf("s_flock : %" PRIu8"\n", u->s.s_flock);
    printf("s_ilock : %" PRIu8"\n", u->s.s_ilock);
    printf("s_fmod : %" PRIu8"\n", u->s.s_fmod);
    printf("s_ronly : %" PRIu16"\n", u->s.s_ronly);
    printf("s_time : %" PRIu16"\n", u->s.s_time[0]);
    printf("**********FS SUPERBLOCK END************");
}
