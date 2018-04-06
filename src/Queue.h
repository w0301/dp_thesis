#ifndef INTERPRETER_QUEUE_H
#define INTERPRETER_QUEUE_H

#include <mutex>
#include <queue>
#include <condition_variable>

template <class T> class Queue {
public:
    T pop() {
        std::unique_lock<std::mutex> lock(mutex);
        while (queue.empty()) cond.wait(lock);

        auto item = queue.top();
        queue.pop();
        return item;
    }

    void pop(T& item) {
        std::unique_lock<std::mutex> lock(mutex);
        while (queue.empty()) cond.wait(lock);
        item = queue.top();
        queue.pop();
    }

    void push(const T& item) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            queue.push(item);
        }
        cond.notify_one();
    }

    void push(T&& item) {
        {
            std::unique_lock<std::mutex> lock(mutex);
            queue.push(std::move(item));
        }
        cond.notify_one();
    }

private:
    std::mutex mutex;
    std::condition_variable cond;
    std::priority_queue<T> queue;
};


#endif //INTERPRETER_QUEUE_H
