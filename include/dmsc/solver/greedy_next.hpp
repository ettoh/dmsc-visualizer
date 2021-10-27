#ifndef DMSC_GREEDY_NEXT_H
#define DMSC_GREEDY_NEXT_H

#include "../solver.hpp"
#include "../solution_types.hpp"

namespace dmsc {
namespace solver {

class GreedyNext : public Solver {
  public:
    GreedyNext(const PhysicalInstance& instance)
        : Solver(instance) {}

    DmscSolution solve();
};

} // namespace solver
} // namespace dmsc

#endif
