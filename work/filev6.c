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
    if(remaining){
        int j = sector_read(fv6->u->f, (uint32_t)sector, buf);
        if(j == ERR_IO || j == ERR_BAD_PARAMETER){
            return j;
        }
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

int filev6_convert_to_big(struct unix_filesystem *u, struct filev6 *fv6){
    int indirec_sector = bm_find_next(u->fbm);
    if(indirec_sector < 0){
        return indirec_sector;
    }
    bm_set(u->fbm, (uint64_t)indirec_sector);
    uint16_t adress[ADDRESSES_PER_SECTOR] = {0};
    for (int i = 0; i < ADDR_SMALL_LENGTH; ++i) {
        adress[i] = fv6->i_node.i_addr[i];
    }
    int err = sector_write(u->f, (uint32_t)indirec_sector, adress);
    if(err < 0){
        return err;
    }
    fv6->i_node.i_addr[0] = (uint16_t)indirec_sector;
    for (int i = 1; i < ADDR_SMALL_LENGTH; ++i) {
        fv6->i_node.i_addr[i] = 0;
    }
    return 0;
}

int filev6_writesector(struct unix_filesystem *u, struct filev6 *fv6, void *buf, int len, int offset){
    int32_t inode_size = inode_getsize(&fv6->i_node);
    if(inode_size > 7 * ADDRESSES_PER_SECTOR * SECTOR_SIZE){
        return ERR_FILE_TOO_LARGE;
    }
    uint32_t nb_bytes = 0;
    uint32_t rest_to_write = (uint32_t)(len - offset);
    int err = 0;
    if(inode_size % SECTOR_SIZE == 0){
        if(inode_size == ADDR_SMALL_LENGTH * SECTOR_SIZE){
            err = filev6_convert_to_big(u,fv6);
            if(err < 0){
                return err;
            }
        }
        if(rest_to_write < SECTOR_SIZE){
            nb_bytes = rest_to_write;
        }else{
            nb_bytes = SECTOR_SIZE;
        }
        //write new content
        int sector = bm_find_next(u->fbm);
        if(sector < 0){
            return sector;
        }
        bm_set(u->fbm, (uint64_t)sector);
        err = sector_write(u->f, (uint32_t)sector, buf + offset);
        if(err < 0){
            return err;
        }
        //index it in inode
        if(inode_size >= ADDR_SMALL_LENGTH * SECTOR_SIZE){
            if(inode_size % (ADDRESSES_PER_SECTOR * SECTOR_SIZE) == 0){
                //big file and new indirection
                int indirec_sector = bm_find_next(u->fbm);
                if(indirec_sector < 0){
                    return indirec_sector;
                }
                bm_set(u->fbm, (uint64_t)sector);
                uint16_t adress[ADDRESSES_PER_SECTOR] = {0};
                adress[0] = (uint16_t)sector;
                err = sector_write(u->f, (uint32_t)indirec_sector, adress);
                if(err < 0){
                    return err;
                }
                fv6->i_node.i_addr[inode_size / (SECTOR_SIZE*ADDRESSES_PER_SECTOR)] = (uint16_t)indirec_sector;
            }else{
                //big file but current indirection not full
                uint16_t data[SECTOR_SIZE];
                err = sector_read(u->f, fv6->i_node.i_addr[inode_size / (SECTOR_SIZE*ADDRESSES_PER_SECTOR)], data);
                if (err < 0) {
                    return err;
                }
                data[inode_size % (SECTOR_SIZE*ADDRESSES_PER_SECTOR)] = (uint16_t)sector;
                err = sector_write(u->f, fv6->i_node.i_addr[inode_size / (SECTOR_SIZE*ADDRESSES_PER_SECTOR)], data);
                if (err < 0) {
                    return err;
                }
            }
        }else{
            //small file
            int32_t addr_index = inode_size/SECTOR_SIZE;
            fv6->i_node.i_addr[addr_index] = (uint16_t)sector;
        }
    }else{
        uint32_t left_in_sector = (uint32_t)(SECTOR_SIZE - (inode_size % SECTOR_SIZE));
        if(rest_to_write < left_in_sector){
            nb_bytes = rest_to_write;
        }else{
            nb_bytes = left_in_sector;
        }
        int sector = inode_findsector(u, &fv6->i_node, inode_size - inode_size % SECTOR_SIZE);
        if(sector < 0){
            return sector;
        }
        uint8_t block[SECTOR_SIZE];
        err = sector_read(u->f,(uint32_t)sector, block);
        if(err < 0){
            return err;
        }
        memcpy(&block[inode_size % SECTOR_SIZE],buf, nb_bytes);
        err = sector_write(u->f, (uint32_t)sector, block);
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
    int read_size = 1;
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

