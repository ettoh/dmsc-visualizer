#include <dmsc/instance.hpp>
#include <dmsc/solver/greedy_next.hpp>
#include <dmsc/visuals.hpp>

using dmsc::solver::GreedyNext;

int main() {
    dmsc::Instance instance;

    // 1. create satellites
    dmsc::StateVector sv; // represents a satellite
    sv.height_perigee = 200.f;
    instance.satellites.push_back(sv); // satellite 0
    sv.initial_true_anomaly = dmsc::rad(15.f);
    sv.inclination = dmsc::rad(45.f);
    instance.satellites.push_back(sv); // satellite 1
    sv.initial_true_anomaly = dmsc::rad(350.f);
    sv.inclination = dmsc::rad(23.f);
    instance.satellites.push_back(sv); // satellite 2

    // 2. schedule communications between satellites
    instance.edges.push_back(dmsc::Edge(0, 1)); // communication between sat 0 and sat 1
    instance.edges.push_back(dmsc::Edge(0, 2)); // communication between sat 0 and sat 2
    instance.edges.push_back(dmsc::Edge(1, 2)); // communication between sat 1 and sat 2

    // 3. solve the instance
    GreedyNext solver = GreedyNext(dmsc::PhysicalInstance(instance));
    dmsc::Solution solution = solver.solve();

    // 4. visualize solution
    visualizeSolution(instance, solution);

    return 0;
}