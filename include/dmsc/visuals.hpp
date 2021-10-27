#include "animation.hpp"
#include "instance.hpp"
#include "solution_types.hpp"
#include "solver.hpp"

namespace dmsc {

void visualizeInstance(const Instance& instance, const float t0 = 0.f);
void visualizeInstance(const PhysicalInstance& instance, const float t0 = 0.f);
void visualizeDmscSolution(const Instance& instance, const DmscSolution& solution, const float t0 = 0.f);
void visualizeDmscSolution(const PhysicalInstance& instance, const DmscSolution& solution, const float t0 = 0.f);
void visualizeCustom(const Instance& instance, const Animation& animation, const float t0 = 0.f);
void visualizeCustom(const PhysicalInstance& instance, const Animation& animation, const float t0 = 0.f);
void visualizeFreezeTagSolution(const PhysicalInstance& instance, const FreezeTagSolution& solution,
                                const float t0 = 0.f);

} // namespace dmsc
