#ifndef SIMPLEKV_INDEX_H
#define SIMPLEKV_INDEX_H

#include <string>
#include <map>
#include <pthread.h>
#include "datanode.h"
#include "recordoffset.h"

using namespace std;

class SimpleKVIndex
{
private:
public:
    map<string, RecordOffset> index;
    map<string, RecordOffset>::iterator it;
    pthread_mutex_t mutex;

    SimpleKVIndex()
    {
        // 互斥锁
        //mutex = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_init(&mutex, 0);
    }

    bool saveVal(const char* str, const int len, const RecordOffset* position)
    {
        pthread_mutex_lock(&mutex);
        string s(str, len);
        index[s] = *position;
        pthread_mutex_unlock(&mutex);
        return true;
    }

    bool getVal(const char* str, const int len, RecordOffset* position)
    {
        pthread_mutex_lock(&mutex);
        it = index.find(string(str, len));
        if (it == index.end())
        {
            pthread_mutex_unlock(&mutex);
            return false;
        }
        *position = it->second;
        pthread_mutex_unlock(&mutex);
        return true;
    }

    bool delKey(const char* str, const int len)
    {
        pthread_mutex_lock(&mutex);
        it = index.find(string(str, len));
        if (it != index.end())
        {
            index.erase(it);
            pthread_mutex_unlock(&mutex);
            return true;
        }
        pthread_mutex_unlock(&mutex);
        return false;
    }

    bool compareAndSet(const char* str, const int len, const RecordOffset* oldVal, const RecordOffset* newVal)
    {
        pthread_mutex_lock(&mutex);
        it = index.find(string(str, len));
        if (it != index.end())
        {
            RecordOffset recordOffset = it->second;
            if (recordOffset.dataNode == oldVal->dataNode && recordOffset.offset == oldVal->offset)
            {
                it->second = *newVal;
                pthread_mutex_unlock(&mutex);
                return true;
            }
        }
        pthread_mutex_unlock(&mutex);
        return false;
    }
};

#endif
