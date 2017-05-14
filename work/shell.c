#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "mount.h"
#include "direntv6.h"
#include "inode.h"
#include "sha.h"
#include "error.h"

struct unix_filesystem u;

#define ARG_LENGTH 30
#define ARG_NB 4
#define INPUT_LENGTH ARG_NB * (ARG_LENGTH+1)

enum error_shell {
    ERR_FIRST_SHELL = 64, // not an actual error but to set the first error number
    ERR_INVALID_COMMAND,
    ERR_CAT_ON_DIR,
    ERR_WRONG_NB_ARG,
    ERR_FS_NOT_MOUNTED,
    ERR_LAST_SHELL // not an actual error but to have e.g. the total number of errors
};

const char * const ERR_MESSAGES_SHELL[] = {
        "", // no error
        "invalid command",
        "cat on a directory is not defined",
        "wrong number of arguments",
        "mount the FS before the operation"
};

typedef int (*shell_fct)(char args[ARG_NB][ARG_LENGTH]);

struct shell_map {
    const char* name;   // nom de la commande
    shell_fct fct;      // fonction réalisant la commande
    const char* help;   // description de la commande
    size_t argc;        // nombre d'arguments de la commande
    const char* args;   // description des arguments de la commande
};

/**
 * @brief exit the shell
 * @param args
 * @return
 */
int do_exit(char args[ARG_NB][ARG_LENGTH]){
    M_REQUIRE_NON_NULL(args);
    return 0;
}

/**
 * @brief quit the shell
 * @param args
 * @return
 */
int do_quit(char args[ARG_NB][ARG_LENGTH]){
    M_REQUIRE_NON_NULL(args);
    return 0;
}

/**
 * @brief print information for each shell commands
 * @param args
 * @return
 */
int do_help(char args[ARG_NB][ARG_LENGTH]);

/**
 * @brief mount specified disk
 * @param args
 * @return
 */
int do_mount(char args[ARG_NB][ARG_LENGTH]){
    M_REQUIRE_NON_NULL(args);
    return mountv6(args[1], &u);
}

/**
 * @brief print content of current directory (root) in tree fashion
 * @param args
 * @return
 */
int do_lsall(char args[ARG_NB][ARG_LENGTH]){
    M_REQUIRE_NON_NULL(args);
    return direntv6_print_tree(&u, ROOT_INUMBER, "");
}

/**
 * @brief print superblock of current disk
 * @param args
 * @return
 */
int do_psb(char args[ARG_NB][ARG_LENGTH]){
    M_REQUIRE_NON_NULL(args);
    mountv6_print_superblock(&u);
    return 0;
}

/**
 * @brief print content of specified file
 * @param args
 * @return
 */
int do_cat(char args[ARG_NB][ARG_LENGTH]){
    M_REQUIRE_NON_NULL(args);
    int inr = direntv6_dirlookup(&u, ROOT_INUMBER, args[1]);
    if(inr < 0){
        return inr;
    }
    struct filev6 fs;
    memset(&fs, 255, sizeof(fs));
    int error = filev6_open(&u, (uint16_t)inr, &fs);
    if(error < 0){
        return error;
    }
    if (fs.i_node.i_mode & IFDIR) {
        return ERR_CAT_ON_DIR;
    } else {
        size_t maxSize = SECTOR_SIZE * (ADDR_SMALL_LENGTH - 1) * ADDRESSES_PER_SECTOR + 1;
        char content[maxSize];
        memset(content, 0, maxSize * sizeof(char));
        size_t totalSize = 0;
        int readsize = 0;
        while (totalSize < maxSize && ((readsize = filev6_readblock(&fs, &content[totalSize])) > 0)){
            totalSize += (size_t)readsize;
        }
        printf("%s", content);
    }
    return 0;
}

/**
 * @brief print sha256 of content of specified file
 * @param args
 * @return
 */
int do_sha(char args[ARG_NB][ARG_LENGTH]){
    M_REQUIRE_NON_NULL(args);
    int inr = direntv6_dirlookup(&u, ROOT_INUMBER, args[1]);
    struct inode inode;
    memset(&inode, 0, sizeof(inode));
    inode_read(&u, (uint16_t)inr, &inode);
    print_sha_inode(&u, inode, inr);
    return 0;
}

/**
 * @brief print corresponding inode to specified absolute path
 * @param args
 * @return
 */
int do_inode(char args[ARG_NB][ARG_LENGTH]){
    M_REQUIRE_NON_NULL(args);
    int inr = direntv6_dirlookup(&u, ROOT_INUMBER, args[1]);
    printf("inode: %d\n", inr);
    return 0;
}

/**
 * @brief print stats about the inode corresponding to specified inr
 * @param args
 * @return
 */
int do_istat(char args[ARG_NB][ARG_LENGTH]){
    M_REQUIRE_NON_NULL(args);
    struct inode i;
    memset(&i, 0, sizeof(i));
    long int inr = strtol(args[1], NULL, 10);
    if(inr < 0){
        return ERR_INODE_OUTOF_RANGE;
    }
    int err = inode_read(&u,(uint16_t)inr, &i);
    if(err < 0){
        return err;
    }
    inode_print(&i);
    return 0;
}

/**
 * @brief create a new filesystem
 * @param args
 * @return
 */
int do_mkfs(char args[ARG_NB][ARG_LENGTH]){
    M_REQUIRE_NON_NULL(args);
    //convert string to long int
    long int nbBlocks = strtol(args[3], NULL, 10);
    long int nbInodes = strtol(args[2], NULL, 10);
    return mountv6_mkfs(args[1], (uint16_t)nbBlocks, (uint16_t)nbInodes);;
}

/**
 * @brief unimplemented
 * @param args
 * @return
 */
int do_mkdir(char args[ARG_NB][ARG_LENGTH]){
    M_REQUIRE_NON_NULL(args);
    return 0;
}

/**
 * @brief unimplemented
 * @param args
 * @return
 */
int do_add(char args[ARG_NB][ARG_LENGTH]){
    M_REQUIRE_NON_NULL(args);
    return 0;
}

/**
 * array of all the shell commands
 */
struct shell_map shell_cmds[13] = {
        { "help", do_help, "display this help", 0, ""},
        { "exit", do_exit, "exit shell", 0, ""},
        { "quit", do_quit, "exit shell", 0, ""},
        { "mkfs", do_mkfs, "create a new filesystem", 3, "<diskname> <#inodes> <#blocks>"},
        { "mount", do_mount, "mount the provided filesystem", 1, "<diskname>"},
        { "mkdir", do_mkdir, "create a new directory", 1, "<dirname>"},
        { "lsall", do_lsall, "list all directories and files contained in the currently mounted filesystem", 0, ""},
        { "add", do_add, "add new file", 2, "<src-fullpath> <dst>"},
        { "cat", do_cat, "display the content of a file", 1, "<pathname>"},
        { "istat", do_istat, "display information about the provided inode", 1, "<inode_nr>"},
        { "inode", do_inode, "display the inode number of a file", 1, "<pathname>"},
        { "sha", do_sha, "display the Sha file", 1, "<pathname>"},
        { "psb", do_psb, "print SuperBlock of the currently mounted filesystem", 0, ""}
};

int do_help(char args[ARG_NB][ARG_LENGTH]) {
    M_REQUIRE_NON_NULL(args);
    for (int i = 0; i < 13; ++i) {
        printf("- %s: ", shell_cmds[i].name);
        if (shell_cmds[i].argc > 0) {
            printf("%s: ", shell_cmds[i].args);
        }
        printf("%s\n", shell_cmds[i].help);
    }
    return 0;
}

/**
 * @brief split the user input into argument array
 * @param input
 * @param args
 */
size_t tokenize_input(char* input, char args[ARG_NB][ARG_LENGTH]){
    size_t i = 0;
    char* p = strtok(input, " ");
    while(p != NULL){
        strcpy(args[i], p);
        p = strtok(NULL, " ");
        ++i;
    }
    return i;
}

/**
 * @brief interprete args to find function to run and handle SHELL errors
 * @param args
 * @param current
 * @param argnr
 * @return
 */
int interprete(char args[ARG_NB][ARG_LENGTH], struct shell_map* current, size_t argnr){
    for (int i = 0; i < 13; ++i) {
        if(strcmp(args[0], shell_cmds[i].name) == 0){
            *current = shell_cmds[i];
        }
    }
    if(current->fct == NULL || argnr == 0){
        return ERR_INVALID_COMMAND;
    }
    if(current->argc != argnr - 1){
        return ERR_WRONG_NB_ARG;
    }
    //if FS not mounted
    if(u.f == NULL && (strcmp(current->name,"help") != 0 && strcmp(current->name,"exit") != 0 &&
            strcmp(current->name,"quit") != 0 && strcmp(current->name,"mount") != 0)){
        return ERR_FS_NOT_MOUNTED;
    }
    return 0;
}

/**
 * @brief run the shell
 * @return
 */
int main(){
    struct shell_map current = {"", NULL, "", 0, ""};
    char input[INPUT_LENGTH];
    char args[ARG_NB][ARG_LENGTH];
    while (!feof(stdin) && !ferror(stdin) && strcmp(current.name, "quit") && strcmp(current.name, "exit")) {
        current.fct = NULL;
        printf(">");
        fgets(input, INPUT_LENGTH , stdin);
        input[strcspn(input, "\n")] = 0;
        size_t argnr = tokenize_input(input, args);
        int errshell = interprete(args, &current, argnr);
        int error = 0;
        if(!errshell){
            error = current.fct(args);
        }
        if (error < 0) {
            printf("ERROR FS: %s\n",ERR_MESSAGES[error - ERR_FIRST]);
        }
        if(error > 0 || errshell){
            if(errshell > 0){
                printf("ERROR SHELL: %s\n", ERR_MESSAGES_SHELL[errshell - ERR_FIRST_SHELL]);
            }else{
                printf("ERROR SHELL: %s\n", ERR_MESSAGES_SHELL[error - ERR_FIRST_SHELL]);
            }
        }
    }
    return 0;
}



