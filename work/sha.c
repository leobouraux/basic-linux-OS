#include <stdio.h>
#include "error.h"
#include <openssl/sha.h>
#include "mount.h"
#include "unixv6fs.h"
#include "filev6.h"


/**
 * week05
 * 
 * @brief print the sha of the content
 * @param content the content of which we want to print the sha
 * @param length the length of the content
 */
void print_sha_from_content(const unsigned char *content, size_t length){
	unsigned char sha[SHA256_DIGEST_LENGTH];
	SHA256(content, length, sha);
	if(sha == NULL) return;
	for (size_t i = 0; i < SHA256_DIGEST_LENGTH ; i++){
		printf("%02x" ,sha[i]);
	}
}


/**
 * week05
 * 
 * @brief print the sha of the content of an inode
 * @param u the filesystem
 * @param inode the inode of which we want to print the content
 * @param inr the inode number
 */
void print_sha_inode(struct unix_filesystem *u, struct inode inod, int inr) {
	if(inod.i_mode & IALLOC) {
		printf("SHA inode %d:", inr);
		if (inod.i_mode & IFDIR) {
			printf(" no SHA for directories. \n");
		}
		else {
			/*aller lire tout le contenu du fichier correspondant
			  afficher le SHA256 du contenu lu.*/
			struct filev6 filv6 = {u, inr, inod, 0};
			struct inode inodes[SECTOR_SIZE]; 
			filev6_readblock(&filv6, inodes);
			print_sha_from_content(inodes, SECTOR_SIZE);
			printf("\n");
		}
		
	}
}
