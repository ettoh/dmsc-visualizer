#ifndef DMSC_EDGE_H
#define DMSC_EDGE_H

#include "dmsc/glm_include.hpp"
#include "satellite.hpp"
#include "timeline.hpp"
#include <vector>

namespace dmsc {

/**
 * @brief Bidirectional intersatellite link between two satellites A and B.
 */
class InterSatelliteLink {
  private:
    const Satellite* v1;
    const Satellite* v2;
    uint32_t v1_idx;
    uint32_t v2_idx;
    float period;   // [sec] time until satellite constellations repeat
    CentralMass cm; // properties of the central mass

  public:
    /**
     * @brief Bidirectional intersatellite link between two satellites A and B.
     *
     * @param satellites vector reference containing all satellites
     * @param v1_idx index of satellite A in the given vector reference
     * @param v2_idx index of satellite B in the given vector reference
     * @param cm information about the central mass
     */
    InterSatelliteLink(const uint32_t& v1_idx, const uint32_t& v2_idx, const std::vector<Satellite>& satellites,
                       const CentralMass cm);

    /**
     * @brief Returns true, if the edge is blocked at the given time.
     */
    bool isBlocked(const float time) const;

    /**
     * @brief Returns true, if there is enough time for both satellites to face each other at the given time.
     *
     * @param sat1 & sat 2 Direction in which a satellite is facing and the time when it changed this direction for
     * the last time.
     * @param t [sec] Time when the satellites have to face each other.
     */
    bool canAlign(const TimelineEvent<glm::vec3>& sat1, const TimelineEvent<glm::vec3>& sat2, const float t) const;

    /**
     * @brief Calculate the directions for both satellites to face each other. Because both satellites have to face each
     * other, the direction of satellite A is the negative direction of satellite B.
     * @param t [sec] time
     * @return Direction vector for one of the satellites (origin) at the time when they face each other.
     */
    glm::vec3 getOrientation(const float time) const;

    // GETTER
    float getPeriod() const { return period; }
    const Satellite& getV1() const { return *v1; }
    const Satellite& getV2() const { return *v2; }
    uint32_t getV1Idx() const { return v1_idx; }
    uint32_t getV2Idx() const { return v2_idx; }
    float getRadiusCentralMass() const { return cm.radius_central_mass; }
};

} // namespace dmsc

#endif
