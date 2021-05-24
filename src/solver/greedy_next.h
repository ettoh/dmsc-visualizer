#ifndef GREEDY_NEXT_H
#define GREEDY_NEXT_H

#include "solver.h"

namespace dmsc {

class GreedyNext : public Solver {
  public:
    GreedyNext(const PhysicalInstance& instance) : Solver(instance) {}
    GreedyNext(const PhysicalInstance& instance, Callback callback) : Solver(instance, callback) {}

    /**
     * @return A sorted scan cover.
     */
    ScanCover solve();
};

} // namespace dmsc

#endif