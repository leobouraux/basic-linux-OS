#include <memory.h>
#include "direntv6.h"

int test(struct unix_filesystem *u){
    return direntv6_print_tree(u, ROOT_INUMBER, "");

    //return direntv6_dirlookup(u, ROOT_INUMBER, "/tmp");
}
///books/aiw/full/11-0.txt
//