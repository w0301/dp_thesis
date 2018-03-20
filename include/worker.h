#ifndef WORKER_H
#define WORKER_H

template <class TState, class TMsg> class worker {
  public:
    void process(TState&, const TMsg&);
};


#endif
