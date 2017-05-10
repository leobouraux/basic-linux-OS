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
    int j = inode_findsector(u, &i, 3);
    printf("%d\n", j);

    struct inode ind = {0};
    struct filev6 fv6 = {u, 6,ind,0};
    int err = filev6_create(u, IFDIR, &fv6);
    printf("err create : %d\n", err);
    inode_scan_print(u);
    return 0;
}
