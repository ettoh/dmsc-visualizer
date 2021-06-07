#include <dmsc/visuals.hpp>

int main() {
    dmsc::Instance instance;

    // 1. create satellites
    dmsc::StateVector sv; // represents a satellite
    sv.height_perigee = 200.f;
    instance.satellites.push_back(sv); // satellite 0
    sv.inclination = dmsc::rad(45.f);
    instance.satellites.push_back(sv); // satellite 1

    // 2. schedule communications between satellites
    instance.edges.push_back(dmsc::Edge(0, 1)); // communication between sat 0 and sat 1

    // 3. visualize instance
    dmsc::visualizeInstance(instance);

    return 0;
}