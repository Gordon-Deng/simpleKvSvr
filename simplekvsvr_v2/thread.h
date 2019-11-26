#ifndef THREAD_H
#define THREAD_H

#include "error.h"
#include "bug.h"
#include "pthread.h"

class Thread
{
private:
public:
    pthread_t tid;


    Thread(void* (*function)(void*), void* arg)
    {
        // 绑定function
        if (pthread_create(&tid, NULL, function, arg) < 0)
            ERROR();
    }

    // 析构释放
    ~Thread()
    {
        void* ptr;
        pthread_join(this->tid, &ptr);
    }
};

#endif
