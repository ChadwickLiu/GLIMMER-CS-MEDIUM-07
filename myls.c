#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h> // 可能不是必需的，但符合POSIX标准
#include <time.h>
/*
    输出文件信息
*/
void parse_permissions(mode_t mode, char *perms){/*这是一个用来表达user，group，others三者分别拥有什么权限的函数*/
char rwx[] = {'r', 'w', 'x'};/*权限分为三个阅读，修改，执行*/

    mode &= 0777;/*以0作为前缀说明这个数是一个八进制数，它的作用是可以让高位都为0，而二进制下的后九位全部由mode决定，即高位屏蔽*/

    for (int i = 0; i < 9; i++) {
        if (1==(mode >> (8 - i)) & 1) {
            perms[i] = rwx[i % 3]; 
        } else {
            perms[i] = '-';
        }
    }/*这个for循环的作用是，将三者对应的权限给输出出来，有的权限就在对应位置输出对应字母，而没有的权限的话就输出-*/

    perms[9] = '\0';/*将perms[9]设置为结束，不可以没有，不然这里会出现垃圾数据*/
}

void print_file_info(const char *name, struct stat *statbuf) {
    char buf[10];
    parse_permissions( statbuf->st_mode, buf);
    printf("%s\t", name);  //文件名
    printf("%lld\t", (long long)statbuf->st_size); // 文件大小
    printf("%s\t", buf); // 文件权限
    // 最后修改时间
    char timebuf[80];
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M", localtime(&statbuf->st_mtime));
    printf("%s\n", timebuf);
}
/*
    文件路径信息的处理
*/
void list_directory(const char *path) {
    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;

    if (!(dir = opendir(path))) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    while ((entry = readdir(dir)) != NULL) {
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        if (stat(full_path, &statbuf) == -1) {
            perror("stat");
            continue;
        }

        print_file_info(entry->d_name, &statbuf);
    }
    closedir(dir);
}
/*
    程序的主函数部分
*/
int main(int argc, char *argv[]) {
    if (argc < 2) {
        list_directory("."); // 默认列出当前目录
    } else {
        for (int i = 1; i < argc; i++) {
            list_directory(argv[i]);
        }
    }

    return 0;
}