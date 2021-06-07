#include <dmsc/visuals.hpp>

int main() {
    dmsc::Instance instance;

    // 1. create satellites
    dmsc::StateVector sv; // represents a satellite
    sv.height_perigee = 200.f;
    instance.satellites.push_back(sv); // satellite 0
    sv.inclination = dmsc::rad(45.f);
    instance.satellites.push_back(sv); // satellite 1
    sv.inclination = dmsc::rad(23.f);
    instance.satellites.push_back(sv); // satellite 2

    // 2. schedule communications between satellites
    instance.edges.push_back(dmsc::Edge(0, 1));       // communication between sat 0 and sat 1
    instance.edges.push_back(dmsc::Edge(0, 2, true)); // optional communication between sat 0 and sat 2
    instance.edges.push_back(dmsc::Edge(1, 2, true)); // optional communication between sat 1 and sat 2

    // 3. visualize instance
    dmsc::visualizeInstance(instance);

    return 0;
}