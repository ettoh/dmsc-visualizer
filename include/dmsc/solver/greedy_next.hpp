#ifndef GREEDY_NEXT_H
#define GREEDY_NEXT_H

#include "../solver.hpp"

namespace dmsc {
namespace solver {

class GreedyNext : public Solver {
  public:
    GreedyNext(const PhysicalInstance& instance)
        : Solver(instance) {}

    Solution solve();
};

} // namespace solver
} // namespace dmsc

#endif
