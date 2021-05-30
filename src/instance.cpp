#include "dmsc/instance.h"
#include <algorithm>
#include <ctime>
#include <fstream>
#include <map>
#include <random>
#include <set>
#include <sstream>

namespace dmsc {

Instance::Instance(const std::string& file) {
    std::string line_cache = "";
    int mode = READ_INIT;
    std::ifstream is(file);
    if (is.fail()) {
        std::cout << "File could not be opened!" << std::endl;
        return;
    }

    while (std::getline(is, line_cache)) {
        if (line_cache == "===END===") {
            mode++;
            continue;
        }

        // split line by delimiter ','
        std::string value_cache = "";
        std::stringstream ss(line_cache);

        try {
            switch (mode) {
            case READ_INIT:
                std::getline(ss, value_cache, ',');
                cm.radius_central_mass = std::stof(value_cache);
                std::getline(ss, value_cache, ',');
                cm.gravitational_parameter = std::stof(value_cache);
                break;
            case READ_ORBIT: {
                StateVector sv;
                std::getline(ss, value_cache, ',');
                sv.argument_periapsis = std::stof(value_cache);
                std::getline(ss, value_cache, ',');
                sv.eccentricity = std::stof(value_cache);
                std::getline(ss, value_cache, ',');
                sv.raan = std::stof(value_cache);
                std::getline(ss, value_cache, ',');
                sv.argument_periapsis = std::stof(value_cache);
                std::getline(ss, value_cache, ',');
                sv.inclination = std::stof(value_cache);
                std::getline(ss, value_cache, ',');
                sv.rotation_speed = std::stof(value_cache);
                std::getline(ss, value_cache, ',');
                float initial_true_anomaly = std::stof(value_cache);
                satellites.push_back(sv);
                break;
            }
            case READ_EDGE: {
                std::getline(ss, value_cache, ',');
                uint32_t from_idx = std::stoi(value_cache);
                std::getline(ss, value_cache, ',');
                uint32_t to_idx = std::stoi(value_cache);
                std::getline(ss, value_cache, ',');
                bool optional = std::stoi(value_cache); // TODO how is bool stored?
                Edge e = Edge(from_idx, to_idx, optional);
                edges.push_back(e); // TODO copied by value? -> scope? || should be ok ...
                break;
            }
            default:
                break;
            }
        } catch (const std::exception&) {
            std::cout << "Error while loading instance!" << std::endl;
        }
    }
    is.close();
}

// ------------------------------------------------------------------------------------------------

void Instance::save(const std::string& file) const {
    std::ofstream fs(file);
    if (fs.fail()) {
        return; // TODO error
    }

    /** File format:
     *
     * radius, gravitational parameter
     * ===END===
     * height_perigee, eccentricity, raan, perigee, inclination, rotation speed, initial_true_anomaly
     * [...]
     * ===END===
     * from_vertex id #1, to_vertex id #2, optional
     * [...]
     */

    // instance properties
    fs << cm.radius_central_mass << ",";
    fs << cm.gravitational_parameter << "\n";
    fs << "===END===\n";

    // orbits
    for (size_t i = 0; i < satellites.size(); i++) {
        const StateVector& orbit = satellites[i];
        fs << orbit.height_perigee << ",";
        fs << orbit.eccentricity << ",";
        fs << orbit.raan << ",";
        fs << orbit.argument_periapsis << ",";
        fs << orbit.inclination << ",";
        fs << orbit.rotation_speed << ",";
        fs << satellites[i].initial_true_anomaly << "\n";
    }
    fs << "===END===\n";

    // Edges
    for (size_t i = 0; i < edges.size(); i++) {
        const Edge& e = edges[i];
        fs << e.from_idx << ",";
        fs << e.to_idx << ",";
        fs << e.optional << "\n";
    }
    fs.close();
}

// ------------------------------------------------------------------------------------------------

PhysicalInstance::PhysicalInstance(const PhysicalInstance& source) {
    cm.radius_central_mass = source.cm.radius_central_mass;
    cm.gravitational_parameter = source.cm.gravitational_parameter;

    // copy all orbits and store old location for edges later
    satellites = source.satellites;
    std::map<const Satellite*, const Satellite*> orbit_map; // old pos: key | new pos: value
    for (int i = 0; i < satellites.size(); i++) {
        orbit_map[&source.satellites[i]] = &satellites[i];
    }

    // edges must point to the new orbit objects
    for (const InterSatelliteLink& edge : source.edges) {
        const Satellite* new_v1 = orbit_map[&edge.getV1()];
        const Satellite* new_v2 = orbit_map[&edge.getV2()];
        edges.emplace_back(new_v1, new_v2, cm);
    }
}

// ------------------------------------------------------------------------------------------------

PhysicalInstance::PhysicalInstance(const Instance& raw_instance) {
    // TODO validate input
    // TODO loops?
    // TODO double edges?
    // TODO double satellites?
    cm = raw_instance.cm;

    // Satellites
    satellites.reserve(raw_instance.satellites.size());
    for (const auto& sv : raw_instance.satellites) {
        satellites.push_back(Satellite(sv, cm));
    }

    // Edges
    edges.reserve(raw_instance.edges.size());
    for (const auto& e : raw_instance.edges) {
        if (e.from_idx >= satellites.size() || e.to_idx >= satellites.size())
            throw std::runtime_error("No such satellite in given vector.");

        edges.push_back(InterSatelliteLink(&satellites[e.from_idx], &satellites[e.to_idx], cm, e.optional));
    }
}

// ------------------------------------------------------------------------------------------------

PhysicalInstance::PhysicalInstance(const std::string& file) {
    // load instance from file
    std::string line_cache = "";
    int mode = READ_INIT;
    std::ifstream is(file);
    if (is.fail()) {
        std::cout << "File could not be opened!" << std::endl;
        return;
    }

    while (std::getline(is, line_cache)) {
        if (line_cache == "===END===") {
            mode++;
            continue;
        }

        // split line by delimiter ','
        std::string value_cache = "";
        std::stringstream ss(line_cache);

        try {
            switch (mode) {
            case READ_INIT:
                std::getline(ss, value_cache, ',');
                cm.radius_central_mass = std::stof(value_cache);
                std::getline(ss, value_cache, ',');
                cm.gravitational_parameter = std::stof(value_cache);
                break;
            case READ_ORBIT: {
                StateVector sv;
                std::getline(ss, value_cache, ','); // ignore id - order implies id
                std::getline(ss, value_cache, ',');
                sv.height_perigee = std::stof(value_cache);
                std::getline(ss, value_cache, ',');
                sv.eccentricity = std::stof(value_cache);
                std::getline(ss, value_cache, ',');
                sv.initial_true_anomaly = std::stof(value_cache);
                std::getline(ss, value_cache, ',');
                sv.raan = std::stof(value_cache);
                std::getline(ss, value_cache, ',');
                sv.argument_periapsis = std::stof(value_cache);
                std::getline(ss, value_cache, ',');
                sv.inclination = std::stof(value_cache);
                std::getline(ss, value_cache, ',');
                sv.rotation_speed = std::stof(value_cache);
                satellites.push_back(Satellite(sv, cm));
                break;
            }
            case READ_EDGE: {
                std::getline(ss, value_cache, ',');
                int index_orbit_a = std::stoi(value_cache);
                std::getline(ss, value_cache, ',');
                int index_orbit_b = std::stoi(value_cache);
                edges.emplace_back(&satellites.at(index_orbit_a), &satellites.at(index_orbit_b), cm);
                break;
            }
            default:
                break;
            }
        } catch (const std::exception&) {
            std::cout << "Error while loading instance!" << std::endl;
        }
    }

    is.close();
}

// ------------------------------------------------------------------------------------------------

PhysicalInstance& PhysicalInstance::operator=(const PhysicalInstance& source) {
    // check for self-assignment
    if (&source == this)
        return *this;

    cm.radius_central_mass = source.cm.radius_central_mass;
    cm.gravitational_parameter = source.cm.gravitational_parameter;

    // copy all orbits and store old location for edges later
    satellites = source.satellites;
    satellites.shrink_to_fit();
    std::map<const Satellite*, const Satellite*> orbit_map; // old pos: key | new pos: value
    for (int i = 0; i < satellites.size(); i++) {
        orbit_map[&source.satellites[i]] = &satellites[i];
    }

    // edges must point to the new orbit objects
    edges.clear();
    for (const InterSatelliteLink& edge : source.edges) {
        const Satellite* new_v1 = orbit_map[&edge.getV1()];
        const Satellite* new_v2 = orbit_map[&edge.getV2()];
        edges.emplace_back(new_v1, new_v2, cm);
    }
    edges.shrink_to_fit();

    return *this;
}

// ------------------------------------------------------------------------------------------------

void PhysicalInstance::removeInvalidEdges() {
    for (int i = (int)edges.size() - 1; i >= 0; i--) {
        const InterSatelliteLink& e = edges[i];
        bool get_visible = false;
        for (float t = 0.0f; t < e.getPeriod(); t += 1.0f) {
            if (!e.isBlocked(t)) {
                get_visible = true;
                break;
            }
        }

        // remove edge
        if (!get_visible) {
            edges.erase(edges.begin() + i);
        }
    }
    edges.shrink_to_fit();
}

// ------------------------------------------------------------------------------------------------

LineGraph PhysicalInstance::lineGraph() const {
    LineGraph g;

    // init graph
    for (int i = 0; i < edges.size(); i++) {
        g.edges.push_back(AdjacentList());
    }

    for (const auto& vertex : satellites) {
        // find all edges that are connected to the vertex
        AdjacentList adj_list;
        for (int i = 0; i < edges.size(); i++) {
            const InterSatelliteLink& edge = edges[i];
            if (&edge.getV1() == &vertex || &edge.getV2() == &vertex) {
                adj_list.push_back(i);
            }
        }

        // add adjacent edges to every edge that is connected to the vertex
        for (const int i : adj_list) {
            g.edges[i].insert(g.edges[i].begin(), adj_list.begin(), adj_list.end());
        }
    }

    // remove double elements and loops
    for (int i = 0; i < g.edges.size(); i++) {
        AdjacentList& adj_list = g.edges[i];
        std::sort(adj_list.begin(), adj_list.end());
        adj_list.erase(std::unique(adj_list.begin(), adj_list.end()), adj_list.end());
        adj_list.erase(std::find(adj_list.begin(), adj_list.end(), i));
    }

    return g;
}

// ------------------------------------------------------------------------------------------------

bool PhysicalInstance::save(const std::string& file) const {
    std::ofstream fs(file);
    if (fs.fail()) {
        return false;
    }

    /* File format:
     *   radius, gravitational parameter
     *   ===END===
     *   orbit index, a, eccentricity, height_perigee, raan, perigee, inclination, rotation speed
     *   [...]
     *   ===END===
     *   edge orbit index A, edge orbit index B
     *   [...]
     */

    // instance properties
    fs << cm.radius_central_mass << ",";
    fs << cm.gravitational_parameter << "\n";
    fs << "===END===\n";

    // orbits
    std::map<const Satellite*, int> orbit_to_id;
    for (int i = 0; i < satellites.size(); i++) {
        const Satellite& orbit = satellites.at(i);
        orbit_to_id[&orbit] = i;
        fs << i << ",";
        fs << orbit.getHeightPerigee() << ",";
        fs << orbit.getEccentricity() << ",";
        fs << orbit.getTrueAnomaly() << ",";
        fs << orbit.getRaan() << ",";
        fs << orbit.getArgumentPeriapsis() << ",";
        fs << orbit.getInclination() << ",";
        fs << orbit.getRotationSpeed() << "\n";
    }
    fs << "===END===\n";

    // Edges
    for (const auto& edge : edges) {
        int orbit_id_a = orbit_to_id[&edge.getV1()];
        int orbit_id_b = orbit_to_id[&edge.getV2()];
        fs << orbit_id_a << "," << orbit_id_b << "\n";
    }
    fs.close();
    return true;
}

// ------------------------------------------------------------------------------------------------

float rad(const float deg) { return deg * 0.01745329251994329577f; }; // convert degrees to radians
float deg(const float rad) { return rad * 57.2957795130823208768f; }; // convert radians to degrees

} // namespace dmsc