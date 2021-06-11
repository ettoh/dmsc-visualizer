#ifndef DMSC_ITERATED_LOCAL_SEARCH_H
#define DMSC_ITERATED_LOCAL_SEARCH_H

#include "solver.h"
#include <vector>

namespace dmsc {

class ILS : public Solver {
  public:
    ILS(const PhysicalInstance& instance)
        : Solver(instance) {}
    ILS(const PhysicalInstance& instance, Callback callback)
        : Solver(instance, callback) {}
    ScanCover solve();

  private:
    EdgeOrder localSearch(EdgeOrder init_order);
};

} // namespace dmsc

#endif
