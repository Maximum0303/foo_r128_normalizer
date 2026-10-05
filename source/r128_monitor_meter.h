#pragma once

#include <algorithm>
#include <cmath>

namespace r128_monitor {

// UI-only meter mapping. Saturation affects the bar, never the displayed value
// or audio. Ranges describe units; they are not processing targets or limits.
enum class meter_kind { loudness, gain, true_peak, reduction, strength };

inline int meter_position(meter_kind kind, double value) {
    if (!std::isfinite(value)) return 0;
    double low = 0.0, high = 100.0;
    switch (kind) {
    case meter_kind::loudness: low = -60.0; high = 0.0; break;
    case meter_kind::true_peak: low = -60.0; high = 3.0; break;
    case meter_kind::gain: value = std::abs(value); high = 24.0; break;
    case meter_kind::reduction: high = 12.0; break;
    case meter_kind::strength: break;
    }
    const double bounded = (std::clamp)(value, low, high);
    return static_cast<int>(std::lround((bounded - low) * 1000.0 / (high - low)));
}

} // namespace r128_monitor
