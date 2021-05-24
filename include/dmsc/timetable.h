#ifndef TIMETABLE_H
#define TIMETABLE_H

#include "satellite.h"
#include <map>
#include <set>

namespace dmsc {

constexpr float TIMELINE_ERR = std::numeric_limits<float>::infinity();

// ------------------------------------------------------------------------------------------------

/**
 * @brief TODO
 *
 * @tparam PayloadData
 */
// TODO move into Timeline?
template <typename PayloadData>
struct TimelineEvent {
    float t_begin;    // time when the event starts
    float t_end;      // time when the event ends
    PayloadData data; // data that is associated with the event

    /**
     * @brief Construct a new Timeline Event object.
     */
    TimelineEvent(const float t_begin, const float t_end, const PayloadData& data = PayloadData())
        : t_begin(t_begin)
        , t_end(t_end)
        , data(data) {}

    /**
     * @brief In a timeline the events must not overlap.
     * An event is smaller than another only if all values in the time interval are smaller.
     * This ensures that the lower_bound() function of a timeline works properly.
     */
    friend bool operator<(const TimelineEvent& l, const TimelineEvent& r) {
        return l.t_begin < r.t_begin && l.t_end < r.t_end;
    }

    /**
     * @return true, if the event is valid. That means that t_end >= t_begin, the time values are positive and no error
     * value was used.
     */
    bool isValid() const {
        return t_end >= t_begin && t_begin >= 0.f && t_begin != TIMELINE_ERR && t_end != TIMELINE_ERR;
    }
};

// ------------------------------------------------------------------------------------------------

/**
 * @brief TODO
 *
 * @tparam PayloadData
 */
template <typename PayloadData>
class Timeline {
  public:
    /**
     * @brief Construct a new Timeline object
     */
    Timeline() = default;

    /**
     * @brief Erases all events in this timeline.
     */
    void clear() { events.clear(); }

    /**
     * @brief Return the number of events in this timeline.
     */
    size_t size() const { return events.size(); }

    /**
     * @brief Insert a new event into this timeline.
     * The event must be valid and must not overlap with previously inserted events.
     *
     * @return true, if the event was inserted into this timeline
     */
    bool insert(const TimelineEvent<PayloadData>& event) {
        if (!event.isValid()) {
            return false;
        }

        float t_next = nextTimeWithEvent(event.t_begin, false);
        if (t_next != -1.f && t_next <= event.t_end) {
            // there is another event that is currently active or will be active soon
            // does these events overlap? -> invalid!
            return false;
        }

        // no problems found -> try to insert it ...
        return events.insert(event).second;
    }

    /**
     * @brief Remove an event from this timeline.
     */
    void remove(const TimelineEvent<PayloadData>& event) {
        auto it = events.find(event);
        if (it != events.end()) {
            events.erase(it);
        }
    }

    /**
     * @brief Return the time when the next event will become active. If an event is active at the given time, the given
     * time will be returned.
     * If there is no event after the given time and loops are allowed, we will look for active events from the
     * beginning. The resulting time will be less than the given time.
     *
     * If no valid time is found, -1 is returned.
     */
    float nextTimeWithEvent(const float t, const bool allow_loop = false) const {
        if (events.size() == 0) {
            return -1.f; // TODO error handling
        }

        TimelineEvent<PayloadData> tmp(t, t);
        auto e = events.lower_bound(tmp); // the first element that is NOT less than tmp (see "<"" of TimelineEvent)
        if (e != events.end()) {
            if (e->t_begin <= t) { // next event is currently active
                return t;
            } else { // next event is ahead
                return e->t_begin;
            }
        } else if (allow_loop) { // no element found -> restart at the beginning?
            return events.begin()->t_begin;
        }

        // TODO error handling
        return -1.f; // no element NOT less than tmp AND no restart allowed
    }

    /**
     * @brief Return the first event that will become active (starting from the given time). If an event is already
     * active at the given time, this event will be returned.
     *
     * If there is no event active after the given time and loops are allowed, we will look for events from the
     * beginning. The resulting event will start before the given time.
     *
     * If no valid time is found, an invalid event is returned.
     */
    TimelineEvent<PayloadData> prevailingEvent(const float t, const bool allow_loop = false) const {
        if (events.size() == 0) {
            return TimelineEvent<PayloadData>(TIMELINE_ERR, -1.f);
        }

        TimelineEvent<PayloadData> tmp(t, t);
        auto e = events.lower_bound(tmp); // the first element that is NOT less than tmp (see "<"" of TimelineEvent)
        if (e != events.end()) {
            return *e;
        } else if (allow_loop) { // no element found -> restart at the beginning?
            return *events.begin();
        }

        return TimelineEvent<PayloadData>(TIMELINE_ERR, -1.f); // no element NOT less than tmp AND no restart allowed
    }

    /**
     * @brief Return the last event that ended before the given time. The returned event won't be active.
     *
     * If there is no event before the given time and loops are allowed, then we will look for events from the
     * end. The resulting event will end after the given time.
     *
     * If no valid time is found, an invalid event is returned.
     */
    TimelineEvent<PayloadData> previousEvent(const float t, const bool allow_loop = false) const {
        if (events.size() == 0) {
            return TimelineEvent<PayloadData>(TIMELINE_ERR, -1.f);
        }

        TimelineEvent<PayloadData> tmp(t, t);
        auto e = events.lower_bound(tmp); // the first element that is NOT less than tmp (see "<"" of TimelineEvent)
        if (e != events.begin()) {
            return *--e; // there is at least one element (see above) -> even if e is end(), we will get a valid event
        } else if (allow_loop) { // e is the first element in the list -> the previous element is the last one
            return *--events.end();
        }

        return TimelineEvent<PayloadData>(TIMELINE_ERR, -1.f); // no element NOT less than tmp AND no restart allowed
    }

  private:
    std::set<TimelineEvent<PayloadData>> events;
};

// ------------------------------------------------------------------------------------------------

template <typename Key, typename Value>
struct Timetable {
  public:
    Timetable() = default;

    void add(const Key* key, const Value& value) { timetable[key].emplace(value); }

    void clear() { timetable.clear(); }
    size_t size(const Key* e) { return timetable[e].size(); }
    Value first(const Key* e) { return *timetable[e].begin(); }

    Value previous(const Key* key, const float time) const {
        // find set
        auto it_edge = timetable.find(key);
        if (it_edge == timetable.end()) {
            return Value(-1.0f);
        }

        // find first item in edge set that is lower than the refrence
        Value search_reference;
        search_reference.start = time;
        auto it_slot = it_edge->second.lower_bound(search_reference);

        if (it_slot == it_edge->second.end() && it_edge->second.size() == 0) {
            return Value(-1.0f);
        } else if (it_slot == it_edge->second.end()) {
            --it_slot;
            return *it_slot;
        }

        // prev timeslot begins at current time
        if (it_slot->start == time) {
            return *it_slot;
        }

        // get the PREVIOUS element not the first one that is not less than key (as it is implemented in set)
        if (it_slot != it_edge->second.begin()) {
            --it_slot;
        } else if (it_slot->start > time) {
            return Value(-1.0f);
        }

        return *it_slot;
    }

    Value next(const Key* orbit, const float time) const {
        // find set
        auto it_edge = timetable.find(orbit);
        if (it_edge == timetable.end()) {
            return Value(-1.0f);
        }

        // find first item in edge set that is lower than the refrence
        Value search;
        search.start = time;
        auto it_slot = it_edge->second.upper_bound(search);
        if (it_slot == it_edge->second.end()) {
            return Value(-1.0f);
        }

        return *it_slot;
    }

  private:
    std::map<const Key*, std::set<Value>> timetable;
};

} // namespace dmsc

#endif // !TIMETABLE_H
