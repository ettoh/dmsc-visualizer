#ifndef DMSC_INSTANCE_H
#define DMSC_INSTANCE_H

#include "edge.hpp"
#include "satellite.hpp"
#include <map>
#include <string>
#include <vector>

namespace dmsc {

/**
 * @brief TODO
 *
 */
class AdjacencyMatrix {
  public:
    struct Item {
        uint32_t weight = ~0u;
        uint32_t edge_idx = ~0u; // index of isl-object in physical instance
        Item() = default;
        Item(const uint32_t weight, const uint32_t edge_idx)
            : weight(weight)
            , edge_idx(edge_idx) {}
    };

    AdjacencyMatrix() = delete;
    AdjacencyMatrix(const size_t size, const Item& default_value = Item());

    std::vector<std::vector<Item>> matrix;

    void clear();

    // GETTER
    std::vector<Item> column(const size_t column) const;
    std::vector<Item> row(const size_t row) const;
};

// ------------------------------------------------------------------------------------------------

/**
 * @brief Edge between two vertices.
 */
struct Edge {
    uint32_t from_idx; // Index of vertices. If Edge is not bidirectional, this is the origin.
    uint32_t to_idx;   // Index of vertices. If Edge is not bidirectional, this is the target.
    bool optional;     // If true, no communication is scheduled for this edge.

    Edge(const uint32_t from_idx, const uint32_t to_idx, const bool optional = false)
        : from_idx(from_idx)
        , to_idx(to_idx)
        , optional(optional) {}
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

    void removeInvalidEdges();

    // GETTER
    float getRadiusCentralMass() const { return cm.radius_central_mass; }
    const std::vector<Satellite>& getSatellites() const { return satellites; }
    const std::vector<InterSatelliteLink>& getEdges() const { return edges; }
    const AdjacencyMatrix& getAdjacencyMatrix() const { return adjacency_matrix; }
    const size_t edgeSize() const { return edges.size(); }
    const size_t satelliteSize() const { return satellites.size(); }

  private:
    std::vector<Satellite> satellites;
    std::vector<InterSatelliteLink> edges;
    CentralMass cm;
    AdjacencyMatrix adjacency_matrix = AdjacencyMatrix(0);
    enum FileReadingMode { READ_INIT, READ_ORBIT, READ_EDGE }; // order must match blocks in file-format

    void buildAdjacencyMatrix();
};

// ------------------------------------------------------------------------------------------------

float rad(const float deg); // convert degrees to radians
float deg(const float rad); // convert radians to degrees

} // namespace dmsc

#endif
