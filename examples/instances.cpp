#include <dmsc/visuals.hpp>

int main() {
    dmsc::Instance instance;

    // 1. create satellites
    dmsc::StateVector sv; // represents a satellite
    sv.height_perigee = 200.f;
    instance.satellites.push_back(sv); // satellite 0
    sv.initial_true_anomaly = dmsc::rad(10);
    sv.inclination = dmsc::rad(45.f);
    instance.satellites.push_back(sv); // satellite 1
    sv.initial_true_anomaly = dmsc::rad(20);
    sv.inclination = dmsc::rad(23.f);
    instance.satellites.push_back(sv); // satellite 2

    // 2. define which satellites can communicate with each other
    instance.edges.push_back(dmsc::Edge(0, 1)); // link between sat 0 and sat 1
    instance.edges.push_back(dmsc::Edge(0, 2)); // link between sat 0 and sat 2
    instance.edges.push_back(dmsc::Edge(1, 2)); // link between sat 1 and sat 2

    // 3. schedule directed communications between two satellites
    instance.edges.push_back(dmsc::Edge(0, 1, dmsc::EdgeType::SCHEDULED_COMMUNICATION));

    // 4. visualize instance
    dmsc::visualizeInstance(instance);

    // 5. save instance
    instance.save("instance.csv");

    return 0;
}