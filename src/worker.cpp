#include <stdexcept>

#include "worker.h"

worker::worker(int vars_count, std::mutex& mutex, std::condition_variable& cond) :
    vars_count(vars_count), scheduler_mutex(mutex), scheduler_cond(cond),
    read_vars(vars_count, false), write_vars(vars_count, false) {
}

void worker::start(const std::function<void(std::shared_ptr<void>, std::shared_ptr<void>)>& func) {
  running = true;
  processing = false;
  process_consumed = false;

  worker_thread = std::thread(&worker::worker_method, std::ref(*this), func);
}

void worker::stop() {
  {
    std::lock_guard<std::mutex> lock(worker_mutex);
    running = false;
  }
  worker_cond.notify_one();

  worker_thread.join();
}

void worker::set_vars(const std::vector<bool>& read, const std::vector<bool>& write) {
  if (read_vars.size() != vars_count || write_vars.size() != vars_count) {
    throw std::runtime_error("Incorrect number of variables in the input vectors.");
  }

  read_vars.assign(read.begin(), read.end());
  write_vars.assign(write.begin(), write.end());
}

bool worker::process(std::shared_ptr<void> state, std::shared_ptr<void> msg) {
  {
    std::lock_guard<std::mutex> lock(worker_mutex);
    if (processing || !process_consumed) return false;

    worker_state = state;
    worker_msg = msg;
  }
  worker_cond.notify_one();

  return true;
}

std::shared_ptr<void> worker::consume_process() {
  std::lock_guard<std::mutex> lock(worker_mutex);
  if (processing) return std::shared_ptr<void>();

  process_consumed = true;
  return worker_state;
}

void worker::worker_method(const std::function<void(std::shared_ptr<void>, std::shared_ptr<void>)>& func) {
  while (running) {
    {
      std::unique_lock<std::mutex> lock(worker_mutex);
      worker_cond.wait(lock, [&]{ return !running || (worker_state && worker_msg); });
      if (!running) break;

      processing = true;
      process_consumed = false;
    }

    func(worker_state, worker_msg);

    {
      std::lock_guard<std::mutex> lock1(worker_mutex);
      std::lock_guard<std::mutex> lock2(scheduler_mutex);

      processing = false;
    }
    scheduler_cond.notify_one();
  }
}
