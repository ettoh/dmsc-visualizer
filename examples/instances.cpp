#include <dmsc/visuals.hpp>

int main() {
    // 1. create a new instance
    dmsc::Instance instance;
    dmsc::StateVector sv;
    sv.height_perigee = 200.f;         // height above the central mass
    instance.satellites.push_back(sv); // satellite 0
    sv.inclination = dmsc::rad(45.f);
    instance.satellites.push_back(sv);          // satellite 1
    instance.edges.push_back(dmsc::Edge(0, 1)); // schedule communication between sat 0 and sat 1

    // 2. visualize instance
    dmsc::visualizeInstance(instance);

    return 0;
}