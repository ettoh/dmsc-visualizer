#ifndef DMSC_INSTANCE_H
#define DMSC_INSTANCE_H

#include "edge.hpp"
#include "satellite.hpp"
#include <map>
#include <string>
#include <vector>

namespace dmsc {

using ScheduledCommunication = std::pair<uint32_t, uint32_t>;

/**
 * @brief TODO
 *
 */
class AdjacencyMatrix {
  public:
    struct Item {
        uint32_t weight = ~0u;
        uint32_t isl_idx = ~0u; // index of isl-object in physical instance
        Item() = default;
        Item(const uint32_t weight, const uint32_t isl_idx)
            : weight(weight)
            , isl_idx(isl_idx) {}
    };

    AdjacencyMatrix() = delete;
    AdjacencyMatrix(const size_t size, const Item& default_value);
    std::vector<Item>& operator[](size_t row) { return matrix[row]; }
    const std::vector<Item>& operator[](size_t row) const { return matrix[row]; }
    void clear();

    std::vector<std::vector<Item>> matrix;
};

// ------------------------------------------------------------------------------------------------

enum class EdgeType {
    INTERSATELLITE_LINK,     // undirected; two satellites can communicate via ISL
    SCHEDULED_COMMUNICATION, // directed; satellite A has to send data to satellite B
};

/**
 * @brief Edge between two vertices.
 */
struct Edge {
    uint32_t from_idx; // Index of vertices. If Edge is not bidirectional, this is the origin.
    uint32_t to_idx;   // Index of vertices. If Edge is not bidirectional, this is the target.
    EdgeType type;

    Edge(const uint32_t from_idx, const uint32_t to_idx, const EdgeType type = EdgeType::INTERSATELLITE_LINK)
        : from_idx(from_idx)
        , to_idx(to_idx)
        , type(type) {}
};

// ------------------------------------------------------------------------------------------------

/**
 * @brief TODO
 *
 */
struct Instance {
    CentralMass cm;
    std::vector<StateVector> satellites;
    std::vector<Edge> edges;

    Instance() = default;

    /**
     * @brief Construct a new Instance object from a given file.
     *
     * @param file Path to file where the instance is stored.
     */
    Instance(const std::string& file);

    /**
     * @brief Save the instance to the given file.
     */
    void save(const std::string& file) const;

  private:
    enum FileReadingMode { READ_INIT, READ_ORBIT, READ_EDGE }; // order must match blocks in file-format
};

// ------------------------------------------------------------------------------------------------

/**
 * @brief TODO
 *
 */
class PhysicalInstance {
  public:
    PhysicalInstance() = default;
    PhysicalInstance(const Instance& raw_instance);
    PhysicalInstance(const PhysicalInstance& source); // Copy
    PhysicalInstance& operator=(const PhysicalInstance& source);

    /**
     * @brief Removes all intersatellite links that will never be visible. I.e. if an ISL is always blocked by the
     * central mass, it will be removed.
     */
    void removeInvalidISL();

    std::vector<ScheduledCommunication> scheduled_communications;

    // GETTER
    float getRadiusCentralMass() const { return cm.radius_central_mass; }
    const std::vector<Satellite>& getSatellites() const { return satellites; }
    const std::vector<InterSatelliteLink>& getISL() const { return intersatellite_links; }
    const AdjacencyMatrix& getAdjacencyMatrix() const { return adjacency_matrix; }
    size_t islCount() const { return intersatellite_links.size(); }
    size_t satelliteCount() const { return satellites.size(); }

  private:
    std::vector<Satellite> satellites;
    std::vector<InterSatelliteLink> intersatellite_links;
    AdjacencyMatrix adjacency_matrix = AdjacencyMatrix(0, AdjacencyMatrix::Item(~0u, ~0u));
    CentralMass cm;
    enum FileReadingMode { READ_INIT, READ_ORBIT, READ_EDGE }; // order must match blocks in file-format

    void buildAdjacencyMatrix();
};

// ------------------------------------------------------------------------------------------------

float rad(const float deg); // convert degrees to radians
float deg(const float rad); // convert radians to degrees

} // namespace dmsc

#endif
