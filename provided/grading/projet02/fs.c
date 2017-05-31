/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.

  gcc -Wall hello.c `pkg-config fuse --cflags --libs` -o hello
*/

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include "mount.h"
#include "inode.h"
#include "direntv6.h"
#include "unixv6fs.h"
#include "error.h"

#define MAXPATHLEN_UV6 1024

struct unix_filesystem fs;

static int fs_getattr(const char *path, struct stat *stbuf)
{
    memset(stbuf, 0, sizeof(struct stat));
    struct inode i;
    int inr = direntv6_dirlookup(&fs, ROOT_INUMBER, path);
    if(inr < 0){
        return inr;
    }
    stbuf->st_ino = (ino_t)inr;
    int err = inode_read(&fs, (uint16_t)inr, &i);
    if(err < 0){
        return err;
    }
    stbuf->st_size = inode_getsize(&i);
    //modes
    stbuf->st_mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
    if(i.i_mode & IFDIR){
         stbuf->st_mode |= S_IFDIR;
    }else{
        stbuf->st_mode |= S_IFREG;
    }
    return 0;
}

static int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                      off_t offset, struct fuse_file_info *fi)
{
    (void) offset;
    (void) fi;
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    int inr = direntv6_dirlookup(&fs, ROOT_INUMBER, path);
    if(inr < 0){
        return inr;
    }
    struct directory_reader d = {0};
    int open = direntv6_opendir(&fs, (uint16_t)inr, &d);
    if(open < 0){
        return open;
    }else{
        //add each child in buf
        char name[MAXPATHLEN_UV6];
        uint16_t child_inr;
        while (direntv6_readdir(&d, name, &child_inr) > 0) {
            // correcteur : il faut contrôler la valeur de retour de filler, peut-être qu'il n'y a plus assez de mémoire notament...
            filler(buf, name, NULL, 0);
        }
    }
    return 0;
}

static int fs_read(const char *path, char *buf, size_t size, off_t offset,
                   struct fuse_file_info *fi)
{
    (void) fi;
    int inr = direntv6_dirlookup(&fs, ROOT_INUMBER, path);
    if(inr < 0){
        // correcteur : en cas d'erreur il faut retourner le nombre d ebytes lu, donc 0
        return inr;
    }
    struct filev6 filv6;
    int err = filev6_open(&fs, (uint16_t)inr, &filv6);
    if(err < 0){
        return err;
    }
    err = filev6_lseek(&filv6, (int32_t)offset);
    if(err < 0){
        return err;
    }
    //fill the content of file in buffer block per block
    size_t total_size = 0;
    int read_size = 0;
    while (total_size < size && ((read_size = filev6_readblock(&filv6, &buf[total_size])) > 0)){
        total_size += (size_t)read_size;
    }
    return (int)total_size;
}

static struct fuse_operations available_ops = {
        .getattr	= fs_getattr,
        .readdir	= fs_readdir,
        .read		= fs_read,
};

/* From https://github.com/libfuse/libfuse/wiki/Option-Parsing.
* This will look up into the args to search for the name of the FS.
*/
static int arg_parse(void *data, const char *filename, int key, struct fuse_args *outargs)
{
    (void) data;
    (void) outargs;
    if (key == FUSE_OPT_KEY_NONOPT && fs.f == NULL && filename != NULL) {
        int err = mountv6(filename, &fs);
        if(err < 0){
            printf("%s\n", ERR_MESSAGES[err - ERR_FIRST]);
            exit(1);
        }
        return 0;
    }
    return 1;
}

int main(int argc, char *argv[])
{
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    int ret = fuse_opt_parse(&args, NULL, NULL, arg_parse);
    if (ret == 0) {
        ret = fuse_main(args.argc, args.argv, &available_ops, NULL);
        (void)umountv6(&fs);
    }
    return ret;
}
