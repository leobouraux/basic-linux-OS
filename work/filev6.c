#include <stdint.h>
#include "filev6.h"
#include "inode.h"
#include "sector.h"
#include "error.h"
#include <memory.h>


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
    if(err < 0){
        return err;
    }
    fv6->i_node = ind;
    return 0;
}

int filev6_writesector(struct unix_filesystem *u, struct filev6 *fv6, void *buf, int len, int offset){
    int32_t inode_size = inode_getsize(&fv6->i_node);
    if(inode_size > 7 * 256 * 512){
        return ERR_FILE_TOO_LARGE;
    }
    uint32_t nb_bytes = 0;
    uint32_t rest_to_write = (uint32_t)(len - offset);
    int err = 0;
    if(inode_size % SECTOR_SIZE == 0){
        if(rest_to_write < SECTOR_SIZE){
            nb_bytes = rest_to_write;
        }else{
            nb_bytes = SECTOR_SIZE;
        }

        int sector = bm_find_next(u->fbm);
        if(sector < 0){
            return sector;
        }
        bm_set(u->fbm, (uint64_t)sector);
        err = sector_write(u->f, (uint32_t)sector, buf + offset);
        if(err < 0){
            return err;
        }
        int32_t addr_index = inode_size/SECTOR_SIZE + 1;
        fv6->i_node.i_addr[addr_index] = (uint16_t)sector;
    }else{
        uint32_t left_in_sector = (uint32_t)(SECTOR_SIZE - (inode_size % SECTOR_SIZE));
        if(rest_to_write < left_in_sector){
            nb_bytes = rest_to_write;
        }else{
            nb_bytes = left_in_sector;
        }
        int sector_index = inode_size/SECTOR_SIZE;
        uint16_t sector = fv6->i_node.i_addr[sector_index];
        uint8_t block[SECTOR_SIZE];
        err = sector_read(u->f,sector, block);
        if(err < 0){
            return err;
        }
        memcpy(&block[inode_size % SECTOR_SIZE],buf, nb_bytes);
        err = sector_write(u->f, sector, block);
        if(err < 0) {
            return err;
        }
    }
    err = inode_setsize(&fv6->i_node, inode_size + nb_bytes);
    if(err < 0){
        return err;
    }
    return nb_bytes;
}

int filev6_writebytes(struct unix_filesystem *u, struct filev6 *fv6, void *buf, int len){
    int offset = 0;
    int read_size = 0;
    while (offset < len && read_size > 0){
        read_size = filev6_writesector(u, fv6, buf, len, offset);
        offset += read_size;
    }
    if(read_size < 0){
        return read_size;
    }
    int err = inode_write(u, fv6->i_number, &fv6->i_node);
    if(err < 0){
        return err;
    }
    return 0;
}

