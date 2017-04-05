#include <stdint.h>
#include "direntv6.h"
#include "error.h"
#include "string.h"
#include "unixv6fs.h"
#include <inttypes.h>

#define MAXPATHLEN_UV6 1024

int direntv6_opendir(const struct unix_filesystem *u, uint16_t inr, struct directory_reader *d){
    M_REQUIRE_NON_NULL(u);
    M_REQUIRE_NON_NULL(d);
    d->cur = 0;
    d->last = 0;
    int err =  filev6_open(u, inr, &d->fv6);
    if(!(d->fv6.i_node.i_mode & IFDIR)){
        return ERR_INVALID_DIRECTORY_INODE;
    }
    return err;
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
    name[DIRENT_MAXLEN+1] = '\0';
    strncpy(name, d->dirs[d->cur].d_name,DIRENT_MAXLEN);
    *child_inr = d->dirs[d->cur].d_inumber;
    d->cur += 1;
    return 1;
}

int direntv6_print_tree(const struct unix_filesystem *u, uint16_t inr, const char *prefix){
    M_REQUIRE_NON_NULL(u);
    M_REQUIRE_NON_NULL(prefix);
    struct directory_reader d;
    int j = direntv6_opendir(u, inr, &d);
    if(j == ERR_INVALID_DIRECTORY_INODE){
        printf("FIL %s\n", prefix);
    }else{
        printf("DIR %s\n", prefix);
        char name[MAXPATHLEN_UV6];
        uint16_t child_inr = 0;
        int done = 1;
        while(done > 0){
            done = direntv6_readdir(&d,name, &child_inr);
            if(done > 0 && child_inr != inr){
                char concat[MAXPATHLEN_UV6];
                strcpy(concat, prefix);
                strcat(concat, name);
                strcat(concat, "/");
                direntv6_print_tree(u, child_inr, concat);
            }
        }
    }
    return 0;
}
