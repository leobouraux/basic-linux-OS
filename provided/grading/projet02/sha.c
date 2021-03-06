#include <stdio.h>
#include <openssl/sha.h>
#include "mount.h"
#include "filev6.h"
#include <string.h>


#define MAX_LENGTH_OF_FILE (SECTOR_SIZE * (ADDR_SMALL_LENGTH-1) * ADDRESSES_PER_SECTOR)


void print_sha_from_content(const unsigned char *content, size_t length){
	unsigned char sha[SHA256_DIGEST_LENGTH];
	SHA256(content, length, sha);
	if(sha == NULL) return;
	for (size_t i = 0; i < SHA256_DIGEST_LENGTH ; i++){
		printf("%02x" ,sha[i]);
	}
}


void print_sha_inode(struct unix_filesystem *u, struct inode inod, int inr) {
    //check if the inode is valid
    if (inod.i_mode & IALLOC) {
        printf("SHA inode %d: ", inr);
        if (inod.i_mode & IFDIR) {
            printf("no SHA for directories. \n");
        } else {

            //when the inode represent a file
            struct filev6 filv6 = {u, (uint16_t) inr, inod, 0};

            char content[MAX_LENGTH_OF_FILE + 1];
            content[MAX_LENGTH_OF_FILE] = '\0';
            char currContent[SECTOR_SIZE + 1];
            currContent[SECTOR_SIZE] = '\0';
            int rem = 0;

            //while the whole content of the inode is not read, read a sector and store in currContent
            //since the content of an inode (->file or directory) might be larger than one sector
            do{
                rem = filev6_readblock(&filv6, currContent);
                strcat(content, currContent);
            } while (rem == SECTOR_SIZE);
            print_sha_from_content((unsigned const char *) content, strlen(content));
            memset(content, 0, sizeof(content));
            printf("\n");
        }
    }
}