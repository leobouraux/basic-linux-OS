#include <stdint.h>
#include "direntv6.h"
#include "error.h"
#include <string.h>
#include "filev6.h"
#include "inode.h"
#define MAXPATHLEN_UV6 1024

int direntv6_opendir(const struct unix_filesystem *u, uint16_t inr, struct directory_reader *d){
    M_REQUIRE_NON_NULL(u);
    M_REQUIRE_NON_NULL(d);
    d->cur = 0;
    d->last = 0;
    int err =  filev6_open(u, inr, &d->fv6);
    if(err >= 0 && !(d->fv6.i_node.i_mode & IFDIR)) {
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


        //while there is still a file to read
        int err = 0;
        while ((err = direntv6_readdir(&d, name, &child_inr)) > 0) {
            char concat[MAXPATHLEN_UV6];
            //store prefix and name followed by / in concat
            snprintf(concat, MAXPATHLEN_UV6, "%s%s%c", prefix, name, PATH_TOKEN);
            err = direntv6_print_tree(u, child_inr, concat);
            if(err<0)
                return err;
        }
        if(err<0)
            return err;
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
    int err = 0;
    while ((err = direntv6_readdir(&d, name, &child_inr)) > 0) {
        int comp = strncmp(name, entry + offset, strlen(name));
        if(comp == 0 && len == strlen(name)){
            //when it's a file
            if(end){
                return child_inr;
            }else{
                return direntv6_dirlookup(u, child_inr, entry+offset+len);
            }
        }
    }
    if(err < 0){
        return err;
    }
    return ERR_INODE_OUTOF_RANGE;
}

/**
 * @brief helper function checking if entry is a valid path to write and computes relative_name and parent_inr
 * @param u
 * @param entry
 * @param relative_name
 * @param parent_inr
 * @return 0 on success, error code otherwise
 */
int direntv6_test_available(struct unix_filesystem *u, const char *entry, char *relative_name, int *parent_inr){
    M_REQUIRE_NON_NULL(u);
    M_REQUIRE_NON_NULL(entry);
    M_REQUIRE_NON_NULL(relative_name);
    M_REQUIRE_NON_NULL(parent_inr);


    char parent[30] = {0};              //TODO magic numbers?
    char *limit = strrchr(entry, '/'); //TODO handle / at the end
    if(limit == NULL){
        return ERR_BAD_PARAMETER;
    }

    strncpy(relative_name, limit+1, 14);  //TODO magic numbers?
    strncpy(parent, entry, strlen(entry)-strlen(relative_name));


    *parent_inr = direntv6_dirlookup(u, ROOT_INUMBER, parent);
    if(*parent_inr < 0){
        return ERR_BAD_PARAMETER;
    }

    int child_inr = direntv6_dirlookup(u, (uint16_t)*parent_inr, relative_name);
    if(child_inr >= 0){
        return ERR_FILENAME_ALREADY_EXISTS;
    }
    return 0;

}

int direntv6_create(struct unix_filesystem *u, const char *entry, uint16_t mode){
    M_REQUIRE_NON_NULL(u);
    M_REQUIRE_NON_NULL(entry);
    char relative_name[14];
    int parent_inr = 0;
    int err = direntv6_test_available(u, entry, relative_name, &parent_inr);
    if(err < 0){
        return err;
    }

    int inr = inode_alloc(u);
    if(inr < 0){
        return inr;
    }
    struct inode ind = {0};
    struct filev6 fv6 = {u, (uint16_t)inr,ind,0};
    err = filev6_create(u, mode, &fv6);
    if(err < 0){
        return err;
    }
    struct direntv6 dir = {0};
    dir.d_inumber = (uint16_t)inr;
    strncpy(dir.d_name, relative_name, 14);

    struct inode parent_inode = {0};
    err = inode_read(u, (uint16_t)parent_inr, &parent_inode);
    if(err < 0){
        return err;
    }
    struct filev6 parent = {u, (uint16_t)parent_inr, parent_inode,0};
    return filev6_writebytes(u, &parent, &dir, sizeof(dir));
}
