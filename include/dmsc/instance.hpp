#ifndef DMSC_INSTANCE_H
#define DMSC_INSTANCE_H

#include "edge.hpp"
#include "satellite.hpp"
#include <map>
#include <string>
#include <vector>

namespace dmsc {

/// Indices of two satellites that are supposed to communicate.
using ScheduledCommunication = std::pair<uint32_t, uint32_t>;

// ------------------------------------------------------------------------------------------------

/**
 * @brief Adjacency list that describes which satellites can communicate with each other and at what costs.
 *
 * Instead of an adjacency matrix, we use an adjacency list to avoid superfluous entries and thus significantly reduce
 * memory consumption. In order to still be able to access elements directly, we store the data of each row in a map.
 */
class AdjacencyList {
  public:
    /**
     * @brief Struct that holds all properties of an communication between two satellites.
     */
    struct Item {
        uint32_t weight = ~0u;  // costs of this communication
        uint32_t isl_idx = ~0u; // index of a isl-object in the physical instance
        Item() = default;
        Item(const uint32_t weight, const uint32_t isl_idx)
            : weight(weight)
            , isl_idx(isl_idx) {}
    };

    /**
     * @brief In order to be able to access elements directly (like an adjacency matrix), we store the data of each
     * row in a map instead of a list. This way we can acces elements by matrix[row][column].
     */
    std::vector<std::map<uint32_t, Item>> matrix;

    AdjacencyList() = delete;
    AdjacencyList(const size_t size, const Item& default_value);
    std::map<uint32_t, Item>& operator[](size_t row) { return matrix[row]; }
    const std::map<uint32_t, Item>& operator[](size_t row) const { return matrix[row]; }
    void clear();
};

// ------------------------------------------------------------------------------------------------

/**
 * @brief Different types of edges between satellites.
 */
enum class EdgeType {
    INTERSATELLITE_LINK,     // undirected; two satellites can communicate via ISL
    SCHEDULED_COMMUNICATION, // directed; satellite A has to send data to satellite B
};

// ------------------------------------------------------------------------------------------------

/**
 * @brief Edge between two satellites.
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
 * @brief Contains all necessary data for the movement of satellites and the graphs that define the connections between
 * satellites. No additional calculations are performed. You can change everything anytime you want.
 * All derived values for the visualization and solvers are only calculated when the instance is converted into a
 * PhysicalInstance.
 *
 * An instance is basically a snapshot of satellites moving around a central mass. The current position of satellites
 * and their movement is defined. From there (t = 0) we can calculate the satellite positions at a given time t.
 */
struct Instance {
    CentralMass cm; // Properties of the central mass
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
     *
     * @param file Path to the location where the instance should be saved.
     */
    void save(const std::string& file) const;

  private:
    enum FileReadingMode { READ_INIT, READ_ORBIT, READ_EDGE }; // order must match blocks in file-format
};

// ------------------------------------------------------------------------------------------------

/**
 * @brief Takes an instance and calculate additional values that are needed in order to evaluate this instance
 * (visualization and solver). Once this is done, you cannot change the resulting physical instance. Don't try to get
 * around this - you could get wrong result without noticing!
 */
class PhysicalInstance {
  public:
    PhysicalInstance() = default;
    PhysicalInstance(const Instance& raw_instance);
    PhysicalInstance(const PhysicalInstance& source);
    PhysicalInstance& operator=(const PhysicalInstance& source);

    /**
     * @brief Removes all intersatellite links that will never be visible. I.e. if an ISL is always blocked by the
     * central mass, it will be removed.
     */
    void removeInvalidISL();

    // ISLs are stored in an adjacency list; scheduled communications are stored in this vector
    std::vector<ScheduledCommunication> scheduled_communications;

    // GETTER
    float getRadiusCentralMass() const { return cm.radius_central_mass; }
    const std::vector<Satellite>& getSatellites() const { return satellites; }
    const std::vector<InterSatelliteLink>& getISLs() const { return intersatellite_links; }
    const AdjacencyList& getAdjacencyMatrix() const { return adjacency_list; }
    size_t islCount() const { return intersatellite_links.size(); }
    size_t satelliteCount() const { return satellites.size(); }

  private:
    std::vector<Satellite> satellites;
    std::vector<InterSatelliteLink> intersatellite_links;
    AdjacencyList adjacency_list = AdjacencyList(0, AdjacencyList::Item(~0u, ~0u));
    CentralMass cm;
    enum FileReadingMode { READ_INIT, READ_ORBIT, READ_EDGE }; // order must match blocks in file-format

    void buildAdjacencyMatrix();
};

// ------------------------------------------------------------------------------------------------

float rad(const float deg); // converts degrees to radians
float deg(const float rad); // converts radians to degrees

} // namespace dmsc

#endif
