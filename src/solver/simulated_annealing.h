#ifndef DMSC_SIMULATED_ANNEALING_H
#define DMSC_SIMULATED_ANNEALING_H

#include "solver.h"

namespace dmsc {

class SimulatedAnnealing : public Solver {
  public:
    SimulatedAnnealing(const PhysicalInstance& instance)
        : Solver(instance) {}
    SimulatedAnnealing(const PhysicalInstance& instance, Callback callback)
        : Solver(instance, callback) {}
    ScanCover solve();
};

} // namespace dmsc

#endif
