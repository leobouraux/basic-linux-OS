#include "mount.h"
#include "inode.h"
#include <stdio.h>
#include <string.h>
#include "filev6.h"

int test(struct unix_filesystem *u){
    inode_scan_print(u);
    struct inode i;
    memset(&i, 0, sizeof(i));
    inode_read(u, 3, &i);
    inode_print(&i);
    int j = inode_findsector(u, &i, 1);
    printf("%d\n", j);

    struct inode ind = {0};
    struct filev6 fv6 = {u, 6,ind,0};
    //int err2 = inode_write(u, 6, &ind);
    int err = filev6_create(u, IFDIR + IALLOC, &fv6);
    printf("err create : %d\n", err);
    //printf("err write : %d\n", err2);
    inode_scan_print(u);
    inode_read(u, 6, &i);
    inode_print(&i);
    return 0;
}
