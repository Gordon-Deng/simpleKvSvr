#include "shm_cqueue.h"

namespace mminternpractice {

    int CShareQueue::CShareQueueAlloc(const long key, const unsigned int queue_len,
                                      const bool bIsExist,
                                      CShareQueue *&pCShareQueue) {
        unsigned int size =
                queue_len * sizeof(struct CShareQueueNode) + sizeof(CShareQueue);
        // printf("Alloc : size[%u]\n" , size);

        int shmid;
        if ((shmid = shmget(key, 0, 0)) < 0) {
            if (bIsExist || (shmid = shmget(key, size, 0666 | IPC_CREAT)) < 0)
                return -1;
            printf("Alloc key[%ld] , shmid[%d] , size[%u]\n", key, shmid, size);
        } else if (bIsExist) {
            struct shmid_ds ds;
            if (shmctl(shmid, IPC_STAT, &ds) < 0) {
                return -2;
            }
            if (ds.shm_segsz != size) {
                return -3;
            }
        } else if (!bIsExist) {
            return -4;
        }

        void *buffer = NULL;
        if ((buffer = shmat(shmid, NULL, 0)) == (void *) -1) {
            return -5;
        }
        if (!bIsExist) {
            memset(buffer, 0, size);
            if ((pCShareQueue = new(buffer) CShareQueue(key, shmid, queue_len)) ==
                NULL) {
                return -6;
            }
        } else {
            pCShareQueue = (CShareQueue *) buffer;
        }
        return 0;
    }

    int CShareQueue::CShareQueueRelease(CShareQueue *pCShareQueue,
                                        const bool bNeedDelete) {
        if (pCShareQueue == NULL) return -1;

        // const long key = pCShareQueue->m_lKey;
        const int shmid = pCShareQueue->m_iShmId;
        // pCShareQueue->~CShareQueue();
        if (shmdt(pCShareQueue) == -1) return -2;

        pCShareQueue = NULL;
        if (bNeedDelete)
            if (shmctl(shmid, IPC_RMID, NULL) == -1) return -3;
        return 0;
    }

    CShareQueue::CShareQueue(const long key, const int shmid, int maxsize)
            : m_lKey(key),
              m_iShmId(shmid),
              m_iFront(0),
              m_iRear(0),
              m_iMaxSize(maxsize),
              m_bShutDown(true) {
        pthread_mutexattr_t m_mutex_attr;

        pthread_mutexattr_init(&m_mutex_attr);
        pthread_mutexattr_setpshared(&m_mutex_attr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(&m_mutex, &m_mutex_attr);

        pthread_condattr_t m_cond_attr;

        pthread_condattr_init(&m_cond_attr);
        pthread_condattr_setpshared(&m_cond_attr, PTHREAD_PROCESS_SHARED);
        pthread_cond_init(&m_condNotEmpty, &m_cond_attr);
        pthread_cond_init(&m_condNotFull, &m_cond_attr);

        m_bShutDown = false;
    }

    CShareQueue::~CShareQueue() {
        if (0) {
            pthread_mutex_destroy(&m_mutex);
            pthread_cond_destroy(&m_condNotEmpty);
            pthread_cond_destroy(&m_condNotFull);
        }
    }

    bool CShareQueue::FullQueue() {
        if (m_iFront == (m_iRear + 1) % m_iMaxSize)
            return true;
        else
            return false;
    }

    bool CShareQueue::EmptyQueue() {
        if (m_iFront == m_iRear)
            return true;
        else
            return false;
    }

    int CShareQueue::Enqueue(const char *buf, const unsigned int buf_len) {
        if (buf == NULL || buf_len == 0 || m_bShutDown) return -1;

        Lock_Safe(&m_mutex);
        while (FullQueue() || EmptyNodes() < NeedNodes(buf_len)) {
            if (m_bShutDown) {
                UnLock_Safe(&m_mutex);
                return -2;
            }
            pthread_cond_wait(&m_condNotFull, &m_mutex);
        }

        unsigned int nodes = NeedNodes(buf_len);
        m_nodes[m_iRear].m_nodes = nodes;
        for (unsigned int i = 0; i < nodes; ++i) {
            if (i == nodes - 1) {
                m_nodes[m_iRear].m_Len = buf_len % g_const_node_size;
                if (m_nodes[m_iRear].m_Len == 0)
                    m_nodes[m_iRear].m_Len = g_const_node_size;
                memcpy(m_nodes[m_iRear].m_buf, buf + i * g_const_node_size,
                       m_nodes[m_iRear].m_Len);
            } else {
                memcpy(m_nodes[m_iRear].m_buf, buf + i * g_const_node_size,
                       g_const_node_size);
            }
            m_iRear = (m_iRear + 1) % m_iMaxSize;
        }
        pthread_cond_signal(&m_condNotEmpty);
        UnLock_Safe(&m_mutex);

        return 0;
    }

    int CShareQueue::Dequeue(char *&buf, unsigned int *buf_len) {
        if (m_bShutDown) return -1;

        Lock_Safe(&m_mutex);

        while (EmptyQueue()) {
            if (m_bShutDown) {
                UnLock_Safe(&m_mutex);
                return -2;
            }
            pthread_cond_wait(&m_condNotEmpty, &m_mutex);
        }

        unsigned int nodes = m_nodes[m_iFront].m_nodes;
        *buf_len = (nodes - 1) * g_const_node_size +
                   m_nodes[(m_iFront + nodes - 1) % m_iMaxSize].m_Len;
        buf = (char *) malloc(*buf_len);
        for (unsigned int i = 0; i < nodes; ++i) {
            if (i == nodes - 1) {
                memcpy(buf + i * g_const_node_size, m_nodes[m_iFront].m_buf,
                       m_nodes[m_iFront].m_Len);
            } else {
                memcpy(buf + i * g_const_node_size, m_nodes[m_iFront].m_buf,
                       g_const_node_size);
            }
            m_iFront = (m_iFront + 1) % m_iMaxSize;
        }

        pthread_cond_signal(&m_condNotFull);
        UnLock_Safe(&m_mutex);
        return 0;
    }

    int CShareQueue::Shutdown() {
        m_bShutDown = true;
        pthread_cond_broadcast(&m_condNotEmpty);
        pthread_cond_broadcast(&m_condNotFull);
        return 0;
    }

}  // namespace mminternpractice
