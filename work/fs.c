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

struct unix_filesystem fs;

static int fs_getattr(const char *path, struct stat *stbuf)
{
	int res = 0;
	memset(stbuf, 0, sizeof(struct stat));
    struct inode i;
    inode_read(&fs, direntv6_dirlookup(&fs, ROOT_INUMBER, path), &i);
    if(i.i_mode & IFDIR){
        stbuf->st_mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH | S_IFDIR;
    }else{
        stbuf->st_mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH | S_IFREG;
    }

	return res;
}

static int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi)
{
	(void) offset;
	(void) fi;
	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	return 0;
}

static int fs_read(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
	(void) fi;
    return 0;
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
            printf("%d", err);
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
