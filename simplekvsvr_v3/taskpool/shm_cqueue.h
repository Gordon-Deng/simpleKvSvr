/*
 * demo
 * Created/Update : 2014-11-01/2014-11-01
 * Author : phoneli<phoneli.wc.li@gmail.com>
 */

#ifndef _SHM_CQUEUE_H_
#define _SHM_CQUEUE_H_

#include <iostream>

#include <pthread.h>
#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#define Lock_Safe(mutex)  do { while ( pthread_mutex_lock(mutex) != 0 ); } while(0)
#define UnLock_Safe(mutex)  do { while ( pthread_mutex_unlock(mutex) != 0 ); } while(0)

namespace mminternpractice {
    const unsigned int g_const_node_size = 2 << 8;

    struct CShareQueueNode {
        unsigned int m_nodes;
        unsigned int m_Len;//m_bIsEnd == true , mLen is real dataLen
        char m_buf[g_const_node_size];
    };

    class CShareQueue {
    public:
        static int CShareQueueAlloc(const long key, const unsigned int queue_len,
                                    const bool bIsExist, CShareQueue *&pCShareQueue);

        static int CShareQueueRelease(CShareQueue *pCShareQueue, const bool bNeedDelete);

        bool FullQueue();

        bool EmptyQueue();

        int Enqueue(const char *buf, const unsigned int buf_len);

        int Dequeue(char *&buf, unsigned int *buf_len);

        int Shutdown();

        bool IsShutdown() const { return m_bShutDown; };

    private:
        CShareQueue(const long key, const int shmid, int maxsize);

        ~CShareQueue();

        unsigned int NeedNodes(unsigned int data_len) {
            return (data_len - 1) / g_const_node_size + 1;
        }

        unsigned int EmptyNodes() {
            unsigned int ret = 0;
            if (m_iRear >= m_iFront)
                ret = m_iMaxSize - 1 - (m_iRear - m_iFront);
            else
                ret = m_iFront - m_iRear - 1;
            assert(ret <= m_iMaxSize - 1 && ret >= 0);
            return ret;
        }

    public:
        void debug() {
            printf("key[%ld] , shmid[%d] , front[%u] , rear[%u] , maxsize[%u] , shutdown[%d]\n",
                   m_lKey, m_iShmId, m_iFront, m_iRear, m_iMaxSize, m_bShutDown);
        }

    public:
        const long m_lKey;
        const int m_iShmId;

    private:
        unsigned int m_iFront;
        unsigned int m_iRear;
        unsigned int m_iMaxSize;

        volatile bool m_bShutDown;
        pthread_mutex_t m_mutex;
        pthread_cond_t m_condNotEmpty;
        pthread_cond_t m_condNotFull;

        struct CShareQueueNode m_nodes[0];
    };

}//mminternpractice

#endif
