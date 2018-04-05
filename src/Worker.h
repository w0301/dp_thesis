#ifndef WORKER_H
#define WORKER_H

#include <mutex>
#include <vector>
#include <thread>
#include <memory>
#include <utility>
#include <functional>
#include <condition_variable>

class Worker {
    public:
        Worker(int, std::mutex&, std::condition_variable&);

        void start(const std::function<void(std::shared_ptr<void>, std::shared_ptr<void>)>&);
        void stop();

        bool isRunning() const {
            return running;
        }

        bool isProcessing() const {
            return processing;
        }

        bool isProcessConsumed() const {
            return processConsumed;
        }

        bool isReading(int index) const {
            return readVars[index];
        }

        bool isWriting(int index) const {
            return writeVars[index];
        }

        const std::vector<bool>& getReadVars() const {
            return readVars;
        }

        const std::vector<bool>& getWriteVars() const {
            return writeVars;
        }

        void clearVars();
        void setVars(const std::vector<bool>&, const std::vector<bool> &);

        void process(std::shared_ptr<void>, std::shared_ptr<void>);
        std::shared_ptr<void> consumeProcess();

    private:
        void workerMethod(const std::function<void(std::shared_ptr<void>, std::shared_ptr<void>)>&);

        std::mutex& schedulerMutex;
        std::condition_variable& schedulerCond;

        int varsCount;
        std::vector<bool> readVars;
        std::vector<bool> writeVars;

        volatile bool running = false;
        std::mutex workerMutex;
        std::thread workerThread;
        std::condition_variable workerCond;

        volatile bool processing = false;
        volatile bool processConsumed = false;
        std::shared_ptr<void> workerState;
        std::shared_ptr<void> workerMessage;
};

#endif
