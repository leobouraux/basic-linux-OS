#include <stdint.h>
#include "direntv6.h"
#include "error.h"
#include "string.h"

int direntv6_opendir(const struct unix_filesystem *u, uint16_t inr, struct directory_reader *d){
    M_REQUIRE_NON_NULL(u);
    M_REQUIRE_NON_NULL(d);
    if(inr & IFMT != IFDIR){
        return ERR_INVALID_DIRECTORY_INODE;
    }
    d->cur = 0;
    d->last = 0;
    return filev6_open(u, inr, &d->fv6);
}

int direntv6_readdir(struct directory_reader *d, char *name, uint16_t *child_inr){
    M_REQUIRE_NON_NULL(d);
    M_REQUIRE_NON_NULL(name);
    M_REQUIRE_NON_NULL(child_inr);

    if(d->cur == d->last){
        int readSize = filev6_readblock(&d->fv6, d->dirs);
        d->cur = 0;
        if(readSize <= 0){
            return readSize;
        }else{
            d->last = readSize / sizeof(struct direntv6);
        }
    }
    size_t nameSize = sizeof(d->dirs[d->cur].d_name);
    strncpy(name, d->dirs[d->cur].d_name,nameSize);
    if(nameSize == DIRENT_MAXLEN){
        name[DIRENT_MAXLEN+1] = '\0';
    }
    *child_inr = d->dirs[d->cur].d_inumber;
    d->cur += 1;
    return 1;
}

int direntv6_print_tree(const struct unix_filesystem *u, uint16_t inr, const char *prefix){
    M_REQUIRE_NON_NULL(u);
    M_REQUIRE_NON_NULL(prefix);
    struct directory_reader d;
    direntv6_opendir(u, inr, &d);

}
