#ifndef SCAN_COVER_H
#define SCAN_COVER_H

#include "vdmsc/edge.h"
#include "vdmsc/orbit.h"
#include <algorithm>
#include <vector>

namespace dmsc {

struct Scan {
    unsigned int edge_index = 0;
    float time = 0.0f;
    EdgeOrientation edge_orientation;

    Scan(const unsigned int edge_index, const float time, EdgeOrientation orientation)
        : edge_index(edge_index), time(time), edge_orientation(orientation) {}

    friend bool operator<(const Scan& e, const Scan& f) { return e.time < f.time; }
};

struct ScanCover {
  public:
    ScanCover() = default;
    void addEdgeDialog(const unsigned int edge_index, const float time, EdgeOrientation orientation) {
        scan_cover.emplace_back(edge_index, time, orientation);
    };

    void sort() { std::sort(scan_cover.begin(), scan_cover.end()); };
    float getScanTime() {
        if (scan_cover.size() == 0) {
            return 0.0f;
        }
        auto it = *std::max_element(scan_cover.begin(), scan_cover.end());
        return it.time;
    };

    const Scan& at(int i) const { return scan_cover.at(i); }
    size_t size() const { return scan_cover.size(); }

    // iterable
    std::vector<Scan>::const_iterator begin() const { return scan_cover.begin(); }
    std::vector<Scan>::const_iterator end() const { return scan_cover.end(); }

    // getter
    float getComputationTime() const { return computation_time; }
    float getLowerBound() const { return lower_bound; }

    // setter
    void setLowerBound(const float lower_bound) { this->lower_bound = lower_bound; }
    void setComputationTime(const float computation_time) { this->computation_time = computation_time; }

  private:
    std::vector<Scan> scan_cover;
    float computation_time = 0.0f;
    float lower_bound = 0.0f;
};

} // namespace dmsc

#endif