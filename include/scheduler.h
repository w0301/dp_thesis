#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <deque>
#include <thread>
#include <vector>
#include <memory>
#include <utility>
#include <functional>

#include "worker.h"

enum scheduler_type {
  rw_locking, w_locking
};

class base_scheduler {
  public:
    base_scheduler(std::shared_ptr<void>, scheduler_type, int, int);
    ~base_scheduler();

    void start();
    void stop();

    bool is_running() {
      return running;
    };

    void schedule(std::shared_ptr<void>);

  protected:
    virtual std::pair< std::vector<bool>, std::vector<bool> > get_message_vars(std::shared_ptr<void>) = 0;

  private:
    void process_main();
    virtual void process_worker(std::shared_ptr<void>, std::shared_ptr<void>) = 0;

    worker* get_free_worker();
    std::shared_ptr<void> get_schedulable_message();

    bool is_var_read_locked(int);
    bool is_var_write_locked(int);
    bool is_schedulable(std::shared_ptr<void>);
    bool is_schedulable(const std::vector<bool>&, const std::vector<bool>&);

    bool running;
    std::mutex main_mutex;
    std::thread main_thread;
    std::condition_variable main_cond;
    std::deque< std::shared_ptr<void> > queue;

    scheduler_type type;
    int workers_count, vars_count;
    std::vector< worker* > workers;

    std::shared_ptr<void> state;
};

template <class TState, class TMsg> class scheduler : public base_scheduler {
  public:
    void schedule(const TMsg& msg) {
      // TODO : reset idle workers here and merge states

      auto vars = get_message_vars(msg);
      int index = get_possible_worker(vars.first, vars.second);

      if (index != -1) {
        set_worker_vars(index, vars.first, vars.second);

      }
      else {

      }
    };

  protected:
    std::pair< std::vector<bool>, std::vector<bool> > get_message_vars(const TMsg& msg) = 0;

  private:
    TState state;
    std::vector< worker > workers;
};

#endif
