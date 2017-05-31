#include "mount.h"
#include "inode.h"
#include <stdio.h>
#include <string.h>

int test(struct unix_filesystem *u){
    // correcteur : il faut retourner la valeur de retour de votre call
    inode_scan_print(u);
    // correcteur : pourquoi tout ce garbage ??
    struct inode i;
    memset(&i, 0, sizeof(i));
    inode_read(u, 5, &i);
    inode_print(&i);
    int j = inode_findsector(u, &i, 8);
    printf("%d", j);
    return 0;
}
