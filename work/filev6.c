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



int filev6_readblock(struct filev6 *fv6, void *buf){
    M_REQUIRE_NON_NULL(fv6);
    M_REQUIRE_NON_NULL(buf);

    //read sector for next portion of file
    int remaining = inode_getsize(&fv6->i_node) - fv6->offset;
    int sector = inode_findsector(fv6->u, &fv6->i_node, fv6->offset/SECTOR_SIZE);
    if(sector < 0){
        return sector;
    }
    int j = sector_read(fv6->u->f, (uint32_t)sector, buf);
    if(j == ERR_IO || j == ERR_BAD_PARAMETER){
        return j;
    }

    //return read size
    if(remaining < SECTOR_SIZE){
        fv6->offset = inode_getsize(&fv6->i_node);
        return remaining;
    }else{
        fv6->offset += SECTOR_SIZE;
        return SECTOR_SIZE;
    }
}



int filev6_lseek(struct filev6 *fv6, int32_t offset){
    M_REQUIRE_NON_NULL(fv6);

    if(offset > inode_getsize(&fv6->i_node)){
        return ERR_OFFSET_OUT_OF_RANGE;
    }
    fv6->offset = offset;
    return 0;
}

int filev6_create(struct unix_filesystem *u, uint16_t mode, struct filev6 *fv6){
    M_REQUIRE_NON_NULL(u);
    M_REQUIRE_NON_NULL(fv6);
    struct inode ind = {0};
    ind.i_mode = mode;
    int err = inode_write(u, fv6->i_number ,&ind);
    printf("errdansF° : %d\n", err);
    if(err < 0){
        return err;
    }
    fv6->i_node = ind;
    return 0;
}