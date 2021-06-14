#ifndef DMSC_GreedyNextKHop_H
#define DMSC_GreedyNextKHop_H

#include "../instance.hpp"
#include "../solver.hpp"

namespace dmsc {
namespace solver {

class GreedyNextKHop : public Solver {
  public:
    /**
     * @brief Construct a new GreedyNextKHop solver object.
     * @param k number of "extra" satellites - e.g. k=1 allows 3 edges (origin, hop, target)
     */
    GreedyNextKHop(const PhysicalInstance& instance, const unsigned int k)
        : Solver(instance)
        , k(k) {}

    Solution solve();

  private:
    struct Communication; // stores all paths for a scheduled communication and the currently chosen path + progress
    using Path = std::vector<uint32_t>;
    unsigned int k; // number of hops allowed

    /**
     * @brief Breadth-first search
     * @return first: if true, at least one path was found; second: adjacency matrix
     */
    std::pair<bool, AdjacencyMatrix> findPaths(const uint32_t from, const uint32_t to) const;
};

} // namespace solver
} // namespace dmsc

#endif
