#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <cstdio>

/**
 * 简单的双向链表
 */
template<typename T>
class LinkedList
{
private:
public:
    T content;
    LinkedList *next, *prev;

    LinkedList()
    {
        next = NULL;
        prev = NULL;
    }

    LinkedList(T obj)
    {
        next = NULL;
        prev = NULL;
        content = obj;
    }

    void unlink()
    {
        if (this->prev != NULL)
            this->prev->next = this->next;
        if (this->next != NULL)
            this->next->prev = this->prev;
        this->prev = NULL;
        this->next = NULL;
    }
};

#endif
