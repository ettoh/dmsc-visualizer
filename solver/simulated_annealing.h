#ifndef SIMULATED_ANNEALING_H
#define SIMULATED_ANNEALING_H

#include "solver.h"

class SimulatedAnnealing : public Solver {
  public:
    SimulatedAnnealing(const ProblemInstance& instance) : Solver(instance) {}
    SimulatedAnnealing(const ProblemInstance& instance, Callback callback) : Solver(instance, callback) {}
    ScanCover solve();
};

#endif // !SIMULATED_ANNEALING_H
