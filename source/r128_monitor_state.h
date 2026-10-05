#pragma once

// Display-only telemetry. No audio buffers, UI handles, allocation, locks or
// waiting are used by publish(). All fields are atomic to make bounded reads
// safe even if an audio publication overlaps the UI timer.
namespace r128_monitor {

static_assert(std::atomic<double>::is_always_lock_free,
    "The x64 monitor requires lock-free double atomics");
static_assert(std::atomic<unsigned long long>::is_always_lock_free,
    "The x64 monitor requires lock-free sequence atomics");
static_assert(std::atomic<bool>::is_always_lock_free,
    "The x64 monitor requires lock-free display-state atomics");

struct snapshot {
    unsigned long long instance = 0;
    unsigned long long sequence = 0;
    unsigned long long epoch = 0;
    unsigned long long tick = 0;
    double input = -200.0;
    double output = -200.0;
    double gain = 0.0;
    double peak = -200.0;
    double limiter = 0.0;
    double safety = 0.0;
    double strength = 0.0;
    bool adaptive = false;
    bool comparing = false;
};

struct telemetry_slot {
    std::atomic<unsigned long long> instance{0};
    std::atomic<unsigned long long> sequence{0};
    std::atomic<unsigned long long> tick{0};
    std::atomic<unsigned long long> peak_ack{0};
    std::atomic<unsigned long long> epoch{0};
    std::atomic<double> input{-200.0}, output{-200.0}, gain{0.0};
    std::atomic<double> peak{-200.0}, limiter{0.0}, safety{0.0}, strength{0.0};
    std::atomic<bool> adaptive{false}, comparing{false};
};

constexpr unsigned kSlotCount = 16;
inline telemetry_slot slots[kSlotCount];
inline std::atomic<unsigned long long> next_instance{1};
inline std::atomic<bool> display_requested{false};
inline std::atomic<unsigned long long> display_epoch{1};

class publisher {
public:
    publisher() = default;
    void enable_for_playback() {
        if (m_slot != nullptr) return;
        const auto id = next_instance.fetch_add(1);
        for (auto& candidate : slots) {
            unsigned long long empty = 0;
            if (candidate.instance.compare_exchange_strong(empty, id)) {
                m_slot = &candidate;
                invalidate();
                break;
            }
        }
    }
    ~publisher() {
        if (m_slot != nullptr) {
            invalidate();
            m_slot->instance.store(0);
        }
    }
    publisher(const publisher&) = delete;
    publisher& operator=(const publisher&) = delete;

    void invalidate() {
        if (m_slot == nullptr) return;
        m_slot->sequence.fetch_add(1);
        m_slot->tick.store(0);
        m_slot->sequence.fetch_add(1);
        m_peak = 0.0;
        m_last_ack = m_slot->peak_ack.load();
    }

    void begin_chunk() {
        if (m_slot == nullptr) return;
        m_capturing = display_requested.load();
        if (!m_capturing) return;
        const auto epoch = display_epoch.load();
        if (epoch != m_epoch) {
            m_epoch = epoch;
            m_peak = 0.0;
            m_last_ack = m_slot->peak_ack.load();
        }
        const auto ack = m_slot->peak_ack.load();
        // An acknowledgement of an older publication must not discard a
        // peak accumulated after that UI read. Keep it for the next display.
        if (ack != m_last_ack && ack == m_slot->sequence.load()) {
            m_peak = 0.0;
            m_last_ack = ack;
        }
    }
    void observe_output_peak(double linear_peak) {
        if (m_capturing) m_peak = std::max(m_peak, linear_peak);
    }
    void publish(snapshot value) {
        if (m_slot == nullptr || !m_capturing) return;
        value.peak = m_peak > 1.0e-10 ? 20.0 * std::log10(m_peak) : -200.0;
        // seq_cst on every atomic is deliberate: a successful bounded read
        // observes one whole publication, without a retry loop on audio.
        m_slot->sequence.fetch_add(1);
        m_slot->epoch.store(m_epoch);
        m_slot->input.store(value.input);
        m_slot->output.store(value.output);
        m_slot->gain.store(value.gain);
        m_slot->peak.store(value.peak);
        m_slot->limiter.store(value.limiter);
        m_slot->safety.store(value.safety);
        m_slot->strength.store(value.strength);
        m_slot->adaptive.store(value.adaptive);
        m_slot->comparing.store(value.comparing);
        m_slot->tick.store(value.tick);
        m_slot->sequence.fetch_add(1);
    }
private:
    telemetry_slot* m_slot = nullptr;
    unsigned long long m_last_ack = 0;
    unsigned long long m_epoch = 0;
    double m_peak = 0.0;
    bool m_capturing = false;
};

inline bool read_slot(telemetry_slot& slot, snapshot& out) {
    // Bounded, UI-only retry. All payload reads remain race-free.
    for (unsigned attempt = 0; attempt < 3; ++attempt) {
        const auto id = slot.instance.load();
        if (id == 0) return false;
        const auto first = slot.sequence.load();
        if (first & 1) continue;
        snapshot value;
        value.instance = id;
        value.sequence = first;
        value.epoch = slot.epoch.load();
        value.tick = slot.tick.load();
        value.input = slot.input.load();
        value.output = slot.output.load();
        value.gain = slot.gain.load();
        value.peak = slot.peak.load();
        value.limiter = slot.limiter.load();
        value.safety = slot.safety.load();
        value.strength = slot.strength.load();
        value.adaptive = slot.adaptive.load();
        value.comparing = slot.comparing.load();
        if (slot.sequence.load() == first && slot.instance.load() == id) {
            out = value;
            return value.tick != 0;
        }
    }
    return false;
}

} // namespace r128_monitor
