#ifndef KV_LOGICLAYER_H
#define KV_LOGICLAYER_H

#include <iostream>
#include "queue.h"
#include "simplekvstore.h"
#include "thread.h"
#include "task.h"
#include "string.h"

using namespace std;

class KVLogicLayer
{
private:
public:
    SimpleKVStore* KVStore;
    Queue<Task*>* workQueue;
    Queue<Task*>* resultQueue;
    Thread **inThrList, **outThrList;
    int inThrNum, outThrNum;

    void errorReq(Task* task)
    {
        strcpy(task->res, "+ERROR!\n");
        task->resLen = strlen("+ERROR!\n");
        this->resultQueue->enqueue(task);
    }

    void addTask(Task* task) { this->workQueue->enqueue(task); }

    Task* getResult() { return this->resultQueue->dequeue(); }

    static void* inThread(void* arg)
    {
        // 运行时再指明类型
        KVLogicLayer* thisLayer = (KVLogicLayer*)arg;
        Task* task;
        for (;;)
        {
            task = thisLayer->workQueue->dequeue();
            if (task->buf[0] != '-')
            {
                thisLayer->errorReq(task);
                continue;
            }
            if (memcmp(task->buf + 1, "get ", strlen("get ")) == 0)
            {
                task->key = task->buf + 1 + strlen("get ");
                task->keyLen = task->buf + task->n - task->key - 1;
                task->value = task->res + 1;
                task->CMD = GET;
            }
            else if (memcmp(task->buf + 1, "set ", strlen("set ")) == 0)
            {
                task->key = task->buf + 1 + strlen("set ");
                char* space = task->key + 1;
                while ((*space != ' ') && (*space != '\n'))
                    space++;
                if ((space[0] == '\n') || (space[1] == '\n'))
                {
                    thisLayer->errorReq(task);
                    continue;
                }
                task->keyLen = space - task->key;
                task->value = space + 1;
                task->valueLen = task->buf + task->n - 1 - task->value;
                task->CMD = SET;
            }
            else if (memcmp(task->buf + 1, "delete ", strlen("delete ")) == 0)
            {
                task->key = task->buf + 1 + strlen("delete ");
                task->keyLen = task->buf + task->n - 1 - task->key;
                task->CMD = DEL;
            }
            else if (memcmp(task->buf + 1, "quit", strlen("quit")) == 0)
            {
                task->state = SUCCESS;
                task->CMD = QUIT;
                strcpy(task->res, "+OK\n");
                task->resLen = strlen("+OK\n");
                thisLayer->resultQueue->enqueue(task);
                continue;
            }
            else if (memcmp(task->buf + 1, "stats\n", strlen("stats\n")) == 0)
            {
                task->state = SUCCESS;
                task->CMD = STATS;
                sprintf(task->res, "+count: %d,mem: %d,file: %d, hits: %d, misses: %d\n", thisLayer->KVStore->keySize(),
                        thisLayer->KVStore->memSize(), thisLayer->KVStore->fileSize(), thisLayer->KVStore->hits,
                        thisLayer->KVStore->miss);
                task->resLen = strchr(task->res, '\n') - task->res + 1;
                thisLayer->resultQueue->enqueue(task);
                continue;
            }
            else
            {
                thisLayer->errorReq(task);
                continue;
            }
            thisLayer->KVStore->addTask(task);
        }
    }

    static void* outThread(void* arg)
    {
        KVLogicLayer* thisLayer = (KVLogicLayer*)arg;
        Task* task;
        for (;;)
        {
            task = thisLayer->KVStore->getResult();
            if (task->state == FAIL)
            {
                thisLayer->errorReq(task);
                continue;
            }
            switch (task->CMD)
            {
                case GET:
                    task->res[0] = '+';
                    task->res[task->valueLen + 1] = '\n';
                    task->resLen = task->valueLen + 2;
                    break;
                case SET:
                case DEL:
                case QUIT:
                    strcpy(task->res, "+OK\n");
                    task->resLen = strlen("+OK\n");
                    break;
            }
            thisLayer->resultQueue->enqueue(task);
        }
    }

    KVLogicLayer(SimpleKVStore* KVStore, int queueSize, int inNum, int outNum)
    {
        this->KVStore = KVStore;
        this->workQueue = new Queue<Task*>(queueSize);
        this->resultQueue = new Queue<Task*>(queueSize);
        inThrNum = inNum;
        outThrNum = outNum;
        inThrList = new Thread*[inThrNum];
        outThrList = new Thread*[outThrNum];
        for (int i = 0; i < inThrNum; i++)
        {
            inThrList[i] = new Thread(&inThread, this);
        }
        for (int i = 0; i < outThrNum; i++)
        {
            outThrList[i] = new Thread(&outThread, this);
        }
    }
};

#endif
