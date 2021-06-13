#ifndef DMSC_INSTANCE_H
#define DMSC_INSTANCE_H

#include "edge.hpp"
#include "satellite.hpp"
#include <map>
#include <string>
#include <vector>

namespace dmsc {

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

  private:
    CentralMass cm;
    enum FileReadingMode { READ_INIT, READ_ORBIT, READ_EDGE }; // order must match blocks in file-format
    std::vector<Satellite> satellites;
    std::vector<InterSatelliteLink> edges;
};

// ------------------------------------------------------------------------------------------------

float rad(const float deg); // convert degrees to radians
float deg(const float rad); // convert radians to degrees

} // namespace dmsc

#endif
