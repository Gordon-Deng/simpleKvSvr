#ifndef MM_TRAINING_PROCESS_POOL_H
#define MM_TRAINING_PROCESS_POOL_H

#include <string>
#include <deque>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

#include "runnable.h"
#include "taskpool/shm_cqueue.h"

namespace mminternpractice {

    class Process {
    public:
        Process();

        Process(Runnable &t);

        virtual ~Process();

        int Start();

        int Kill();

        int Wait();

        void SetTarger(Runnable &t) { m_pTarget = &t; };

        int GetId() const { return m_pid; };

    private:
        int Run();

    private:
        pid_t m_pid;

        Runnable *m_pTarget;
    };

    class ProcessPool {
    public:
        ProcessPool();

        ~ProcessPool();

        int Start(int procCount, Runnable &target);

        int KillAll();

        int WaitAll();

    private:
        Process *m_processes;
        unsigned int m_processes_len;
    };

    class Task {
    public:
        virtual ~Task() {}

        virtual int DoTask(void *ptr) = 0;
    };

    class TaskProcessor : public Runnable {
    public:
        TaskProcessor();

        ~TaskProcessor();

        void SetParams(CShareQueue *q, void *ptr) {
            m_pQueue = q;
            m_ptr = ptr;
        }

        int Run();

    private:
        CShareQueue *m_pQueue;
        void *m_ptr;
    };

    class TaskProcessPool {
    public:
        TaskProcessPool(const long key, unsigned int queue_size, void *ptr);

        ~TaskProcessPool();

        int Start(int processCount);

        int AddTask(Task &task, unsigned int size);

        int KillAll();

        int WaitAll();

    private:
        CShareQueue *m_pQueue;
        TaskProcessor m_processor;
        ProcessPool m_pool;
    };
}
#endif
