#include <stdint.h>
#include "filev6.h"
#include "inode.h"

int filev6_open(const struct unix_filesystem *u, uint16_t inr, struct filev6 *fv6){
    fv6->u = u;
    fv6->i_number = inr;
    inode_read(u, inr, &fv6->i_node);
    fv6->offset = 0;
    return 0;
}

int filev6_readblock(struct filev6 *fv6, void *buf){

}