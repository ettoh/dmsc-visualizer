#ifndef SIMULATED_ANNEALING_H
#define SIMULATED_ANNEALING_H

#include "solver.h"

namespace dmsc {

class SimulatedAnnealing : public Solver {
  public:
    SimulatedAnnealing(const PhysicalInstance& instance) : Solver(instance) {}
    SimulatedAnnealing(const PhysicalInstance& instance, Callback callback) : Solver(instance, callback) {}
    ScanCover solve();
};

} // namespace dmsc

#endif // !SIMULATED_ANNEALING_H
