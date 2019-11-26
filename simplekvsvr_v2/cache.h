#ifndef CACHE_H
#define CACHE_H

#include <map>
#include <string>
#include "task.h"
#include "linkedlist.h"

using namespace std;

#define MAXCACHED 128

typedef LinkedList<Task> LruNode;

class Cache
{
private:
public:
    map<string, LruNode*> index;
    map<string, LruNode*>::iterator it;
    LruNode lruHead;
    LruNode* tail;
    int totalCached;

    Cache()
    {
        tail = &lruHead;
        totalCached = 0;
    }

    ~Cache()
    {
        for (it = index.begin(); it != index.end(); ++it)
        {
            string s = it->first;
            deleteCache(s);
        }
    }

    void addToHead(LruNode* node)
    {
        node->next = lruHead.next;
        node->prev = &lruHead;
        if (node->next != NULL)
            node->next->prev = node;
        lruHead.next = node;
        if (tail == &lruHead)
            tail = node;
    }

    void removeTail()
    {
        LruNode* tmp = tail;
        tail = tail->prev;
        Task* task = &tmp->content; //
        it = index.find(string(task->key, task->keyLen));
        if (it != index.end())
            index.erase(it);
        tmp->unlink();
        delete tmp;
        //	delete task;
    }

    Task* getCache(string key)
    {
        it = index.find(key);
        if (it == index.end())
        {
            return NULL;
        }
        LruNode* node = it->second;
        if (node == tail)
        {
            tail = tail->prev;
        }
        node->unlink();
        addToHead(node);
        return &(node->content);
    }

    void addToCache(Task* task)
    {
        LruNode* node = new LruNode(*task);
        //	task->cached = true;
        index[string(task->key, task->keyLen)] = node;
        addToHead(node);
        totalCached++;
        if (totalCached > MAXCACHED)
        {
            removeTail();
            totalCached--;
        }
    }

    void deleteCache(string key)
    {
        it = index.find(key);
        if (it == index.end())
            return;
        LruNode* node = it->second;
        if (node == tail)
        {
            tail = tail->prev;
        }
        if (node != NULL)
        {
            node->unlink();
            //	delete node->content;
            delete node;
        }
        index.erase(it);
        totalCached--;
    }
};

#endif
