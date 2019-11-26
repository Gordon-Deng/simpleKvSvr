#ifndef KV_DAOLAYER_H
#define KV_DAOLAYER_H

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
static const char* manFile = "KVDBFile/manFile";
static const char* filePrefix = "KVDB";

class SimpleKVStore
{
private:
public:
    Queue<Task*>* workQueue;
    Queue<Task*>* resultQueue;
    Thread* ioThread;
    DataNode** fileList;
    int fileNum;
    SimpleKVIndex* index;
    Cache* cache;
    int hits, miss;

    int keySize() { return index->index.size(); }

    int memSize() { return 0; }

    int fileSize()
    {
        int total = 0;
        for (int i = 0; i < fileNum; i++)
            total += fileList[i]->totalOffset;
        return total;
    }

    SimpleKVStore(int queueSize)
    {
        this->workQueue = new Queue<Task*>(queueSize);
        this->resultQueue = new Queue<Task*>(queueSize);
        fileList = new DataNode*[MAXFILE];
        this->ioThread = new Thread(&diskIO, this);
        this->cache = new Cache();
        this->index = new SimpleKVIndex();
        hits = 0;
        miss = 0;
    }

    ~SimpleKVStore()
    {
        delete workQueue;
        delete resultQueue;
        for (int i = 0; i < fileNum; i++)
            delete fileList[i];
        delete fileList;
        delete ioThread;
        delete index;
        delete cache;
    }

    void addTask(Task* task) { this->workQueue->enqueue(task); }

    Task* getResult() { return this->resultQueue->dequeue(); }

    DataNode* reBuild(DataNode* oldNode)
    {
        DataNode* newNode = new DataNode();
        strcpy(newNode->fileName, oldNode->fileName);
        strcpy(newNode->path, tmpDir);
        if (newNode->createAndOpen() < 0)
        {
            ERROR();
        }
        Record record;
        record.key = new char[MAXKEY];
        record.value = new char[MAXVALUE];
        int offset = 0;
        int n;
        RecordOffset oldOffset, newOffset;
        oldOffset.dataNode = oldNode;
        newOffset.dataNode = newNode;
        for (;;)
        {
            n = oldNode->getRecord(offset, &record);
            if (!n)
                break;
            if (n < 0)
            {
                ERROR();
                break;
            }
            //CRC
            oldOffset.offset = offset;
            newOffset.offset = newNode->totalOffset;
            offset += n;
            int wrote = newNode->appendRecord(&record);
            if (wrote < 0)
            {
                ERROR();
                continue;
            }

            if ((record.valueLen > 0) && (!index->compareAndSet(record.key, record.keyLen, &oldOffset, &newOffset)))
            {
                newNode->totalOffset -= n;
            }
        }
        ftruncate(newNode->fd, newNode->totalOffset);
        delete record.key;
        delete record.value;
        return newNode;
    }

    static void* rebuildFile(void* arg)
    {
        SimpleKVStore* thisDAO = (SimpleKVStore*)arg;
        DataNode** pFile = &(thisDAO->fileList[thisDAO->fileNum - 2]);
        DataNode* oldNode = *pFile;
        DataNode* newNode = thisDAO->reBuild(oldNode);
        *pFile = newNode;
        strcpy(newNode->path, oldNode->path);
        newNode->closeIt();
        rename(newNode->pathname, oldNode->pathname);
        newNode->openit();
        delete oldNode;
    }

    void initlization()
    {
        int fd = open(manFile, O_RDONLY);
        // if file do not exist, create
        if (fd < 0)
        {
            fileNum = 0;
            fd = open(manFile, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
            if (fd < 0)
                ERROR();
            if (write(fd, (char*)&fileNum, sizeof(fileNum)) != sizeof(fileNum))
                ERROR();
            close(fd);
            return;
        }
        if (read(fd, (char*)&fileNum, sizeof(fileNum)) != sizeof(fileNum))
        {
            ERROR();
        }
        RecordOffset position;
        Record record;
        record.key = new char[MAXKEY];
        record.value = new char[MAXVALUE];
        for (int i = 0; i < fileNum; i++)
        {
            int len;
            if (read(fd, (char*)&len, 4) < 4)
                ERROR();
            fileList[i] = new DataNode();
            if (read(fd, fileList[i]->fileName, len) < len)
                ERROR();
            strcpy(fileList[i]->path, dir);
            if (fileList[i]->openit() < 0)
                ERROR();
            position.dataNode = fileList[i];
            position.offset = 0;
            for (;;)
            {
                int n = fileList[i]->getRecord(position.offset, &record);
                if (n == 0)
                {
                    break;
                }
                if (n < 0)
                {
                    ERROR();
                    break;
                }
                if (record.valueLen > 0)
                {
                    index->saveVal(record.key, record.keyLen, &position);
                }
                position.offset += n;
                fileList[i]->totalOffset += n;
            }
        }
        delete (record.key);
        delete (record.value);
        close(fd);
    }

    void itoa(int num, char* buf)
    {
        int array[20];
        int len = 0;
        while (num)
        {
            array[len++] = num % 10;
            num = num / 10;
        }
        for (int i = 0; i < len; i++)
            buf[i] = array[len - 1 - i] + '0';
        buf[len] = 0;
    }

    void check()
    {
        //find new place to insert
        if (!fileNum || fileList[fileNum - 1]->totalOffset > OFFLIMIT)
        {
            if (fileNum >= MAXFILE)
                ERROR();
            fileList[fileNum] = new DataNode();
            char tmp[128], tmp2[128];
            itoa(fileNum, tmp2);
            strcpy(tmp, filePrefix);
            strcat(tmp, tmp2);
            strcpy(fileList[fileNum]->fileName, tmp);
            strcpy(fileList[fileNum]->path, dir);
            if (fileList[fileNum]->createAndOpen() < 0)
                ERROR();
            fileNum++;
            int fd;
            if ((fd = open(manFile, O_RDWR)) < 0)
                ERROR();
            if (lseek(fd, 0, SEEK_SET) == -1)
                ERROR(strerror(errno));
            if (write(fd, (char*)&fileNum, sizeof(fileNum)) != sizeof(fileNum))
                ERROR();
            if (lseek(fd, 0, SEEK_END) == -1)
                ERROR();
            int len = strlen(fileList[fileNum - 1]->fileName);
            if (write(fd, (char*)&len, 4) != 4)
                ERROR();
            if (write(fd, fileList[fileNum - 1]->fileName, len) != len)
                ERROR();
            if (fileNum > 1)
            {
                pthread_t tid;
                if (pthread_create(&tid, NULL, &rebuildFile, (void*)this) < 0)
                    ERROR();
            }
            close(fd);
        }
    }

    void getVal(Task* task)
    {
        task->state = SUCCESS;
        //	Task *tmp = cache->getCache(string(task->key, task->keyLen));
        //	if (0){
        //		hits++;
        //		memcpy(task->value,tmp->value,tmp->valueLen);
        //		task->valueLen = tmp->valueLen;
        //		return;
        //	}
        miss++;
        Record record;
        RecordOffset position;
        //where the value will save;
        record.value = task->value;
        record.key = task->key;
        if (!index->getVal(task->key, task->keyLen, &position)
            || position.dataNode->getRecord(position.offset, &record) < 0)
        {
            task->state = FAIL;
            return;
        }
        //CRC examination
        task->valueLen = record.valueLen;
        //	cache->addToCache(task);
    }

    void setVal(Task* task)
    {
        //	cache->deleteCache(string(task->key,task->keyLen));
        //	cache->addToCache(task);
        task->state = SUCCESS;
        Record record;
        RecordOffset position;
        record.key = task->key;
        record.keyLen = task->keyLen;
        record.value = task->value;
        record.valueLen = task->valueLen;
        record.CRC = 0;
        check();
        position.dataNode = fileList[fileNum - 1];
        position.offset = position.dataNode->totalOffset;
        int n = fileList[fileNum - 1]->appendRecord(&record);
        if (n < 0)
        {
            task->state = FAIL;
            return;
        }
        if (!index->saveVal(task->key, task->keyLen, &position))
        {
            task->state = FAIL;
        }
    }

    void delVal(Task* task)
    {
        //	cache->deleteCache(string(task->key,task->keyLen));
        task->state = SUCCESS;
        Record record;
        record.key = task->key;
        record.keyLen = task->keyLen;
        record.value = task->value;
        record.valueLen = 0;
        record.CRC = 0;
        check();
        int n = fileList[fileNum - 1]->appendRecord(&record);
        if (n < 0)
        {
            task->state = FAIL;
            return;
        }
        if (!index->delKey(task->key, task->keyLen))
        {
            task->state = FAIL;
        }
    }

    void processTask(Task* task)
    {
        switch (task->CMD)
        {
            case GET:
                getVal(task);
                //CRC should be examed;
                break;
            case SET:
                setVal(task);
                break;
            case DEL:
                delVal(task);
                break;
            default:
                task->state = FAIL;
                break;
        }
    }

    static void* diskIO(void* arg)
    {
        SimpleKVStore* thisDAO = (SimpleKVStore*)arg;
        thisDAO->initlization();
        Task* task;
        for (;;)
        {
            task = thisDAO->workQueue->dequeue();
            thisDAO->processTask(task);
            thisDAO->resultQueue->enqueue(task);
        }
    }
};

#endif
