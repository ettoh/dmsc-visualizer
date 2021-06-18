#include <dmsc/instance.hpp>
#include <dmsc/solver/greedy_next.hpp>
#include <dmsc/solver/greedy_next_khop.hpp>
#include <dmsc/visuals.hpp>

using dmsc::solver::GreedyNext;
using dmsc::solver::GreedyNextKHop;

int main() {
    dmsc::Instance instance;

    // 1. create satellites
    dmsc::StateVector sv; // represents a satellite
    sv.initial_true_anomaly = dmsc::rad(90.f);
    sv.height_perigee = 200.f;
    instance.satellites.push_back(sv); // satellite 0
    sv.inclination = dmsc::rad(50.f);
    instance.satellites.push_back(sv); // satellite 1
    sv.inclination = dmsc::rad(25.f);
    instance.satellites.push_back(sv); // satellite 2

    // 2. define which satellites can communicate with each other
    instance.edges.push_back(dmsc::Edge(0, 1)); // link between sat 0 and sat 1
    instance.edges.push_back(dmsc::Edge(0, 2)); // link between sat 0 and sat 2
    instance.edges.push_back(dmsc::Edge(1, 2)); // link between sat 1 and sat 2

    // 3. schedule directed communications between two satellites
    instance.edges.push_back(dmsc::Edge(0, 1, dmsc::EdgeType::SCHEDULED_COMMUNICATION));
    instance.edges.push_back(dmsc::Edge(1, 0, dmsc::EdgeType::SCHEDULED_COMMUNICATION));

    // 4. solve the instance
    GreedyNextKHop solver = GreedyNextKHop(dmsc::PhysicalInstance(instance), 1);
    dmsc::Solution solution = solver.solve();

    // 5. visualize solution
    dmsc::visualizeSolution(instance, solution);

    return 0;
}
