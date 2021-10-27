#include <dmsc/instance.hpp>
#include <dmsc/solver.hpp>
#include <dmsc/visuals.hpp>

//===================================
// Once you defined a problem instance, it is time to solve it. How you gonna solve it, is up to you. Here we want to
// solve an instance of the Dynamic Minimum Scan Cover problem and visualize the correspondig solution. For this we can
// use a visualization function you can find in the visuals.hpp header. This function needs the physical instance and a
// scan cover (basically a timetable at which time the edges are scanned).
//
// You can fill the solution (scan cover) object completely by yourself. If you want to use the provided functions to
// evaluate the instance, you can use the virtual solver class (as done below).
//
// In the namespace dsmc::solver you can find complete solvers, too.
//===================================
class SampleSolver : public dmsc::Solver {
  public:
    SampleSolver(const dmsc::PhysicalInstance& instance)
        : Solver(instance) {}

    dmsc::DmscSolution solve() {
        // you can access the physical instance
        const dmsc::Satellite s = instance.getSatellites()[0];

        // you can use functions to evaluate the instance
        // e.g. get the time when the given intersatellite link will be avaiable for communication for the next time
        // (when starting at the given time - here 60 seconds)
        float t = nextCommunication(instance.getISLs()[0], 60.f);

        // do the magic here and build your solution
        dmsc::DmscSolution solution;
        solution.scheduleEdge(0, t); // the edge with index 0 will be scanned at time t
        solution.scheduleEdge(1, t + 100);
        solution.scheduleEdge(2, t + 600);
        // ...

        return solution;
    }
};

// --------------------------------------------------------------------------

int main() {
    //===================================
    // 1. create/load an instance
    //===================================

    dmsc::Instance instance;
    dmsc::StateVector sv; // represents a satellite
    sv.initial_true_anomaly = dmsc::rad(90.f);
    sv.height_perigee = 200.f;
    instance.satellites.push_back(sv); // satellite 0
    sv.cone_angle = dmsc::rad(45);
    sv.inclination = dmsc::rad(50.f);
    instance.satellites.push_back(sv); // satellite 1
    sv.inclination = dmsc::rad(25.f);
    instance.satellites.push_back(sv); // satellite 2

    // define which satellites can communicate with each other
    instance.edges.push_back(dmsc::Edge(0, 1)); // link between sat 0 and sat 1
    instance.edges.push_back(dmsc::Edge(0, 2)); // link between sat 0 and sat 2
    instance.edges.push_back(dmsc::Edge(1, 2)); // link between sat 1 and sat 2

    // schedule directed communications between two satellites
    instance.edges.push_back(dmsc::Edge(0, 1, dmsc::EdgeType::SCHEDULED_COMMUNICATION));
    instance.edges.push_back(dmsc::Edge(1, 0, dmsc::EdgeType::SCHEDULED_COMMUNICATION));

    //===================================
    // 2. solve the instance
    //===================================

    SampleSolver solver(instance);
    dmsc::DmscSolution solution = solver.solve();

    //===================================
    // 3. visualize the solution
    //===================================

    dmsc::visualizeDmscSolution(instance, solution);

    return 0;
}
