#ifndef WORKER_H
#define WORKER_H

#include <mutex>
#include <vector>
#include <thread>
#include <memory>
#include <utility>
#include <functional>
#include <condition_variable>

class worker {
  public:
    worker(int, std::mutex&, std::condition_variable&);

    void start(const std::function<void(std::shared_ptr<void>, std::shared_ptr<void>)>&);
    void stop();

    bool is_running() const {
      return running;
    }

    bool is_processing() const {
      return running;
    }

    bool is_process_consumed() const {
      return running;
    }

    bool is_reading(int var_index) const {
      return read_vars[var_index];
    };

    bool is_writing(int var_index) const {
      return write_vars[var_index];
    };

    void set_vars(const std::vector<bool>&, const std::vector<bool>&);

    void process(std::shared_ptr<void>, std::shared_ptr<void>);
    std::shared_ptr<void> consume_process();

  private:
    void worker_method(const std::function<void(std::shared_ptr<void>, std::shared_ptr<void>)>&);

    std::mutex& scheduler_mutex;
    std::condition_variable& scheduler_cond;

    int vars_count;
    std::vector<bool> read_vars;
    std::vector<bool> write_vars;

    volatile bool running = false;
    std::mutex worker_mutex;
    std::thread worker_thread;
    std::condition_variable worker_cond;

    volatile bool processing = false;
    volatile bool process_consumed = false;
    std::shared_ptr<void> worker_state;
    std::shared_ptr<void> worker_msg;
};

#endif
