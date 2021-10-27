#include <dmsc/visuals.hpp>
#include <dmsc/solution_types.hpp>

int main() {
    dmsc::Instance instance;

    // 1. create satellites
    dmsc::StateVector sv; // represents a satellite
    sv.height_perigee = 200.f;
    sv.initial_true_anomaly = dmsc::rad(20.f);
    instance.satellites.push_back(sv); // satellite 0
    sv.inclination = dmsc::rad(45.f);
    instance.satellites.push_back(sv); // satellite 1
    sv.inclination = dmsc::rad(23.f);
    instance.satellites.push_back(sv); // satellite 2
    sv.inclination = dmsc::rad(33.f);
    sv.initial_true_anomaly -= dmsc::rad(5.f);
    instance.satellites.push_back(sv); // satellite 3

    // 2. define which satellites can communicate with each other
    instance.edges.push_back(dmsc::Edge(0, 1)); // link between sat 0 and sat 1
    instance.edges.push_back(dmsc::Edge(0, 2)); // link between sat 0 and sat 2
    instance.edges.push_back(dmsc::Edge(1, 2)); // link between sat 1 and sat 2
    instance.edges.push_back(dmsc::Edge(1, 3)); // link between sat 1 and sat 3

    // 3. solve the instanve - here we will just build a solution without really solving it ...
    dmsc::FreezeTagSolution solution;
    solution.satellites_with_message.push_back(0); // satellite with idx 0 will initially carry the message
    solution.scheduleEdge(2, 10);
    solution.scheduleEdge(0, 100);
    solution.scheduleEdge(1, 600);
    solution.scheduleEdge(3, 250);

    // 4. visualize instance and animation
    dmsc::visualizeFreezeTagSolution(instance, solution, 0.f);

    return 0;
}
