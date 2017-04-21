#include <stdint.h>
#include "direntv6.h"
#include "error.h"
#include <string.h>
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

    //update data of the next entry according to data stored in *d
    name[DIRENT_MAXLEN+1] = '\0';
    strncpy(name, d->dirs[d->cur].d_name, DIRENT_MAXLEN);
    *child_inr = d->dirs[d->cur].d_inumber;

    //TODO
    //strncpy(name, (d->dirs + d->cur)->d_name, DIRENT_MAXLEN);
    //*(name+DIRENT_MAXLEN)='\0';
    //*child_inr = (d->dirs + d->cur)->d_inumber;

    d->cur += 1;
    return 1;
}
int direntv6_print_tree(const struct unix_filesystem *u, uint16_t inr, const char *prefix){
    M_REQUIRE_NON_NULL(u);
    M_REQUIRE_NON_NULL(prefix);
    struct directory_reader d;
    //TODO rzjouté
    memset(&d, 0, sizeof(d));
    int open = direntv6_opendir(u, inr, &d);
    printf("OPEN%d  ", open);
    if(open == ERR_INVALID_DIRECTORY_INODE){           //if file (end of recursion)
        char* c =  strrchr(prefix, PATH_TOKEN);     //return a pointer to last occurrence of '/' in prefix
        *c = '\0';
        printf(SHORT_FIL_NAME" "ROOTDIR_NAME"%s\n", prefix);
    }else{
        printf(SHORT_DIR_NAME" "ROOTDIR_NAME"%s\n", prefix);
        char name[MAXPATHLEN_UV6];
        uint16_t child_inr;
        printf("IN\n");

        while (direntv6_readdir(&d, name, &child_inr) > 0) {
            printf("child%d, ", child_inr);
            printf("cur%d, last%d    ", d.cur, d.last);
            char concat[MAXPATHLEN_UV6];
            strcpy(concat, prefix);
            strcat(concat, name);
            strcat(concat, "/");
            direntv6_print_tree(u, child_inr, concat);
        }
        //if(1) memset(&d, 0, sizeof(d));*/
        printf("OUT\n\n");
    }
    return 0;
}




int direntv6_dirlookup_core(const struct unix_filesystem *u, uint16_t inr, const char *entry,
                            size_t length) {
    M_REQUIRE_NON_NULL(u);
    M_REQUIRE_NON_NULL(entry);

    //TODO si aucun inode ne correspond à entry --> return ERR_INODE_OUTOF_RANGE

    struct directory_reader d;
    int j = direntv6_opendir(u, inr, &d);
    if (j < 0)
        return j;
    char name[MAXPATHLEN_UV6];
    uint16_t child_inr = 0;
    while (direntv6_readdir(&d, name, &child_inr) > 0) {
        char *end = strchr(entry + 1, '/');
        //TODO nbr de /
        unsigned long int length_of_curr = (end < entry + 1) ? length - 1 : end - (entry + 1);
        int comp = strncmp(name, entry + 1, length_of_curr);
        if (strrchr(entry + 1, '/') != NULL && length_of_curr<MAXPATHLEN_UV6){ // for directory
            name[length_of_curr] = '/';
        }
        int final_comp = strcmp(entry+1, name);

        if (final_comp == 0) {
            printf("\n\nINR -------> %d\n\n\n", child_inr);
            return child_inr;
        }

        else if (comp == 0) {
            direntv6_dirlookup_core(u, child_inr, end, length - (length_of_curr + 1));
        }
    }
    return 0;
    //problème : qaund on fait un test avec test-dirent.c <./test-dirent disks/aiw.uv6>
    // en passant juste "/books/" on a une SEGFAULT
    // dirlookup retourne toujours 0
    //ERR_INODE_OUTOF_RANGE;
}


int direntv6_dirlookup(const struct unix_filesystem *u, uint16_t inr, const char *entry){
    int j = direntv6_dirlookup_core(u, inr, entry,  strlen(entry));
    printf("FINAL SORTIE : %d", j);
    return j;
    //return direntv6_dirlookup_core(u, 1234, "coucou/", 2);
}
