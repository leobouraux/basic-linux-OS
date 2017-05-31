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
    if(err >= 0 && !(d->fv6.i_node.i_mode & IFDIR)){
        return ERR_INVALID_DIRECTORY_INODE;
    }
    return err;
}


int direntv6_readdir(struct directory_reader *d, char *name, uint16_t *child_inr){
    M_REQUIRE_NON_NULL(d);
    M_REQUIRE_NON_NULL(name);
    M_REQUIRE_NON_NULL(child_inr);

    //launch the lecture of the next sector if the last element = the current of the directory
    if(d->cur == d->last){
        int readSize = filev6_readblock(&d->fv6, d->dirs);
        d->cur = 0;
        if(readSize <= 0){
            return readSize;
        }else{
            d->last = readSize / (int)sizeof(struct direntv6);
        }
    }

    //update data of the next entry according to data stored in *d
    name[DIRENT_MAXLEN+1] = '\0';
    strncpy(name, d->dirs[d->cur].d_name, DIRENT_MAXLEN);
    *child_inr = d->dirs[d->cur].d_inumber;

    //update the position of the current entry
    d->cur += 1;
    return 1;
}



int direntv6_print_tree(const struct unix_filesystem *u, uint16_t inr, const char *prefix){
    M_REQUIRE_NON_NULL(u);
    M_REQUIRE_NON_NULL(prefix);
    struct directory_reader d;
    memset(&d, 0, sizeof(d));
    int open = direntv6_opendir(u, inr, &d);

    //if file (end of recursion)
    if(open == ERR_INVALID_DIRECTORY_INODE){
        char* c =  strrchr(prefix, PATH_TOKEN);        //return a pointer to last occurrence of '/' in prefix
        *c = '\0';
        printf(SHORT_FIL_NAME" "ROOTDIR_NAME"%s\n", prefix);
    }else{
        printf(SHORT_DIR_NAME" "ROOTDIR_NAME"%s\n", prefix);
        char name[MAXPATHLEN_UV6];
        uint16_t child_inr;

	// correcteur : il faut retourner les éventuelles erreurs de readdir
        //while there is still a file to read
        while (direntv6_readdir(&d, name, &child_inr) > 0) {
            char concat[MAXPATHLEN_UV6];
            //store prefix and name followed by / in concat
            snprintf(concat, MAXPATHLEN_UV6, "%s%s%c", prefix, name, PATH_TOKEN);
	    // correcteur : il faut retourner les éventuelles erreurs de print_tree
            direntv6_print_tree(u, child_inr, concat);
        }
    }
    return 0;
}



int direntv6_dirlookup(const struct unix_filesystem *u, uint16_t inr, const char *entry){
    M_REQUIRE_NON_NULL(u);
    M_REQUIRE_NON_NULL(entry);

    struct directory_reader d;
    int j = direntv6_opendir(u, inr, &d);
    //return possible errors coming from direntv6_opendir()
    if (j < 0){
        return j;
    }
    //delete initial chars '/'
    size_t offset = 0;
    while(entry[offset] == PATH_TOKEN){
        offset++;
    }

    int end = 0;
    size_t len;
    char* next = strchr(entry+offset, PATH_TOKEN);  //the next directory or file in the tree
    if(next == NULL){
        len = strlen(entry+offset);
        end = 1;
    }else{
        len = (size_t)(next - (entry+offset));
    }

    //case in which we search a directory, and its name is followed by multiples '/'
    if(len == 0 ){
        return inr;
    }

    char name[MAXPATHLEN_UV6];
    uint16_t child_inr = 0;

    //while there is still a file to read, it compares name to the current entry
    while (direntv6_readdir(&d, name, &child_inr) > 0) {
        int comp = strncmp(name, entry + offset, strlen(name));
        if(comp == 0){
            //when it's a file
            if(end){
                return child_inr;
            }else{
                return direntv6_dirlookup(u, child_inr, entry+offset+len);
            }
        }
    }
    return ERR_INODE_OUTOF_RANGE;
}
