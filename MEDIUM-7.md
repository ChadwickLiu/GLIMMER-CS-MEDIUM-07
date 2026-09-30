# MEDIUM-3：初识Linux系统
## 第一境：算者 —— 命令行参数与路径
1.  
   1. 
- 绝对路径：从文件的根目录开始，一层一层写到底的完整路径
    >特点：任何位置都不会留下歧义，以/开头，与现在所处哪个目录无关
-  相对路径：以当前工作目录为基准，描述目标位置离我有多远的路径
    >特点：不以/开头，同一个相对路径在不同目录下指向的目标不同
    2. 绝对路径命令：`cd /var/backups/turing/`
    相对路径命令：`cd ../../../var/backups/turing/`
    >cd（change directory），.代表当前目录，..代表父目录，所以写相对路径时先找到当前目录所处的位置，然后先回到与目标目录相同父母录的位置，最后补全目标目录的路径
2. 1.   - argc（argument account参数计数）：代表命令行传给程序的参数的总个数
        -  argv[](argument vector参数向量)：字符串指针数组，每个元素都指向一个参数字符串
        >argv[0]指向的是启动程序本身的路径（不算是用户额外传的参数），真正的命令行参数从argv[1]开始
    2.  - `cd /var/backups/turing/`argc=2（一个命令符，一个参数）
    argv[0]指向cd，argv[1]指向/var/backups/turing/，argv[2]不指向任何参数，C语言规定argv[argc]=NULL
    - `cd ../../../var/backups/turing/`argc=2（一个命令符，一个参数）
    argv[0]指向cd，argv[1]指向../../../var/backups/turing/,argv[2]不指向任何参数
## 第二境：算王—— 重建 ls 命令与位运算
1.   
    1. 见过的头文件：#include <stdio.h>，#include <string.h>，#include <stdlib.h>  

        没见过的：#include <dirent.h>，#include <sys/stat.h>，#include <sys/types.h>，#include <time.h>
    2. - <dirent.h>：  

    | 名称 | 类型 | 作用 |
    |------|------|------|
    | `DIR` | 结构体 | 表示一个**正在打开的目录流**，类似文件的 `FILE*` |
    | `struct dirent` | 结构体 | 表示**目录中的一个条目** |
    | `opendir(const char *path)` | 函数 | 打开目录，成功返回 `DIR *`，失败返回 `NULL` |
    | `readdir(DIR *dir)` | 函数 | 逐条读出目录项，读完（或出错）返回 `NULL`|
    | `closedir(DIR *dir)` | 函数 | 关闭目录流，释放资源 |

    - <sys/stat.h>

    | 名称 | 类型 | 作用 |
    |------|------|------|
    | `struct stat` | 结构体 | 存放一个文件的元信息|
    | `stat(const char *path, struct stat *buf)` | 函数 | 把 `path` 指向的文件的元信息读进 `buf`，成功返回 0，失败返回 -1 |
2.   >整数如何表示文件的权限：；利用二进制，让每一位代表不同的权限，1代表有这个权限，0代表没有这个权限

        ```c
        void parse_permissions(mode_t mode, char *perms){/*这是一个用来表达user，group，others三者分别拥有什么权限的函数*/
        char rwx[] = {'r', 'w', 'x'};/*权限分为三个阅读，修改，执行*/
        // 请补全代码写在 TO DO 与 END OF TO DO 之间：
        // TO DO
        mode &= 0777;/*以0作为前缀说明这个数是一个八进制数，它的作用是可以让高位都为0，而二进制下的后九位全部由mode决定，即高位屏蔽*/

        for (int i = 0; i < 9; i++) {
        if (1==(mode >> (8 - i)) & 1) {
            perms[i] = rwx[i % 3]; 
        } else {
            perms[i] = '-';
        }
        }/*这个for循环的作用是，将三者对应的权限给输出出来，有的权限就在对应位置输出对应字母，而没有的权限的话就输出-*/
        // END OF TO DO

        perms[9] = '\0';/*将perms[9]设置为结束，不可以没有，不然这里会出现垃圾数据*/
        }
        ```
## 第三境：算尊—— 深入理解 Inode 与文件树
1. 
    1. inode里存储了文件的元数据（**文件类型**，**极限位**（即刚刚拆分过的9位数表示文件的权限），**所属组**，**文件大小**，**访问时间**，**内容修改时间**，**元数据修改时间**，**数据块指针**（文件内容在磁盘上的位置），**inode编号本身**，**硬链接数**（有几个文件名同时指向这一个inode））
    2. 不包含文件名（文件名只是一个标签，inode才是内核），不包含文件内容
    3. - '.'代表当前目录，它的inode也就是当前目录的inode
        - '..'代表的是父目录，它的inode编号也就是上一层目录的inode编号
        - 系统根目录并没有父目录，因此根节点的父目录就指向它自己，因此二者的inode编号是相同的
2. ```c
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
    ```
    >先进行递归将每一步都strcpy下来，直到根节点触发第一个return然后依次返回，并且将自己复制好的内容打印出来
    >从当前目录出发，先记录自己的inode，再用 chdir("..") 进父目录，在父目录的目录流里逐行比对，找出 inode 和刚才存的inode一致的那一行—，它里面的d_name就是本层在父目录里的名字；用strcpy把这一层名字存下来；然后对父目录递归重复同样的过程，一层层往上推，直到根目录。
    >哪些位置使用了ai：代码编写部分,问了ai几个问题
    ![tupian](https://cdn.postimage.me/2026/09/28/13c56208b608dab08.png)
    ![tupian](https://cdn.postimage.me/2026/09/28/2.png)
## 最终境：算帝—— 递归与文件筛选
1. ```c
        // TO DO
        if(strcmp(entry->d_name,target_name)==0){
            printf("%s",full_path);
        }
        if((statbuf.st_mode & S_IFMT) == S_IFDIR){
            find_file(full_path,target_name)
        }

        // END OF TO DO
    ```