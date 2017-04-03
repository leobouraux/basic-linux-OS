#include <stdio.h>
#include "error.h"
#include <openssl/sha.h>
#include "mount.h"
#include "unixv6fs.h"
#include "filev6.h"
#include "inode.h"

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
		printf("SHA inode %d: ", inr);
		if (inod.i_mode & IFDIR) {
			printf("no SHA for directories. \n");

		}
		else {
            //content ne contient pas tout le content d'un inode
            //j'ai le sha de l'inode/fichier sur un secteur mais pas sur plusieurs
            //inode_scan_print(&inod);

//CHAQUE FICHIER N'A QU'UN INODE



            struct filev6 filv6 = {u, inr, inod, 0};
            char content[SECTOR_SIZE +1];// *7*256];
            content[SECTOR_SIZE] = '\0';
            int rem = filev6_readblock(&filv6, &content);
            printf("%d c'etait rem", rem);
            while(rem==512) {
                    //en étudiant que l'inode 5 de aiuw
                    //la boucle est faite 32fois où 512 + 33*512+489 = 17385 = sizeOfFiledeInode5

                    //filv6.offset+=rem; déjà mis à jour
                    rem = filev6_readblock(&filv6, &content);  //dernier content et ça va trop loin
                    printf("%d WWWII  ", rem);
            }

            //printf("\nCONTENT----\n%s----\nSHA:", &content);
            //printf("CURSOR IN INODE : %d ///////////  ", rem);
            print_sha_from_content(&content, rem);
            printf("\n\n\n\n\n\n\n\n");


            //--> lire  l'inode directement ?
            /*inode_read(u, inr, &inod);
            print_sha_from_content(&inod, inode_getsize(&inod));
            printf("\n");*/
		}
        //aiw       inode1SIZE:16    2:16        3:32     4:240       5:17385    6:631   7:11761
        //8:11332      9:9938      10:14282     11:12527     12:14411     13:13459
        //14:14145     15:13339    16:12147     17:10871     18:12149     19:1428     20:16
        // 21:169856
        //simple    inode1:16    2:16        3:18
		//simple : 338fc4bb0d037f3747396a4c852d2a9d8b545d622c6c744c670cf95f715731d3
        //aiw :    819b3529b07803dcb47741634ae1b40e497ec95a446ab1e1b1487bb222179dd5

    }
}
