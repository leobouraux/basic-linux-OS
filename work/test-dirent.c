#include "direntv6.h"

int test(struct unix_filesystem *u){
    //return direntv6_print_tree(u, ROOT_INUMBER, "");

    int j = direntv6_dirlookup(u, ROOT_INUMBER, "/books/aiw/by_chapters/11-0-c09.txt");
    printf("inr : %d\n", j);
    return 0;
}