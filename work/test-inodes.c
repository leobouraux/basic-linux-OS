#include "mount.h"
#include "inode.h"
#include <stdio.h>
#include <string.h>

int test(struct unix_filesystem *u){
    //inode_scan_print(u);
    struct inode i;
    memset(&i, 0, sizeof(i));
    inode_read(u, 3, &i);
    inode_print(&i);
    int j = inode_findsector(u, &i, 0);
    printf("%d", j);
    return 0;
}
