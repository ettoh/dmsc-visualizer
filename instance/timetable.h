#ifndef TIMETABLE_H
#define TIMETABLE_H

#include "orbit.h"
#include "vector3d.h"
#include <map>
#include <set>

template <typename Key, typename Value> struct Timetable {
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

#endif // !TIMETABLE_H
