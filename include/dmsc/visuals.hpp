#include "instance.hpp"
#include "solver.hpp"

namespace dmsc {

void visualizeInstance(const Instance& instance, const float t0 = 0.f);
void visualizeInstance(const PhysicalInstance& instance, const float t0 = 0.f);
void visualizeSolution(const Instance& instance, const Solution& solution);
void visualizeSolution(const PhysicalInstance& instance, const Solution& solution);

} // namespace dmsc
