#include <string>
#include "unistd.h"
#include "thread.h"
#include "queue.h"
#include "datanode.h"
#include "record.h"
#include "recordoffset.h"
#include "simplekvindex.h"
#include "task.h"
#include "error.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "errno.h"
#include "cache.h"

#define MAXFILE 1024

static const char* dir = "KVDBFile";
static const char* tmpDir = "./tmpDir";
static const char* manFile = "KVDBFile/Data";
static const char* filePrefix = "KVDB";

int main(int argc, const char** argv)
{
    int fd = 0;
    char block[1] = {'1'};
    // int fileNum = 1;
    fd = open(manFile, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    write(fd, &block, sizeof(block));
    std::cout << block << std::endl;

    // int fileNum = 0;
    // fd = open(manFile, O_RDONLY);
    // read(fd, (char*)&fileNum, sizeof(fileNum)) != sizeof(fileNum);
    // std::cout << (char*)&fileNum << std::endl;
    // std::cout << fileNum << std::endl;
    // if (read(fd, (char*)&fileNum, sizeof(fileNum)) != sizeof(fileNum))
    // {
    //     std::cout << fileNum << std::endl;
    //     ERROR();
    // }
    // if (fd < 0)
    //     ERROR();
    // if (write(fd, &block, sizeof(block)) != sizeof(block))
    //     ERROR();
    // read(fd, &block_2, sizeof(block_2));
    // printf("^^^^ = %s\n", block_2);
    return 0;
}