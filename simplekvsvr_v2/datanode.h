#ifndef DATANODE_H
#define DATANODE_H

#include <fcntl.h>
#include <iostream>
#include <cstring>
#include "record.h"
#include "error.h"
#include "bug.h"

using namespace std;

#define MAXNAME 256
#define OFFLIMIT 10 * 1024 * 1024

class DataNode
{
public:
    char path[MAXNAME];
    char fileName[MAXNAME];
    char pathname[MAXNAME * 2];
    int totalOffset;
    int fd;

    DataNode()
    {
        path[0] = fileName[0] = 0;
        totalOffset = 0;
    }

    ~DataNode() { close(fd); }

    void closeIt() { close(fd); }

    char* getPathname()
    {
        strcpy(pathname, path);
        if (pathname[strlen(pathname) - 1] != '/')
        {
            strcat(pathname, "/");
        }
        strcat(pathname, fileName);
        // cout << "PATHNAME:" << pathname << endl;
        return pathname;
    }

    int openit()
    {
        // O_RDWR 读写
        fd = open(getPathname(), O_RDWR);
        return fd;
    }

    int createAndOpen()
    {
        fd = open(getPathname(), O_RDWR | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR);
        return fd;
    }

    int getRecord(int offset, Record* record)
    {
        if (lseek(fd, offset, SEEK_SET) < 0)
        {
            ERROR();
            return -1;
        }
        int toRead, haveRead;
        toRead = sizeof(record->CRC) + sizeof(record->keyLen) + sizeof(record->valueLen);
        haveRead = 0;
        int n;
        if ((n = read(fd, (char*)record, toRead)) != toRead)
        {
            if (n == 0)
                return 0;
            ERROR();
            return -1;
        }
        haveRead += toRead;
        toRead = record->keyLen;
        if (read(fd, record->key, toRead) != toRead)
        {
            ERROR();
            return -1;
        }
        haveRead += toRead;
        toRead = record->valueLen;
        if (read(fd, record->value, toRead) != toRead)
        {
            ERROR();
            return -1;
        }
        haveRead += toRead;
        return haveRead;
    }

    int appendRecord(const Record* record)
    {
        if (lseek(fd, 0, SEEK_END) < 0)
        {
            ERROR();
            return -1;
        }
        int toWrite, wrote = 0;
        toWrite = sizeof(record->CRC) + sizeof(record->keyLen) + sizeof(record->valueLen);
        if (write(fd, (char*)record, toWrite) != toWrite)
        {
            ERROR();
            return -1;
        }
        wrote += toWrite;
        toWrite = record->keyLen;
        if (write(fd, record->key, toWrite) != toWrite)
        {
            ERROR();
            return -1;
        }
        wrote += toWrite;
        if (record->valueLen > 0)
        {
            toWrite = record->valueLen;
            if (write(fd, record->value, toWrite) != toWrite)
            {
                ERROR();
                return -1;
            }
            wrote += toWrite;
        }
        this->totalOffset += wrote;
        return wrote;
    }
};

#endif
