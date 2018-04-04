#include <stdexcept>

#include "scheduler.h"

using namespace std;

base_scheduler::base_scheduler(std::shared_ptr<void> state, scheduler_type type, int workers_count, int vars_count) :
    running(false), type(type), workers_count(workers_count), vars_count(vars_count),
    workers(workers_count, new worker(vars_count, main_mutex, main_cond)), state(state) {
}

base_scheduler::~base_scheduler() {
  for (worker* w : workers) delete w;
  workers.clear();
}

void base_scheduler::start() {
  running = true;
  main_thread = thread(&base_scheduler::process_main, ref(*this));
  for (worker* w : workers) w->start(std::bind(&base_scheduler::process_worker, this, std::placeholders::_1, std::placeholders::_2));
}

void base_scheduler::stop() {
    for (worker* w : workers) w->stop();

    {
      std::lock_guard<std::mutex> lock(main_mutex);
      running = false;
    }
    main_cond.notify_one();

    main_thread.join();
}

void base_scheduler::schedule(std::shared_ptr<void> msg) {
  {
    std::lock_guard<std::mutex> lock(main_mutex);
    queue.push_back(msg);
  }
  main_cond.notify_one();
}

void base_scheduler::process_main() {
  while (running) {
    worker* freeWorker = NULL;
    std::shared_ptr<void> schedulableMsg;

    std::unique_lock<std::mutex> lock(main_mutex);
    main_cond.wait(lock, [&]{
      if (!running) return true;

      freeWorker = get_free_worker();
      schedulableMsg = get_schedulable_message();

      return freeWorker || schedulableMsg;
    });
    if (!running) break;

    // consume worker results
    if (freeWorker) {
      // TODO : merge states
    }

    // schedule message
    if (schedulableMsg) {
      // TODO : mark variables
      // TODO : copy state if needed
      freeWorker->process(state, schedulableMsg);

      // delete msg
      for (auto it = queue.begin(); it != queue.end(); it++) {
        if (schedulableMsg == *it) {
          queue.erase(it);
          break;
        }
      }
    }
  }
}

worker* base_scheduler::get_free_worker() {
  for (auto worker : workers) {
    if (!worker->is_processing()) return worker;
  }
  return NULL;
}

std::shared_ptr<void> base_scheduler::get_schedulable_message() {
  for (auto msg : queue) {
    if (is_schedulable(msg)) return msg;
  }
  return std::shared_ptr<void>();
}

bool base_scheduler::is_var_read_locked(int var_index) {
  for (int i = 0; i < workers_count; i++) {
    if (workers[i]->is_reading(var_index)) return true;
  }
  return false;
}

bool base_scheduler::is_var_write_locked(int var_index) {
  for (int i = 0; i < workers_count; i++) {
    if (workers[i]->is_writing(var_index)) return true;
  }
  return false;
}

bool base_scheduler::is_schedulable(std::shared_ptr<void> msg) {
  auto vars = get_message_vars(msg);
  return is_schedulable(vars.first, vars.second);
}

bool base_scheduler::is_schedulable(const std::vector<bool>& read_vars, const std::vector<bool>& write_vars) {
  if (read_vars.size() != vars_count || write_vars.size() != vars_count) {
    throw runtime_error("Incorrect number of variables in the input vectors.");
  }

  for (int i = 0; i < vars_count; i++) {
    bool locked = false;
    bool need_locked = false;

    if (type == scheduler_type::rw_locking) {
      locked = is_var_read_locked(i) || is_var_write_locked(i);
      need_locked = read_vars[i] || write_vars[i];
    }
    else if (type == scheduler_type::w_locking) {
      locked = is_var_write_locked(i);
      need_locked = write_vars[i];
    }

    if (locked && need_locked) return false;
  }

  return true;
}
