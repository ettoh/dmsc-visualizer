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

// ========================
// = Physical Instance
// ========================

PhysicalInstance::PhysicalInstance(const PhysicalInstance& source) {
    cm.radius_central_mass = source.cm.radius_central_mass;
    cm.gravitational_parameter = source.cm.gravitational_parameter;

    // copy all orbits and store old location for edges later
    satellites = source.satellites;

    // edges must point to the new orbit objects
    for (const InterSatelliteLink& edge : source.edges) {
        edges.push_back(InterSatelliteLink(edge.getV1Idx(), edge.getV2Idx(), satellites, cm, edge.isOptional()));
    }

    adjacency_matrix = source.adjacency_matrix;
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

        edges.push_back(InterSatelliteLink(e.from_idx, e.to_idx, satellites, cm, e.optional));
    }

    buildAdjacencyMatrix();
}

// ------------------------------------------------------------------------------------------------

void PhysicalInstance::buildAdjacencyMatrix() {
    adjacency_matrix = AdjacencyMatrix(edges.size(), AdjacencyMatrix::Item(0u, ~0u));

    // fill matrix
    for (uint32_t edge_idx = 0; edge_idx < edges.size(); edge_idx++) {
        const InterSatelliteLink& isl = edges[edge_idx];
        adjacency_matrix.matrix[isl.getV1Idx()][isl.getV2Idx()] = AdjacencyMatrix::Item(1u, edge_idx);
        adjacency_matrix.matrix[isl.getV2Idx()][isl.getV1Idx()] = AdjacencyMatrix::Item(1u, edge_idx);
    }
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

    // edges must point to the new orbit objects
    edges.clear();
    for (const InterSatelliteLink& edge : source.edges) {
        edges.push_back(InterSatelliteLink(edge.getV1Idx(), edge.getV2Idx(), satellites, cm, edge.isOptional()));
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

float rad(const float deg) { return deg * 0.01745329251994329577f; }; // convert degrees to radians
float deg(const float rad) { return rad * 57.2957795130823208768f; }; // convert radians to degrees

} // namespace dmsc
