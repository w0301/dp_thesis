#ifndef WORKER_H
#define WORKER_H

#include <atomic>
#include <thread>
#include <functional>

#include "Queue.h"

template <typename T> class Worker {
public:
    bool isWaiting() const {
        return waiting;
    }

    virtual void start() {
        thread = std::thread([&] {
            while (true) {
                waiting = true;

                T msg = queue.pop();

                waiting = false;

                if (!process(msg)) break;
            }
        });
    }

    void join() {
        thread.join();
    }

    void send(const T& msg) {
        queue.push(msg);
    }

    void send(T&& msg) {
        queue.push(msg);
    }

protected:
    virtual bool process(T& msg) = 0;

    void waitFor(const std::function<bool(const T&)>& func) {
        queue.waitFor(func);
    }

private:
    Queue<T> queue;
    std::thread thread;
    std::atomic<bool> waiting;
};

#endif
