#include "mount.h"
#include "inode.h"
#include <string.h>
#include "filev6.h"
#include "direntv6.h"

int test(struct unix_filesystem *u){
    return inode_scan_print(u);
}
