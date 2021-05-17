#ifndef ITERATED_LOCAL_SEARCH_H
#define ITERATED_LOCAL_SEARCH_H

#include "solver.h"
#include <vector>

namespace dmsc {

class ILS : public Solver {
  public:
    ILS(const Instance& instance) : Solver(instance) {}
    ILS(const Instance& instance, Callback callback) : Solver(instance, callback) {}
    ScanCover solve();

  private:
    EdgeOrder localSearch(EdgeOrder init_order);
};

} // namespace dmsc

#endif // !ITERATED_LOCAL_SEARCH_H
