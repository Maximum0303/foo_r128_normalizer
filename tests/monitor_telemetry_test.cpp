#include <algorithm>
#include <atomic>
#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>
#include "../source/r128_monitor_state.h"

using namespace r128_monitor;

telemetry_slot* only_source(snapshot& value) {
    telemetry_slot* result = nullptr;
    for (auto& slot : slots) {
        snapshot candidate;
        if (read_slot(slot, candidate)) {
            assert(result == nullptr);
            result = &slot;
            value = candidate;
        }
    }
    return result;
}

int main() {
    display_requested.store(true);
    snapshot value;
    {
        publisher unclassified;
        value.tick = 1;
        unclassified.publish(value);
        assert(only_source(value) == nullptr);
    }
    {
        publisher playback;
        playback.enable_for_playback();
        playback.begin_chunk();
        playback.observe_output_peak(0.5);
        value.tick = 1;
        playback.publish(value);
        auto* slot = only_source(value);
        assert(slot != nullptr);
        assert(std::abs(value.peak - 20 * std::log10(0.5)) < 1e-10);
        const auto old_sequence = value.sequence;
        playback.observe_output_peak(0.8);
        playback.publish(value);
        slot->peak_ack.store(old_sequence); // UI read predates the new peak.
        playback.begin_chunk();
        playback.observe_output_peak(0.1);
        playback.publish(value);
        assert(only_source(value) == slot);
        assert(std::abs(value.peak - 20 * std::log10(0.8)) < 1e-10);
        slot->peak_ack.store(value.sequence);
        playback.begin_chunk();
        playback.observe_output_peak(0.1);
        playback.publish(value);
        assert(only_source(value) == slot);
        assert(std::abs(value.peak - 20 * std::log10(0.1)) < 1e-10);
        display_requested.store(false);
        playback.begin_chunk();
        const auto previous_sequence = value.sequence;
        playback.observe_output_peak(10.0);
        playback.publish(value);
        assert(only_source(value) == slot && value.sequence == previous_sequence);
        display_epoch.fetch_add(1); // Reopen or resume: old peak must not survive.
        display_requested.store(true);
        playback.begin_chunk();
        playback.observe_output_peak(0.2);
        playback.publish(value);
        assert(only_source(value) == slot);
        assert(value.epoch == display_epoch.load());
        assert(std::abs(value.peak - 20 * std::log10(0.2)) < 1e-10);
        playback.invalidate();
        assert(only_source(value) == nullptr);
    }
    assert(only_source(value) == nullptr);
    {
        publisher playback;
        playback.enable_for_playback();
        std::atomic<bool> done{false};
        std::atomic<unsigned> successful{0};
        std::thread reader([&] {
            while (!done.load()) {
                snapshot sample;
                if (auto* slot = only_source(sample)) {
                    assert(sample.output == sample.input * 2);
                    assert(sample.gain == sample.input * 3);
                    assert(sample.limiter == sample.input * 4);
                    assert(sample.safety == sample.input * 5);
                    assert(sample.strength == sample.input * 6);
                    slot->peak_ack.store(sample.sequence);
                    ++successful;
                }
            }
        });
        for (unsigned index = 1; index <= 100000; ++index) {
            snapshot sample;
            sample.tick = index;
            sample.input = index;
            sample.output = index * 2.0;
            sample.gain = index * 3.0;
            sample.limiter = index * 4.0;
            sample.safety = index * 5.0;
            sample.strength = index * 6.0;
            playback.begin_chunk();
            playback.observe_output_peak(0.5);
            playback.publish(sample);
        }
        done.store(true);
        reader.join();
        assert(successful.load() > 0);
        std::cout << "coherent parallel snapshots: " << successful.load() << '\n';
    }
    // Slot exhaustion fails closed; released slots can be reused.
    {
        std::vector<std::unique_ptr<publisher>> sources;
        for (unsigned index = 0; index < kSlotCount + 1; ++index) {
            auto source = std::make_unique<publisher>();
            source->enable_for_playback();
            source->begin_chunk();
            snapshot sample;
            sample.tick = 1;
            source->publish(sample);
            sources.push_back(std::move(source));
        }
        unsigned count = 0;
        for (auto& slot : slots) if (read_slot(slot, value)) ++count;
        assert(count == kSlotCount);
    }
    assert(only_source(value) == nullptr);
    std::cout << "PASS: disabled source, peaks, stale ACK, reset, lifetime, capacity\n";
}
