#ifndef QUEUE_H
#include <stdio.h>
#define QUEUE_H
#define MAX_SIZE 51
template <class T>
class Queue {
private:
    int head, tail, len;
    T content[MAX_SIZE];
public:
    Queue(): head(0), tail(0) {}

    void push(const T& item) {
        if(!full()) {
            content[tail] = item;
            tail = (tail + 1) % MAX_SIZE;
            len++;
        }
    }

    void pop() {
        if(!empty()) {
            head = (head + 1) % MAX_SIZE;
            len--;
        }
    }

    const T& front() {
        return empty() ? (const T&)NULL : content[head];
    }

    bool empty() {
        return head == tail;
    }

    bool full() {
        return (tail + 1) % MAX_SIZE == head;
    }

    int size() {
        return len;
    }

    int clear() {
        head = tail = 0;
        len = 0;
    }
};
#endif
