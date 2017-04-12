#include <string.h>
#include <stdio.h>
#include "mount.h"
#include "direntv6.h"
#include "inode.h"

struct unix_filesystem u;

typedef int (*shell_fct)(char**);

struct shell_map {
    const char* name;    // nom de la commande
    shell_fct fct;      // fonction réalisant la commande
    const char* help;   // description de la commande
    size_t argc;        // nombre d'arguments de la commande
    const char* args;   // description des arguments de la commande
};

int do_exit(char** args);

int do_quit(char** args);

int do_help(char** args);

int do_mount(char** args);

int do_lsall(char** args);

int do_psb(char** args);

int do_cat(char** args);

int do_sha(char** args);

int do_inode(char** args);

int do_istat(char** args);

int do_mkfs(char** args);

int do_mkdir(char** args);

int do_add(char** args);


struct shell_map shell_cmds[13] = {
        { "exit", do_exit, "exit shell", 0, ""},
        { "quit", do_quit, "exit shell", 0, ""},
        { "help", do_help, "display this help", 0, ""},
        { "mount", do_mount, "mount the provided filesystem", 1, "<diskname>"},
        { "lsall", do_lsall, "list all direcoties and files contained in the currently mounted filesystem", 0, ""},
        { "psb", do_psb, "print SuperBlock of the currently mounted filesystem", 0, ""},
        { "cat", do_cat, "display the content of a file", 1, "<pathname>"},
        { "sha", do_sha, "display the Sha file", 1, "<pathname>"},
        { "inode", do_inode, "display the inode number of a file", 1, "<pathname>"},
        { "istats", do_istat, "display information about the provided inode", 1, "<inode_nr>"},
        { "mkfs", do_mkfs, "create a new filesystem", 3, "<diskname> <#inodes> <#blocks>"},
        { "mkdir", do_mkdir, "create a new directory", 1, "<dirname>"},
        { "add", do_add, "add new file", 2, "<src-fullpath> <dst>"}
};

int do_exit(char** args){
    return 0;
}

int do_quit(char** args){
    return 0;
}

int do_help(char** args){
    for (int i = 0; i < 13; ++i) {
        printf("- %s: %s: %s\n", shell_cmds[i].name, shell_cmds[i].args, shell_cmds[i].help);
    }
    return 0;
}

int do_mount(char** args){
    return mountv6(args[1], &u);
}

int do_lsall(char** args){
    return direntv6_print_tree(&u, ROOT_INUMBER, ROOTDIR_NAME);
}

int do_psb(char** args){
    mountv6_print_superblock(&u);
    return 0;
}

int do_cat(char** args){

}

int do_sha(char** args){

}

int do_inode(char** args){

}

int do_istat(char** args){
    struct inode i;
    memset(&i, 0, sizeof(i));
    int err = inode_read(&u, *args[1], &i);
    if(err < 0){
        return err;
    }
    inode_print(&i);
    return 0;
}

int do_mkfs(char** args){

}

int do_mkdir(char** args){

}

int do_add(char** args){

}

void tokenize_input(char* input, char args[][30]){
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
    char input[100];
    char args[4][30];
    while (!feof(stdin) && !ferror(stdin) && strcmp(current.name, "quit") && strcmp(current.name, "exit")) {
        printf(">");
        fgets(input, 100 , stdin);
        input[strcspn(input, "\n")] = 0;
        tokenize_input(input, args);
        for (int i = 0; i < 13; ++i) {
            if(strcmp(args[0], shell_cmds[i].name) == 0){
                current = shell_cmds[i];
            }
        }
        //printf("%s\n", current.name);
        current.fct(args);
    }
    return 0;
}



