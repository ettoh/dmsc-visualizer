#include <dmsc/visuals.hpp>

int main() {
    dmsc::Instance instance;
    dmsc::StateVector sv;
    sv.height_perigee = 200.f;
    instance.satellites.push_back(sv);
    sv.inclination = dmsc::rad(45.f);
    instance.satellites.push_back(sv);
    instance.edges.push_back(dmsc::Edge(0, 1));
    dmsc::visualizeInstance(instance);
    return 0;
}