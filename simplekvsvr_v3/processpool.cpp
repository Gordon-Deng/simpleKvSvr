#include "processpool.h"

namespace mminternpractice {

//Process
    Process::Process() : m_pid(-1), m_pTarget(NULL) {}

    Process::Process(Runnable &t) : m_pid(-1), m_pTarget(&t) {}

    Process::~Process() {
        if (m_pid > 1)
            Wait();
    }

    int Process::Start() {
        m_pid = fork();
        if (m_pid < 0)
            return -1;
        else if (m_pid == 0) {
            exit(Run());
        }
        return 0;
    }

    int Process::Kill() {
        return kill(m_pid, SIGINT);
    }

    int Process::Wait() {
        if (m_pid > 1)
            return waitpid(m_pid, NULL, 0);
        return -1;
    }

    int Process::Run() {
        if (m_pTarget != NULL)
            return m_pTarget->Run();
        return -1;
    }

//ProcessPool
    ProcessPool::ProcessPool() : m_processes(NULL), m_processes_len(0) {}

    ProcessPool::~ProcessPool() {
        if (m_processes != NULL) {
            WaitAll();
            delete[] m_processes;
        }
    }

    int ProcessPool::Start(int procCount, Runnable &target) {
        m_processes = new Process[procCount];
        if (m_processes == NULL)
            return -1;
        m_processes_len = procCount;
        for (unsigned int i = 0; i < m_processes_len; ++i) {
            m_processes[i].SetTarger(target);
            m_processes[i].Start();
        }
        return 0;
    }

    int ProcessPool::KillAll() {
        for (unsigned int i = 0; i < m_processes_len; ++i) {
            m_processes[i].Kill();
        }
        return 0;
    }

    int ProcessPool::WaitAll() {
        for (unsigned int i = 0; i < m_processes_len; ++i) {
            m_processes[i].Wait();
        }
        return 0;
    }

//Processor
    TaskProcessor::TaskProcessor() : m_pQueue(NULL) {}

    TaskProcessor::~TaskProcessor() {}

    int TaskProcessor::Run() {
        char *data;
        unsigned int data_len;
        while (true) {
            if (m_pQueue->Dequeue(data, &data_len) < 0)
                break;
            printf("data_len[%d]\n", data_len);
            m_pQueue->debug();
            ((Task *) data)->DoTask(m_ptr);
            free(data);
        }
        return 0;
    }

//TaskProcessPool
    TaskProcessPool::TaskProcessPool(const long key, unsigned int queue_size, void *ptr) {
        queue_size = queue_size > 100 ? queue_size : 100;
        int ret;
        if ((ret = CShareQueue::CShareQueueAlloc(key, queue_size, false, m_pQueue)) != 0) {
            m_pQueue = NULL;
            printf("CShareQueueAlloc Error[%d]!\n", ret);
            return;
        }
        m_processor.SetParams(m_pQueue, ptr);
    }

    TaskProcessPool::~TaskProcessPool() {
        if (m_pQueue != NULL) {
            m_pQueue->Shutdown();
            int ret;
            if ((ret = CShareQueue::CShareQueueRelease(m_pQueue, true)) != 0) {
                printf("CShareQueueRelease Error[%d]!\n", ret);
                return;
            }
        }
    }

    int TaskProcessPool::Start(int processCount) {
        return m_pool.Start(processCount, m_processor);
    }

    int TaskProcessPool::AddTask(Task &task, unsigned int size) {
        return m_pQueue->Enqueue((char *) &task, size);
    }

    int TaskProcessPool::KillAll() {
        return m_pool.KillAll();
    }

    int TaskProcessPool::WaitAll() {
        return m_pool.WaitAll();
    }

} // namespace mminternpractice
