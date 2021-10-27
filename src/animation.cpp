#include "dmsc/animation.hpp"

namespace dmsc {

bool Animation::addSatelliteAnimation(const size_t satellite_idx, const float t_begin, const float t_end,
                                      const AnimationDetails& animation) {

    TimelineEvent<AnimationDetails> event(t_begin, t_end, animation);
    return satellites[satellite_idx].insert(event);
}

// ------------------------------------------------------------------------------------------------

bool Animation::addISLAnimation(const size_t isl_idx, const float t_begin, const float t_end,
                                const AnimationDetails& animation) {

    TimelineEvent<AnimationDetails> event(t_begin, t_end, animation);
    return intersatellite_links[isl_idx].insert(event);
}

// ------------------------------------------------------------------------------------------------

bool Animation::addOrientationAnimation(const size_t satellite_idx, const float t,
                                        const OrientationDetails& orientation) {
    TimelineEvent<OrientationDetails> event(t, t, orientation);
    return satellite_orientations[satellite_idx].insert(event);
}

// ------------------------------------------------------------------------------------------------

std::pair<bool, AnimationDetails> Animation::getSatelliteAnimation(const size_t satellite_idx, const float t) const {
    // 1. are there animations scheduled for the satellite at all?
    auto it = satellites.find(satellite_idx);
    if (it == satellites.end()) {
        return {false, AnimationDetails()};
    }

    // 2. are there animation details active right now?
    auto event = it->second.prevailingEvent(t);
    if (event.isValid() && event.t_begin <= t) { // the prevailing event must not be active rn - we have to check!
        return {true, event.data};
    }

    return {false, AnimationDetails()};
}

// ------------------------------------------------------------------------------------------------

std::pair<bool, AnimationDetails> Animation::getISLAnimation(const size_t isl_idx, const float t) const {
    // 1. are there animations scheduled for the isl at all?
    auto it = intersatellite_links.find(isl_idx);
    if (it == intersatellite_links.end()) {
        return {false, AnimationDetails()};
    }

    // 2. are there animation details active right now?
    auto event = it->second.prevailingEvent(t);
    if (event.isValid() && event.t_begin <= t) { // the prevailing event must not be active rn - we have to check!
        return {true, event.data};
    }

    return {false, AnimationDetails()};
}

} // namespace dmsc
