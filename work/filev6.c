#include <stdint.h>
#include "filev6.h"
#include "inode.h"
#include "sector.h"
#include "error.h"

int filev6_open(const struct unix_filesystem *u, uint16_t inr, struct filev6 *fv6){
    M_REQUIRE_NON_NULL(fv6);
    M_REQUIRE_NON_NULL(u);

    fv6->u = u;
    fv6->i_number = inr;
    fv6->offset = 0;
    return inode_read(u, inr, &fv6->i_node);
}

/**
 * @brief read at most SECTOR_SIZE from the file at the current cursor
 * @param fv6 the filev6 (IN-OUT; offset will be changed)
 * @param buf points to SECTOR_SIZE bytes of available memory (OUT)
 * @return >0: the number of bytes of the file read; 0: end of file;
 *             the appropriate error code (<0) on error
 */
int filev6_readblock(struct filev6 *fv6, void *buf){
    M_REQUIRE_NON_NULL(fv6);
    M_REQUIRE_NON_NULL(buf);

    int remaining = inode_getsize(&fv6->i_node) - fv6->offset;
    /*printf("FILEV6 INODESIZE %d, \n", inode_getsize(&fv6->i_node));

    printf("FILEV6 REM %d, \n", remaining);*/
    //printf("FILEV6 OFFSET %d, \n", fv6->offset);

    int j = sector_read(fv6->u->f, inode_findsector(fv6->u, &fv6->i_node, fv6->offset), buf);
    if(j == ERR_IO || j == ERR_BAD_PARAMETER){
        return j;
    }
    if(remaining < SECTOR_SIZE){
        fv6->offset = inode_getsize(&fv6->i_node);
        return remaining;
    }else{
        fv6->offset += SECTOR_SIZE;
        return SECTOR_SIZE;
    }
}