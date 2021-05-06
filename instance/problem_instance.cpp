#include "problem_instance.h"
#include <algorithm>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <random>
#include <ctime>

ProblemInstance::ProblemInstance(const ProblemInstance& source) {
    radius_central_mass = source.radius_central_mass;
    gravitational_parameter = source.gravitational_parameter;

    // copy all orbits and store old location for edges later
    orbits = source.orbits;
    std::map<const Orbit*, const Orbit*> orbit_map; // old pos: key | new pos: value
    for (int i = 0; i < orbits.size(); i++) {
        orbit_map[&source.orbits[i]] = &orbits[i];
    }

    // edges must point to the new orbit objects
    for (const Edge& edge : source.edges) {
        const Orbit* new_v1 = orbit_map[&edge.getV1()];
        const Orbit* new_v2 = orbit_map[&edge.getV2()];
        edges.emplace_back(new_v1, new_v2, edge.getRadiusCentralMass());
    }
}

ProblemInstance::ProblemInstance(const std::string& file) {
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
                radius_central_mass = std::stof(value_cache);
                std::getline(ss, value_cache, ',');
                gravitational_parameter = std::stof(value_cache);
                break;
            case READ_ORBIT: {
                std::getline(ss, value_cache, ','); // ignore id - order implies id
                std::getline(ss, value_cache, ',');
                float a = std::stof(value_cache);
                std::getline(ss, value_cache, ',');
                float eccentricity = std::stof(value_cache);
                std::getline(ss, value_cache, ',');
                float anomaly = std::stof(value_cache);
                std::getline(ss, value_cache, ',');
                float raan = std::stof(value_cache);
                std::getline(ss, value_cache, ',');
                float perigee = std::stof(value_cache);
                std::getline(ss, value_cache, ',');
                float inclination = std::stof(value_cache);
                std::getline(ss, value_cache, ',');
                float rotation_speed = std::stof(value_cache);
                orbits.emplace_back(true, gravitational_parameter, a, anomaly, inclination, raan, perigee,
                                    rotation_speed, eccentricity);
                break;
            }
            case READ_EDGE: {
                std::getline(ss, value_cache, ',');
                int index_orbit_a = std::stoi(value_cache);
                std::getline(ss, value_cache, ',');
                int index_orbit_b = std::stoi(value_cache);
                edges.emplace_back(&orbits.at(index_orbit_a), &orbits.at(index_orbit_b), radius_central_mass);
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

ProblemInstance& ProblemInstance::operator=(const ProblemInstance& source) {
    // check for self-assignment
    if (&source == this)
        return *this;

    radius_central_mass = source.radius_central_mass;
    gravitational_parameter = source.gravitational_parameter;

    // copy all orbits and store old location for edges later
    orbits = source.orbits;
    orbits.shrink_to_fit();
    std::map<const Orbit*, const Orbit*> orbit_map; // old pos: key | new pos: value
    for (int i = 0; i < orbits.size(); i++) {
        orbit_map[&source.orbits[i]] = &orbits[i];
    }

    // edges must point to the new orbit objects
    edges.clear();
    for (const Edge& edge : source.edges) {
        const Orbit* new_v1 = orbit_map[&edge.getV1()];
        const Orbit* new_v2 = orbit_map[&edge.getV2()];
        edges.emplace_back(new_v1, new_v2, edge.getRadiusCentralMass());
    }
    edges.shrink_to_fit();

    return *this;
}

void ProblemInstance::removeInvalidEdges() {
    for (int i = (int)edges.size() - 1; i >= 0; i--) {
        const Edge& e = edges[i];
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

LineGraph ProblemInstance::lineGraph() const {
    LineGraph g;

    // init graph
    for (int i = 0; i < edges.size(); i++) {
        g.edges.push_back(AdjacentList());
    }

    for (const auto& vertex : orbits) {
        // find all edges that are connected to the vertex
        AdjacentList adj_list;
        for (int i = 0; i < edges.size(); i++) {
            const Edge& edge = edges[i];
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

bool ProblemInstance::save(const std::string& file) const {
    std::ofstream fs(file);
    if (fs.fail()) {
        return false;
    }

    /* File format:
     *   radius, gravitational parameter
     *   ===END===
     *   orbit index, a, eccentricity, anomaly, raan, perigee, inclination, rotation speed
     *   [...]
     *   ===END===
     *   edge orbit index A, edge orbit index B
     *   [...]
     */

    // instance properties
    fs << radius_central_mass << ",";
    fs << gravitational_parameter << "\n";
    fs << "===END===\n";

    // orbits
    std::map<const Orbit*, int> orbit_to_id;
    for (int i = 0; i < orbits.size(); i++) {
        const Orbit& orbit = orbits.at(i);
        orbit_to_id[&orbit] = i;
        fs << i << ",";
        fs << orbit.getSemiMajorAxis() << ",";
        fs << orbit.getEccentricity() << ",";
        fs << orbit.getTrueAnomaly() << ",";
        fs << orbit.getRaan() << ",";
        fs << orbit.getArgumentPeriapsis() << ",";
        fs << orbit.getInclination() << ",";
        fs << orbit.getMeanRotationSpeed() << "\n";
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