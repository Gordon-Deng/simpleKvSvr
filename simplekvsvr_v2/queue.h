#ifndef TASKQUEUE_H
#define TASKQUEUE_H

#include <semaphore.h>
#include <pthread.h>
#include <iostream>

#define QUEUE_SIZE 1024

template<typename T>
class Queue
{
private:
    T* queue;
    sem_t full, empty, enqueue_mutex, dequeue_mutex;
    int head, tail, size;

public:
    Queue(int queueSize = 1024)
    {
        size = queueSize;
        head = 0;
        tail = 0;
        queue = new T[queueSize];
        sem_init(&full, 0, 0);
        sem_init(&empty, 0, queueSize);
        sem_init(&enqueue_mutex, 0, 1);
        sem_init(&dequeue_mutex, 0, 1);
    }

    ~Queue() { delete[] queue; }

    void enqueue(T obj)
    {
        sem_wait(&empty);
        sem_wait(&enqueue_mutex);
        queue[head] = obj;
        head = (head + 1) % size;
        sem_post(&enqueue_mutex);
        sem_post(&full);
    }

    T dequeue()
    {
        T ret;
        sem_wait(&full);
        sem_wait(&dequeue_mutex);
        ret = queue[tail];
        tail = (tail + 1) % size;
        sem_post(&dequeue_mutex);
        sem_post(&empty);
        return ret;
    }

    int used()
    {
        int ret;
        sem_getvalue(&full, &ret);
        return ret;
    }
};

#endif
