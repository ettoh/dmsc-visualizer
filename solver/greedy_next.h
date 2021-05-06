#ifndef GREEDY_NEXT_H
#define GREEDY_NEXT_H

#include "solver.h"

class GreedyNext : public Solver {
  public:
    GreedyNext(const ProblemInstance& instance) : Solver(instance) {}
    GreedyNext(const ProblemInstance& instance, Callback callback) : Solver(instance, callback) {}

    /**
     * @return A sorted scan cover.
     */
    ScanCover solve(); 
};

#endif