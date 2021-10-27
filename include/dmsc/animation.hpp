#ifndef DMSC_ANIMATION_H
#define DMSC_ANIMATION_H

#include "glm_include.hpp"
#include "timeline.hpp"
#include <map>

namespace dmsc {

/**
 * @brief TODO
 *
 */
struct AnimationDetails {
    bool visible = true;
    glm::vec4 color = glm::vec4(1.f);

    AnimationDetails() = default;
    AnimationDetails(const bool visible, const glm::vec4& color = glm::vec4(1.f))
        : visible(visible)
        , color(color){};
};

// ------------------------------------------------------------------------------------------------

/**
 * @brief TODO
 *
 */
struct OrientationDetails {
    glm::vec3 orientation = glm::vec3(0.f);
    float cone_length = 0.5f;

    OrientationDetails() = default;
    OrientationDetails(const glm::vec3& orientation, const float cone_length = 0.5f)
        : orientation(orientation)
        , cone_length(cone_length){};
};

// ------------------------------------------------------------------------------------------------

/**
 * @brief TODO
 * id of satellite refers the position in vector (instance)
 *
 */
struct Animation {
    std::map<size_t, Timeline<AnimationDetails>> satellites;
    std::map<size_t, Timeline<AnimationDetails>> intersatellite_links;
    std::map<size_t, Timeline<OrientationDetails>> satellite_orientations; // TODO ONLY time point; NO interval

    bool addSatelliteAnimation(const size_t satellite_idx, const float t_begin, const float t_end,
                               const AnimationDetails& animation);
    bool addISLAnimation(const size_t isl_idx, const float t_begin, const float t_end,
                         const AnimationDetails& animation);
    bool addOrientationAnimation(const size_t satellite_idx, const float t, const OrientationDetails& orientation);

    std::pair<bool, AnimationDetails> getSatelliteAnimation(const size_t satellite_idx, const float t) const;
    std::pair<bool, AnimationDetails> getISLAnimation(const size_t isl_idx, const float t) const;
};

} // namespace dmsc

#endif
