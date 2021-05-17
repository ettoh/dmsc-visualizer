#ifndef SIMULATED_ANNEALING_H
#define SIMULATED_ANNEALING_H

#include "solver.h"

namespace dmsc {

class SimulatedAnnealing : public Solver {
  public:
    SimulatedAnnealing(const Instance& instance) : Solver(instance) {}
    SimulatedAnnealing(const Instance& instance, Callback callback) : Solver(instance, callback) {}
    ScanCover solve();
};

} // namespace dmsc

#endif // !SIMULATED_ANNEALING_H
