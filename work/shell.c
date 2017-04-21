#include <string.h>
#include <stdio.h>
#include "mount.h"
#include "direntv6.h"
#include "inode.h"

struct unix_filesystem u;

#define ARG_LENGTH 30
#define ARG_NB 4
#define INPUT_LENGTH ARG_NB * (ARG_LENGTH+1)

typedef int (*shell_fct)(char args[ARG_NB][ARG_LENGTH]);

struct shell_map {
    const char* name;   // nom de la commande
    shell_fct fct;      // fonction réalisant la commande
    const char* help;   // description de la commande
    size_t argc;        // nombre d'arguments de la commande
    const char* args;   // description des arguments de la commande
};

int do_exit(char args[ARG_NB][ARG_LENGTH]);

int do_quit(char args[ARG_NB][ARG_LENGTH]);

int do_help(char args[ARG_NB][ARG_LENGTH]);

int do_mount(char args[ARG_NB][ARG_LENGTH]);

int do_lsall(char args[ARG_NB][ARG_LENGTH]);

int do_psb(char args[ARG_NB][ARG_LENGTH]);

int do_cat(char args[ARG_NB][ARG_LENGTH]);

int do_sha(char args[ARG_NB][ARG_LENGTH]);

int do_inode(char args[ARG_NB][ARG_LENGTH]);

int do_istat(char args[ARG_NB][ARG_LENGTH]);

int do_mkfs(char args[ARG_NB][ARG_LENGTH]);

int do_mkdir(char args[ARG_NB][ARG_LENGTH]);

int do_add(char args[ARG_NB][ARG_LENGTH]);


struct shell_map shell_cmds[13] = {
        { "help", do_help, "display this help", 0, ""},
        { "exit", do_exit, "exit shell", 0, ""},
        { "quit", do_quit, "exit shell", 0, ""},
        { "mkfs", do_mkfs, "create a new filesystem", 3, "<diskname> <#inodes> <#blocks>"},
        { "mount", do_mount, "mount the provided filesystem", 1, "<diskname>"},
        { "mkdir", do_mkdir, "create a new directory", 1, "<dirname>"},
        { "lsall", do_lsall, "list all direcoties and files contained in the currently mounted filesystem", 0, ""},
        { "add", do_add, "add new file", 2, "<src-fullpath> <dst>"},
        { "cat", do_cat, "display the content of a file", 1, "<pathname>"},
        { "istat", do_istat, "display information about the provided inode", 1, "<inode_nr>"},
        { "inode", do_inode, "display the inode number of a file", 1, "<pathname>"},
        { "sha", do_sha, "display the Sha file", 1, "<pathname>"},
        { "psb", do_psb, "print SuperBlock of the currently mounted filesystem", 0, ""}
};

int do_exit(char args[ARG_NB][ARG_LENGTH]){
    return 0;
}

int do_quit(char args[ARG_NB][ARG_LENGTH]){
    return 0;
}

int do_help(char args[ARG_NB][ARG_LENGTH]){
    for (int i = 0; i < 13; ++i) {
        printf("- %s: ", shell_cmds[i].name);
        if(shell_cmds[i].argc > 0){
            printf("%s: ", shell_cmds[i].args);
        }
        printf("%s\n", shell_cmds[i].help);
    }
    return 0;
}

int do_mount(char args[ARG_NB][ARG_LENGTH]){
    return mountv6(args[1], &u);
}

int do_lsall(char args[ARG_NB][ARG_LENGTH]){
    return direntv6_print_tree(&u, ROOT_INUMBER, ROOTDIR_NAME);
}

int do_psb(char args[ARG_NB][ARG_LENGTH]){
    mountv6_print_superblock(&u);
    return 0;
}

int do_cat(char args[ARG_NB][ARG_LENGTH]){
    int inr = direntv6_dirlookup(&u, ROOT_INUMBER, args[1]);
    struct filev6 fs;
    memset(&fs, 255, sizeof(fs));
    filev6_open(&u, inr, &fs);
    if (fs.i_node.i_mode & IFDIR) {
        printf("ERROR SHELL: cat on a directory is not defined\n");
    } else {
        char content[SECTOR_SIZE * (ADDR_SMALL_LENGTH - 1) * ADDRESSES_PER_SECTOR + 1];
        int rem = filev6_readblock(&fs, content);
        content[SECTOR_SIZE * 7 * 256] = '\0';
        while (rem == SECTOR_SIZE) {
            char currContent[SECTOR_SIZE + 1];
            rem = filev6_readblock(&fs, currContent);
            currContent[SECTOR_SIZE] = '\0';
            strcat(content, currContent);
        }
        printf("%s", content);
    }
    return 0;
}

int do_sha(char args[ARG_NB][ARG_LENGTH]){

    return 0;
}

int do_inode(char args[ARG_NB][ARG_LENGTH]){
    int inr = direntv6_dirlookup(&u, ROOT_INUMBER, args[1]);
    printf("inode: %d", inr);
    return 0;
}

int do_istat(char args[ARG_NB][ARG_LENGTH]){
    struct inode i;
    memset(&i, 0, sizeof(i));
    int err = inode_read(&u, *args[1], &i);
    if(err < 0){
        return err;
    }
    inode_print(&i);
    return 0;
}

int do_mkfs(char args[ARG_NB][ARG_LENGTH]){
    return 0;
}

int do_mkdir(char args[ARG_NB][ARG_LENGTH]){
    return 0;
}

int do_add(char args[ARG_NB][ARG_LENGTH]){
    return 0;
}

void tokenize_input(char* input, char args[ARG_NB][ARG_LENGTH]){
    int i = 0;
    char* p = strtok(input, " ");
    while(p != NULL){
        strcpy(args[i], p);
        p = strtok(NULL, " ");
        ++i;
    }
}

int main(){
    struct shell_map current = {"", NULL, "", 0, ""};
    char input[INPUT_LENGTH];
    char args[ARG_NB][ARG_LENGTH];
    while (!feof(stdin) && !ferror(stdin) && strcmp(current.name, "quit") && strcmp(current.name, "exit")) {
        printf(">");
        fgets(input, INPUT_LENGTH , stdin);
        input[strcspn(input, "\n")] = 0;
        tokenize_input(input, args);
        for (int i = 0; i < 13; ++i) {
            if(strcmp(args[0], shell_cmds[i].name) == 0){
                current = shell_cmds[i];
            }
        }
        current.fct(args);
    }
    return 0;
}



