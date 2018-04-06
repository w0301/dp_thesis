#ifndef INTERPRETER_QUEUE_H
#define INTERPRETER_QUEUE_H

#include <mutex>
#include <queue>
#include <functional>
#include <condition_variable>

template <class T> class Queue {
public:
    void waitFor(const std::function<bool(const T&)>& func) {
        std::unique_lock<std::mutex> lock(mutex);
        while (queue.empty() || !func(queue.top())) cond.wait(lock);
    }

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
