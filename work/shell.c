#include <string.h>
#include <stdio.h>

int do_exit(char** args){

}

int do_quit(char** args){

}

int do_help(char** args){

}

int do_mount(char** args){

}

int do_lsall(char** args){

}

int do_psb(char** args){

}

int do_cat(char** args){

}

int do_sha(char** args){

}

int do_inode(char** args){

}

int do_istat(char** args){

}

int do_mkfs(char** args){

}

int do_mkdir(char** args){

}

int do_add(char** args){

}

//struct unix_filesystem u;

typedef int (*shell_fct)(char**);

struct shell_map {
    const char* name;    // nom de la commande
    shell_fct fct;      // fonction réalisant la commande
    const char* help;   // description de la commande
    size_t argc;        // nombre d'arguments de la commande
    const char* args;   // description des arguments de la commande
};

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

void tokenize_input(char* input, char args[][30]){
    int i = 0;
    char* p = strtok(input, " ");
    while(p != NULL){
        strcpy(args[i], p);
        p = strtok(NULL, " ");
        ++i;
    }
}

int main1(){
    struct shell_map current;
    char input[100];
    char* args[4];
    while (!feof(stdin) && !ferror(stdin) && strcmp(current.name, "quit") && strcmp(current.name, "exit")) {
        scanf("%s", input);
        tokenize_input(input, args);
        for (int i = 0; i < 13; ++i) {
            if(strcmp(input, shell_cmds[i].name) == 0){
                current = shell_cmds[i];
            }
        }
        current.fct(args);
    }
}

int main(){
    char input[100];
    char args[4][30];
    scanf("%s", input);
    tokenize_input(input, args);
    for (int i = 0; i < 4; ++i) {
        printf("%s", args[i]);
    }
}



