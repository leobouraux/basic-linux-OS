#include "mount.h"
#include "inode.h"
#include <stdio.h>

int test(struct unix_filesystem *u){
    inode_scan_print(u);
    return 0;
}
