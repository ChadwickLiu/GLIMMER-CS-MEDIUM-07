#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

ino_t get_inode(const char *path) {         /*一个将目录的元数据全部存放到一个结构体中的函数*/
    struct stat statbuf;                    /*构建结构体*/
    if (stat(path, &statbuf) == -1) {       /*作用一：将对应目录的元数据都给存进statbuf当中*/
        perror("stat failed");
        exit(EXIT_FAILURE);                 /*作用二：进行是否填入成功判断，若填入失败，输出原因，并且直接退出程序*/
    }
    return statbuf.st_ino;
}

void print_path(ino_t this_inode) {          /*打印绝对路径的核心函数*/
    if (this_inode == get_inode("..")) {
        putchar('/');            
        return;                              /*先用if判断，如果说当前的inode和这个目录的父目录的inode相同，说明他是根目录*/
    }                                        /*根目录只需要在开头的位置补上/就可以了*/

    char name[256];                         /*用name数组来存放目录的绝对路径*/
    ino_t parent_inode = get_inode("..");  

    chdir("..");                            /*将当前目录的这个记号移到父目录上*/

    DIR *dir = opendir(".");                /*打开父节点的目录，返回一个目录流句柄，后面readdir的时候可以顺着它往下读*/
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {    /*readdir这个函数是一个目录一个目录顺着读下去，先是.，而后..，再然后是其它子目录*/
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;       /*比较两个字符数组，如果readdir里每一次读出来的都和当前目录，或者父目录一样就跳出这一次循环，用于排除当前目录和父目录的干扰*/
        if (get_inode(entry->d_name) == this_inode) {
            strcpy(name, entry->d_name);/*如果读到了一个和this_inode相同inode的目录，就把它的文件名copy到name的位置*/
            break;
        }
    }
    closedir(dir);/*把目录给关上，对应opendir*/
    print_path(parent_inode);/*用递目录一层层往上推直到推到根目录的位置结束*/
    printf("/%s", name);
}

int main(void) {
    print_path(get_inode(".")); 
    putchar('\n');
    return 0;
}                                   /*代码由ai生成，但是注释都是我自己写的*/