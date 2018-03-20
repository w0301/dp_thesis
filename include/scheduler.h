#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <vector>
#include <functional>

#include "worker.h"

class base_scheduler {
  protected:
    int get_possible_worker(const std::vector<bool>&, const std::vector<bool>&);

  private:
    std::vector< std::vector<bool> > workersReadVars;
    std::vector< std::vector<bool> > workersWriteVars;
};

template <class TState, class TMsg> class scheduler : public base_scheduler {
  public:
    void schedule(const TMsg& msg) {
      // TODO
    };

  protected:


  private:
    TState state;
    std::vector< worker<TState, TMsg> > workers;
};

#endif
