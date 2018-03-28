#include <stdexcept>

#include "scheduler.h"

using namespace std;

base_scheduler::base_scheduler(std::shared_ptr<void> state, scheduler_type type, int workers_count, int vars_count) :
    running(false), type(type), workers_count(workers_count), vars_count(vars_count),
    state(state),
    workers_read_vars(workers_count, vector<bool>(vars_count, false)),
    workers_write_vars(workers_count, vector<bool>(vars_count, false)) {
}

void base_scheduler::start() {
  running = true;
  main_thread = thread(&base_scheduler::process_main, ref(*this));
  for (int i = 0; i < workers_count; i++) {
    worker_threads.push_back(thread(&base_scheduler::process_worker, ref(*this), i));
  }
}

void base_scheduler::process_main() {

}

void base_scheduler::process_worker(int index) {

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

void base_scheduler::set_worker_vars(int index, const std::vector<bool>& read_vars, const std::vector<bool>& write_vars) {
  if (index < 0 || index >= workers_count) {
    throw runtime_error("Worker with the given index does not exist.");
  }
  if (read_vars.size() != vars_count || write_vars.size() != vars_count) {
    throw runtime_error("Incorrect number of variables in the input vectors.");
  }

  workers_read_vars[index].assign(read_vars.begin(), read_vars.end());
  workers_write_vars[index].assign(write_vars.begin(), write_vars.end());
}

bool base_scheduler::is_var_read_locked(int var_index) {
  for (int i = 0; i < workers_count; i++) {
    if (workers_read_vars[i][var_index]) return true;
  }
  return false;
}

bool base_scheduler::is_var_write_locked(int var_index) {
  for (int i = 0; i < workers_count; i++) {
    if (workers_write_vars[i][var_index]) return true;
  }
  return false;
}
