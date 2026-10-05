#include "../source/r128_monitor_meter.h"
#include <cassert>
#include <limits>
#include <iostream>

int main() {
    using namespace r128_monitor;
    assert(meter_position(meter_kind::loudness, -200) == 0);
    assert(meter_position(meter_kind::loudness, -60) == 0);
    assert(meter_position(meter_kind::loudness, -30) == 500);
    assert(meter_position(meter_kind::loudness, 0) == 1000);
    assert(meter_position(meter_kind::loudness, 6) == 1000);
    assert(meter_position(meter_kind::true_peak, -60) == 0);
    assert(meter_position(meter_kind::true_peak, -1) == 937);
    assert(meter_position(meter_kind::true_peak, 3) == 1000);
    assert(meter_position(meter_kind::gain, 0) == 0);
    assert(meter_position(meter_kind::gain, -12) == 500);
    assert(meter_position(meter_kind::gain, 12) == 500);
    assert(meter_position(meter_kind::gain, -40) == 1000);
    assert(meter_position(meter_kind::reduction, -1) == 0);
    assert(meter_position(meter_kind::reduction, 6) == 500);
    assert(meter_position(meter_kind::reduction, 20) == 1000);
    assert(meter_position(meter_kind::strength, -1) == 0);
    assert(meter_position(meter_kind::strength, 50) == 500);
    assert(meter_position(meter_kind::strength, 101) == 1000);
    for (auto kind : {meter_kind::loudness, meter_kind::gain,
         meter_kind::true_peak, meter_kind::reduction, meter_kind::strength}) {
        assert(meter_position(kind, std::numeric_limits<double>::quiet_NaN()) == 0);
        assert(meter_position(kind, std::numeric_limits<double>::infinity()) == 0);
        assert(meter_position(kind, -std::numeric_limits<double>::infinity()) == 0);
    }
    for (int i = -1000; i <= 1000; ++i) {
        const auto pos = meter_position(meter_kind::loudness, i * .1);
        assert(pos >= 0 && pos <= 1000);
        if (i < 1000) assert(pos <= meter_position(meter_kind::loudness, (i+1) * .1));
    }
    std::cout << "PASS: meter units, sign/magnitude, clipping, unavailable values, monotonicity\n";
}
