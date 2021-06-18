#include "dmsc/instance.hpp"
#include <algorithm>
#include <ctime>
#include <fstream>
#include <map>
#include <random>
#include <set>
#include <sstream>

namespace dmsc {

// ========================
// = Adjacency matrix
// ========================

AdjacencyMatrix::AdjacencyMatrix(const size_t size, const Item& default_value) {
    matrix.reserve(size);
    matrix.assign(size, std::vector<Item>());
    for (auto& list : matrix) {
        list.reserve(size);
        list.assign(size, default_value);
    }
}

// ------------------------------------------------------------------------------------------------

void AdjacencyMatrix::clear() {
    size_t size = matrix.capacity();
    matrix.clear();
    matrix.assign(size, std::vector<Item>());
    for (auto& list : matrix) {
        list.reserve(size);
        list.assign(size, Item());
    }
}

// ------------------------------------------------------------------------------------------------

std::vector<AdjacencyMatrix::Item> AdjacencyMatrix::column(const size_t column) const {
    std::vector<Item> result;
    result.reserve(matrix.size());
    for (int i = 0; i < matrix.size(); i++) {
        result.push_back(matrix[i][column]);
    }
    return result;
}

// ------------------------------------------------------------------------------------------------

std::vector<AdjacencyMatrix::Item> AdjacencyMatrix::row(const size_t row) const { return matrix[row]; }

// ------------------------------------------------------------------------------------------------

// ========================
// = Instance
// ========================

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
                sv.initial_true_anomaly = std::stof(value_cache);
                satellites.push_back(sv);
                break;
            }
            case READ_EDGE: {
                std::getline(ss, value_cache, ',');
                uint32_t from_idx = std::stoi(value_cache);
                std::getline(ss, value_cache, ',');
                uint32_t to_idx = std::stoi(value_cache);
                std::getline(ss, value_cache, ',');
                int type = std::stoi(value_cache);
                Edge e = Edge(from_idx, to_idx, static_cast<EdgeType>(type));
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
     * from_vertex id #1, to_vertex id #2, type
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
        fs << static_cast<int>(e.type) << "\n";
    }
    fs.close();
}

// ------------------------------------------------------------------------------------------------

// ========================
// = Physical Instance
// ========================

// TODO get rid
PhysicalInstance::PhysicalInstance(const PhysicalInstance& source) {
    cm.radius_central_mass = source.cm.radius_central_mass;
    cm.gravitational_parameter = source.cm.gravitational_parameter;
    satellites = source.satellites;
    adjacency_matrix = source.adjacency_matrix;
    scheduled_communications = source.scheduled_communications;

    // edges must point to the new orbit objects
    for (const InterSatelliteLink& isl : source.intersatellite_links) {
        intersatellite_links.push_back(InterSatelliteLink(isl.getV1Idx(), isl.getV2Idx(), satellites, cm));
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
    for (const auto& e : raw_instance.edges) {
        if (e.from_idx >= satellites.size() || e.to_idx >= satellites.size())
            throw std::runtime_error("No such satellite in given vector.");

        switch (e.type) {
        case EdgeType::INTERSATELLITE_LINK:
            intersatellite_links.push_back(InterSatelliteLink(e.from_idx, e.to_idx, satellites, cm));
            break;
        case EdgeType::SCHEDULED_COMMUNICATION:
            scheduled_communications.push_back({e.from_idx, e.to_idx});
            break;
        default:
            break;
        }
    }

    buildAdjacencyMatrix();
}

// ------------------------------------------------------------------------------------------------

void PhysicalInstance::buildAdjacencyMatrix() {
    adjacency_matrix = AdjacencyMatrix(satellites.size(), AdjacencyMatrix::Item(0u, ~0u));

    // fill matrix
    for (uint32_t isl_idx = 0; isl_idx < intersatellite_links.size(); isl_idx++) {
        const InterSatelliteLink& isl = intersatellite_links[isl_idx];
        adjacency_matrix[isl.getV1Idx()][isl.getV2Idx()] = AdjacencyMatrix::Item(1u, isl_idx);
        adjacency_matrix[isl.getV2Idx()][isl.getV1Idx()] = AdjacencyMatrix::Item(1u, isl_idx);
    }
}

// ------------------------------------------------------------------------------------------------

// TODO get rid
PhysicalInstance& PhysicalInstance::operator=(const PhysicalInstance& source) {
    // check for self-assignment
    if (&source == this)
        return *this;

    cm.radius_central_mass = source.cm.radius_central_mass;
    cm.gravitational_parameter = source.cm.gravitational_parameter;
    satellites = source.satellites;
    satellites.shrink_to_fit();
    adjacency_matrix = source.adjacency_matrix;
    scheduled_communications = source.scheduled_communications;

    // edges must point to the new orbit objects
    intersatellite_links.clear();
    for (const InterSatelliteLink& isl : source.intersatellite_links) {
        intersatellite_links.push_back(InterSatelliteLink(isl.getV1Idx(), isl.getV2Idx(), satellites, cm));
    }

    intersatellite_links.shrink_to_fit();
    return *this;
}

// ------------------------------------------------------------------------------------------------

void PhysicalInstance::removeInvalidISL() {
    for (int i = (int)intersatellite_links.size() - 1; i >= 0; i--) {
        const InterSatelliteLink& e = intersatellite_links[i];
        bool get_visible = false;
        for (float t = 0.0f; t < e.getPeriod(); t += 1.0f) {
            if (!e.isBlocked(t)) {
                get_visible = true;
                break;
            }
        }

        // remove edge
        if (!get_visible) {
            intersatellite_links.erase(intersatellite_links.begin() + i);
        }
    }
    intersatellite_links.shrink_to_fit();
    buildAdjacencyMatrix();
}

// ------------------------------------------------------------------------------------------------

float rad(const float deg) { return deg * 0.01745329251994329577f; }; // convert degrees to radians
float deg(const float rad) { return rad * 57.2957795130823208768f; }; // convert radians to degrees

} // namespace dmsc
