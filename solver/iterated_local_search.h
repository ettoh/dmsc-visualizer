#ifndef ITERATED_LOCAL_SEARCH_H
#define ITERATED_LOCAL_SEARCH_H

#include "solver.h"
#include <vector>

class ILS : public Solver {
  public:
    ILS(const ProblemInstance& instance) : Solver(instance) {}
    ILS(const ProblemInstance& instance, Callback callback) : Solver(instance, callback) {}
    ScanCover solve();

  private:
    EdgeOrder localSearch(EdgeOrder init_order);
};

#endif // !ITERATED_LOCAL_SEARCH_H
