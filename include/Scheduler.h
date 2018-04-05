#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <deque>
#include <thread>
#include <vector>
#include <memory>
#include <utility>
#include <functional>

#include "Worker.h"

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
    virtual std::shared_ptr<void> acquire_state(std::shared_ptr<void>) = 0;
    virtual std::shared_ptr<void> merge_states(std::shared_ptr<void>, std::shared_ptr<void>, const std::vector<bool>&) = 0;

    virtual void process_worker(std::shared_ptr<void>, std::shared_ptr<void>) = 0;
    virtual std::pair< std::vector<bool>, std::vector<bool> > get_message_vars(std::shared_ptr<void>) = 0;

    void process_main();

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
    scheduler(std::shared_ptr<TState> s, scheduler_type type, int workersCount, int varsCount) :
        base_scheduler(std::static_pointer_cast<TState, void>(s), type, workersCount, varsCount) {
    };

    void schedule(std::shared_ptr<TMsg> msg) {
      schedule(std::static_pointer_cast<TMsg, void>(msg));
    };

  protected:
    virtual std::shared_ptr<TState> acquire_state(std::shared_ptr<TState>) = 0;
    std::shared_ptr<void> acquire_state(std::shared_ptr<void> globalState) override {
      return std::static_pointer_cast<void, TState>(acquire_state(
        std::static_pointer_cast<TState, void>(globalState)
      ));
    };

    virtual std::shared_ptr<TState> merge_states(std::shared_ptr<TState>, std::shared_ptr<TState>, const std::vector<bool>&) = 0;
    std::shared_ptr<void> merge_states(std::shared_ptr<void> globalState, std::shared_ptr<void> workerState, const std::vector<bool>& writeVars) override {
      return std::static_pointer_cast<void, TState>(merge_states(
        std::static_pointer_cast<TState, void>(globalState), std::static_pointer_cast<TState, void>(workerState), writeVars
      ));
    };

    virtual void process_worker(std::shared_ptr<TState>, std::shared_ptr<TMsg>) = 0;
    void process_worker(std::shared_ptr<void> state, std::shared_ptr<void> msg) override {
       process_worker(std::static_pointer_cast<TState, void>(state), std::static_pointer_cast<TMsg, void>(msg));
    };

    virtual std::pair< std::vector<bool>, std::vector<bool> > get_message_vars(std::shared_ptr<TMsg>) = 0;
    std::pair< std::vector<bool>, std::vector<bool> > get_message_vars(std::shared_ptr<void> msg) override {
      return get_message_vars(std::static_pointer_cast<TMsg, void>(msg));
    };
};

#endif
