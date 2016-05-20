/**
* @Author LPQ
*/
#include <stdlib.h>
#include <string.h>
#include "iostream"
#include "fstream"
#include "time.h"
using namespace std;



///***** 定义变量 *****/
#define BLOCKSIZE       1024        // 磁盘块大小
#define SIZE            1024000     // 虚拟磁盘空间大小
#define END             65535       // FAT中的文件结束标志
#define FREE            0           // FAT中盘块空闲标志
#define MAXOPENFILE     10          // 最多同时打开文件个数

#define MAX_TEXT_SIZE  10000

/***** 定义数据结构 *****/

// 文件控制块
// 用于记录文件的描述和控制信息，每个文件设置一个FCB，它也是文件的目录项的内容
typedef struct FCB {
    char filename[8];           // 文件名
    char exname[3];             // 文件扩展名
    unsigned char attribute;    // 文件属性字段
    unsigned short time;        // 文件创建时间
    unsigned short date;        // 文件创建日期
    unsigned short first;       // 文件起始盘块号
    unsigned long length;       // 文件长度（字节数）
    char free;                  // 表示目录项是否为空，若值为0，表示空，值为1，表示已分配
} fcb;

// 文件分配表
typedef struct FAT {
    unsigned short id;
} fat;

// 用户打开文件表
typedef struct USEROPEN {
    char filename[8];           // 文件名
    char exname[3];             // 文件扩展名
    unsigned char attribute;    // 文件属性字段
    unsigned short time;        // 文件创建时间
    unsigned short date;        // 文件创建日期
    unsigned short first;       // 文件起始盘块号
    unsigned long length;       // 文件长度（对数据文件是字节数，对目录文件可以是目录项个数）
    char free;                  // 表示目录项是否为空，若值为0，表示空，值为1，表示已分配

    int dirno;                  // 相应打开文件的目录项在父目录文件中的盘块号
    int diroff;                 // 相应打开文件的目录项在父目录文件的dirno盘块中的目录项序号
    char dir[80];               // 相应打开文件所在的目录名，这样方便快速检查出指定文件是否已经打开
    int count;                  // 读写指针在文件中的位置
    char fcbstate;              // 是否修改了文件的FCB的内容，如果修改了置为1，否则为0
    char topenfile;             // 表示该用户打开表项是否为空，若值为0，表示为空，否则表示已被某打开文件占据
} useropen;

// 引导块 BLOCK0
typedef struct BLOCK0 {
    char magic_number[8];       // 文件系统的魔数
    char information[200];
    unsigned short root;        // 根目录文件的起始盘块号
    unsigned char *startblock;  // 虚拟磁盘上数据区开始位置
} block0;

/***** 全局变量定义 *****/
unsigned char *myvhard;             // 指向虚拟磁盘的起始地址
useropen openfilelist[MAXOPENFILE]; // 用户打开文件表数组
int currfd;                         // 记录当前用户打开文件表项的下标
unsigned char *startp;              // 记录虚拟磁盘上数据区开始位置
char *FileName = "myfsys.txt";
unsigned char buffer[SIZE];

/***** 函数申明 *****/
void startSys();                // 进入文件系统
void exitsys();                 // 退出文件系统
void my_format();                  // 磁盘格式化函数
void my_mkdir(char *dirname);      // 创建子目录
void my_rmdir(char *dirname);      // 删除子目录
void my_ls();                      // 显示目录中的内容
void my_cd(char *dirname);         // 用于更改当前目录
int  my_create(char *filename);    // 创建文件
void my_rm(char *filename);        // 删除文件
int  my_open(char *filename);      // 打开文件
int  my_close(int fd);             // 关闭文件
int  my_write(int fd);             // 写文件
int  my_read(int fd);     // 读文件
int  do_write(int fd, char *text, int len, char wstyle);
int  do_read(int fd, int len, char *text);



unsigned short getFreeBLOCK();      // 获取一个空闲的磁盘块
int getFreeOpenfilelist();          // 获取一个空闲的文件打开表项
int find_father_dir(int fd);        // 寻找一个打开文件的父目录打开文件

int do_read(int fd, int len, char *text){
    //① 使用malloc()申请1024B空间作为缓冲区buf，申请失败则返回-1，并显示出错信息；
    //	② 将读写指针转化为逻辑块块号及块内偏移量off，利用打开文件表表项中的首块号查找FAT表，找到该逻辑块所在的磁盘块块号；将该磁盘块块号转化为虚拟磁盘上的内存位置；
    //	③ 将该内存位置开始的1024B（一个磁盘块）内容读入buf中；
    //	④ 比较buf中从偏移量off开始的剩余字节数是否大于等于应读写的字节数len，如果是，则将从off开始的buf中的len长度的内容读入到text[]中；否则，将从off开始的buf中的剩余内容读入到text[]中；
    //	⑤ 将读写指针增加④中已读字节数，将应读写的字节数len减去④中已读字节数，若len大于0，则转②；否则转⑥；
    //	⑥ 使用free()释放①中申请的buf。
    //	⑦ 返回实际读出的字节数。
    //lenTmp 用于记录要求读取的长度,一会返回实际读取的长度
    int lenTmp = len;

    unsigned char* buf = (unsigned char*)malloc(1024);
    if(buf == NULL){
        cout << "do_read申请内存空间失败" << endl;
        return -1;
    }

    int off = openfilelist[fd].count;
    //当前fd对应的起始盘块号, 后面变成当前盘块号
    int blockNum = openfilelist[fd].first;
    //ptrfat 当前盘块对应的fat
    fat* fatPtr = (fat *)(myvhard+BLOCKSIZE) + blockNum;
    while(off >= BLOCKSIZE){
        off -= BLOCKSIZE;
        blockNum = fatPtr->id;
        if(blockNum == END){
            cout <<"do_read寻找的块不存在" <<endl;
            return -1;
        }
        fatPtr = (fat*)(myvhard + BLOCKSIZE) + blockNum;
    }
    //当前盘块号对应的盘块
    unsigned char* blockPtr = myvhard + BLOCKSIZE*blockNum;
    //把文件内容读进buf
    memcpy(buf, blockPtr, BLOCKSIZE);
    char *textPtr = text;
    fcb* debug = (fcb*)text;
    while(len > 0 ){
        //一个盘块就能放的下
        if( BLOCKSIZE - off > len){
            memcpy(textPtr,buf + off, len);
            textPtr += len;
            off += len;
            openfilelist[fd].count += len;
            len = 0;
        }
        else{
            memcpy(textPtr, buf + off, BLOCKSIZE - off);
            textPtr += BLOCKSIZE - off;
            off = 0;
            len -= BLOCKSIZE - off;
            //寻找下一个块
            blockNum = fatPtr->id;
            if(blockNum == END){
                cout << "len长度过长! 超出了文件大小!" << endl;
                break;
            }
            fatPtr = (fat*)(myvhard + BLOCKSIZE ) + blockNum;
            blockPtr = myvhard + BLOCKSIZE * blockNum;
            memcpy(buf,blockPtr,BLOCKSIZE);
        }
    }
    free(buf);
    return lenTmp - len;
}

int do_write(int fd, char *text, int len, char wstyle){
    //这里面的逻辑很有趣, 听我细细道来
    //首先这是文件系统,所以你不能通过blockPtr = myvhard + BLOCKSIZE * blockNum 定位到盘块,然后就直接在盘块上写数据了!!
    //不可以!!至于为什么你懂的. 你得用一个buf, 把盘块内容读取进来, 然后在buf里面修改, 然后再把buf内容写会打盘块里去
    //这里会有两个循环. 在覆盖写的情况下, 会先找到openfilelist[].count, 也就是游标, 指向文件现在写到哪了
    //1. 如果count>BLOCKSIZE, 这就意味着游标指向的不是第一个盘块, 所以你要通过FAT找到count指向的盘块,如果中途发现有些盘块是END
    //   那么你需要占用这些盘块,一直占用到游标count指向的盘块(比如count = 5*BLCOKSIZE + x, 那么要一直往下找5个盘块)
    //2. 如果len>BLCOKSIZE-off, 那么改写的内容还没写完, 你要申请盘块, 继续往下写, 一直写到len个字符
    //PS: 在my_write中有3中写法,
    // 0:截断写, 也就是把文件内容全部删除重新写.
    // 1:覆盖写:也就是从count指向的内容继续往下写
    // 2:追加写,从文件的最后继续写


    //盘块号
    int blockNum = openfilelist[fd].first;
    fat *fatPtr = (fat*)(myvhard + BLOCKSIZE) + blockNum ;
    //三种写入方式预处理
    // 0截断写,直接从头开始写,偏移量就是0了,而且长度变成0
    fat* fat1 = (fat*)(myvhard + BLOCKSIZE);
    if(wstyle == 0){
        openfilelist[fd].count = 0;
        openfilelist[fd].length = 0;
    }
    //1,覆盖写, 如果是数据文件,那么要考虑删除文件末尾的\0才能继续往下写
    else if(wstyle == 1){
        if(openfilelist[fd].attribute == 1 && openfilelist[fd].length != 0){
            openfilelist[fd].count -= 1;
        }
    }
    //2追加写,就把游标指向末尾
    else if(wstyle == 2){
        if(openfilelist[fd].attribute == 0){
            openfilelist[fd].count = openfilelist[fd].length;
        }
        //同理,如果是数据文件要删除末尾的\0
        else if(openfilelist[fd].attribute == 1 && openfilelist[fd].length != 0){
            openfilelist[fd].count = openfilelist[fd].length - 1;
        }
    }


    int off = openfilelist[fd].count;										//??????????

    //如果off > BLOCKSIZE,也就是游标现在指向的不是文件中的第一个盘块,那么需要找到那个盘块
    //另外,如果游标很大,但是在寻找对应盘块的时候发现没有那个盘块,那么把缺少的盘块全都补上
    while(off >= BLOCKSIZE){
        blockNum = fatPtr->id;
        if(blockNum == END){
            blockNum = getFreeBLOCK();
            if(blockNum == END){
                cout << "盘块不足"<<endl;
                return -1;
            }
            else{
                //修改了fat, 后面要修改fat2, 不必判断修改了没有fat, 直接无脑复制fat1到fat2就行
                fatPtr->id = blockNum;
                fatPtr = (fat*)(myvhard + BLOCKSIZE + blockNum);
                fatPtr->id = END;
            }
        }
        fatPtr = (fat*)(myvhard + BLOCKSIZE) + blockNum;
        off -= BLOCKSIZE;
    }

    unsigned char *buf = (unsigned char*)malloc(BLOCKSIZE * sizeof(unsigned char));
    if(buf == NULL){
        cout << "申请内存空间失败!";
        return -1;
    }
    fcb *dBlock = (fcb *)(myvhard + BLOCKSIZE * blockNum);
    fcb *dFcb = (fcb *)(text);
    unsigned char *blockPtr = (unsigned char *)(myvhard + BLOCKSIZE * blockNum);					//盘块指针
    int lenTmp = 0;
    char *textPtr = text;
    fcb *dFcbBuf = (fcb *)(buf);
    //第二个循环,读取盘块内容到buf, 把text内容写入buf, 然后再从buf写入到盘块
    while(len > lenTmp){
        //盘块内容读取到buf里
        memcpy(buf,blockPtr,BLOCKSIZE);
        //把text内容写到buf里面去
        for (; off < BLOCKSIZE; off++){
            *(buf + off) = *textPtr;
            textPtr ++;
            lenTmp++;
            if(len == lenTmp){
                break;
            }
        }
        //把buf内容写到盘块里面去
        memcpy(blockPtr, buf, BLOCKSIZE);
        //如果off==BLCOKSIZE,意味着buf写满了, 如果len != lebTmp 意味着数据还没写完, 那么就要看看这个文件还有没有剩余盘块
        //没有剩余盘块,那就要分配新的盘块了
        if(off == BLOCKSIZE && len != lenTmp){
            off = 0;
            blockNum = fatPtr->id;
            if(blockNum == END){
                blockNum = getFreeBLOCK();
                if(blockNum == END){
                    cout << "盘块用完了" <<endl;
                    return -1;
                }
                else{
                    blockPtr = (unsigned char *)(myvhard + BLOCKSIZE * blockNum);
                    fatPtr->id = blockNum;
                    fatPtr = (fat *)(myvhard + BLOCKSIZE) + blockNum;
                    fatPtr->id = END;
                }
            }
            else{
                blockPtr = (unsigned char *)(myvhard + BLOCKSIZE * blockNum);
                fatPtr = (fat *)(myvhard + BLOCKSIZE ) + blockNum;
            }
        }
    }
    openfilelist[fd].count += len;
    //若读写指针大于原来文件的长度，则修改文件的长度
    if(openfilelist[fd].count > openfilelist[fd].length)
        openfilelist[fd].length = openfilelist[fd].count;
    free(buf);
    //如果原来文件占几个盘块,现在修改了文件,结果占用的盘块变少了,那就要把后面占用的盘块全部释放掉
    int i = blockNum;
    while (1){
        //如果这个fat的下一个fat不是end,那么就是释放掉它,一路释放下去
        if(fat1[i].id != END){
            int next = fat1[i].id;
            fat1[i].id = FREE;
            i = next;
        }
        else{
            break;
        }
    }
    //按照上面这种操作,会把本文件的最后一个盘块也变成free,这里要把他重新设置成END
    fat1[blockNum].id = END;
    //备份fat2
    memcpy((fat*)(myvhard + BLOCKSIZE * 3), (fat*)(myvhard + BLOCKSIZE), sizeof(fcb));
    return len;

}

int my_read(int fd){
    if(fd >= MAXOPENFILE || fd < 0){
        cout << "文件不存在" <<endl;
        return -1;
    }
    openfilelist[fd].count = 0;
    char text[MAX_TEXT_SIZE] = "\0";
    do_read(fd,openfilelist[fd].length,text);
    cout << text ;
    return 1;
}

int my_write(int fd){
    if(fd < 0 || fd >= MAXOPENFILE){
        cout << "文件不存在" <<endl;
        return -1;
    }
    int wstyle;
    while(1){
        cout << "输入: 0=截断写, 1=覆盖写, 2=追加写" <<endl;
        cin >> wstyle;
        if(wstyle > 2 || wstyle < 0){
            cout << "指令错误!" << endl;
        }
        else{
            break;
        }
    }
    char text[MAX_TEXT_SIZE] = "\0";
    char textTmp[MAX_TEXT_SIZE] = "\0";
    cout << "请输入文件数据, 以换行+ctrl+z 为文件结尾" <<endl;
    getchar();
    while(gets(textTmp)){
        textTmp[strlen(textTmp)] = '\n';
        strcat(text,textTmp);
    }
    text[strlen(text)] = '\0';
    //+1是因为要把结尾的\0也写进去
    do_write(fd,text,strlen(text)+1,wstyle);
    openfilelist[fd].fcbstate = 1;
    return 1;
}


int getFreeOpenfilelist(){
    for(int i=0; i<MAXOPENFILE; i++){
        if(openfilelist[i].topenfile == 0){
            openfilelist[i].topenfile = 1;
            return i;
        }
    }
    return -1;
}

// FAT---
unsigned short int getFreeBLOCK(){
    fat* fat1 = (fat*)(myvhard + BLOCKSIZE);
    for(int i=0; i < (int)(SIZE/BLOCKSIZE); i++){
        if(fat1[i].id == FREE){
            return i;
        }
    }
    return END;
}

// ->OFT
int find_father_dir(int fd){
    for(int i=0; i<MAXOPENFILE; i++){
        if(openfilelist[i].first == openfilelist[fd].dirno){
            return i;
        }
    }
    return -1;
}

//##
void my_format(){
    //① 将虚拟磁盘第一个块作为引导块，开始的8个字节是文件系统的魔数，记为“10101010”；在之后写入文件系统的描述信息，如FAT表大小及位置、根目录大小及位置、盘块大小、盘块数量、数据区开始位置等信息；
    //	② 在引导块后建立两张完全一样的FAT表，用于记录文件所占据的磁盘块及管理虚拟磁盘块的分配，每个FAT占据两个磁盘块；对于每个FAT中，前面5个块设置为已分配，后面995个块设置为空闲；
    //	③ 在第二张FAT后创建根目录文件root，将数据区的第1块（即虚拟磁盘的第6块）分配给根目录文件，在该磁盘上创建两个特殊的目录项：“.”和“..”，其内容除了文件名不同之外，其他字段完全相同。
    //	⑥ 将虚拟磁盘中的内容保存到myfsys文件中；转⑦
    //	⑦ 使用c语言的库函数fclose()关闭myfsys文件；

    //总结: 1. 设置引导块(一个盘块)
    //2. 设置FAT1(2个盘块) FAT2(2个盘块)
    //3. 设置根目录文件的两个特殊目录项.和..(根目录文件占一个盘块,两个目录项是在写在这个盘块里面的)

    //引导块信息
    block0 *boot = (block0 *)myvhard;
    strcpy(boot->magic_number,"10101010");
    strcpy(boot->information,"文件系统,外存分配方式:FAT,磁盘空间管理:结合于FAT的位示图,目录结构:单用户多级目录结构.");
    boot->root = 5;
    boot->startblock = myvhard + BLOCKSIZE*5;

    //设置两个FAT表信息
    fat* fat1 = (fat*)(myvhard + BLOCKSIZE);
    for(int i=0; i<5; i++){
        fat1[i].id = END;
    }
    for(int i=5; i<1000; i++){
        fat1[i].id = FREE;
    }
    fat* fat2 = (fat*)(myvhard + BLOCKSIZE*3);
    memcpy(fat2,fat1,BLOCKSIZE);

    //5号盘块被根目录区占用了
    fat1[5].id = fat2[5].id = END;

    //根目录区的fcb,特殊目录项., 指向自己
    fcb *root = (fcb*)(myvhard + BLOCKSIZE*5);
    strcpy(root->filename,".");
    strcpy(root->exname,"di");
    root->attribute = 0;

    time_t rawTime = time(NULL);
    struct tm *time = localtime(&rawTime);
    //root->time 是unsigned short int类型的数据, 32位编译器下长16位,64位编译器下长32位
    //这里考虑32位编译器, 所以完整的表示一个时间的秒数是不够长的,所以,保存秒数的一半
    //这样小时占5位,分钟占6位,秒占5位
    root->time = time->tm_hour * 2048 + time->tm_min*32 + time->tm_sec/2;
   
    root->date = (time->tm_year-100)*512 + (time->tm_mon+1)*32 + (time->tm_mday);
    root->first = 5;
    root->free = 1;
    root->length = 2 * sizeof(fcb);

    //root2 指向根目录区的第二个fcb,即特俗或目录项..,因为根目录区没有上级目录,所以指向自己
    fcb* root2 = root + 1;
    memcpy(root2,root,sizeof(fcb));
    strcpy(root2->filename,"..");

    for(int i=2; i < int(BLOCKSIZE / sizeof(fcb)); i++){
        root2 ++;
        strcpy(root2->filename,"");
        root2->free = 0;
    }

    //写到文件里面去
    FILE *fp = fopen(FileName, "w");
    fwrite(myvhard, SIZE, 1, fp);
    fclose(fp);
}

void startSys(){
    //① 申请虚拟磁盘空间；
    //	② 使用c语言的库函数fopen()打开myfsys文件：若文件存在，则转③；若文件不存在，则创建之，转⑤
    //	③ 使用c语言的库函数fread()读入myfsys文件内容到用户空间中的一个缓冲区中，并判断其开始的8个字节内容是否为“10101010”（文件系统魔数），如果是，则转④；否则转⑤；
    //	④ 将上述缓冲区中的内容复制到内存中的虚拟磁盘空间中；转⑦
    //	⑤ 在屏幕上显示“myfsys文件系统不存在，现在开始创建文件系统”信息，并调用my_format()对①中申请到的虚拟磁盘空间进行格式化操作。转⑥；

    //总结: 1. 寻找myfsys.txt文件,如果存在而且开头是文件魔数,就读取到myvhard,否则创建文件并写入初始化信息
    //2. 设置用户打开文件表的第一个表项, 内容为根目录内容, 也就是默认打开根目录
    //3. 初始化一堆全局变量
    myvhard = (unsigned char *)malloc(SIZE);
    //如果文件不存在或者开头不是文件魔数,都重新创建文件
    FILE *file;
    if((file = fopen(FileName, "r")) != NULL){
        fread(buffer, SIZE, 1, file);   //将二进制文件读取到缓冲区
        fclose(file);
        if(memcmp(buffer,"10101010",8) == 0){
            memcpy(myvhard,buffer,SIZE);
            cout << "myfsys文件读取成功!" <<endl;
        }
            //有文件但是开头不是文件魔数
        else{
            cout << "myfsys文件系统不存在，现在开始创建文件系统" <<endl;
            my_format();
            memcpy(buffer,myvhard,SIZE);
        }
    }
    else{
        cout << "myfsys文件系统不存在，现在开始创建文件系统" <<endl;
        my_format();
        memcpy(buffer,myvhard,SIZE);
    }


    //	⑧ 初始化用户打开文件表，将表项0分配给根目录文件使用，并填写根目录文件的相关信息，由于根目录没有上级目录，
    // 所以表项中的dirno和diroff分别置为5（根目录所在起始块号）和0；并将ptrcurdir指针指向该用户打开文件表项。
    //	⑨ 将当前目录设置为根目录。
    fcb *root;
    root = (fcb *)(myvhard + 5 * BLOCKSIZE);
    strcpy(openfilelist[0].filename, root->filename);
    strcpy(openfilelist[0].exname, root->exname);
    openfilelist[0].attribute = root->attribute;
    openfilelist[0].time = root->time;
    openfilelist[0].date = root->date;
    openfilelist[0].first = root->first;
    openfilelist[0].length = root->length;
    openfilelist[0].free = root->free;


    openfilelist[0].dirno = 5;\\
    openfilelist[0].diroff = 0;
    strcpy(openfilelist[0].dir, "\\root\\");
    openfilelist[0].count = 0;
    openfilelist[0].fcbstate = 0;
    openfilelist[0].topenfile = 1;

    //初始化一堆全局变量
    //startp指向数据区的开头
    startp = ((block0*)myvhard)->startblock;
    currfd = 0;
    return ;
}

void exitsys(){
    //心累, 还是解释下, 当currfd=0的时候,也就是根节点, 它是不用更新的
    //因为我们之前在任何目录下mkdir或者create时,已经把length的变化写到目录文件下,名字叫.的目录项里了
    //即任何目录文件的'.'目录项都是实时更新的,但是他的父目录文件的没有被更新,所以需要一个个close
    //但是根目录没有父目录文件, 所以不需要close, 此处也许比较难理解, 大家随缘吧, 不过努力理解下,还是能懂的
    while(currfd){
        my_close(currfd);
    }
    FILE *fp = fopen(FileName, "w");
    fwrite(myvhard, SIZE, 1, fp);
    fclose(fp);
}

void my_cd(char *dirname){
    //总结: 1. 如果当前是目录文件下,那么需要把这个目录文件读取到buf里, 然后检索这个文件里的fcb有没有匹配dirname的目录项(而且必须是目录文件)
    //          如果有,那就在openfilelist里取一个打开文件表项,把这个dirname这个目录文件的fcb写进去,然后切换currfd=fd
    //          这样就算是打开一个目录
    if(openfilelist[currfd].attribute == 1){
        cout << "数据文件里不能使用cd, 要是退出文件, 请用close指令" <<endl;
        return ;
    }
    //如果是目录文件
    else{
        //寻找目录文件里面有没有匹配的名字, 先把目录文件的信息读取到buf里
        char *buf = (char *)malloc(MAX_TEXT_SIZE);
        openfilelist[currfd].count = 0;
        do_read(currfd,openfilelist[currfd].length,buf);
        int i = 0;
        fcb* fcbPtr = (fcb*)buf;
        for(; i < int(openfilelist[currfd].length / sizeof(fcb)); i++,fcbPtr++){
            if(strcmp(fcbPtr->filename, dirname) == 0 && fcbPtr->attribute == 0){
                break;
            }
        }
        //不允许cd非文件
        if(strcmp(fcbPtr->exname, "di") != 0){
            cout << "不允许cd非目录文件!" <<endl;
            return;
        }
        //如果cd 了一个目录文件, 那么判断是.还是..还是子文件,如果是子文件则打开这个目录文件到openfilelist里
        else{
            //cd .不会有反应
            if(strcmp(fcbPtr->filename,".") == 0){
                return;
            }
            //cd ..需要判断现在是不是根目录, 如果是根目录,不操作, 否则,返回上一层
            else if(strcmp(fcbPtr->filename, "..") == 0){
                if(currfd == 0){
                    return;
                }
                else{
                    currfd = my_close(currfd);
                    return;
                }
            }
            //cd 子文件
            else{
                int fd = getFreeOpenfilelist();
                if(fd == -1){
                    return;
                }
                else{
                    openfilelist[fd].attribute = fcbPtr->attribute;
                    openfilelist[fd].count = 0;
                    openfilelist[fd].date = fcbPtr->date;
                    openfilelist[fd].time = fcbPtr->time;
                    strcpy(openfilelist[fd].filename, fcbPtr->filename);
                    strcpy(openfilelist[fd].exname,fcbPtr->exname);
                    openfilelist[fd].first = fcbPtr->first;
                    openfilelist[fd].free = fcbPtr->free;
                    //前面是FCB内容
                    openfilelist[fd].fcbstate = 0;
                    openfilelist[fd].length = fcbPtr->length;
                    // 修改 openfilelist[fd].dir[fd] = openfilelist[currfd].dir[currfd] + dirname;
                    strcpy(openfilelist[fd].dir,
                   (char*)(string(openfilelist[currfd].dir) + string(dirname) + string("\\")).c_str());
                    openfilelist[fd].topenfile = 1;
                    openfilelist[fd].dirno = openfilelist[currfd].first;
                    openfilelist[fd].diroff = i;
                    currfd = fd;
                }
            }
        }

    }

}

int my_open(char *filename){
    //总结: 把当前目录文件读取到buf里,buf里面就是一个个fcb了,在这些fcb里寻找匹配上filename的数据文件
    //      然后从openfilelist里取出一个打开文件表项,把这个数据文件的信息写进去,切换currfd=fd就算是打开文件了

    //把当前目录文件读取到buf里
    char buf[MAX_TEXT_SIZE];
    openfilelist[currfd].count = 0;
    do_read(currfd, openfilelist[currfd].length, buf);
    char *fname = strtok(filename,".");
    char *exname = strtok(NULL, ".");
    if(!exname){
        cout << "请输入后缀名" << endl;
        return -1 ;
    }
    int i;
    fcb* fcbPtr = (fcb*)buf;
    //寻找此文件
    for(i=0; i < int(openfilelist[currfd].length / sizeof(fcb)); i++, fcbPtr++){
        if(strcmp(fcbPtr->filename,fname) == 0 && strcmp(fcbPtr->exname,exname) == 0 && fcbPtr->attribute == 1){
            break;
        }
    }
    if(i == int(openfilelist[currfd].length / sizeof(fcb))){
        cout << "不存在此文件!" << endl;
        return -1;
    }
    //为它创建一个打开文件表项
    int fd = getFreeOpenfilelist();
    if(fd == -1){
        cout << "用户打开文件表已经用满" <<endl;
        return -1;
    }
    openfilelist[fd].attribute = 1;
    openfilelist[fd].count = 0;
    openfilelist[fd].date = fcbPtr->date;
    openfilelist[fd].time = fcbPtr->time;
    strcpy(openfilelist[fd].exname, exname);
    strcpy(openfilelist[fd].filename,fname);
    openfilelist[fd].length = fcbPtr->length;
    openfilelist[fd].first = fcbPtr->first;
    strcpy(openfilelist[fd].dir,(string(openfilelist[currfd].dir) + string(filename)).c_str());
    openfilelist[fd].dirno = openfilelist[currfd].first;
    openfilelist[fd].diroff = i;
    openfilelist[fd].free = 1;
    openfilelist[fd].topenfile = 1;
    openfilelist[fd].fcbstate = 0;
    currfd = fd;
    return 1;
}

int my_close(int fd){
    //① 检查fd的有效性（fd不能超出用户打开文件表所在数组的最大下标），如果无效则返回-1；
    //	② 检查用户打开文件表表项中的fcbstate字段的值，如果为1则需要将该文件的FCB的内容保存到虚拟磁盘上该文件的目录项中，
    //    方法是：打开该文件的父目录文件，以覆盖写方式调用do_write()将欲关闭文件的FCB写入父目录文件的相应盘块中；
    //	③ 回收该文件占据的用户打开文件表表项（进行清空操作），并将topenfile字段置为0；
    //	④ 返回。

    if(fd > MAXOPENFILE || fd < 0){
        cout << "不存在这个打开文件" << endl;
        return-1;
    }
    else{
        //判断父目录文件是否存在, 不存在报错
        int fatherFd = find_father_dir(fd);
        if(fatherFd == -1){
            cout << "父目录不存!" <<endl;
            return -1;
        }
        //fcb被修改了, 要写回去
        //那就要先把父目录文件从磁盘中读取到buf中,然后把更新后的fcb内容写到buf里, 然后再从buf写到磁盘上
        //多说一句,写回到磁盘的时候,只要让游标count指向这个文件的对应fcb的位置,然后写入fcb就好了
        //这样只要写一个fcb大小的数据就行了
        if(openfilelist[fd].fcbstate == 1){
            char buf[MAX_TEXT_SIZE];
            do_read(fatherFd,openfilelist[fatherFd].length, buf);
            //更新fcb内容
            fcb* fcbPtr = (fcb *)(buf + sizeof(fcb) * openfilelist[fd].diroff);
            strcpy(fcbPtr->exname, openfilelist[fd].exname);
            strcpy(fcbPtr->filename, openfilelist[fd].filename);
            fcbPtr->first = openfilelist[fd].first;
            fcbPtr->free = openfilelist[fd].free;
            fcbPtr->length = openfilelist[fd].length;
            openfilelist[fatherFd].count = 0;
            fcbPtr->time = openfilelist[fd].time;
            fcbPtr->date = openfilelist[fd].date;
            fcbPtr->attribute = openfilelist[fd].attribute;
            openfilelist[fatherFd].count = openfilelist[fd].diroff * sizeof(fcb);
            do_write(fatherFd, (char*)fcbPtr, sizeof(fcb), 1);
        }
        // 释放openfilelist[fd], 用了个偷懒的办法,不知道行不行
        memset(&openfilelist[fd], 0, sizeof(USEROPEN));
        currfd = fatherFd;
        return fatherFd;
    }

}

void my_mkdir(char *dirname){
    //1. 判断dirname是否合法
    //2. 打开当前目录的目录文件,查找是否重名
    //3. 占用一个盘块, 在FAT里要写好这个盘块被占用了
    //4. 占用一个打开文件表项,这是因为一会要在这个目录文件里面写入默认的.和..两个特殊文件目录项
    //   而调用do_write,是需要先打开这个文件的.
    //   写入.和..两个文件目录项到你新建的目录文件里
    //5. 完毕这个openfilelist
    //6. 这一步比较难理解,需要修改父目录文件的大小,因为添加了一个文件目录项
    //例 你在\a\这个目录下创建了b这个目录, 那么a这个目录文件的大小要+=sizeof(fcb)

    //判断dirname是否合法
    char* fname = strtok(dirname,".");
    char* exname = strtok(NULL,".");
    if(exname){
        cout << "不允许输入后缀名!" << endl;
        return ;
    }
    char text[MAX_TEXT_SIZE];
    openfilelist[currfd].count = 0;
    //在打开文件表里找一个空文件表项
    int fileLen = do_read(currfd, openfilelist[currfd].length, text);
    //text里的内容就是一个个fcb
    fcb *fcbPtr = (fcb*)text;
    for(int i=0; i < (int)(fileLen/sizeof(fcb)); i++){
        if(strcmp(dirname,fcbPtr[i].filename) == 0 && fcbPtr->attribute == 0){
            cout << "目录名已经存在!"<<endl;
            return;
        }
    }
    //在打开文件表里找一个空文件表项
    int fd = getFreeOpenfilelist();
    if(fd == -1){
        cout << "打开文件表已全部被占用" << endl;
        return;
    }
    //在FAT里找一个空盘块
    unsigned short int blockNum = getFreeBLOCK();
    if(blockNum == END){
        cout << "盘块已经用完" << endl;
        openfilelist[fd].topenfile = 0;
        return ;
    }
    fat *fat1 = (fat *)(myvhard + BLOCKSIZE);
    fat *fat2 = (fat *)(myvhard + BLOCKSIZE*3);
    fat1[blockNum].id = END;
    fat2[blockNum].id = END;
    //在当前目录里面添加一个我们要的目录项
    int i = 0;
    for(; i < (int)(fileLen/sizeof(fcb)); i++){
        if(fcbPtr[i].free == 0){
            break;
        }
    }
    openfilelist[currfd].count = i * sizeof(fcb);
    //修改了fcb,fcbstate置1
    openfilelist[currfd].fcbstate = 1;
    //修改新建的目录项,即fcb内容,有趣的是,你不能直接修改fcbptr
    //因为我们现在是在模拟文件系统,我们要先写到临时的fcb里,然后用do_write转写到磁盘里
    fcb* fcbtmp = new fcb;
    fcbtmp->attribute = 0;
    time_t rawtime = time(NULL);
    struct tm* time = localtime(&rawtime);
    fcbtmp->date = (time->tm_year-100)*512 + (time->tm_mon+1)*32 + (time->tm_mday);
    fcbtmp->time = (time->tm_hour)*2048 + (time->tm_min)*32 + (time->tm_sec) / 2;
    strcpy(fcbtmp->filename , dirname);
    strcpy(fcbtmp->exname, "di");
    fcbtmp->first = blockNum;
    fcbtmp->length = 2 * sizeof(fcb);
    fcbtmp->free = 1;
    //用do_write把fcbtmp写入到目录文件里
    do_write(currfd,(char *)fcbtmp,sizeof(fcb),1);

    //设置打开文件表项
    openfilelist[fd].attribute = 0;
    openfilelist[fd].count = 0;
    openfilelist[fd].date = fcbtmp->date;
    openfilelist[fd].time = fcbtmp->time;
    openfilelist[fd].dirno = openfilelist[currfd].first;
    openfilelist[fd].diroff = i;
    strcpy(openfilelist[fd].exname,"di");
    strcpy(openfilelist[fd].filename,dirname);
    openfilelist[fd].fcbstate = 0;
    openfilelist[fd].first = fcbtmp->first;
    openfilelist[fd].free = fcbtmp->free;
    openfilelist[fd].length = fcbtmp->length;
    openfilelist[fd].topenfile = 1;
    // 修改 openfilelist[fd].dir[fd] = openfilelist[currfd].dir[currfd] + dirname;
    strcpy(openfilelist[fd].dir, (char*)(string(openfilelist[currfd].dir) + string(dirname) + string("\\")).c_str());

    //在对应的盘块里添加两个特殊的目录.和..
    //同理这里也不能直接写进fcbPtr里,只能用fcbTmp写进内存里,然后用do_wriite转写进文件
    fcbtmp->attribute = 0;
    fcbtmp->date = fcbtmp->date;
    fcbtmp->time = fcbtmp->time;
    strcpy(fcbtmp->filename, ".");
    strcpy(fcbtmp->exname, "di");
    fcbtmp->first = blockNum;
    fcbtmp->length = 2 * sizeof(fcb);
    do_write(fd,(char*)fcbtmp,sizeof(fcb),1);
    //复制..目录
    fcb *fcbtmp2 = new fcb;
    memcpy(fcbtmp2,fcbtmp,sizeof(fcb));
    strcpy(fcbtmp2->filename,"..");
    fcbtmp2->first = openfilelist[currfd].first;
    fcbtmp2->length = openfilelist[currfd].length;
    fcbtmp2->date = openfilelist[currfd].date;
    fcbtmp2->time = openfilelist[currfd].time;
    do_write(fd,(char*)fcbtmp2,sizeof(fcb),1);

    my_close(fd);
    //更新本currfd目录文件的fcb
    fcbPtr = (fcb *)text;
    fcbPtr->length =  openfilelist[currfd].length;
    openfilelist[currfd].count = 0;
    do_write(currfd,(char*)fcbPtr,sizeof(fcb),1);
    openfilelist[currfd].fcbstate = 1;
    delete fcbtmp;
    delete fcbtmp2;
}

//这里,我不允许删除非空的目录文件
void my_rmdir(char *dirname){
    //1. 把当前目录的目录文件读取到buf里
    //2. 在里面查找有没有匹配dirname的目录文件
    //3. 删除这个目录文件占用的所有盘块(也就是把他占用的fat全部释放,并且备份到fat2)
    //4. 更新当前目录文件,把dirname对应的fcb清除,并且更新当前目录文件的大小
    //例 你在\a\目录下,删除b这个目录,那么删除完之后,a这个目录文件的大小(length)要-=sizeof(fcb)
    //   并且,b这个目录文件是有一个fcb在a这个目录文件里的,把这个fcb也删掉

    char* fname = strtok(dirname, ".");
    char* exname = strtok(NULL, ".");
    //不允许删除.和..这两个特殊目录文件
    if(strcmp(dirname,".") == 0 || strcmp(dirname,"..") == 0){
        cout << "不能删除" << dirname <<"这个特殊目录项" <<endl;
        return ;
    }
    if(exname){
        cout << "删除目录文件不用输入后缀名!" << endl;
        return;
    }
    //读取currfd对应的目录文件到buf
    char buf[MAX_TEXT_SIZE];
    openfilelist[currfd].count = 0;
    do_read(currfd,openfilelist[currfd].length,buf);
    int i;
    fcb* fcbPtr = (fcb*)buf;
    //寻找叫这个名字的文件目录项
    for(i=0; i < int(openfilelist[currfd].length / sizeof(fcb)); i++, fcbPtr++){
        if(strcmp(fcbPtr->filename,fname) == 0  && fcbPtr->attribute == 0){
            break;
        }
    }
    if( i == int(openfilelist[currfd].length / sizeof(fcb))){
        cout << "没有这个目录文件" <<endl;
        return;
    }
    //判断这个目录文件里,清空了没有,我们不允许删除没有清空的目录
    if(fcbPtr->length > 2 * sizeof(fcb)){
        cout << "请先清空这个目录下的所有文件,再删除目录文件" << endl;
        return;
    }
    //清空这个目录项占据的FAT
    int blockNum = fcbPtr->first;
    fat* fat1 = (fat*)(myvhard + BLOCKSIZE);
    int next = 0;
    while(1){
        next = fat1[blockNum].id;
        fat1[blockNum].id = END;
        if(next != END){
            blockNum = next;
        }
        else{
            break;
        }
    }
    //备份fat2
    fat1 = (fat*)(myvhard + BLOCKSIZE);
    fat* fat2 = (fat*)(myvhard + BLOCKSIZE*3);
    memcpy(fat2, fat1, sizeof(fat));
    //修改这个fcb为空
    fcbPtr->date = 0;
    fcbPtr->time = 0;
    fcbPtr->exname[0] = '\0';
    fcbPtr->filename[0] = '\0';
    fcbPtr->first = 0;
    fcbPtr->free = 0;
    fcbPtr->length = 0;
    //写到磁盘上去, 更新fcb内容为空
    openfilelist[currfd].count = i * sizeof(fcb);
    do_write(currfd,(char*)fcbPtr,sizeof(fcb),1);
    openfilelist[currfd].length -= sizeof(fcb);
    //更新.目录项的长度
    fcbPtr = (fcb*)buf;
    fcbPtr->length = openfilelist[currfd].length;
    openfilelist[currfd].count = 0;
    do_write(currfd,(char*)fcbPtr,sizeof(fcb),1);
    openfilelist[currfd].fcbstate = 1;
}

int my_create(char *filename){
    //① 为新文件分配一个空闲打开文件表项，如果没有空闲表项则返回-1，并显示错误信息；
    //	② 若新文件的父目录文件还没有打开，则调用my_open()打开；若打开失败，则释放①中为新建文件分配的空闲文件打开表项，返回-1，并显示错误信息；q
    //	③ 调用do_read()读出该父目录文件内容到内存，检查该目录下新文件是否重名，若重名则释放①中分配的打开文件表项，并调用my_close()关闭②中打开的目录文件；然后返回-1，并显示错误信息；
    //	④ 检查FAT是否有空闲的盘块，如有则为新文件分配一个盘块，否则释放①中分配的打开文件表项，并调用my_close()关闭②中打开的目录文件；返回-1，并显示错误信息；
    //	⑤ 在父目录中为新文件寻找一个空闲的目录项或为其追加一个新的目录项;需修改该目录文件的长度信息，并将该目录文件的用户打开文件表项中的fcbstate置为1；
    //	⑥ 准备好新文件的FCB的内容，文件的属性为数据文件，长度为0，以覆盖写方式调用do_write()将其填写到⑤中分配到的空目录项中；
    //	⑦ 为新文件填写①中分配到的空闲打开文件表项，fcbstate字段值为0，读写指针值为0；
    //	⑧ 调用my_close()关闭②中打开的父目录文件；
    //	⑨ 将新文件的打开文件表项序号作为其文件描述符返回。


    //注释写到900行,基本上文件系统的操作流程你也懂了, 我就懒得继续写总结了
    char* fname = strtok(filename,".");
    char* exname = strtok(NULL,".");
    if(strcmp(fname,"") == 0){
        cout << "请输入文件名!" << endl;
        return -1;
    }
    if(!exname){
        cout << "请输入后缀名!" << endl;
        return -1;
    }
    if(openfilelist[currfd].attribute == 1){
        cout << "数据文件下不允许使用create" << endl;
        return -1;
    }
    //读取currfd对应的文件
    openfilelist[currfd].count = 0;
    char buf[MAX_TEXT_SIZE];
    do_read(currfd, openfilelist[currfd].length, buf);
    int i;
    fcb* fcbPtr = (fcb*)(buf);
    //看看有没有重名文件
    for(i=0; i < int(openfilelist[currfd].length / sizeof(fcb)); i++,fcbPtr++){
        if(strcmp(fcbPtr->filename,filename)==0 && strcmp(fcbPtr->exname,exname)==0){
            cout << "??????????!" << endl;
            return -1;
        }
    }
    //寻找空的fcb块
    fcbPtr = (fcb*)(buf);
    fcb* debug = (fcb*)(buf);
    for(i=0; i < int(openfilelist[currfd].length / sizeof(fcb)); i++,fcbPtr++){
        if(fcbPtr->free == 0)break;
    }
    //取一个盘块
    int blockNum = getFreeBLOCK();
    if(blockNum == -1){
        return -1;
    }
    fat* fat1 = (fat*)(myvhard + BLOCKSIZE);
    fat1[blockNum].id = END;
    fat* fat2 = (fat*)(myvhard + BLOCKSIZE * 3);
    memcmp(fat2, fat1, BLOCKSIZE*2);
    //往fcb里写信息
    strcpy(fcbPtr->filename,filename);
    strcpy(fcbPtr->exname,exname);
    time_t rawtime = time(NULL);
    struct tm* time = localtime(&rawtime);
    fcbPtr->date = (time->tm_year-100)*512 + (time->tm_mon+1)*32 + (time->tm_mday);
    fcbPtr->time = (time->tm_hour)*2048 + (time->tm_min)*32 + (time->tm_sec) / 2;
    fcbPtr->first = blockNum;
    fcbPtr->free = 1;
    fcbPtr->length = 0;
    fcbPtr->attribute = 1;
    openfilelist[currfd].count = i * sizeof(fcb);
    do_write(currfd,(char *)fcbPtr,sizeof(fcb),1);
    //修改当前目录文件的.目录项的长度
    fcbPtr = (fcb*)buf;
    fcbPtr->length = openfilelist[currfd].length;
    openfilelist[currfd].count = 0;
    do_write(currfd,(char*)fcbPtr,sizeof(fcb),1);
    openfilelist[currfd].fcbstate = 1;
}

void my_rm(char *filename){
    //怕后人看不懂, 还是继续写总结好了
    //例 你现在在\a\目录下
    //1. 读取当前目录的目录文件(a)到buf里(buf里是一个个fcb)
    //2. 在buf里寻找匹配filename的数据文件
    //3. 清空这个文件占据的盘块(也就是释放它占据的fat,并且备份)
    //4. 在父目录文件里,删除filename对应的fcb   (a里删除b的fcb)
    //5. 更新父目录文件的长度(a的目录文件长度)

    char* fname = strtok(filename, ".");
    char* exname = strtok(NULL, ".");
    if(!exname){
        cout << "请输入后缀名!" << endl;
        return;
    }
    if(strcmp(exname,"di") == 0){
        cout << "不能删除目录项" << endl;
        return ;
    }
    //读取currfd对应的目录文件到buf
    char buf[MAX_TEXT_SIZE];
    openfilelist[currfd].count = 0;
    do_read(currfd,openfilelist[currfd].length,buf);
    int i;
    fcb* fcbPtr = (fcb*)buf;
    //寻找叫这个名字的文件目录项
    for(i=0; i < int(openfilelist[currfd].length / sizeof(fcb)); i++, fcbPtr++){
        if(strcmp(fcbPtr->filename,fname) == 0  && strcmp(fcbPtr->exname,exname) == 0){
            break;
        }
    }
    if( i == int(openfilelist[currfd].length / sizeof(fcb))){
        cout << "没有这个文件" <<endl;
        return;
    }
    //清空这个目录项占据的FAT
    int blockNum = fcbPtr->first;
    fat* fat1 = (fat*)(myvhard + BLOCKSIZE);
    int next = 0;
    while(1){
        next = fat1[blockNum].id;
        fat1[blockNum].id = FREE;
        if(next != END){
            blockNum = next;
        }
        else{
            break;
        }
    }
    //备份fat2
    fat1 = (fat*)(myvhard + BLOCKSIZE);
    fat* fat2 = (fat*)(myvhard + BLOCKSIZE*3);
    memcpy(fat2, fat1, sizeof(fat));
    //修改这个fcb为空
    fcbPtr->date = 0;
    fcbPtr->time = 0;
    fcbPtr->exname[0] = '\0';
    fcbPtr->filename[0] = '\0';
    fcbPtr->first = 0;
    fcbPtr->free = 0;
    fcbPtr->length = 0;
    //写到磁盘上去, 更新fcb内容为空
    openfilelist[currfd].count = i * sizeof(fcb);
    do_write(currfd,(char*)fcbPtr,sizeof(fcb),1);
    openfilelist[currfd].length -= sizeof(fcb);
    //更新.目录项的长度
    fcbPtr = (fcb*)buf;
    fcbPtr->length = openfilelist[currfd].length;
    openfilelist[currfd].count = 0;
    do_write(currfd,(char*)fcbPtr,sizeof(fcb),1);
    openfilelist[currfd].fcbstate = 1;
}

void my_ls(){
    //① 调用do_read()读出当前目录文件内容到内存；
    //	② 将读出的目录文件的信息按照一定的格式显示到屏幕上；
    //	③ 返回。


    if(openfilelist[currfd].attribute == 1){
        cout << "在数据文件里不能使用ls" << endl;
        return;
    }
    char buf[MAX_TEXT_SIZE];

    openfilelist[currfd].count = 0;
    do_read(currfd, openfilelist[currfd].length, buf);

    fcb* fcbPtr = (fcb*)buf;
    for(int i=0; i < (int)(openfilelist[currfd].length / sizeof(fcb)); i++){
        if(fcbPtr->free == 1){
            //目录文件
            //同理,年份占7位,月份占4位,日期占5位
            //小时占5位,分钟占6位,秒占5位
            if(fcbPtr->attribute == 0){
                printf("%s\\\t<DIR>\t%d/%d/%d\t%d:%d:%d\n",
                       fcbPtr->filename,
                       (fcbPtr->date>>9)+2000,
                       (fcbPtr->date>>5)&0x000f,
                       (fcbPtr->date)&0x001f,
                       (fcbPtr->time>>11),
                       (fcbPtr->time>>5)&0x003f,
                       (fcbPtr->time)&0x001f * 2);
            }
            else{
                // length - 2 是因为末尾有/n和/0两个字符
                unsigned int length = fcbPtr->length;
                if(length != 0)
                    length -= 2;
                printf("%s.%s\t%dB\t%d/%d/%d\t%02d:%02d:%02d\n",
                       fcbPtr->filename,
                       fcbPtr->exname,
                       length,
                       (fcbPtr->date>>9)+2000,
                       (fcbPtr->date>>5)&0x000f,
                       (fcbPtr->date)&0x001f,
                       (fcbPtr->time>>11),
                       (fcbPtr->time>>5)&0x003f,
                       (fcbPtr->time)&0x001f * 2);
            }
        }
        fcbPtr++;
    }
}

void show_help(){
    printf("命令名\t\t命令参数\t\t命令说明\n\n");
    printf("cd\t\t目录名(路径名)\t\t切换当前目录到指定目录\n");
    printf("mkdir\t\t目录名\t\t\t在当前目录创建新目录\n");
    printf("rmdir\t\t目录名\t\t\t在当前目录删除指定目录\n");
    printf("ls\t\t无\t\t\t显示当前目录下的目录和文件\n");
    printf("create\t\t文件名\t\t\t在当前目录下创建指定文件\n");
    printf("rm\t\t文件名\t\t\t在当前目录下删除指定文件\n");
    printf("open\t\t文件名\t\t\t在当前目录下打开指定文件\n");
    printf("write\t\t无\t\t\t在打开文件状态下，写该文件\n");
    printf("read\t\t无\t\t\t在打开文件状态下，读取该文件\n");
    printf("close\t\t无\t\t\t在打开文件状态下，关闭该文件\n");
    printf("exit\t\t无\t\t\t退出系统\n\n");
}

void show_big_file(){
    for(int i=0; i<500; i++){
        cout << i << ",";
    }
    cout << endl;
}
void error(char *command){
    printf("%s : 缺少参数\n", command);
    printf("输入 'help' 来查看命令提示.\n");
}
int main(){
    char cmd[15][10] = {"mkdir", "rmdir", "ls", "cd", "create", "rm", "open", "close", "write", "read", "exit", "help","bigfile"};
    char command[30], *sp, *len, yesorno;
    int indexOfCmd, i;

    /*************************************************/
    printf("*********** 文件系统 **********\n");
    startSys();
    printf("开启文件系统.\n");
    printf("输入help来显示帮助页面.\n");

    while(1){
        printf("%s>", openfilelist[currfd].dir);
        gets(command);
        indexOfCmd = -1;
        if (strcmp(command, "")){
            sp = strtok(command, " ");  // 取下命令
            for (i = 0; i < 15; i++){
                if (strcmp(sp, cmd[i]) == 0){
                    indexOfCmd = i;
                    break;
                }
            }
            switch(indexOfCmd){
                case 0:         // mkdir
                    sp = strtok(NULL, " ");
                    if (sp != NULL)
                        my_mkdir(sp);
                    else
                        error("mkdir");
                    break;
                case 1:         // rmdir
                    sp = strtok(NULL, " ");
                    if (sp != NULL)
                        my_rmdir(sp);
                    else
                        error("rmdir");
                    break;
                case 2:         // ls
                    my_ls();
                    break;
                case 3:         // cd
                    sp = strtok(NULL, " ");
                    if (sp != NULL)
                        my_cd(sp);
                    else
                        error("cd");
                    break;
                case 4:         // create
                    sp = strtok(NULL, " ");
                    if (sp != NULL)
                        my_create(sp);
                    else
                        error("create");
                    break;
                case 5:         // rm
                    sp = strtok(NULL, " ");
                    if (sp != NULL)
                        my_rm(sp);
                    else
                        error("rm");
                    break;
                case 6:         // open
                    sp = strtok(NULL, " ");
                    if (sp != NULL)
                        my_open(sp);
                    else
                        error("open");
                    break;
                case 7:         // close
                    if (openfilelist[currfd].attribute == 1)
                        my_close(currfd);
                    else
                        cout << "当前没有的打开的文件" << endl;
                    break;
                case 8:         // write
                    if (openfilelist[currfd].attribute == 1)
                        my_write(currfd);
                    else
                        cout << "请先打开文件,然后再使用wirte操作" <<endl;
                    break;
                case 9:         // read
                    if (openfilelist[currfd].attribute == 1)
                        my_read(currfd);
                    else
                        cout << "请先打开文件,然后再使用read操作" <<endl;
                    break;
                case 10:        // exit
                    exitsys();
                    printf("退出文件系统.\n");
                    return 0;
                    break;
                case 11:        // help
                    show_help();
                    break;
                case 12:
                    show_big_file();
                    break;
                default:
                    printf("没有 %s 这个命令\n", sp);
                    break;
            }
        }
        else
            printf("\n");
    }
    return 0;
}
