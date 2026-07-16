#include "stdafx.h"

namespace {

static const GUID guid_r128_normalizer =
{ 0x4d3f8b85, 0x61d2, 0x47f4, { 0xa8, 0x51, 0x7c, 0x1b, 0x3d, 0xd7, 0x92, 0x46 } };

constexpr t_uint32 kPresetVersion = 7;

constexpr double kPi = 3.1415926535897932384626433832795;
constexpr double kLoudnessOffset = -0.691;

constexpr double kBlockSeconds = 0.400;
constexpr double kBlockHopSeconds = 0.100;
constexpr double kShortTermSeconds = 3.000;
constexpr unsigned kLraSampleHopBlocks = 10;
constexpr double kLraRelativeGateOffsetLu = -20.0;
constexpr t_size kMaximumStoredLraSamples = 14400;
constexpr double kAbsoluteGateLufs = -70.0;
constexpr double kRelativeGateOffsetLu = -10.0;

constexpr double kFastAttenuationSeconds = 0.060;
constexpr double kSlowBoostSeconds = 2.000;

constexpr double kModernCompressorThresholdDb = -12.0;
constexpr double kModernCompressorKneeDb = 6.0;
constexpr double kModernCompressorAttackSeconds = 0.020;
constexpr double kModernCompressorReleaseSeconds = 0.200;
constexpr unsigned kModernClipperOversampleFactor = 4;
constexpr double kAutoSafetyMaximumReductionDb = 3.0;
constexpr double kAutoSafetyAttackSeconds = 0.350;
constexpr double kAutoSafetyReleaseSeconds = 3.000;
constexpr double kRiskStrongHoldSeconds = 0.400;
constexpr double kRiskExcessiveHoldSeconds = 0.750;
constexpr double kAdaptiveStrengthResponseSeconds = 2.000;
constexpr double kAdaptiveMinimumStrengthPercent = 20.0;
constexpr double kAdaptiveMaximumStrengthPercent = 70.0;

constexpr double kThreeBandLowCrossoverHz = 160.0;
constexpr double kThreeBandHighCrossoverHz = 4000.0;
constexpr double kThreeBandKneeDb = 6.0;
constexpr double kThreeBandLowThresholdDb = -14.0;
constexpr double kThreeBandMidThresholdDb = -16.0;
constexpr double kThreeBandHighThresholdDb = -20.0;
constexpr double kThreeBandLowAttackSeconds = 0.040;
constexpr double kThreeBandLowReleaseSeconds = 0.280;
constexpr double kThreeBandMidAttackSeconds = 0.020;
constexpr double kThreeBandMidReleaseSeconds = 0.180;
constexpr double kThreeBandHighAttackSeconds = 0.008;
constexpr double kThreeBandHighReleaseSeconds = 0.120;

constexpr double kOriginalCompareFadeSeconds = 0.015;
constexpr double kCompareMatchMinimumDb = -12.0;
constexpr double kCompareMatchMaximumDb = 12.0;
constexpr double kClipEventReleaseLinear = 0.995;
constexpr double kSafeAudioMagnitudeLimit = 64.0;

constexpr unsigned kTruePeakFactor = 4;
constexpr unsigned kTruePeakTapCount = 17;
constexpr unsigned kTruePeakDelay = (kTruePeakTapCount - 1) / 2;

constexpr t_size kMaximumStoredBlocks = 108000;

constexpr UINT_PTR kDiagnosticsTimerId = 1;
constexpr UINT kDiagnosticsRefreshMilliseconds = 250;

// foobar2000 audio_chunk channel flags. Interleaved sample order follows
// the ascending order of these channel-map flags.
constexpr unsigned kChannelFrontLeft = 1u << 0;
constexpr unsigned kChannelFrontRight = 1u << 1;
constexpr unsigned kChannelFrontCenter = 1u << 2;
constexpr unsigned kChannelLfe = 1u << 3;
constexpr unsigned kChannelBackLeft = 1u << 4;
constexpr unsigned kChannelBackRight = 1u << 5;
constexpr unsigned kChannelFrontCenterLeft = 1u << 6;
constexpr unsigned kChannelFrontCenterRight = 1u << 7;
constexpr unsigned kChannelBackCenter = 1u << 8;
constexpr unsigned kChannelSideLeft = 1u << 9;
constexpr unsigned kChannelSideRight = 1u << 10;
constexpr unsigned kChannelTopCenter = 1u << 11;
constexpr unsigned kChannelTopFrontLeft = 1u << 12;
constexpr unsigned kChannelTopFrontCenter = 1u << 13;
constexpr unsigned kChannelTopFrontRight = 1u << 14;
constexpr unsigned kChannelTopBackLeft = 1u << 15;
constexpr unsigned kChannelTopBackCenter = 1u << 16;
constexpr unsigned kChannelTopBackRight = 1u << 17;

constexpr unsigned kChannelConfigMono = kChannelFrontCenter;
constexpr unsigned kChannelConfigStereo =
    kChannelFrontLeft | kChannelFrontRight;
constexpr unsigned kChannelConfig5Point1 =
    kChannelFrontLeft | kChannelFrontRight | kChannelFrontCenter |
    kChannelLfe | kChannelBackLeft | kChannelBackRight;
constexpr unsigned kChannelConfig5Point1Side =
    kChannelFrontLeft | kChannelFrontRight | kChannelFrontCenter |
    kChannelLfe | kChannelSideLeft | kChannelSideRight;
constexpr unsigned kChannelConfig7Point1 =
    kChannelConfig5Point1 | kChannelSideLeft | kChannelSideRight;

constexpr double kSurroundEnergyWeight = 1.41;

std::atomic<double> g_diagnostic_momentary_lufs(-200.0);
std::atomic<double> g_diagnostic_short_term_lufs(-200.0);
std::atomic<double> g_diagnostic_integrated_lufs(-200.0);
std::atomic<double> g_diagnostic_lra_lu(-200.0);
std::atomic<double> g_diagnostic_applied_gain_db(0.0);
std::atomic<double> g_diagnostic_normalization_gain_db(0.0);
std::atomic<double> g_diagnostic_true_peak_dbtp(-200.0);
std::atomic<int> g_diagnostic_peak_guard_state(0);
std::atomic<int> g_diagnostic_stream_active(0);
std::atomic<unsigned long long> g_diagnostic_last_update_tick(0);
std::atomic<unsigned> g_diagnostic_channel_count(0);
std::atomic<unsigned> g_diagnostic_channel_mask(0);
std::atomic<int> g_diagnostic_lfe_excluded(0);
std::atomic<double> g_diagnostic_limiter_reduction_db(0.0);
std::atomic<double> g_diagnostic_compressor_reduction_db(0.0);
std::atomic<double> g_diagnostic_clipper_reduction_db(0.0);
std::atomic<int> g_diagnostic_modern_boost_state(0);
std::atomic<double> g_diagnostic_output_integrated_lufs(-200.0);
std::atomic<double> g_diagnostic_target_difference_lu(-200.0);
std::atomic<int> g_diagnostic_processing_risk_state(0);
std::atomic<double> g_diagnostic_safety_reduction_db(0.0);
std::atomic<int> g_diagnostic_original_compare_state(0);
std::atomic<double> g_diagnostic_compare_match_gain_db(0.0);
std::atomic<int> g_diagnostic_adaptive_master_state(0);
std::atomic<int> g_diagnostic_three_band_master_state(0);
std::atomic<double> g_diagnostic_effective_strength_percent(0.0);
std::atomic<double> g_diagnostic_three_band_low_reduction_db(0.0);
std::atomic<double> g_diagnostic_three_band_mid_reduction_db(0.0);
std::atomic<double> g_diagnostic_three_band_high_reduction_db(0.0);
std::atomic<double> g_diagnostic_track_max_three_band_low_reduction_db(0.0);
std::atomic<double> g_diagnostic_track_max_three_band_mid_reduction_db(0.0);
std::atomic<double> g_diagnostic_track_max_three_band_high_reduction_db(0.0);
std::atomic<double> g_diagnostic_track_max_true_peak_dbtp(-200.0);
std::atomic<double> g_diagnostic_track_max_compressor_reduction_db(0.0);
std::atomic<double> g_diagnostic_track_max_clipper_reduction_db(0.0);
std::atomic<double> g_diagnostic_track_max_limiter_reduction_db(0.0);
std::atomic<unsigned long long> g_diagnostic_clip_event_count(0);
std::atomic<unsigned long long> g_diagnostic_recovered_sample_count(0);
std::atomic<int> g_diagnostic_track_evaluation_state(0);
std::atomic<unsigned> g_diagnostic_sample_rate_hz(0);
std::atomic<double> g_diagnostic_cpu_load_percent(0.0);
std::atomic<int> g_diagnostic_final_summary_valid(0);
std::atomic<double> g_diagnostic_final_input_integrated_lufs(-200.0);
std::atomic<double> g_diagnostic_final_output_integrated_lufs(-200.0);
std::atomic<double> g_diagnostic_final_target_difference_lu(-200.0);
std::atomic<double> g_diagnostic_final_lra_lu(-200.0);
std::atomic<double> g_diagnostic_final_max_true_peak_dbtp(-200.0);
std::atomic<double> g_diagnostic_final_max_compressor_reduction_db(0.0);
std::atomic<double> g_diagnostic_final_max_clipper_reduction_db(0.0);
std::atomic<double> g_diagnostic_final_max_limiter_reduction_db(0.0);
std::atomic<int> g_diagnostic_final_three_band_master_state(0);
std::atomic<double> g_diagnostic_final_max_three_band_low_reduction_db(0.0);
std::atomic<double> g_diagnostic_final_max_three_band_mid_reduction_db(0.0);
std::atomic<double> g_diagnostic_final_max_three_band_high_reduction_db(0.0);
std::atomic<unsigned long long> g_diagnostic_final_clip_event_count(0);
std::atomic<unsigned long long> g_diagnostic_final_recovered_sample_count(0);
std::atomic<int> g_diagnostic_final_evaluation_state(0);
std::atomic<unsigned> g_diagnostic_final_sample_rate_hz(0);
std::atomic<double> g_diagnostic_final_cpu_load_percent(0.0);
std::atomic<int> g_original_compare_request(0);
std::atomic<double> g_diagnostic_latency_ms(0.0);
std::atomic<int> g_diagnostic_normalization_state(0);
std::atomic<int> g_diagnostic_gain_lock_state(0);
std::atomic<double> g_diagnostic_gain_lock_remaining_seconds(0.0);
std::atomic<double> g_diagnostic_locked_gain_db(0.0);
std::atomic<unsigned long long> g_measurement_reset_request(0);

struct r128_settings {
    float target_lufs = -18.0f;
    float max_boost_db = 12.0f;
    float max_attenuation_db = 18.0f;
    float true_peak_limit_dbtp = -1.0f;
    float lookahead_ms = 5.0f;
    float limiter_release_ms = 120.0f;
    float startup_analysis_seconds = 3.0f;
    float silence_guard_lufs = -45.0f;
    float gain_lock_seconds = 10.0f;
    float gain_lock_tolerance_lu = 0.5f;
    float modern_strength_percent = 50.0f;
    bool reset_each_track = true;
    bool enable_peak_guard = true;
    bool enable_silence_guard = true;
    bool enable_gain_lock = true;
    bool enable_modern_boost = false;
    bool enable_adaptive_master = false;
    bool enable_three_band_master = false;
};

r128_settings default_settings() {
    return {};
}

r128_settings standard_profile() {
    return default_settings();
}

r128_settings streaming_profile() {
    r128_settings value = default_settings();
    value.target_lufs = -14.0f;
    value.max_boost_db = 8.0f;
    value.true_peak_limit_dbtp = -1.0f;
    value.silence_guard_lufs = -42.0f;
    return value;
}

r128_settings broadcast_profile() {
    r128_settings value = default_settings();
    value.target_lufs = -23.0f;
    value.true_peak_limit_dbtp = -1.0f;
    value.silence_guard_lufs = -50.0f;
    value.gain_lock_seconds = 12.0f;
    return value;
}

r128_settings night_profile() {
    r128_settings value = default_settings();
    value.target_lufs = -22.0f;
    value.max_boost_db = 6.0f;
    value.true_peak_limit_dbtp = -2.0f;
    value.limiter_release_ms = 180.0f;
    value.startup_analysis_seconds = 2.0f;
    value.silence_guard_lufs = -48.0f;
    value.gain_lock_seconds = 8.0f;
    return value;
}

r128_settings modern_profile() {
    r128_settings value = default_settings();
    value.target_lufs = -9.0f;
    value.max_boost_db = 14.0f;
    value.true_peak_limit_dbtp = -1.0f;
    value.lookahead_ms = 5.0f;
    value.limiter_release_ms = 180.0f;
    value.startup_analysis_seconds = 2.0f;
    value.silence_guard_lufs = -42.0f;
    value.gain_lock_seconds = 6.0f;
    value.modern_strength_percent = 50.0f;
    value.enable_modern_boost = true;
    return value;
}

r128_settings adaptive_profile() {
    r128_settings value = default_settings();
    value.target_lufs = -10.0f;
    value.max_boost_db = 12.0f;
    value.true_peak_limit_dbtp = -1.0f;
    value.lookahead_ms = 5.0f;
    value.limiter_release_ms = 200.0f;
    value.startup_analysis_seconds = 3.0f;
    value.silence_guard_lufs = -42.0f;
    value.gain_lock_seconds = 8.0f;
    value.modern_strength_percent = 65.0f;
    value.enable_modern_boost = true;
    value.enable_adaptive_master = true;
    return value;
}

r128_settings three_band_profile() {
    r128_settings value = adaptive_profile();
    value.target_lufs = -10.0f;
    value.modern_strength_percent = 55.0f;
    value.limiter_release_ms = 220.0f;
    value.enable_three_band_master = true;
    return value;
}


enum class recognized_profile {
    standard,
    streaming,
    broadcast,
    night,
    modern,
    adaptive,
    three_band,
    custom
};

bool settings_float_equal(float left, float right) {
    return std::fabs(left - right) <= 0.0005f;
}

bool settings_equal(
    const r128_settings& left,
    const r128_settings& right
) {
    return
        settings_float_equal(left.target_lufs, right.target_lufs) &&
        settings_float_equal(left.max_boost_db, right.max_boost_db) &&
        settings_float_equal(
            left.max_attenuation_db,
            right.max_attenuation_db
        ) &&
        settings_float_equal(
            left.true_peak_limit_dbtp,
            right.true_peak_limit_dbtp
        ) &&
        settings_float_equal(left.lookahead_ms, right.lookahead_ms) &&
        settings_float_equal(
            left.limiter_release_ms,
            right.limiter_release_ms
        ) &&
        settings_float_equal(
            left.startup_analysis_seconds,
            right.startup_analysis_seconds
        ) &&
        settings_float_equal(
            left.silence_guard_lufs,
            right.silence_guard_lufs
        ) &&
        settings_float_equal(
            left.gain_lock_seconds,
            right.gain_lock_seconds
        ) &&
        settings_float_equal(
            left.gain_lock_tolerance_lu,
            right.gain_lock_tolerance_lu
        ) &&
        settings_float_equal(
            left.modern_strength_percent,
            right.modern_strength_percent
        ) &&
        left.reset_each_track == right.reset_each_track &&
        left.enable_peak_guard == right.enable_peak_guard &&
        left.enable_silence_guard == right.enable_silence_guard &&
        left.enable_gain_lock == right.enable_gain_lock &&
        left.enable_modern_boost == right.enable_modern_boost &&
        left.enable_adaptive_master == right.enable_adaptive_master &&
        left.enable_three_band_master ==
            right.enable_three_band_master;
}

recognized_profile detect_recognized_profile(
    const r128_settings& value
) {
    if (settings_equal(value, standard_profile())) {
        return recognized_profile::standard;
    }
    if (settings_equal(value, streaming_profile())) {
        return recognized_profile::streaming;
    }
    if (settings_equal(value, broadcast_profile())) {
        return recognized_profile::broadcast;
    }
    if (settings_equal(value, night_profile())) {
        return recognized_profile::night;
    }
    if (settings_equal(value, modern_profile())) {
        return recognized_profile::modern;
    }
    if (settings_equal(value, adaptive_profile())) {
        return recognized_profile::adaptive;
    }
    if (settings_equal(value, three_band_profile())) {
        return recognized_profile::three_band;
    }

    return recognized_profile::custom;
}

const wchar_t* recognized_profile_name(
    recognized_profile profile
) {
    switch (profile) {
    case recognized_profile::standard:
        return L"自然 -18";
    case recognized_profile::streaming:
        return L"パワー -14";
    case recognized_profile::broadcast:
        return L"リラックス -23";
    case recognized_profile::night:
        return L"ナイト -22";
    case recognized_profile::modern:
        return L"モダン -9";
    case recognized_profile::adaptive:
        return L"自動1帯 -10";
    case recognized_profile::three_band:
        return L"自動3帯 -10";
    default:
        return L"カスタム設定";
    }
}

void update_profile_indicator(
    HWND wnd,
    const r128_settings& value,
    bool pending
) {
    wchar_t text[96] = {};
    swprintf_s(
        text,
        pending ? L"選択: %s" : L"現在: %s",
        recognized_profile_name(
            detect_recognized_profile(value)
        )
    );
    SetDlgItemTextW(
        wnd,
        IDC_PROFILE_DESCRIPTION,
        text
    );
}

template<typename T>
T clamp_value(T value, T minimum, T maximum) {
    if (value < minimum) return minimum;
    if (value > maximum) return maximum;
    return value;
}

double db_to_linear(double db) {
    return std::pow(10.0, db / 20.0);
}

double linear_to_db(double value) {
    if (value <= 0.0) return -200.0;
    return 20.0 * std::log10(value);
}

double energy_to_lufs(double energy) {
    if (energy <= 1.0e-30) return -200.0;
    return kLoudnessOffset + 10.0 * std::log10(energy);
}

double calculate_gated_integrated_lufs(
    const std::vector<double>& absolute_gated_energies
) {
    if (absolute_gated_energies.empty()) {
        return -200.0;
    }

    const double ungated_sum = std::accumulate(
        absolute_gated_energies.begin(),
        absolute_gated_energies.end(),
        0.0
    );

    const double ungated_mean =
        ungated_sum /
        static_cast<double>(absolute_gated_energies.size());

    const double ungated_lufs = energy_to_lufs(ungated_mean);
    const double effective_gate_lufs = std::max(
        kAbsoluteGateLufs,
        ungated_lufs + kRelativeGateOffsetLu
    );

    double gated_sum = 0.0;
    t_size gated_count = 0;

    for (double energy : absolute_gated_energies) {
        if (energy_to_lufs(energy) >= effective_gate_lufs) {
            gated_sum += energy;
            ++gated_count;
        }
    }

    if (gated_count == 0) {
        return -200.0;
    }

    const double result = energy_to_lufs(
        gated_sum / static_cast<double>(gated_count)
    );

    return std::isfinite(result) ? result : -200.0;
}

const wchar_t* processing_risk_to_text(int state) {
    switch (state) {
    case 1:
        return L"適正";
    case 2:
        return L"強め";
    case 3:
        return L"過剰・自動保護中";
    default:
        return L"無効";
    }
}

const wchar_t* track_evaluation_to_text(int state) {
    switch (state) {
    case 1:
        return L"安全";
    case 2:
        return L"強め";
    case 3:
        return L"要調整";
    default:
        return L"未測定";
    }
}

void make_preset(const r128_settings& value, dsp_preset& out) {
    dsp_preset_builder builder;
    builder << kPresetVersion;
    builder << value.target_lufs;
    builder << value.max_boost_db;
    builder << value.max_attenuation_db;
    builder << value.true_peak_limit_dbtp;
    builder << value.lookahead_ms;
    builder << value.limiter_release_ms;
    builder << value.startup_analysis_seconds;
    builder << value.silence_guard_lufs;
    builder << value.gain_lock_seconds;
    builder << value.gain_lock_tolerance_lu;
    builder << value.modern_strength_percent;
    builder << static_cast<t_uint8>(value.reset_each_track ? 1 : 0);
    builder << static_cast<t_uint8>(value.enable_peak_guard ? 1 : 0);
    builder << static_cast<t_uint8>(value.enable_silence_guard ? 1 : 0);
    builder << static_cast<t_uint8>(value.enable_gain_lock ? 1 : 0);
    builder << static_cast<t_uint8>(value.enable_modern_boost ? 1 : 0);
    builder << static_cast<t_uint8>(value.enable_adaptive_master ? 1 : 0);
    builder << static_cast<t_uint8>(value.enable_three_band_master ? 1 : 0);
    builder.finish(guid_r128_normalizer, out);
}

r128_settings parse_preset(const dsp_preset& in) {
    r128_settings value = default_settings();

    try {
        dsp_preset_parser parser(in);
        t_uint32 version = 0;
        parser >> version;

        t_uint8 reset_each_track = 1;
        t_uint8 enable_peak_guard = 1;
        t_uint8 enable_silence_guard = 1;
        t_uint8 enable_gain_lock = 1;
        t_uint8 enable_modern_boost = 0;
        t_uint8 enable_adaptive_master = 0;
        t_uint8 enable_three_band_master = 0;

        if (version == 1) {
            parser >> value.target_lufs;
            parser >> value.max_boost_db;
            parser >> value.max_attenuation_db;
            parser >> value.true_peak_limit_dbtp;
            parser >> reset_each_track;
            parser >> enable_peak_guard;
        }
        else if (version == 2) {
            parser >> value.target_lufs;
            parser >> value.max_boost_db;
            parser >> value.max_attenuation_db;
            parser >> value.true_peak_limit_dbtp;
            parser >> value.lookahead_ms;
            parser >> value.limiter_release_ms;
            parser >> reset_each_track;
            parser >> enable_peak_guard;
        }
        else if (version == 3) {
            parser >> value.target_lufs;
            parser >> value.max_boost_db;
            parser >> value.max_attenuation_db;
            parser >> value.true_peak_limit_dbtp;
            parser >> value.lookahead_ms;
            parser >> value.limiter_release_ms;
            parser >> value.startup_analysis_seconds;
            parser >> value.silence_guard_lufs;
            parser >> reset_each_track;
            parser >> enable_peak_guard;
            parser >> enable_silence_guard;
        }
        else if (version == 4) {
            parser >> value.target_lufs;
            parser >> value.max_boost_db;
            parser >> value.max_attenuation_db;
            parser >> value.true_peak_limit_dbtp;
            parser >> value.lookahead_ms;
            parser >> value.limiter_release_ms;
            parser >> value.startup_analysis_seconds;
            parser >> value.silence_guard_lufs;
            parser >> value.gain_lock_seconds;
            parser >> value.gain_lock_tolerance_lu;
            parser >> reset_each_track;
            parser >> enable_peak_guard;
            parser >> enable_silence_guard;
            parser >> enable_gain_lock;
        }
        else if (version == 5) {
            parser >> value.target_lufs;
            parser >> value.max_boost_db;
            parser >> value.max_attenuation_db;
            parser >> value.true_peak_limit_dbtp;
            parser >> value.lookahead_ms;
            parser >> value.limiter_release_ms;
            parser >> value.startup_analysis_seconds;
            parser >> value.silence_guard_lufs;
            parser >> value.gain_lock_seconds;
            parser >> value.gain_lock_tolerance_lu;
            parser >> value.modern_strength_percent;
            parser >> reset_each_track;
            parser >> enable_peak_guard;
            parser >> enable_silence_guard;
            parser >> enable_gain_lock;
            parser >> enable_modern_boost;
        }
        else if (version == 6) {
            parser >> value.target_lufs;
            parser >> value.max_boost_db;
            parser >> value.max_attenuation_db;
            parser >> value.true_peak_limit_dbtp;
            parser >> value.lookahead_ms;
            parser >> value.limiter_release_ms;
            parser >> value.startup_analysis_seconds;
            parser >> value.silence_guard_lufs;
            parser >> value.gain_lock_seconds;
            parser >> value.gain_lock_tolerance_lu;
            parser >> value.modern_strength_percent;
            parser >> reset_each_track;
            parser >> enable_peak_guard;
            parser >> enable_silence_guard;
            parser >> enable_gain_lock;
            parser >> enable_modern_boost;
            parser >> enable_adaptive_master;
        }
        else if (version == kPresetVersion) {
            parser >> value.target_lufs;
            parser >> value.max_boost_db;
            parser >> value.max_attenuation_db;
            parser >> value.true_peak_limit_dbtp;
            parser >> value.lookahead_ms;
            parser >> value.limiter_release_ms;
            parser >> value.startup_analysis_seconds;
            parser >> value.silence_guard_lufs;
            parser >> value.gain_lock_seconds;
            parser >> value.gain_lock_tolerance_lu;
            parser >> value.modern_strength_percent;
            parser >> reset_each_track;
            parser >> enable_peak_guard;
            parser >> enable_silence_guard;
            parser >> enable_gain_lock;
            parser >> enable_modern_boost;
            parser >> enable_adaptive_master;
            parser >> enable_three_band_master;
        }
        else {
            return value;
        }

        value.reset_each_track = (reset_each_track != 0);
        value.enable_peak_guard = (enable_peak_guard != 0);
        value.enable_silence_guard = (enable_silence_guard != 0);
        value.enable_gain_lock = (enable_gain_lock != 0);
        value.enable_modern_boost = (enable_modern_boost != 0);
        value.enable_adaptive_master = (enable_adaptive_master != 0);
        value.enable_three_band_master =
            (enable_three_band_master != 0);
        if (value.enable_three_band_master) {
            value.enable_adaptive_master = true;
        }
        if (value.enable_adaptive_master) value.enable_modern_boost = true;
        value.modern_strength_percent = clamp_value(
            value.modern_strength_percent,
            0.0f,
            100.0f
        );
    }
    catch (const exception_io_data&) {
        return default_settings();
    }

    return value;
}


constexpr int kBasicPageControls[] = {
    IDC_BASIC_HEADER,
    IDC_LABEL_TARGET_LUFS,
    IDC_TARGET_LUFS,
    IDC_UNIT_TARGET_LUFS,
    IDC_LABEL_MAX_BOOST,
    IDC_MAX_BOOST,
    IDC_UNIT_MAX_BOOST,
    IDC_LABEL_MAX_ATTENUATION,
    IDC_MAX_ATTENUATION,
    IDC_UNIT_MAX_ATTENUATION,
    IDC_LABEL_TRUE_PEAK,
    IDC_TRUE_PEAK,
    IDC_UNIT_TRUE_PEAK,
    IDC_LABEL_LOOKAHEAD,
    IDC_LOOKAHEAD_MS,
    IDC_UNIT_LOOKAHEAD,
    IDC_LABEL_LIMITER_RELEASE,
    IDC_LIMITER_RELEASE_MS,
    IDC_UNIT_LIMITER_RELEASE,
    IDC_LABEL_STARTUP,
    IDC_STARTUP_ANALYSIS_SECONDS,
    IDC_UNIT_STARTUP,
    IDC_LABEL_SILENCE_THRESHOLD,
    IDC_SILENCE_GUARD_LUFS,
    IDC_UNIT_SILENCE_THRESHOLD,
    IDC_LABEL_GAIN_LOCK_SECONDS,
    IDC_GAIN_LOCK_SECONDS,
    IDC_UNIT_GAIN_LOCK_SECONDS,
    IDC_LABEL_GAIN_LOCK_TOLERANCE,
    IDC_GAIN_LOCK_TOLERANCE_LU,
    IDC_UNIT_GAIN_LOCK_TOLERANCE,
    IDC_RESET_EACH_TRACK,
    IDC_ENABLE_PEAK_GUARD,
    IDC_ENABLE_SILENCE_GUARD,
    IDC_ENABLE_GAIN_LOCK
};

constexpr int kAdvancedPageControls[] = {
    IDC_ADV_HEADER,
    IDC_ENABLE_MODERN_BOOST,
    IDC_ENABLE_ADAPTIVE_MASTER,
    IDC_ENABLE_THREE_BAND_MASTER,
    IDC_LABEL_MODERN_STRENGTH,
    IDC_MODERN_STRENGTH,
    IDC_UNIT_MODERN_STRENGTH,
    IDC_ADV_INFO_1,
    IDC_ADV_INFO_2,
    IDC_ADV_INFO_3
};

constexpr int kDiagnosticPageControls[] = {
    IDC_LABEL_DIAG_NORMALIZATION,
    IDC_DIAG_NORMALIZATION_STATE,
    IDC_LABEL_DIAG_MOMENTARY,
    IDC_DIAG_MOMENTARY,
    IDC_LABEL_DIAG_SHORT_TERM,
    IDC_DIAG_SHORT_TERM,
    IDC_LABEL_DIAG_GAIN_LOCK,
    IDC_DIAG_GAIN_LOCK,
    IDC_LABEL_DIAG_INPUT_INT,
    IDC_DIAG_INTEGRATED,
    IDC_LABEL_DIAG_OUTPUT_INT,
    IDC_DIAG_OUTPUT_INTEGRATED,
    IDC_LABEL_DIAG_TARGET_DIFF,
    IDC_DIAG_TARGET_DIFFERENCE,
    IDC_LABEL_DIAG_LRA,
    IDC_DIAG_LRA,
    IDC_LABEL_DIAG_GAIN,
    IDC_DIAG_GAIN,
    IDC_LABEL_DIAG_PROCESSING,
    IDC_DIAG_PROCESSING_RISK,
    IDC_LABEL_DIAG_SAFETY,
    IDC_DIAG_SAFETY_REDUCTION,
    IDC_LABEL_DIAG_COMPRESSOR,
    IDC_DIAG_COMPRESSOR_REDUCTION,
    IDC_LABEL_DIAG_CLIPPER,
    IDC_DIAG_CLIPPER_REDUCTION,
    IDC_LABEL_DIAG_LIMITER,
    IDC_DIAG_LIMITER_REDUCTION,
    IDC_LABEL_DIAG_TRUE_PEAK,
    IDC_DIAG_TRUE_PEAK,
    IDC_LABEL_DIAG_LATENCY,
    IDC_DIAG_LATENCY,
    IDC_LABEL_DIAG_PEAK_GUARD,
    IDC_DIAG_PEAK_GUARD,
    IDC_LABEL_DIAG_SAMPLE_RATE,
    IDC_DIAG_SAMPLE_RATE,
    IDC_LABEL_DIAG_CHANNEL_LAYOUT,
    IDC_DIAG_CHANNEL_LAYOUT,
    IDC_LABEL_DIAG_CPU,
    IDC_DIAG_CPU_LOAD,
    IDC_LABEL_DIAG_CLIP_EVENTS,
    IDC_DIAG_CLIP_EVENT_COUNT,
    IDC_LABEL_DIAG_THREE_BAND,
    IDC_DIAG_THREE_BAND_REDUCTION,
    IDC_LABEL_DIAG_EVALUATION,
    IDC_DIAG_PROCESSING_EVALUATION,
    IDC_LABEL_DIAG_MAX_TRUE_PEAK,
    IDC_DIAG_MAX_TRUE_PEAK,
    IDC_LABEL_DIAG_MAX_COMPRESSOR,
    IDC_DIAG_MAX_COMPRESSOR_REDUCTION,
    IDC_LABEL_DIAG_MAX_CLIPPER,
    IDC_DIAG_MAX_CLIPPER_REDUCTION,
    IDC_LABEL_DIAG_MAX_LIMITER,
    IDC_DIAG_MAX_LIMITER_REDUCTION
};

template <t_size Count>
void set_page_controls_visible(
    HWND wnd,
    const int (&controls)[Count],
    bool visible
) {
    for (int control_id : controls) {
        HWND control = GetDlgItem(wnd, control_id);

        if (control != nullptr) {
            ShowWindow(control, visible ? SW_SHOW : SW_HIDE);
        }
    }
}

template <t_size Count>
bool page_contains_control(
    const int (&controls)[Count],
    int control_id
) {
    for (int page_control_id : controls) {
        if (page_control_id == control_id) {
            return true;
        }
    }

    return false;
}

void update_config_tab_page(HWND wnd, int selected_page) {
    const int page = clamp_value(selected_page, 0, 2);

    set_page_controls_visible(
        wnd,
        kBasicPageControls,
        page == 0
    );
    set_page_controls_visible(
        wnd,
        kAdvancedPageControls,
        page == 1
    );
    set_page_controls_visible(
        wnd,
        kDiagnosticPageControls,
        page == 2
    );

    InvalidateRect(wnd, nullptr, TRUE);
}

void setup_config_tabs(HWND wnd) {
    HWND tabs = GetDlgItem(wnd, IDC_CONFIG_TABS);

    if (tabs == nullptr) {
        return;
    }

    const wchar_t* titles[] = {
        L"基本設定",
        L"追加処理",
        L"診断"
    };

    for (int index = 0; index < 3; ++index) {
        TCITEMW item = {};
        item.mask = TCIF_TEXT;
        item.pszText = const_cast<LPWSTR>(titles[index]);
        TabCtrl_InsertItem(tabs, index, &item);
    }

    TabCtrl_SetCurSel(tabs, 0);
    update_config_tab_page(wnd, 0);
}

void select_config_tab_for_control(HWND wnd, int control_id) {
    int page = 0;

    if (page_contains_control(
            kAdvancedPageControls,
            control_id
        )) {
        page = 1;
    }
    else if (page_contains_control(
                 kDiagnosticPageControls,
                 control_id
             )) {
        page = 2;
    }

    HWND tabs = GetDlgItem(wnd, IDC_CONFIG_TABS);

    if (tabs != nullptr) {
        TabCtrl_SetCurSel(tabs, page);
    }

    update_config_tab_page(wnd, page);
}

void fit_dialog_to_monitor_work_area(HWND wnd) {
    RECT window_rect = {};

    if (!GetWindowRect(wnd, &window_rect)) {
        return;
    }

    HMONITOR monitor = MonitorFromWindow(
        wnd,
        MONITOR_DEFAULTTONEAREST
    );

    MONITORINFO monitor_info = {};
    monitor_info.cbSize = sizeof(monitor_info);

    if (!GetMonitorInfoW(monitor, &monitor_info)) {
        return;
    }

    const LONG width =
        window_rect.right - window_rect.left;
    const LONG height =
        window_rect.bottom - window_rect.top;
    const RECT& work = monitor_info.rcWork;
    const LONG work_width = work.right - work.left;
    const LONG work_height = work.bottom - work.top;

    LONG x = window_rect.left;
    LONG y = window_rect.top;

    if (width <= work_width) {
        x = clamp_value<LONG>(
            x,
            work.left,
            work.right - width
        );
    }
    else {
        x = work.left;
    }

    if (height <= work_height) {
        y = clamp_value<LONG>(
            y,
            work.top,
            work.bottom - height
        );
    }
    else {
        y = work.top;
    }

    SetWindowPos(
        wnd,
        nullptr,
        x,
        y,
        0,
        0,
        SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE
    );
}

void set_float_text(HWND wnd, int control_id, float value) {
    wchar_t text[64] = {};
    swprintf_s(text, L"%.1f", static_cast<double>(value));
    SetDlgItemTextW(wnd, control_id, text);
}

bool read_float(
    HWND wnd,
    int control_id,
    float minimum,
    float maximum,
    float& out,
    const wchar_t* field_name
) {
    wchar_t text[128] = {};
    GetDlgItemTextW(wnd, control_id, text, static_cast<int>(std::size(text)));

    wchar_t* end = nullptr;
    const float value = std::wcstof(text, &end);

    while (end != nullptr && *end == L' ') {
        ++end;
    }

    if (end == text || (end != nullptr && *end != L'\0') ||
        !std::isfinite(value) || value < minimum || value > maximum) {
        wchar_t message[256] = {};
        swprintf_s(
            message,
            L"%sの値が正しくありません。\n範囲：%.1f ～ %.1f",
            field_name,
            static_cast<double>(minimum),
            static_cast<double>(maximum)
        );
        MessageBoxW(wnd, message, L"入力値の確認", MB_OK | MB_ICONWARNING);
        select_config_tab_for_control(wnd, control_id);
        SetFocus(GetDlgItem(wnd, control_id));
        return false;
    }

    out = value;
    return true;
}

void settings_to_dialog(HWND wnd, const r128_settings& value) {
    set_float_text(wnd, IDC_TARGET_LUFS, value.target_lufs);
    set_float_text(wnd, IDC_MAX_BOOST, value.max_boost_db);
    set_float_text(wnd, IDC_MAX_ATTENUATION, value.max_attenuation_db);
    set_float_text(wnd, IDC_TRUE_PEAK, value.true_peak_limit_dbtp);
    set_float_text(wnd, IDC_LOOKAHEAD_MS, value.lookahead_ms);
    set_float_text(wnd, IDC_LIMITER_RELEASE_MS, value.limiter_release_ms);
    set_float_text(
        wnd,
        IDC_STARTUP_ANALYSIS_SECONDS,
        value.startup_analysis_seconds
    );
    set_float_text(
        wnd,
        IDC_SILENCE_GUARD_LUFS,
        value.silence_guard_lufs
    );
    set_float_text(
        wnd,
        IDC_GAIN_LOCK_SECONDS,
        value.gain_lock_seconds
    );
    set_float_text(
        wnd,
        IDC_GAIN_LOCK_TOLERANCE_LU,
        value.gain_lock_tolerance_lu
    );
    set_float_text(
        wnd,
        IDC_MODERN_STRENGTH,
        value.modern_strength_percent
    );

    CheckDlgButton(
        wnd,
        IDC_RESET_EACH_TRACK,
        value.reset_each_track ? BST_CHECKED : BST_UNCHECKED
    );
    CheckDlgButton(
        wnd,
        IDC_ENABLE_PEAK_GUARD,
        value.enable_peak_guard ? BST_CHECKED : BST_UNCHECKED
    );
    CheckDlgButton(
        wnd,
        IDC_ENABLE_SILENCE_GUARD,
        value.enable_silence_guard ? BST_CHECKED : BST_UNCHECKED
    );
    CheckDlgButton(
        wnd,
        IDC_ENABLE_GAIN_LOCK,
        value.enable_gain_lock ? BST_CHECKED : BST_UNCHECKED
    );
    CheckDlgButton(
        wnd,
        IDC_ENABLE_MODERN_BOOST,
        value.enable_modern_boost ? BST_CHECKED : BST_UNCHECKED
    );
    CheckDlgButton(
        wnd,
        IDC_ENABLE_ADAPTIVE_MASTER,
        value.enable_adaptive_master ? BST_CHECKED : BST_UNCHECKED
    );
    CheckDlgButton(
        wnd,
        IDC_ENABLE_THREE_BAND_MASTER,
        value.enable_three_band_master ? BST_CHECKED : BST_UNCHECKED
    );
}


void set_control_text(HWND wnd, int control_id, const wchar_t* text) {
    SetDlgItemTextW(wnd, control_id, text);
}

unsigned count_channel_flags(unsigned channel_mask) {
    unsigned count = 0;

    while (channel_mask != 0) {
        count += channel_mask & 1u;
        channel_mask >>= 1;
    }

    return count;
}

bool is_surround_channel_flag(unsigned channel_flag) {
    return channel_flag == kChannelBackLeft ||
        channel_flag == kChannelBackRight ||
        channel_flag == kChannelBackCenter ||
        channel_flag == kChannelSideLeft ||
        channel_flag == kChannelSideRight;
}

void format_channel_layout_text(
    wchar_t* output,
    t_size output_count,
    unsigned channels,
    unsigned channel_mask,
    bool lfe_excluded
) {
    if (output == nullptr || output_count == 0) {
        return;
    }

    const wchar_t* layout_name = nullptr;

    if (channels == 1 && channel_mask == kChannelConfigMono) {
        layout_name = L"モノラル";
    }
    else if (channels == 2 && channel_mask == kChannelConfigStereo) {
        layout_name = L"ステレオ";
    }
    else if (channels == 6 && channel_mask == kChannelConfig5Point1) {
        layout_name = L"5.1（バック）";
    }
    else if (channels == 6 && channel_mask == kChannelConfig5Point1Side) {
        layout_name = L"5.1（サイド）";
    }
    else if (channels == 8 && channel_mask == kChannelConfig7Point1) {
        layout_name = L"7.1";
    }

    if (layout_name != nullptr) {
        if (lfe_excluded) {
            swprintf_s(
                output,
                output_count,
                L"%s / LFE除外",
                layout_name
            );
        }
        else {
            swprintf_s(output, output_count, L"%s", layout_name);
        }
        return;
    }

    if (channels == 0) {
        swprintf_s(output, output_count, L"未検出");
        return;
    }

    if (channel_mask == 0) {
        swprintf_s(output, output_count, L"%u ch / 配置不明", channels);
        return;
    }

    if (lfe_excluded) {
        swprintf_s(
            output,
            output_count,
            L"%u ch / LFE除外",
            channels
        );
    }
    else {
        swprintf_s(output, output_count, L"%u ch", channels);
    }
}

const wchar_t* normalization_state_to_text(int state) {
    switch (state) {
    case 1:
        return L"測定中・保留";
    case 2:
        return L"静音保護・保留";
    case 4:
        return L"安全減衰中";
    default:
        return L"通常補正中";
    }
}

const wchar_t* peak_guard_state_to_text(int state) {
    switch (state) {
    case 0:
        return L"無効";
    case 2:
        return L"作動中";
    default:
        return L"待機";
    }
}

bool copy_unicode_text_to_clipboard(HWND wnd, const std::wstring& value) {
    if (!OpenClipboard(wnd)) {
        return false;
    }

    bool success = false;
    HGLOBAL memory = nullptr;

    do {
        if (!EmptyClipboard()) {
            break;
        }

        const SIZE_T bytes =
            (value.size() + 1) * sizeof(wchar_t);

        memory = GlobalAlloc(GMEM_MOVEABLE, bytes);
        if (memory == nullptr) {
            break;
        }

        void* destination = GlobalLock(memory);
        if (destination == nullptr) {
            break;
        }

        CopyMemory(destination, value.c_str(), bytes);
        GlobalUnlock(memory);

        if (SetClipboardData(CF_UNICODETEXT, memory) == nullptr) {
            break;
        }

        memory = nullptr;
        success = true;
    } while (false);

    CloseClipboard();

    if (memory != nullptr) {
        GlobalFree(memory);
    }

    return success;
}

std::wstring format_diagnostic_number(
    double value,
    const wchar_t* unit,
    int decimals
) {
    wchar_t buffer[64] = {};

    if (!std::isfinite(value) || value <= -190.0) {
        return L"未測定";
    }

    if (decimals == 1) {
        swprintf_s(buffer, L"%.1f %s", value, unit);
    }
    else {
        swprintf_s(buffer, L"%.2f %s", value, unit);
    }

    return std::wstring(buffer);
}

std::wstring build_diagnostic_report() {
    const unsigned long long last_update_tick =
        g_diagnostic_last_update_tick.load(std::memory_order_relaxed);
    const unsigned long long current_tick =
        static_cast<unsigned long long>(GetTickCount64());

    const bool stream_active =
        g_diagnostic_stream_active.load(std::memory_order_relaxed) != 0 &&
        last_update_tick != 0 &&
        current_tick >= last_update_tick &&
        (current_tick - last_update_tick) < 1500;

    const double momentary_lufs =
        g_diagnostic_momentary_lufs.load(std::memory_order_relaxed);
    const double short_term_lufs =
        g_diagnostic_short_term_lufs.load(std::memory_order_relaxed);
    const double integrated_lufs =
        g_diagnostic_integrated_lufs.load(std::memory_order_relaxed);
    const double lra_lu =
        g_diagnostic_lra_lu.load(std::memory_order_relaxed);
    const double applied_gain_db =
        g_diagnostic_applied_gain_db.load(std::memory_order_relaxed);
    const double normalization_gain_db =
        g_diagnostic_normalization_gain_db.load(
            std::memory_order_relaxed
        );
    const double true_peak_dbtp =
        g_diagnostic_true_peak_dbtp.load(std::memory_order_relaxed);
    const double limiter_reduction_db =
        g_diagnostic_limiter_reduction_db.load(std::memory_order_relaxed);
    const double compressor_reduction_db =
        g_diagnostic_compressor_reduction_db.load(
            std::memory_order_relaxed
        );
    const double clipper_reduction_db =
        g_diagnostic_clipper_reduction_db.load(
            std::memory_order_relaxed
        );
    const bool modern_boost_enabled =
        g_diagnostic_modern_boost_state.load(
            std::memory_order_relaxed
        ) != 0;
    const double output_integrated_lufs =
        g_diagnostic_output_integrated_lufs.load(
            std::memory_order_relaxed
        );
    const double target_difference_lu =
        g_diagnostic_target_difference_lu.load(
            std::memory_order_relaxed
        );
    const int processing_risk_state =
        g_diagnostic_processing_risk_state.load(
            std::memory_order_relaxed
        );
    const double safety_reduction_db =
        g_diagnostic_safety_reduction_db.load(
            std::memory_order_relaxed
        );
    const int original_compare_mode =
        g_diagnostic_original_compare_state.load(
            std::memory_order_relaxed
        );
    const bool original_compare_active = original_compare_mode != 0;
    const double compare_match_gain_db =
        g_diagnostic_compare_match_gain_db.load(
            std::memory_order_relaxed
        );
    const bool adaptive_master_enabled =
        g_diagnostic_adaptive_master_state.load(
            std::memory_order_relaxed
        ) != 0;
    const bool three_band_master_enabled =
        g_diagnostic_three_band_master_state.load(
            std::memory_order_relaxed
        ) != 0;
    const double effective_strength_percent =
        g_diagnostic_effective_strength_percent.load(
            std::memory_order_relaxed
        );
    const double three_band_low_reduction_db =
        g_diagnostic_three_band_low_reduction_db.load(
            std::memory_order_relaxed
        );
    const double three_band_mid_reduction_db =
        g_diagnostic_three_band_mid_reduction_db.load(
            std::memory_order_relaxed
        );
    const double three_band_high_reduction_db =
        g_diagnostic_three_band_high_reduction_db.load(
            std::memory_order_relaxed
        );
    const double track_max_three_band_low_reduction_db =
        g_diagnostic_track_max_three_band_low_reduction_db.load(
            std::memory_order_relaxed
        );
    const double track_max_three_band_mid_reduction_db =
        g_diagnostic_track_max_three_band_mid_reduction_db.load(
            std::memory_order_relaxed
        );
    const double track_max_three_band_high_reduction_db =
        g_diagnostic_track_max_three_band_high_reduction_db.load(
            std::memory_order_relaxed
        );
    const bool final_summary_valid =
        g_diagnostic_final_summary_valid.load(
            std::memory_order_relaxed
        ) != 0;
    const double track_max_true_peak_dbtp =
        g_diagnostic_track_max_true_peak_dbtp.load(
            std::memory_order_relaxed
        );
    const double track_max_compressor_reduction_db =
        g_diagnostic_track_max_compressor_reduction_db.load(
            std::memory_order_relaxed
        );
    const double track_max_clipper_reduction_db =
        g_diagnostic_track_max_clipper_reduction_db.load(
            std::memory_order_relaxed
        );
    const double track_max_limiter_reduction_db =
        g_diagnostic_track_max_limiter_reduction_db.load(
            std::memory_order_relaxed
        );
    const unsigned long long clip_event_count =
        g_diagnostic_clip_event_count.load(
            std::memory_order_relaxed
        );
    const unsigned long long recovered_sample_count =
        g_diagnostic_recovered_sample_count.load(
            std::memory_order_relaxed
        );
    const int track_evaluation_state =
        g_diagnostic_track_evaluation_state.load(
            std::memory_order_relaxed
        );
    const unsigned sample_rate_hz =
        g_diagnostic_sample_rate_hz.load(
            std::memory_order_relaxed
        );
    const double cpu_load_percent =
        g_diagnostic_cpu_load_percent.load(
            std::memory_order_relaxed
        );
    const double final_input_integrated_lufs =
        g_diagnostic_final_input_integrated_lufs.load(
            std::memory_order_relaxed
        );
    const double final_output_integrated_lufs =
        g_diagnostic_final_output_integrated_lufs.load(
            std::memory_order_relaxed
        );
    const double final_target_difference_lu =
        g_diagnostic_final_target_difference_lu.load(
            std::memory_order_relaxed
        );
    const double final_lra_lu =
        g_diagnostic_final_lra_lu.load(
            std::memory_order_relaxed
        );
    const double final_max_true_peak_dbtp =
        g_diagnostic_final_max_true_peak_dbtp.load(
            std::memory_order_relaxed
        );
    const double final_max_compressor_reduction_db =
        g_diagnostic_final_max_compressor_reduction_db.load(
            std::memory_order_relaxed
        );
    const double final_max_clipper_reduction_db =
        g_diagnostic_final_max_clipper_reduction_db.load(
            std::memory_order_relaxed
        );
    const double final_max_limiter_reduction_db =
        g_diagnostic_final_max_limiter_reduction_db.load(
            std::memory_order_relaxed
        );
    const bool final_three_band_master_enabled =
        g_diagnostic_final_three_band_master_state.load(
            std::memory_order_relaxed
        ) != 0;
    const double final_max_three_band_low_reduction_db =
        g_diagnostic_final_max_three_band_low_reduction_db.load(
            std::memory_order_relaxed
        );
    const double final_max_three_band_mid_reduction_db =
        g_diagnostic_final_max_three_band_mid_reduction_db.load(
            std::memory_order_relaxed
        );
    const double final_max_three_band_high_reduction_db =
        g_diagnostic_final_max_three_band_high_reduction_db.load(
            std::memory_order_relaxed
        );
    const unsigned long long final_clip_event_count =
        g_diagnostic_final_clip_event_count.load(
            std::memory_order_relaxed
        );
    const unsigned long long final_recovered_sample_count =
        g_diagnostic_final_recovered_sample_count.load(
            std::memory_order_relaxed
        );
    const int final_evaluation_state =
        g_diagnostic_final_evaluation_state.load(
            std::memory_order_relaxed
        );
    const unsigned final_sample_rate_hz =
        g_diagnostic_final_sample_rate_hz.load(
            std::memory_order_relaxed
        );
    const double final_cpu_load_percent =
        g_diagnostic_final_cpu_load_percent.load(
            std::memory_order_relaxed
        );
    const double latency_ms =
        g_diagnostic_latency_ms.load(std::memory_order_relaxed);
    const int normalization_state =
        g_diagnostic_normalization_state.load(std::memory_order_relaxed);
    const int gain_lock_state =
        g_diagnostic_gain_lock_state.load(std::memory_order_relaxed);
    const double remaining_seconds =
        g_diagnostic_gain_lock_remaining_seconds.load(
            std::memory_order_relaxed
        );
    const double locked_gain_db =
        g_diagnostic_locked_gain_db.load(std::memory_order_relaxed);
    const int peak_guard_state =
        g_diagnostic_peak_guard_state.load(std::memory_order_relaxed);
    const unsigned channel_count =
        g_diagnostic_channel_count.load(std::memory_order_relaxed);
    const unsigned channel_mask =
        g_diagnostic_channel_mask.load(std::memory_order_relaxed);
    const bool lfe_excluded =
        g_diagnostic_lfe_excluded.load(std::memory_order_relaxed) != 0;

    wchar_t channel_text[128] = {};
    format_channel_layout_text(
        channel_text,
        std::size(channel_text),
        channel_count,
        channel_mask,
        lfe_excluded
    );

    wchar_t gain_lock_text[160] = {};
    switch (gain_lock_state) {
    case 0:
        swprintf_s(gain_lock_text, L"無効");
        break;
    case 2:
        swprintf_s(
            gain_lock_text,
            L"固定済み %+.2f dB",
            locked_gain_db
        );
        break;
    case 3:
        if (normalization_gain_db < -0.01) {
            swprintf_s(
                gain_lock_text,
                L"固定 %+.2f → 現在 %+.2f dB（安全減衰）",
                locked_gain_db,
                normalization_gain_db
            );
        }
        else {
            swprintf_s(
                gain_lock_text,
                L"固定 %+.2f → 現在 %+.2f dB（増幅抑制）",
                locked_gain_db,
                normalization_gain_db
            );
        }
        break;
    default:
        swprintf_s(
            gain_lock_text,
            L"残り %.1f秒",
            std::max(0.0, remaining_seconds)
        );
        break;
    }

    const std::wstring momentary =
        format_diagnostic_number(momentary_lufs, L"LUFS", 1);
    const std::wstring short_term =
        format_diagnostic_number(short_term_lufs, L"LUFS", 1);
    const std::wstring integrated =
        format_diagnostic_number(integrated_lufs, L"LUFS", 1);
    const std::wstring output_integrated =
        format_diagnostic_number(output_integrated_lufs, L"LUFS", 1);
    const std::wstring target_difference =
        format_diagnostic_number(target_difference_lu, L"LU", 1);
    const std::wstring lra =
        format_diagnostic_number(lra_lu, L"LU", 1);
    const std::wstring true_peak =
        format_diagnostic_number(true_peak_dbtp, L"dBTP", 2);
    const std::wstring track_max_true_peak =
        format_diagnostic_number(track_max_true_peak_dbtp, L"dBTP", 2);
    const std::wstring final_input_integrated =
        format_diagnostic_number(final_input_integrated_lufs, L"LUFS", 1);
    const std::wstring final_output_integrated =
        format_diagnostic_number(final_output_integrated_lufs, L"LUFS", 1);
    const std::wstring final_target_difference =
        format_diagnostic_number(final_target_difference_lu, L"LU", 1);
    const std::wstring final_lra =
        format_diagnostic_number(final_lra_lu, L"LU", 1);
    const std::wstring final_max_true_peak =
        format_diagnostic_number(final_max_true_peak_dbtp, L"dBTP", 2);

    wchar_t report[6656] = {};
    swprintf_s(
        report,
        L"R128 音量ノーマライザー 1.5.0\r\n"
        L"再生状態: %s\r\n"
        L"補正状態: %s\r\n"
        L"補正ゲイン固定: %s\r\n"
        L"モダンブースト: %s\r\n"
        L"A/B比較: %s\r\n"
        L"A/B一致ゲイン: %+.2f dB（15msフェード）\r\n"
        L"アダプティブ・マスター: %s\r\n"
        L"3バンド・マスター: %s\r\n"
        L"実効モダン強度: %.1f %%\r\n"
        L"3バンド減衰（現在・低／中／高）: %.2f / %.2f / %.2f dB\r\n"
        L"3バンド最大減衰（低／中／高）: %.2f / %.2f / %.2f dB\r\n"
        L"Momentary（入力）: %s\r\n"
        L"Short-term（入力）: %s\r\n"
        L"Integrated（入力）: %s\r\n"
        L"Integrated（出力）: %s\r\n"
        L"目標との差: %s\r\n"
        L"LRA推定: %s\r\n"
        L"処理状態: %s\r\n"
        L"自動セーフティ: %.2f dB\r\n"
        L"現在のノーマライズゲイン: %+.2f dB\r\n"
        L"適用中の総ゲイン: %+.2f dB\r\n"
        L"コンプレッサー減衰: %.2f dB\r\n"
        L"クリッパー減衰: %.2f dB\r\n"
        L"リミッター減衰: %.2f dB\r\n"
        L"検出True Peak: %s\r\n"
        L"ピーク保護: %s\r\n"
        L"実効レイテンシー: %.1f ms\r\n"
        L"サンプルレート: %u Hz\r\n"
        L"推定CPU負荷: %.2f %%\r\n"
        L"トラック最大True Peak: %s\r\n"
        L"最大コンプレッサー減衰: %.2f dB\r\n"
        L"最大クリッパー減衰: %.2f dB\r\n"
        L"最大リミッター減衰: %.2f dB\r\n"
        L"0 dBTP超過イベント: %llu 回\r\n"
        L"異常値保護作動: %llu サンプル\r\n"
        L"トラック評価: %s\r\n"
        L"チャンネル構成: %s\r\n"
        L"\r\n"
        L"前回トラック確定結果: %s\r\n"
        L"入力Integrated: %s\r\n"
        L"出力Integrated: %s\r\n"
        L"目標との差: %s\r\n"
        L"LRA推定: %s\r\n"
        L"最大True Peak: %s\r\n"
        L"最大コンプレッサー減衰: %.2f dB\r\n"
        L"最大クリッパー減衰: %.2f dB\r\n"
        L"最大リミッター減衰: %.2f dB\r\n"
        L"3バンド使用: %s\r\n"
        L"3バンド最大減衰（低／中／高）: %.2f / %.2f / %.2f dB\r\n"
        L"0 dBTP超過イベント: %llu 回\r\n"
        L"異常値保護作動: %llu サンプル\r\n"
        L"処理評価: %s\r\n"
        L"サンプルレート: %u Hz\r\n"
        L"推定CPU負荷: %.2f %%\r\n",
        stream_active ? L"再生中" : L"待機中",
        normalization_state_to_text(normalization_state),
        gain_lock_text,
        modern_boost_enabled ? L"有効" : L"無効",
        original_compare_mode == 2
            ? L"音量一致"
            : (original_compare_mode == 1 ? L"完全バイパス" : L"通常"),
        compare_match_gain_db,
        adaptive_master_enabled ? L"有効" : L"無効",
        three_band_master_enabled ? L"有効" : L"無効",
        effective_strength_percent,
        three_band_low_reduction_db,
        three_band_mid_reduction_db,
        three_band_high_reduction_db,
        track_max_three_band_low_reduction_db,
        track_max_three_band_mid_reduction_db,
        track_max_three_band_high_reduction_db,
        momentary.c_str(),
        short_term.c_str(),
        integrated.c_str(),
        output_integrated.c_str(),
        target_difference.c_str(),
        lra.c_str(),
        processing_risk_to_text(processing_risk_state),
        safety_reduction_db,
        normalization_gain_db,
        applied_gain_db,
        compressor_reduction_db,
        clipper_reduction_db,
        limiter_reduction_db,
        true_peak.c_str(),
        peak_guard_state_to_text(peak_guard_state),
        latency_ms,
        sample_rate_hz,
        cpu_load_percent,
        track_max_true_peak.c_str(),
        track_max_compressor_reduction_db,
        track_max_clipper_reduction_db,
        track_max_limiter_reduction_db,
        clip_event_count,
        recovered_sample_count,
        track_evaluation_to_text(track_evaluation_state),
        channel_text,
        final_summary_valid ? L"あり" : L"なし",
        final_input_integrated.c_str(),
        final_output_integrated.c_str(),
        final_target_difference.c_str(),
        final_lra.c_str(),
        final_max_true_peak.c_str(),
        final_max_compressor_reduction_db,
        final_max_clipper_reduction_db,
        final_max_limiter_reduction_db,
        final_three_band_master_enabled ? L"有効" : L"無効",
        final_max_three_band_low_reduction_db,
        final_max_three_band_mid_reduction_db,
        final_max_three_band_high_reduction_db,
        final_clip_event_count,
        final_recovered_sample_count,
        track_evaluation_to_text(final_evaluation_state),
        final_sample_rate_hz,
        final_cpu_load_percent
    );
    return std::wstring(report);
}

void refresh_diagnostic_controls(HWND wnd) {
    const unsigned long long last_update_tick =
        g_diagnostic_last_update_tick.load(std::memory_order_relaxed);
    const unsigned long long current_tick =
        static_cast<unsigned long long>(GetTickCount64());

    const bool stream_active =
        g_diagnostic_stream_active.load(std::memory_order_relaxed) != 0 &&
        last_update_tick != 0 &&
        current_tick >= last_update_tick &&
        (current_tick - last_update_tick) < 1500;

    const double momentary_lufs =
        g_diagnostic_momentary_lufs.load(std::memory_order_relaxed);
    const double short_term_lufs =
        g_diagnostic_short_term_lufs.load(std::memory_order_relaxed);
    const double integrated_lufs =
        g_diagnostic_integrated_lufs.load(std::memory_order_relaxed);
    const double lra_lu =
        g_diagnostic_lra_lu.load(std::memory_order_relaxed);
    const double applied_gain_db =
        g_diagnostic_applied_gain_db.load(std::memory_order_relaxed);
    const double normalization_gain_db =
        g_diagnostic_normalization_gain_db.load(
            std::memory_order_relaxed
        );
    const double true_peak_dbtp =
        g_diagnostic_true_peak_dbtp.load(std::memory_order_relaxed);
    const int peak_guard_state =
        g_diagnostic_peak_guard_state.load(std::memory_order_relaxed);
    const unsigned channel_count =
        g_diagnostic_channel_count.load(std::memory_order_relaxed);
    const unsigned channel_mask =
        g_diagnostic_channel_mask.load(std::memory_order_relaxed);
    const bool lfe_excluded =
        g_diagnostic_lfe_excluded.load(std::memory_order_relaxed) != 0;
    const double limiter_reduction_db =
        g_diagnostic_limiter_reduction_db.load(std::memory_order_relaxed);
    const double compressor_reduction_db =
        g_diagnostic_compressor_reduction_db.load(
            std::memory_order_relaxed
        );
    const double clipper_reduction_db =
        g_diagnostic_clipper_reduction_db.load(
            std::memory_order_relaxed
        );
    const bool modern_boost_enabled =
        g_diagnostic_modern_boost_state.load(
            std::memory_order_relaxed
        ) != 0;
    const double output_integrated_lufs =
        g_diagnostic_output_integrated_lufs.load(
            std::memory_order_relaxed
        );
    const double target_difference_lu =
        g_diagnostic_target_difference_lu.load(
            std::memory_order_relaxed
        );
    const int processing_risk_state =
        g_diagnostic_processing_risk_state.load(
            std::memory_order_relaxed
        );
    const double safety_reduction_db =
        g_diagnostic_safety_reduction_db.load(
            std::memory_order_relaxed
        );
    const int original_compare_mode =
        g_diagnostic_original_compare_state.load(
            std::memory_order_relaxed
        );
    const bool original_compare_active = original_compare_mode != 0;
    const double compare_match_gain_db =
        g_diagnostic_compare_match_gain_db.load(
            std::memory_order_relaxed
        );
    const bool adaptive_master_enabled =
        g_diagnostic_adaptive_master_state.load(
            std::memory_order_relaxed
        ) != 0;
    const bool three_band_master_enabled =
        g_diagnostic_three_band_master_state.load(
            std::memory_order_relaxed
        ) != 0;
    const double effective_strength_percent =
        g_diagnostic_effective_strength_percent.load(
            std::memory_order_relaxed
        );
    const double three_band_low_reduction_db =
        g_diagnostic_three_band_low_reduction_db.load(
            std::memory_order_relaxed
        );
    const double three_band_mid_reduction_db =
        g_diagnostic_three_band_mid_reduction_db.load(
            std::memory_order_relaxed
        );
    const double three_band_high_reduction_db =
        g_diagnostic_three_band_high_reduction_db.load(
            std::memory_order_relaxed
        );
    const bool final_summary_valid =
        g_diagnostic_final_summary_valid.load(
            std::memory_order_relaxed
        ) != 0;
    const double track_max_true_peak_dbtp =
        g_diagnostic_track_max_true_peak_dbtp.load(
            std::memory_order_relaxed
        );
    const double track_max_compressor_reduction_db =
        g_diagnostic_track_max_compressor_reduction_db.load(
            std::memory_order_relaxed
        );
    const double track_max_clipper_reduction_db =
        g_diagnostic_track_max_clipper_reduction_db.load(
            std::memory_order_relaxed
        );
    const double track_max_limiter_reduction_db =
        g_diagnostic_track_max_limiter_reduction_db.load(
            std::memory_order_relaxed
        );
    const unsigned long long clip_event_count =
        g_diagnostic_clip_event_count.load(
            std::memory_order_relaxed
        );
    const int track_evaluation_state =
        g_diagnostic_track_evaluation_state.load(
            std::memory_order_relaxed
        );
    const unsigned sample_rate_hz =
        g_diagnostic_sample_rate_hz.load(
            std::memory_order_relaxed
        );
    const double cpu_load_percent =
        g_diagnostic_cpu_load_percent.load(
            std::memory_order_relaxed
        );
    const double final_input_integrated_lufs =
        g_diagnostic_final_input_integrated_lufs.load(
            std::memory_order_relaxed
        );
    const double final_output_integrated_lufs =
        g_diagnostic_final_output_integrated_lufs.load(
            std::memory_order_relaxed
        );
    const double final_target_difference_lu =
        g_diagnostic_final_target_difference_lu.load(
            std::memory_order_relaxed
        );
    const double final_lra_lu =
        g_diagnostic_final_lra_lu.load(
            std::memory_order_relaxed
        );
    const double final_max_true_peak_dbtp =
        g_diagnostic_final_max_true_peak_dbtp.load(
            std::memory_order_relaxed
        );
    const double final_max_compressor_reduction_db =
        g_diagnostic_final_max_compressor_reduction_db.load(
            std::memory_order_relaxed
        );
    const double final_max_clipper_reduction_db =
        g_diagnostic_final_max_clipper_reduction_db.load(
            std::memory_order_relaxed
        );
    const double final_max_limiter_reduction_db =
        g_diagnostic_final_max_limiter_reduction_db.load(
            std::memory_order_relaxed
        );
    const bool final_three_band_master_enabled =
        g_diagnostic_final_three_band_master_state.load(
            std::memory_order_relaxed
        ) != 0;
    const double final_max_three_band_low_reduction_db =
        g_diagnostic_final_max_three_band_low_reduction_db.load(
            std::memory_order_relaxed
        );
    const double final_max_three_band_mid_reduction_db =
        g_diagnostic_final_max_three_band_mid_reduction_db.load(
            std::memory_order_relaxed
        );
    const double final_max_three_band_high_reduction_db =
        g_diagnostic_final_max_three_band_high_reduction_db.load(
            std::memory_order_relaxed
        );
    const unsigned long long final_clip_event_count =
        g_diagnostic_final_clip_event_count.load(
            std::memory_order_relaxed
        );
    const int final_evaluation_state =
        g_diagnostic_final_evaluation_state.load(
            std::memory_order_relaxed
        );
    const unsigned final_sample_rate_hz =
        g_diagnostic_final_sample_rate_hz.load(
            std::memory_order_relaxed
        );
    const double final_cpu_load_percent =
        g_diagnostic_final_cpu_load_percent.load(
            std::memory_order_relaxed
        );
    const double latency_ms =
        g_diagnostic_latency_ms.load(std::memory_order_relaxed);
    const int normalization_state =
        g_diagnostic_normalization_state.load(
            std::memory_order_relaxed
        );
    const int gain_lock_state =
        g_diagnostic_gain_lock_state.load(
            std::memory_order_relaxed
        );
    const double gain_lock_remaining_seconds =
        g_diagnostic_gain_lock_remaining_seconds.load(
            std::memory_order_relaxed
        );
    const double locked_gain_db =
        g_diagnostic_locked_gain_db.load(
            std::memory_order_relaxed
        );

    wchar_t text[128] = {};

    format_channel_layout_text(
        text,
        std::size(text),
        channel_count,
        channel_mask,
        lfe_excluded
    );
    set_control_text(wnd, IDC_DIAG_CHANNEL_LAYOUT, text);

    const double displayed_max_true_peak = stream_active
        ? track_max_true_peak_dbtp
        : final_max_true_peak_dbtp;
    const double displayed_max_compressor = stream_active
        ? track_max_compressor_reduction_db
        : final_max_compressor_reduction_db;
    const double displayed_max_clipper = stream_active
        ? track_max_clipper_reduction_db
        : final_max_clipper_reduction_db;
    const double displayed_max_limiter = stream_active
        ? track_max_limiter_reduction_db
        : final_max_limiter_reduction_db;
    const unsigned long long displayed_clip_events = stream_active
        ? clip_event_count
        : final_clip_event_count;
    const int displayed_evaluation = stream_active
        ? track_evaluation_state
        : final_evaluation_state;
    const unsigned displayed_sample_rate = stream_active
        ? sample_rate_hz
        : final_sample_rate_hz;
    const double displayed_cpu_load = stream_active
        ? cpu_load_percent
        : final_cpu_load_percent;

    if (std::isfinite(displayed_max_true_peak) &&
        displayed_max_true_peak > -190.0) {
        swprintf_s(text, L"%.2f dBTP", displayed_max_true_peak);
        set_control_text(wnd, IDC_DIAG_MAX_TRUE_PEAK, text);
    }
    else {
        set_control_text(wnd, IDC_DIAG_MAX_TRUE_PEAK, L"未測定");
    }

    swprintf_s(text, L"%.2f dB", displayed_max_compressor);
    set_control_text(wnd, IDC_DIAG_MAX_COMPRESSOR_REDUCTION, text);
    swprintf_s(text, L"%.2f dB", displayed_max_clipper);
    set_control_text(wnd, IDC_DIAG_MAX_CLIPPER_REDUCTION, text);
    swprintf_s(text, L"%.2f dB", displayed_max_limiter);
    set_control_text(wnd, IDC_DIAG_MAX_LIMITER_REDUCTION, text);
    swprintf_s(text, L"%llu 回", displayed_clip_events);
    set_control_text(wnd, IDC_DIAG_CLIP_EVENT_COUNT, text);
    set_control_text(
        wnd,
        IDC_DIAG_PROCESSING_EVALUATION,
        track_evaluation_to_text(displayed_evaluation)
    );
    if (displayed_sample_rate > 0) {
        swprintf_s(text, L"%u Hz", displayed_sample_rate);
        set_control_text(wnd, IDC_DIAG_SAMPLE_RATE, text);
    }
    else {
        set_control_text(wnd, IDC_DIAG_SAMPLE_RATE, L"未検出");
    }
    swprintf_s(text, L"%.2f %%", displayed_cpu_load);
    set_control_text(wnd, IDC_DIAG_CPU_LOAD, text);

    if (!stream_active) {
        set_control_text(wnd, IDC_DIAG_NORMALIZATION_STATE, L"待機中");
        set_control_text(wnd, IDC_DIAG_GAIN_LOCK, L"待機中");
        set_control_text(wnd, IDC_DIAG_MOMENTARY, L"待機中");
        set_control_text(wnd, IDC_DIAG_SHORT_TERM, L"待機中");
        if (final_summary_valid) {
            swprintf_s(text, L"前回 %.1f LUFS", final_input_integrated_lufs);
            set_control_text(wnd, IDC_DIAG_INTEGRATED, text);
            swprintf_s(text, L"前回 %.1f LUFS", final_output_integrated_lufs);
            set_control_text(wnd, IDC_DIAG_OUTPUT_INTEGRATED, text);
            swprintf_s(text, L"前回 %+.1f LU", final_target_difference_lu);
            set_control_text(wnd, IDC_DIAG_TARGET_DIFFERENCE, text);
            swprintf_s(text, L"前回 %.1f LU", final_lra_lu);
            set_control_text(wnd, IDC_DIAG_LRA, text);
        }
        else {
            set_control_text(wnd, IDC_DIAG_INTEGRATED, L"待機中");
            set_control_text(wnd, IDC_DIAG_OUTPUT_INTEGRATED, L"待機中");
            set_control_text(wnd, IDC_DIAG_TARGET_DIFFERENCE, L"待機中");
            set_control_text(wnd, IDC_DIAG_LRA, L"待機中");
        }
        set_control_text(
            wnd,
            IDC_DIAG_PROCESSING_RISK,
            original_compare_mode == 2
                ? L"A/B音量一致"
                : (original_compare_mode == 1
                    ? L"原音比較中"
                    : (modern_boost_enabled ? L"待機中" : L"無効"))
        );
        set_control_text(
            wnd,
            IDC_DIAG_SAFETY_REDUCTION,
            modern_boost_enabled ? L"0.00 dB" : L"無効"
        );
        set_control_text(wnd, IDC_DIAG_GAIN, L"待機中");
        set_control_text(wnd, IDC_DIAG_TRUE_PEAK, L"待機中");
        set_control_text(
            wnd,
            IDC_DIAG_COMPRESSOR_REDUCTION,
            modern_boost_enabled ? L"待機中" : L"無効"
        );
        set_control_text(
            wnd,
            IDC_DIAG_CLIPPER_REDUCTION,
            modern_boost_enabled ? L"待機中" : L"無効"
        );
        set_control_text(wnd, IDC_DIAG_LIMITER_REDUCTION, L"待機中");
        if (final_summary_valid && final_three_band_master_enabled) {
            swprintf_s(
                text,
                L"前回 低 %.1f / 中 %.1f / 高 %.1f dB",
                final_max_three_band_low_reduction_db,
                final_max_three_band_mid_reduction_db,
                final_max_three_band_high_reduction_db
            );
            set_control_text(wnd, IDC_DIAG_THREE_BAND_REDUCTION, text);
        }
        else {
            set_control_text(
                wnd,
                IDC_DIAG_THREE_BAND_REDUCTION,
                three_band_master_enabled ? L"待機中" : L"無効"
            );
        }
        swprintf_s(text, L"%.1f ms", latency_ms);
        set_control_text(wnd, IDC_DIAG_LATENCY, text);

        if (peak_guard_state == 0) {
            set_control_text(wnd, IDC_DIAG_PEAK_GUARD, L"無効");
        }
        else {
            set_control_text(wnd, IDC_DIAG_PEAK_GUARD, L"待機中");
        }

        return;
    }

    set_control_text(
        wnd,
        IDC_DIAG_NORMALIZATION_STATE,
        normalization_state_to_text(normalization_state)
    );

    switch (gain_lock_state) {
    case 0:
        set_control_text(wnd, IDC_DIAG_GAIN_LOCK, L"無効");
        break;
    case 2:
        swprintf_s(text, L"%+.2f dB 固定", locked_gain_db);
        set_control_text(wnd, IDC_DIAG_GAIN_LOCK, text);
        break;
    case 3:
        if (normalization_gain_db < -0.01) {
            swprintf_s(
                text,
                L"%+.2f→%+.2f 減衰",
                locked_gain_db,
                normalization_gain_db
            );
        }
        else {
            swprintf_s(
                text,
                L"%+.2f→%+.2f 抑制",
                locked_gain_db,
                normalization_gain_db
            );
        }
        set_control_text(wnd, IDC_DIAG_GAIN_LOCK, text);
        break;
    default:
        swprintf_s(
            text,
            L"残り %.1f秒",
            std::max(0.0, gain_lock_remaining_seconds)
        );
        set_control_text(wnd, IDC_DIAG_GAIN_LOCK, text);
        break;
    }

    if (std::isfinite(momentary_lufs) && momentary_lufs > -190.0) {
        swprintf_s(text, L"%.1f LUFS", momentary_lufs);
        set_control_text(wnd, IDC_DIAG_MOMENTARY, text);
    }
    else {
        set_control_text(wnd, IDC_DIAG_MOMENTARY, L"測定中…");
    }

    if (std::isfinite(short_term_lufs) && short_term_lufs > -190.0) {
        swprintf_s(text, L"%.1f LUFS", short_term_lufs);
        set_control_text(wnd, IDC_DIAG_SHORT_TERM, text);
    }
    else {
        set_control_text(wnd, IDC_DIAG_SHORT_TERM, L"測定中…");
    }

    if (std::isfinite(integrated_lufs) && integrated_lufs > -190.0) {
        swprintf_s(text, L"%.1f LUFS", integrated_lufs);
        set_control_text(wnd, IDC_DIAG_INTEGRATED, text);
    }
    else {
        set_control_text(wnd, IDC_DIAG_INTEGRATED, L"測定中…");
    }

    if (std::isfinite(output_integrated_lufs) &&
        output_integrated_lufs > -190.0) {
        swprintf_s(text, L"%.1f LUFS", output_integrated_lufs);
        set_control_text(wnd, IDC_DIAG_OUTPUT_INTEGRATED, text);
    }
    else {
        set_control_text(wnd, IDC_DIAG_OUTPUT_INTEGRATED, L"測定中…");
    }

    if (std::isfinite(target_difference_lu) &&
        target_difference_lu > -190.0) {
        swprintf_s(text, L"%+.1f LU", target_difference_lu);
        set_control_text(wnd, IDC_DIAG_TARGET_DIFFERENCE, text);
    }
    else {
        set_control_text(wnd, IDC_DIAG_TARGET_DIFFERENCE, L"測定中…");
    }

    if (std::isfinite(lra_lu) && lra_lu > -190.0) {
        swprintf_s(text, L"%.1f LU", lra_lu);
        set_control_text(wnd, IDC_DIAG_LRA, text);
    }
    else {
        set_control_text(wnd, IDC_DIAG_LRA, L"測定中…");
    }

    if (original_compare_active) {
        if (original_compare_mode == 2) {
            swprintf_s(text, L"A/B %+.1f dB", compare_match_gain_db);
            set_control_text(wnd, IDC_DIAG_PROCESSING_RISK, text);
        }
        else {
            set_control_text(wnd, IDC_DIAG_PROCESSING_RISK, L"原音比較中");
        }
    }
    else if (adaptive_master_enabled) {
        swprintf_s(
            text,
            L"%s・%.1f%%",
            processing_risk_to_text(processing_risk_state),
            effective_strength_percent
        );
        set_control_text(wnd, IDC_DIAG_PROCESSING_RISK, text);
    }
    else {
        set_control_text(
            wnd,
            IDC_DIAG_PROCESSING_RISK,
            processing_risk_to_text(processing_risk_state)
        );
    }

    if (modern_boost_enabled && std::isfinite(safety_reduction_db)) {
        swprintf_s(text, L"%.2f dB", safety_reduction_db);
        set_control_text(wnd, IDC_DIAG_SAFETY_REDUCTION, text);
    }
    else {
        set_control_text(wnd, IDC_DIAG_SAFETY_REDUCTION, L"無効");
    }

    if (std::isfinite(applied_gain_db)) {
        swprintf_s(text, L"%+.2f dB", applied_gain_db);
        set_control_text(wnd, IDC_DIAG_GAIN, text);
    }
    else {
        set_control_text(wnd, IDC_DIAG_GAIN, L"—");
    }

    if (modern_boost_enabled && std::isfinite(compressor_reduction_db)) {
        swprintf_s(text, L"%.2f dB", compressor_reduction_db);
        set_control_text(wnd, IDC_DIAG_COMPRESSOR_REDUCTION, text);
    }
    else {
        set_control_text(wnd, IDC_DIAG_COMPRESSOR_REDUCTION, L"無効");
    }

    if (three_band_master_enabled) {
        swprintf_s(
            text,
            L"低 %.1f / 中 %.1f / 高 %.1f dB",
            three_band_low_reduction_db,
            three_band_mid_reduction_db,
            three_band_high_reduction_db
        );
        set_control_text(wnd, IDC_DIAG_THREE_BAND_REDUCTION, text);
    }
    else {
        set_control_text(wnd, IDC_DIAG_THREE_BAND_REDUCTION, L"無効");
    }

    if (modern_boost_enabled && std::isfinite(clipper_reduction_db)) {
        swprintf_s(text, L"%.2f dB", clipper_reduction_db);
        set_control_text(wnd, IDC_DIAG_CLIPPER_REDUCTION, text);
    }
    else {
        set_control_text(wnd, IDC_DIAG_CLIPPER_REDUCTION, L"無効");
    }

    if (std::isfinite(limiter_reduction_db)) {
        swprintf_s(text, L"%.2f dB", limiter_reduction_db);
        set_control_text(wnd, IDC_DIAG_LIMITER_REDUCTION, text);
    }
    else {
        set_control_text(wnd, IDC_DIAG_LIMITER_REDUCTION, L"—");
    }

    swprintf_s(text, L"%.1f ms", latency_ms);
    set_control_text(wnd, IDC_DIAG_LATENCY, text);

    if (std::isfinite(true_peak_dbtp) && true_peak_dbtp > -190.0) {
        swprintf_s(text, L"%.2f dBTP", true_peak_dbtp);
        set_control_text(wnd, IDC_DIAG_TRUE_PEAK, text);
    }
    else {
        set_control_text(wnd, IDC_DIAG_TRUE_PEAK, L"未検出");
    }

    switch (peak_guard_state) {
    case 0:
        set_control_text(wnd, IDC_DIAG_PEAK_GUARD, L"無効");
        break;
    case 2:
        set_control_text(wnd, IDC_DIAG_PEAK_GUARD, L"作動中");
        break;
    default:
        set_control_text(wnd, IDC_DIAG_PEAK_GUARD, L"待機");
        break;
    }
}


struct glossary_entry {
    const wchar_t* term;
    const wchar_t* description;
};

constexpr glossary_entry kGlossaryEntries[] = {
    {
        L"診断の読み方",
        L"総合評価は「安全」「強め」「要調整」の3段階です。\r\n"
        L"\r\n"
        L"安全：最大コンプ6 dB未満、最大クリップと最大リミッターが"
        L"各1.5 dB未満で、0 dBTP超過がない状態です。\r\n"
        L"\r\n"
        L"強め：要調整には達していませんが、圧縮やピーク処理が"
        L"目立つ可能性があります。\r\n"
        L"\r\n"
        L"要調整：最大クリップまたは最大リミッターが3 dB以上、"
        L"最大TPが+0.01 dBTP超、または0 dBTP超過が1回以上です。\r\n"
        L"\r\n"
        L"要調整時はモダン強度を10～15%下げるか、"
        L"目標LUFSを1～2 LU下げてから測定をリセットしてください。"
    },
    {
        L"EBU R128",
        L"番組や音楽の聞こえる大きさをそろえるための"
        L"ラウドネス測定方式です。\r\n\r\n"
        L"このコンポーネントはR128の考え方を基礎に、"
        L"再生中の音量をリアルタイムで調整します。"
    },
    {
        L"LUFS",
        L"Loudness Units relative to Full Scaleの略です。\r\n\r\n"
        L"人が感じる音の大きさを表す単位で、値が0に近いほど"
        L"大きく聞こえます。\r\n"
        L"例：-10 LUFSは-18 LUFSより大きな音量感です。"
    },
    {
        L"LU",
        L"Loudness Unitの略です。\r\n\r\n"
        L"ラウドネスの差を表します。1 LUの差は数値上1 dBの差に"
        L"相当しますが、用途は聞こえる大きさの比較です。"
    },
    {
        L"Momentary",
        L"約400ミリ秒の短い区間で測ったラウドネスです。\r\n\r\n"
        L"瞬間的な音量変化を確認できます。ドラムや歌声の一音ごとに"
        L"大きく動くため、曲全体の音量判断には使いません。"
    },
    {
        L"Short-term",
        L"直近約3秒間のラウドネスです。\r\n\r\n"
        L"Momentaryより安定し、現在聞いている部分の音量感を"
        L"確認できます。音量一致A/B比較にも使用します。"
    },
    {
        L"Integrated",
        L"測定開始から現在までをまとめた平均ラウドネスです。\r\n\r\n"
        L"入力Int.は処理前、出力Int.は処理後を示します。"
        L"曲全体の音量を判断する中心的な値です。"
    },
    {
        L"LRA",
        L"Loudness Rangeの略です。\r\n\r\n"
        L"曲の静かな部分と大きな部分の幅、つまり抑揚の大きさを"
        L"推定します。値が大きい曲ほどダイナミックです。"
    },
    {
        L"True Peak / dBTP",
        L"デジタルサンプル間を補間して推定した実際のピークです。\r\n\r\n"
        L"dBTPはTrue Peakの単位です。通常は-1.0 dBTP前後を"
        L"上限にすると、再生機器や変換時の余裕を確保できます。"
    },
    {
        L"目標LUFS",
        L"処理後に近づけたいラウドネスです。\r\n\r\n"
        L"値を0に近づけるほど音量感は大きくなりますが、"
        L"コンプレッサーやクリッパーの処理量も増えやすくなります。"
    },
    {
        L"最大増幅 / 最大減衰",
        L"R128補正ゲインが動ける範囲です。\r\n\r\n"
        L"最大増幅は小さい音源を持ち上げる上限、最大減衰は"
        L"大きい音源を下げる上限です。安全用の制限として働きます。"
    },
    {
        L"先読み",
        L"ピークが来る少し前にリミッターを動かすための時間です。\r\n\r\n"
        L"ピークを確実に抑えやすくなりますが、その分だけDSPの"
        L"遅延が増えます。既定値は5 msです。"
    },
    {
        L"解放時間",
        L"リミッターが音量を抑えたあと、通常のゲインへ戻る速さです。\r\n\r\n"
        L"短すぎると揺れや歪みが出やすく、長すぎると抑えた状態が"
        L"長く残ります。"
    },
    {
        L"曲頭安定化",
        L"曲の冒頭を測定し、増幅判断を安定させるための待ち時間です。\r\n\r\n"
        L"この間は不用意な大幅増幅を保留します。短い曲や無音から"
        L"始まる曲では表示がしばらく「測定中・保留」になります。"
    },
    {
        L"静音保護 / 増幅保留",
        L"非常に静かな区間や無音を、大きく持ち上げないための機能です。\r\n\r\n"
        L"「増幅保留」は異常ではなく、ノイズや曲間の無音を"
        L"過剰に増幅しないための安全動作です。"
    },
    {
        L"ゲイン固定",
        L"ラウドネスが安定したあと、補正ゲインを一定に保つ機能です。\r\n\r\n"
        L"曲中で音量がふらつくのを防ぎます。安全上必要な減衰や"
        L"ピーク抑制は、固定後でも追加される場合があります。"
    },
    {
        L"コンプレッサー",
        L"大きすぎる部分だけを自動的に抑える処理です。\r\n\r\n"
        L"最大コンプは、そのトラックで最も強く抑えた量です。"
        L"大きいほど音量密度は上がりますが、抑揚が小さくなります。"
    },
    {
        L"ソフトクリッパー",
        L"鋭いピークを滑らかに丸め、音量感を確保する処理です。\r\n\r\n"
        L"最大クリップは丸めた最大量で、0 dBTP超過回数や"
        L"デジタル音割れの回数そのものではありません。"
    },
    {
        L"リミッター",
        L"最終段でTrue Peak上限を超えないように音量を抑えます。\r\n\r\n"
        L"最大リミッターが3 dB以上になる場合は、目標LUFSや"
        L"モダン強度を下げることを推奨します。"
    },
    {
        L"自動セーフティ",
        L"処理が強くなりすぎたとき、追加で最大3 dBまで"
        L"全体を安全側へ下げる機能です。\r\n\r\n"
        L"診断の「安全補正」に現在の減衰量が表示されます。"
    },
    {
        L"モダンブースト",
        L"R128補正にコンプレッサー、ソフトクリッパー、"
        L"True Peakリミッターを組み合わせる高密度モードです。\r\n\r\n"
        L"通常のノーマライズより音色やダイナミクスが変化します。"
    },
    {
        L"アダプティブ・マスター",
        L"入力ラウドネス、LRA、処理負荷を見ながら、モダン処理の"
        L"強度を自動調整するモードです。\r\n\r\n"
        L"設定したモダン強度は、自動調整の上限として働きます。"
    },
    {
        L"3バンド処理",
        L"音を低域・中域・高域に分け、それぞれを別の"
        L"コンプレッサーで制御します。\r\n\r\n"
        L"約160 Hzと約4 kHzがクロスオーバーの目安です。"
        L"固定EQではありませんが、処理量によって音色は変化します。"
    },
    {
        L"クロスオーバー",
        L"3バンド処理で低域・中域・高域を分ける境界周波数です。\r\n\r\n"
        L"このコンポーネントでは約160 Hzと約4 kHzを使用します。"
    },
    {
        L"音量一致A/B比較",
        L"処理前と処理後のShort-termラウドネスを近づけて、"
        L"音量差の影響を減らして比較します。\r\n\r\n"
        L"音色、圧縮感、低音の潰れ、高域のざらつきなどを"
        L"公平に判断しやすくする機能です。"
    },
    {
        L"完全バイパス比較",
        L"「音量一致」のチェックを外して比較ボタンを押すと、"
        L"このコンポーネントの処理をすべて外します。\r\n\r\n"
        L"音量差を含めた実際の効果を確認できます。前段の別DSPは残ります。"
    },
    {
        L"最大TP",
        L"トラック中に測定した処理後の最大True Peakです。\r\n\r\n"
        L"設定したTP上限の近くなら正常です。+0.01 dBTPを超えた場合や"
        L"0 dBTP超過が記録された場合は要調整です。"
    },
    {
        L"最大コンプ / クリップ / リミッター",
        L"各処理がそのトラックで最も強く音量を抑えた量です。\r\n\r\n"
        L"コンプ6 dB以上は「強め」の目安です。クリップまたは"
        L"リミッターが3 dB以上なら「要調整」と判定します。"
    },
    {
        L"0 dBTP超過",
        L"処理後のTrue Peakが0 dBTPを超えたイベント回数です。\r\n\r\n"
        L"通常は0回になることを想定しています。1回以上なら"
        L"True Peak設定と処理強度を見直してください。"
    },
    {
        L"CPU",
        L"このDSPの推定処理負荷です。\r\n\r\n"
        L"パソコン、サンプルレート、チャンネル数で変化します。"
        L"総合評価の「安全／強め／要調整」には使用しません。"
    },
    {
        L"異常値保護",
        L"デコーダーや前段DSPからNaN、無限値、極端な振幅が"
        L"渡された場合に、そのサンプルを安全な値へ置き換えます。\r\n\r\n"
        L"通常の有限な音声には作用しません。"
        L"作動回数は診断コピーで確認できます。"
    }
};

struct tooltip_entry {
    int control_id;
    const wchar_t* text;
};

constexpr tooltip_entry kPresetTooltips[] = {
    {
        IDC_PROFILE_STANDARD,
        L"ナチュラル -18：普段の音楽再生向け。"
        L"自然な音量感を保ってそろえます。"
    },
    {
        IDC_PROFILE_STREAMING,
        L"パワーブースト -14：小さい音源を強めに持ち上げ、"
        L"迫力のある音量感にします。"
    },
    {
        IDC_PROFILE_BROADCAST,
        L"リラックス -23：全体を控えめにそろえ、"
        L"長時間でも聴きやすくします。"
    },
    {
        IDC_PROFILE_NIGHT,
        L"ナイトセーフ -22：夜間向け。"
        L"音量とピークを低めに抑えます。"
    },
    {
        IDC_PROFILE_MODERN,
        L"モダンブースト -9：圧縮とソフトクリップで"
        L"高密度な音に近づけます。"
    },
    {
        IDC_PROFILE_ADAPTIVE,
        L"アダプティブ -10：曲のLRAと入力音量を解析し、"
        L"モダン処理の強度を自動調整します。"
    },
    {
        IDC_PROFILE_THREE_BAND,
        L"3バンド自動 -10：低・中・高域を個別に制御し、"
        L"低音の潰れと高域のざらつきを抑えます。"
    },
    {
        IDC_COMPARE_LOUDNESS_MATCH,
        L"オン：ラウドネスをそろえた公平なA/B比較。"
        L"オフ：このDSPを外す完全バイパス比較。"
    },
    {
        IDC_ORIGINAL_COMPARE,
        L"押している間だけ比較音へ切り替えます。"
        L"離すと処理音へ戻ります。"
    }
};

struct context_help_entry {
    int control_id;
    const wchar_t* title;
    const wchar_t* description;
};

constexpr context_help_entry kContextHelpEntries[] = {
    {
        IDC_TARGET_LUFS,
        L"目標LUFS",
        L"処理後に近づけたいラウドネスです。"
        L"0に近づけるほど大きくなりますが、"
        L"圧縮やピーク処理も強くなりやすくなります。"
    },
    {
        IDC_MAX_BOOST,
        L"最大増幅",
        L"小さい音源を持ち上げられる最大量です。"
        L"静かな音源や無音を過剰に増幅しないための上限です。"
    },
    {
        IDC_MAX_ATTENUATION,
        L"最大減衰",
        L"大きい音源を下げられる最大量です。"
        L"極端な設定値にならないための安全上限です。"
    },
    {
        IDC_TRUE_PEAK,
        L"TP上限",
        L"処理後のTrue Peak上限です。"
        L"一般的な開始値は-1.0 dBTPです。"
    },
    {
        IDC_LOOKAHEAD_MS,
        L"先読み",
        L"ピークの少し前からリミッターを動かす時間です。"
        L"長くするとピークを抑えやすくなりますが、遅延も増えます。"
    },
    {
        IDC_LIMITER_RELEASE_MS,
        L"解放時間",
        L"リミッターが通常状態へ戻る速さです。"
        L"短すぎると揺れ、長すぎると抑制が長く残ることがあります。"
    },
    {
        IDC_STARTUP_ANALYSIS_SECONDS,
        L"曲頭安定化",
        L"曲の冒頭を測定して増幅判断を安定させる時間です。"
        L"この間の増幅保留は正常な安全動作です。"
    },
    {
        IDC_SILENCE_GUARD_LUFS,
        L"静音しきい値",
        L"この値より静かな区間では増幅を保留します。"
        L"無音や小さなノイズの過剰増幅を防ぎます。"
    },
    {
        IDC_GAIN_LOCK_SECONDS,
        L"固定判定",
        L"補正ゲインを固定するまでの安定確認時間です。"
        L"長くすると慎重に、短くすると早く固定します。"
    },
    {
        IDC_GAIN_LOCK_TOLERANCE_LU,
        L"固定許容",
        L"ゲイン固定を判断する際に許容するラウドネス変動幅です。"
        L"小さいほど厳密な安定を求めます。"
    },
    {
        IDC_MODERN_STRENGTH,
        L"モダン強度／上限",
        L"コンプレッサーとクリッパーの処理強度です。"
        L"アダプティブモードでは自動強度の上限になります。"
    },
    {
        IDC_RESET_EACH_TRACK,
        L"曲変更時に測定をリセット",
        L"次の曲へ移った際にIntegrated、LRA、最大値などを"
        L"新しい曲用にリセットします。"
    },
    {
        IDC_ENABLE_PEAK_GUARD,
        L"True Peak＋先読み制限",
        L"先読みリミッターを使い、設定したTP上限を守ります。"
    },
    {
        IDC_ENABLE_SILENCE_GUARD,
        L"静かな区間は増幅を保留",
        L"無音や非常に静かな区間を大きく持ち上げない安全機能です。"
    },
    {
        IDC_ENABLE_GAIN_LOCK,
        L"安定後に曲中ゲイン固定",
        L"測定が安定した後の補正ゲインを固定し、"
        L"曲中の音量のふらつきを減らします。"
    },
    {
        IDC_ENABLE_MODERN_BOOST,
        L"モダンブースト",
        L"コンプレッサー、ソフトクリッパー、"
        L"True Peakリミッターを組み合わせます。"
    },
    {
        IDC_ENABLE_ADAPTIVE_MASTER,
        L"アダプティブ",
        L"曲のLRAや入力音量に応じ、モダン処理の強度を自動調整します。"
    },
    {
        IDC_ENABLE_THREE_BAND_MASTER,
        L"3バンド・マスター",
        L"低域・中域・高域を分けて個別に制御します。"
        L"固定EQではなく、帯域別の動的処理です。"
    },
    {
        IDC_DIAG_NORMALIZATION_STATE,
        L"補正",
        L"通常補正、測定中・保留、静音保護・保留など、"
        L"現在のR128補正状態を表示します。"
    },
    {
        IDC_DIAG_GAIN_LOCK,
        L"固定",
        L"ゲイン固定の待ち時間、固定値、安全減衰、"
        L"増幅抑制などの状態を表示します。"
    },
    {
        IDC_DIAG_MOMENTARY,
        L"Momentary",
        L"約400ミリ秒の瞬間的なラウドネスです。"
    },
    {
        IDC_DIAG_SHORT_TERM,
        L"Short-term",
        L"直近約3秒間のラウドネスです。"
        L"音量一致A/B比較の中心値にも使用します。"
    },
    {
        IDC_DIAG_INTEGRATED,
        L"入力Integrated",
        L"処理前の測定開始から現在までの平均ラウドネスです。"
    },
    {
        IDC_DIAG_OUTPUT_INTEGRATED,
        L"出力Integrated",
        L"処理後の測定開始から現在までの平均ラウドネスです。"
    },
    {
        IDC_DIAG_TARGET_DIFFERENCE,
        L"目標差",
        L"出力Integratedと目標LUFSの差です。"
        L"0 LUに近いほど目標へ近づいています。"
    },
    {
        IDC_DIAG_LRA,
        L"LRA",
        L"曲の静かな部分と大きな部分の幅を推定した値です。"
    },
    {
        IDC_DIAG_PROCESSING_RISK,
        L"処理状態",
        L"現在のモダン／アダプティブ処理の強さや、"
        L"A/B比較状態を表示します。"
    },
    {
        IDC_DIAG_SAFETY_REDUCTION,
        L"安全補正",
        L"処理が強くなりすぎた際に追加された安全減衰量です。"
        L"最大3 dBまで動作します。"
    },
    {
        IDC_DIAG_GAIN,
        L"総ゲイン",
        L"R128補正と安全補正を含む、現在の全体ゲインです。"
    },
    {
        IDC_DIAG_COMPRESSOR_REDUCTION,
        L"コンプ",
        L"現在のコンプレッサー減衰量です。"
    },
    {
        IDC_DIAG_CLIPPER_REDUCTION,
        L"クリップ",
        L"現在のソフトクリッパーによるピーク整形量です。"
    },
    {
        IDC_DIAG_LIMITER_REDUCTION,
        L"リミッター",
        L"現在の最終True Peakリミッター減衰量です。"
    },
    {
        IDC_DIAG_TRUE_PEAK,
        L"True Peak",
        L"現在の処理後True Peak推定値です。単位はdBTPです。"
    },
    {
        IDC_DIAG_LATENCY,
        L"遅延",
        L"先読み処理などによって発生するDSP遅延です。"
    },
    {
        IDC_DIAG_PEAK_GUARD,
        L"ピーク保護",
        L"True Peak保護が無効、待機、作動中のどれかを表示します。"
    },
    {
        IDC_DIAG_CHANNEL_LAYOUT,
        L"構成",
        L"検出したチャンネル構成と、LFEをラウドネス計算から"
        L"除外しているかを表示します。"
    },
    {
        IDC_DIAG_THREE_BAND_REDUCTION,
        L"3バンド",
        L"再生中は低域・中域・高域の現在の減衰量を表示します。"
        L"停止後は前回トラックで記録した各帯域の最大減衰量を表示します。"
    },
    {
        IDC_DIAG_MAX_TRUE_PEAK,
        L"最大TP",
        L"トラック中の処理後最大True Peakです。"
        L"+0.01 dBTPを超えた場合は要調整です。"
    },
    {
        IDC_DIAG_MAX_COMPRESSOR_REDUCTION,
        L"最大コンプ",
        L"トラック中の最大コンプレッサー減衰量です。"
        L"6 dB以上は「強め」の目安です。"
    },
    {
        IDC_DIAG_MAX_CLIPPER_REDUCTION,
        L"最大クリップ",
        L"トラック中の最大ソフトクリッパー整形量です。"
        L"3 dB以上は「要調整」です。"
    },
    {
        IDC_DIAG_MAX_LIMITER_REDUCTION,
        L"最大リミッター",
        L"トラック中の最大リミッター減衰量です。"
        L"3 dB以上は「要調整」です。"
    },
    {
        IDC_DIAG_SAMPLE_RATE,
        L"レート",
        L"再生中のサンプルレートです。例：44100 Hz、48000 Hz。"
    },
    {
        IDC_DIAG_CPU_LOAD,
        L"CPU",
        L"このDSPの推定処理負荷です。総合評価には使用しません。"
    },
    {
        IDC_DIAG_CLIP_EVENT_COUNT,
        L"0 dBTP超過",
        L"処理後のTrue Peakが0 dBTPを超えたイベント回数です。"
        L"通常は0回を想定しています。"
    },
    {
        IDC_DIAG_PROCESSING_EVALUATION,
        L"総合評価",
        L"最大TP、最大クリップ、最大リミッター、"
        L"0 dBTP超過などから「安全／強め／要調整」を表示します。"
    },
    {
        IDC_COMPARE_LOUDNESS_MATCH,
        L"音量一致",
        L"オンでは処理前後のラウドネスをそろえて比較します。"
        L"オフではこのDSPを外す完全バイパス比較です。"
    },
    {
        IDC_ORIGINAL_COMPARE,
        L"比較（押している間）",
        L"押している間だけ比較音へ切り替えます。"
        L"ボタンを離すと処理音へ戻ります。"
    },
    {
        IDC_RESET_MEASUREMENT,
        L"測定リセット",
        L"Integrated、LRA、トラック最大値などの測定を"
        L"現在位置からやり直します。"
    },
    {
        IDC_COPY_DIAGNOSTICS,
        L"診断コピー",
        L"現在の設定と診断結果をクリップボードへコピーします。"
    },
    {
        IDC_SHOW_DIAGNOSTIC_HELP,
        L"用語集",
        L"用語一覧から、このコンポーネント内での意味と"
        L"数値の読み方を確認できます。"
    }
};

struct label_help_link {
    const wchar_t* label;
    int control_id;
};

constexpr label_help_link kLabelHelpLinks[] = {
    { L"目標LUFS：", IDC_TARGET_LUFS },
    { L"最大増幅：", IDC_MAX_BOOST },
    { L"最大減衰：", IDC_MAX_ATTENUATION },
    { L"TP上限：", IDC_TRUE_PEAK },
    { L"先読み：", IDC_LOOKAHEAD_MS },
    { L"解放時間：", IDC_LIMITER_RELEASE_MS },
    { L"曲頭安定化：", IDC_STARTUP_ANALYSIS_SECONDS },
    { L"静音しきい値：", IDC_SILENCE_GUARD_LUFS },
    { L"固定判定：", IDC_GAIN_LOCK_SECONDS },
    { L"固定許容：", IDC_GAIN_LOCK_TOLERANCE_LU },
    { L"モダン強度／上限：", IDC_MODERN_STRENGTH },
    { L"補正：", IDC_DIAG_NORMALIZATION_STATE },
    { L"固定：", IDC_DIAG_GAIN_LOCK },
    { L"Momentary：", IDC_DIAG_MOMENTARY },
    { L"Short-term：", IDC_DIAG_SHORT_TERM },
    { L"入力Int.：", IDC_DIAG_INTEGRATED },
    { L"出力Int.：", IDC_DIAG_OUTPUT_INTEGRATED },
    { L"目標差：", IDC_DIAG_TARGET_DIFFERENCE },
    { L"LRA：", IDC_DIAG_LRA },
    { L"処理状態：", IDC_DIAG_PROCESSING_RISK },
    { L"安全補正：", IDC_DIAG_SAFETY_REDUCTION },
    { L"総ゲイン：", IDC_DIAG_GAIN },
    { L"コンプ：", IDC_DIAG_COMPRESSOR_REDUCTION },
    { L"クリップ：", IDC_DIAG_CLIPPER_REDUCTION },
    { L"リミッター：", IDC_DIAG_LIMITER_REDUCTION },
    { L"True Peak：", IDC_DIAG_TRUE_PEAK },
    { L"遅延：", IDC_DIAG_LATENCY },
    { L"ピーク保護：", IDC_DIAG_PEAK_GUARD },
    { L"構成：", IDC_DIAG_CHANNEL_LAYOUT },
    { L"3バンド：", IDC_DIAG_THREE_BAND_REDUCTION },
    { L"最大TP：", IDC_DIAG_MAX_TRUE_PEAK },
    { L"最大コンプ：", IDC_DIAG_MAX_COMPRESSOR_REDUCTION },
    { L"最大クリップ：", IDC_DIAG_MAX_CLIPPER_REDUCTION },
    { L"最大リミッター：", IDC_DIAG_MAX_LIMITER_REDUCTION },
    { L"レート：", IDC_DIAG_SAMPLE_RATE },
    { L"CPU：", IDC_DIAG_CPU_LOAD },
    { L"0 dBTP超過：", IDC_DIAG_CLIP_EVENT_COUNT },
    { L"総合評価：", IDC_DIAG_PROCESSING_EVALUATION }
};

const context_help_entry* find_context_help(int control_id) {
    for (const auto& entry : kContextHelpEntries) {
        if (entry.control_id == control_id) {
            return &entry;
        }
    }

    return nullptr;
}

int help_control_id_from_label(HWND item) {
    if (item == nullptr) {
        return 0;
    }

    wchar_t text[128] = {};
    GetWindowTextW(item, text, static_cast<int>(std::size(text)));

    for (const auto& link : kLabelHelpLinks) {
        if (wcscmp(text, link.label) == 0) {
            return link.control_id;
        }
    }

    return 0;
}

void show_context_help(HWND wnd, HWND item) {
    int control_id = item != nullptr ? GetDlgCtrlID(item) : 0;

    if (control_id <= 0) {
        control_id = help_control_id_from_label(item);
    }

    const context_help_entry* entry = find_context_help(control_id);

    if (entry == nullptr) {
        const int linked_control_id =
            help_control_id_from_label(item);

        if (linked_control_id > 0) {
            entry = find_context_help(linked_control_id);
        }
    }

    if (entry != nullptr) {
        MessageBoxW(
            wnd,
            entry->description,
            entry->title,
            MB_OK | MB_ICONINFORMATION
        );
        return;
    }

    MessageBoxW(
        wnd,
        L"説明を確認したい設定欄、診断値、チェック項目、"
        L"またはプリセットをクリックしてください。\r\n\r\n"
        L"詳しい用語一覧は画面下部の「用語集」から開けます。",
        L"項目ヘルプ",
        MB_OK | MB_ICONINFORMATION
    );
}

void add_tooltip(
    HWND tooltip,
    HWND dialog,
    int control_id,
    const wchar_t* text
) {
    if (tooltip == nullptr || text == nullptr) {
        return;
    }

    HWND control = GetDlgItem(dialog, control_id);

    if (control == nullptr) {
        return;
    }

    TTTOOLINFOW tool = {};
    tool.cbSize = sizeof(tool);
    tool.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    tool.hwnd = dialog;
    tool.uId = reinterpret_cast<UINT_PTR>(control);
    tool.lpszText = const_cast<LPWSTR>(text);

    SendMessageW(
        tooltip,
        TTM_ADDTOOLW,
        0,
        reinterpret_cast<LPARAM>(&tool)
    );
}

HWND create_help_tooltips(HWND dialog) {
    INITCOMMONCONTROLSEX controls = {};
    controls.dwSize = sizeof(controls);
    controls.dwICC = ICC_WIN95_CLASSES | ICC_TAB_CLASSES;
    InitCommonControlsEx(&controls);

    HWND tooltip = CreateWindowExW(
        WS_EX_TOPMOST,
        TOOLTIPS_CLASSW,
        nullptr,
        WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON | TTS_NOPREFIX,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        dialog,
        nullptr,
        core_api::get_my_instance(),
        nullptr
    );

    if (tooltip == nullptr) {
        return nullptr;
    }

    SetWindowPos(
        tooltip,
        HWND_TOPMOST,
        0,
        0,
        0,
        0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE
    );

    SendMessageW(tooltip, TTM_SETMAXTIPWIDTH, 0, 420);
    SendMessageW(tooltip, TTM_SETDELAYTIME, TTDT_INITIAL, 450);
    SendMessageW(tooltip, TTM_SETDELAYTIME, TTDT_AUTOPOP, 12000);

    for (const auto& entry : kPresetTooltips) {
        add_tooltip(
            tooltip,
            dialog,
            entry.control_id,
            entry.text
        );
    }

    for (const auto& entry : kContextHelpEntries) {
        bool has_custom_tooltip = false;

        for (const auto& custom : kPresetTooltips) {
            if (custom.control_id == entry.control_id) {
                has_custom_tooltip = true;
                break;
            }
        }

        if (!has_custom_tooltip) {
            add_tooltip(
                tooltip,
                dialog,
                entry.control_id,
                entry.description
            );
        }
    }

    return tooltip;
}

void update_glossary_description(HWND wnd) {
    const LRESULT selection = SendDlgItemMessageW(
        wnd,
        IDC_GLOSSARY_LIST,
        LB_GETCURSEL,
        0,
        0
    );

    if (selection == LB_ERR ||
        static_cast<t_size>(selection) >= std::size(kGlossaryEntries)) {
        SetDlgItemTextW(
            wnd,
            IDC_GLOSSARY_DESCRIPTION,
            L"左の用語を選択してください。"
        );
        return;
    }

    SetDlgItemTextW(
        wnd,
        IDC_GLOSSARY_DESCRIPTION,
        kGlossaryEntries[static_cast<t_size>(selection)].description
    );
}

INT_PTR CALLBACK glossary_dialog_proc(
    HWND wnd,
    UINT message,
    WPARAM wp,
    LPARAM
) {
    auto* dark_mode = reinterpret_cast<fb2k::CCoreDarkModeHooks*>(
        GetWindowLongPtrW(wnd, GWLP_USERDATA)
    );

    switch (message) {
    case WM_INITDIALOG:
        dark_mode = new fb2k::CCoreDarkModeHooks();
        SetWindowLongPtrW(
            wnd,
            GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(dark_mode)
        );
        dark_mode->AddDialogWithControls(wnd);

        for (const auto& entry : kGlossaryEntries) {
            SendDlgItemMessageW(
                wnd,
                IDC_GLOSSARY_LIST,
                LB_ADDSTRING,
                0,
                reinterpret_cast<LPARAM>(entry.term)
            );
        }

        SendDlgItemMessageW(
            wnd,
            IDC_GLOSSARY_LIST,
            LB_SETCURSEL,
            0,
            0
        );
        update_glossary_description(wnd);
        return TRUE;

    case WM_COMMAND:
        if (LOWORD(wp) == IDC_GLOSSARY_LIST &&
            HIWORD(wp) == LBN_SELCHANGE) {
            update_glossary_description(wnd);
            return TRUE;
        }

        if (LOWORD(wp) == IDOK || LOWORD(wp) == IDCANCEL) {
            EndDialog(wnd, LOWORD(wp));
            return TRUE;
        }
        break;

    case WM_NCDESTROY:
        SetWindowLongPtrW(wnd, GWLP_USERDATA, 0);
        delete dark_mode;
        return FALSE;
    }

    return FALSE;
}

struct dialog_context {
    r128_settings value;
    dsp_preset_edit_callback* callback = nullptr;
    HWND tooltip = nullptr;
    fb2k::CCoreDarkModeHooks dark_mode;

    // Direct main-menu launch only.
    // Modal DSP Manager dialogs leave these at their defaults.
    bool modeless = false;
    HWND* tracked_window = nullptr;
    void* cleanup_state = nullptr;
    void (*cleanup)(dialog_context*) = nullptr;
};

void close_config_dialog(
    HWND wnd,
    INT_PTR result,
    dialog_context* context
) {
    if (context != nullptr && context->modeless) {
        DestroyWindow(wnd);
    }
    else {
        EndDialog(wnd, result);
    }
}

bool read_settings_from_dialog(
    HWND wnd,
    r128_settings& value
) {
    if (!read_float(
            wnd, IDC_TARGET_LUFS, -36.0f, -5.0f,
            value.target_lufs, L"目標ラウドネス")) {
        return false;
    }
    if (!read_float(
            wnd, IDC_MAX_BOOST, 0.0f, 24.0f,
            value.max_boost_db, L"最大増幅量")) {
        return false;
    }
    if (!read_float(
            wnd, IDC_MAX_ATTENUATION, 0.0f, 36.0f,
            value.max_attenuation_db, L"最大減衰量")) {
        return false;
    }
    if (!read_float(
            wnd, IDC_TRUE_PEAK, -12.0f, 0.0f,
            value.true_peak_limit_dbtp, L"True Peak上限")) {
        return false;
    }
    if (!read_float(
            wnd, IDC_LOOKAHEAD_MS, 0.0f, 20.0f,
            value.lookahead_ms, L"先読み時間")) {
        return false;
    }
    if (!read_float(
            wnd, IDC_LIMITER_RELEASE_MS, 20.0f, 1000.0f,
            value.limiter_release_ms, L"リミッター解放時間")) {
        return false;
    }
    if (!read_float(
            wnd, IDC_STARTUP_ANALYSIS_SECONDS, 0.0f, 15.0f,
            value.startup_analysis_seconds,
            L"曲頭の測定安定化時間")) {
        return false;
    }
    if (!read_float(
            wnd, IDC_SILENCE_GUARD_LUFS, -70.0f, -20.0f,
            value.silence_guard_lufs,
            L"静音保護しきい値")) {
        return false;
    }
    if (!read_float(
            wnd, IDC_GAIN_LOCK_SECONDS, 0.0f, 60.0f,
            value.gain_lock_seconds,
            L"補正ゲイン固定判定時間")) {
        return false;
    }
    if (!read_float(
            wnd, IDC_GAIN_LOCK_TOLERANCE_LU, 0.1f, 3.0f,
            value.gain_lock_tolerance_lu,
            L"固定許容変動")) {
        return false;
    }
    if (!read_float(
            wnd, IDC_MODERN_STRENGTH, 0.0f, 100.0f,
            value.modern_strength_percent,
            L"モダン強度")) {
        return false;
    }

    value.reset_each_track =
        IsDlgButtonChecked(wnd, IDC_RESET_EACH_TRACK) == BST_CHECKED;
    value.enable_peak_guard =
        IsDlgButtonChecked(wnd, IDC_ENABLE_PEAK_GUARD) == BST_CHECKED;
    value.enable_silence_guard =
        IsDlgButtonChecked(
            wnd,
            IDC_ENABLE_SILENCE_GUARD
        ) == BST_CHECKED;
    value.enable_gain_lock =
        IsDlgButtonChecked(
            wnd,
            IDC_ENABLE_GAIN_LOCK
        ) == BST_CHECKED;
    value.enable_modern_boost =
        IsDlgButtonChecked(
            wnd,
            IDC_ENABLE_MODERN_BOOST
        ) == BST_CHECKED;
    value.enable_adaptive_master =
        IsDlgButtonChecked(
            wnd,
            IDC_ENABLE_ADAPTIVE_MASTER
        ) == BST_CHECKED;
    value.enable_three_band_master =
        IsDlgButtonChecked(
            wnd,
            IDC_ENABLE_THREE_BAND_MASTER
        ) == BST_CHECKED;

    if (value.enable_three_band_master) {
        value.enable_adaptive_master = true;
        CheckDlgButton(wnd, IDC_ENABLE_ADAPTIVE_MASTER, BST_CHECKED);
    }

    if (value.enable_adaptive_master) {
        value.enable_modern_boost = true;
        CheckDlgButton(wnd, IDC_ENABLE_MODERN_BOOST, BST_CHECKED);
    }

    return true;
}

bool apply_dialog_settings(
    HWND wnd,
    dialog_context* context
) {
    if (context == nullptr) {
        return false;
    }

    r128_settings new_value = context->value;

    if (!read_settings_from_dialog(wnd, new_value)) {
        return false;
    }

    context->value = new_value;

    if (context->callback != nullptr) {
        dsp_preset_impl new_preset;
        make_preset(context->value, new_preset);
        context->callback->on_preset_changed(new_preset);
    }

    update_profile_indicator(
        wnd,
        context->value,
        false
    );
    set_control_text(
        wnd,
        IDC_APPLY_STATUS,
        L"設定を適用しました"
    );

    return true;
}

INT_PTR CALLBACK config_dialog_proc(HWND wnd, UINT message, WPARAM wp, LPARAM lp) {
    auto* context = reinterpret_cast<dialog_context*>(
        GetWindowLongPtrW(wnd, GWLP_USERDATA)
    );

    switch (message) {
    case WM_INITDIALOG:
        context = reinterpret_cast<dialog_context*>(lp);
        SetWindowLongPtrW(
            wnd,
            GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(context)
        );

        if (context != nullptr && context->modeless) {
            modeless_dialog_manager::g_add(wnd);

            if (context->tracked_window != nullptr) {
                *context->tracked_window = wnd;
            }
        }

        if (context != nullptr) {
            context->dark_mode.AddDialogWithControls(wnd);
        }

        g_original_compare_request.store(0, std::memory_order_relaxed);
        setup_config_tabs(wnd);
        fit_dialog_to_monitor_work_area(wnd);

        if (context != nullptr) {
            context->tooltip = create_help_tooltips(wnd);

            if (context->tooltip != nullptr) {
                SetWindowTheme(
                    context->tooltip,
                    context->dark_mode
                        ? L"DarkMode_Explorer"
                        : nullptr,
                    nullptr
                );
            }
        }
        CheckDlgButton(
            wnd,
            IDC_COMPARE_LOUDNESS_MATCH,
            BST_CHECKED
        );
        settings_to_dialog(wnd, context->value);
        if (context != nullptr) {
            update_profile_indicator(
                wnd,
                context->value,
                false
            );
        }
        set_control_text(
            wnd,
            IDC_APPLY_STATUS,
            L"変更後は［適用］または［OK］"
        );
        refresh_diagnostic_controls(wnd);
        SetTimer(
            wnd,
            kDiagnosticsTimerId,
            kDiagnosticsRefreshMilliseconds,
            nullptr
        );
        return TRUE;

    case WM_NOTIFY:
        if (lp != 0) {
            const auto* header =
                reinterpret_cast<const NMHDR*>(lp);

            if (header->idFrom == IDC_CONFIG_TABS &&
                header->code == TCN_SELCHANGE) {
                const int selected_page =
                    TabCtrl_GetCurSel(
                        GetDlgItem(wnd, IDC_CONFIG_TABS)
                    );

                update_config_tab_page(
                    wnd,
                    selected_page
                );
                return TRUE;
            }
        }
        break;

    case WM_DPICHANGED:
        if (lp != 0) {
            const auto* suggested =
                reinterpret_cast<const RECT*>(lp);

            SetWindowPos(
                wnd,
                nullptr,
                suggested->left,
                suggested->top,
                suggested->right - suggested->left,
                suggested->bottom - suggested->top,
                SWP_NOZORDER | SWP_NOACTIVATE
            );
        }

        fit_dialog_to_monitor_work_area(wnd);
        return TRUE;

    case WM_HELP:
        if (lp != 0) {
            const auto* help = reinterpret_cast<const HELPINFO*>(lp);

            if (help->iContextType == HELPINFO_WINDOW) {
                show_context_help(
                    wnd,
                    reinterpret_cast<HWND>(help->hItemHandle)
                );
                return TRUE;
            }
        }

        show_context_help(wnd, nullptr);
        return TRUE;

    case WM_TIMER:
        if (wp == kDiagnosticsTimerId) {
            refresh_diagnostic_controls(wnd);
            return TRUE;
        }
        break;

    case WM_CLOSE:
        close_config_dialog(wnd, IDCANCEL, context);
        return TRUE;

    case WM_DESTROY:
        KillTimer(wnd, kDiagnosticsTimerId);
        g_original_compare_request.store(0, std::memory_order_relaxed);

        if (context != nullptr && context->tooltip != nullptr) {
            DestroyWindow(context->tooltip);
            context->tooltip = nullptr;
        }

        if (context != nullptr && context->modeless) {
            modeless_dialog_manager::g_remove(wnd);
        }

        return TRUE;

    case WM_NCDESTROY:
        if (context != nullptr && context->modeless) {
            SetWindowLongPtrW(wnd, GWLP_USERDATA, 0);

            if (context->tracked_window != nullptr) {
                *context->tracked_window = nullptr;
            }

            const auto cleanup = context->cleanup;

            if (cleanup != nullptr) {
                cleanup(context);
            }
        }

        return FALSE;

    case WM_COMMAND:
        if (LOWORD(wp) == IDC_ORIGINAL_COMPARE) {
            const WORD notification = HIWORD(wp);

            if (notification == BN_HILITE) {
                const bool loudness_matched =
                    IsDlgButtonChecked(
                        wnd,
                        IDC_COMPARE_LOUDNESS_MATCH
                    ) == BST_CHECKED;

                g_original_compare_request.store(
                    loudness_matched ? 2 : 1,
                    std::memory_order_relaxed
                );
                set_control_text(
                    wnd,
                    IDC_APPLY_STATUS,
                    loudness_matched
                        ? L"A/B音量一致中"
                        : L"原音比較中"
                );
                return TRUE;
            }

            if (notification == BN_UNHILITE ||
                notification == BN_CLICKED) {
                g_original_compare_request.store(
                    0,
                    std::memory_order_relaxed
                );
                set_control_text(
                    wnd,
                    IDC_APPLY_STATUS,
                    L"処理音へ戻りました"
                );
                return TRUE;
            }
        }

        switch (LOWORD(wp)) {
        case IDC_DEFAULTS:
        case IDC_PROFILE_STANDARD:
            if (context != nullptr) {
                context->value = standard_profile();
                settings_to_dialog(wnd, context->value);
                update_profile_indicator(
                    wnd,
                    context->value,
                    true
                );
                set_control_text(
                    wnd,
                    IDC_APPLY_STATUS,
                    L"未適用の変更があります"
                );
            }
            return TRUE;

        case IDC_PROFILE_STREAMING:
            if (context != nullptr) {
                context->value = streaming_profile();
                settings_to_dialog(wnd, context->value);
                update_profile_indicator(
                    wnd,
                    context->value,
                    true
                );
                set_control_text(
                    wnd,
                    IDC_APPLY_STATUS,
                    L"未適用の変更があります"
                );
            }
            return TRUE;

        case IDC_PROFILE_BROADCAST:
            if (context != nullptr) {
                context->value = broadcast_profile();
                settings_to_dialog(wnd, context->value);
                update_profile_indicator(
                    wnd,
                    context->value,
                    true
                );
                set_control_text(
                    wnd,
                    IDC_APPLY_STATUS,
                    L"未適用の変更があります"
                );
            }
            return TRUE;

        case IDC_PROFILE_NIGHT:
            if (context != nullptr) {
                context->value = night_profile();
                settings_to_dialog(wnd, context->value);
                update_profile_indicator(
                    wnd,
                    context->value,
                    true
                );
                set_control_text(
                    wnd,
                    IDC_APPLY_STATUS,
                    L"未適用の変更があります"
                );
            }
            return TRUE;

        case IDC_PROFILE_MODERN:
            if (context != nullptr) {
                context->value = modern_profile();
                settings_to_dialog(wnd, context->value);
                update_profile_indicator(
                    wnd,
                    context->value,
                    true
                );
                set_control_text(
                    wnd,
                    IDC_APPLY_STATUS,
                    L"未適用の変更があります"
                );
            }
            return TRUE;

        case IDC_PROFILE_ADAPTIVE:
            if (context != nullptr) {
                context->value = adaptive_profile();
                settings_to_dialog(wnd, context->value);
                update_profile_indicator(
                    wnd,
                    context->value,
                    true
                );
                set_control_text(
                    wnd,
                    IDC_APPLY_STATUS,
                    L"未適用の変更があります"
                );
            }
            return TRUE;

        case IDC_PROFILE_THREE_BAND:
            if (context != nullptr) {
                context->value = three_band_profile();
                settings_to_dialog(wnd, context->value);
                update_profile_indicator(
                    wnd,
                    context->value,
                    true
                );
                set_control_text(
                    wnd,
                    IDC_APPLY_STATUS,
                    L"未適用の変更があります"
                );
            }
            return TRUE;

        case IDC_RESET_MEASUREMENT:
            g_measurement_reset_request.fetch_add(
                1,
                std::memory_order_relaxed
            );
            set_control_text(
                wnd,
                IDC_DIAG_NORMALIZATION_STATE,
                L"測定リセットを要求しました"
            );
            return TRUE;

        case IDC_COPY_DIAGNOSTICS:
            if (copy_unicode_text_to_clipboard(
                    wnd,
                    build_diagnostic_report())) {
                MessageBoxW(
                    wnd,
                    L"診断結果をクリップボードへコピーしました。",
                    L"診断結果のコピー",
                    MB_OK | MB_ICONINFORMATION
                );
            }
            else {
                MessageBoxW(
                    wnd,
                    L"クリップボードへコピーできませんでした。",
                    L"診断結果のコピー",
                    MB_OK | MB_ICONWARNING
                );
            }
            return TRUE;


        case IDC_SHOW_DIAGNOSTIC_HELP:
            DialogBoxParamW(
                core_api::get_my_instance(),
                MAKEINTRESOURCEW(IDD_R128_GLOSSARY),
                wnd,
                glossary_dialog_proc,
                0
            );
            return TRUE;

        case IDC_SHOW_LICENSE:
            MessageBoxW(
                wnd,
                L"R128 リアルタイム音量ノーマライザー 1.5.0\n"
                L"\n"
                L"作者：Maximum\n"
                L"Copyright (c) 2026 Maximum\n"
                L"ライセンス：MIT License\n"
                L"\n"
                L"先行作品への謝辞：\n"
                L"EBU R128 Normalizer by mudlord\n"
                L"本実装は独立して作成された非公式コンポーネントです。\n"
                L"\n"
                L"foobar2000 SDKを使用してビルドしています。\n"
                L"mudlord氏およびfoobar2000との提携・承認関係はありません。\n"
                L"\n"
                L"全文はパッケージ内のlicense.txtと\n"
                L"THIRD-PARTY-NOTICES.txtをご覧ください。",
                L"ライセンスとクレジット",
                MB_OK | MB_ICONINFORMATION
            );
            return TRUE;

        case IDC_APPLY_SETTINGS:
            apply_dialog_settings(wnd, context);
            return TRUE;

        case IDOK:
            if (apply_dialog_settings(wnd, context)) {
                close_config_dialog(wnd, IDOK, context);
            }
            return TRUE;

        case IDCANCEL:
            close_config_dialog(wnd, IDCANCEL, context);
            return TRUE;
        }
        break;
    }

    return FALSE;
}

struct biquad_filter {
    double b0 = 1.0;
    double b1 = 0.0;
    double b2 = 0.0;
    double a1 = 0.0;
    double a2 = 0.0;

    double z1 = 0.0;
    double z2 = 0.0;

    void reset() {
        z1 = 0.0;
        z2 = 0.0;
    }

    double process(double input) {
        const double output = b0 * input + z1;
        z1 = b1 * input - a1 * output + z2;
        z2 = b2 * input - a2 * output;
        return output;
    }
};

biquad_filter make_high_shelf(
    double sample_rate,
    double frequency,
    double gain_db,
    double q
) {
    biquad_filter filter;

    const double k = std::tan(kPi * frequency / sample_rate);
    const double vh = std::pow(10.0, gain_db / 20.0);
    const double vb = std::pow(vh, 0.4996667741545416);
    const double a0 = 1.0 + k / q + k * k;

    filter.b0 = (vh + vb * k / q + k * k) / a0;
    filter.b1 = 2.0 * (k * k - vh) / a0;
    filter.b2 = (vh - vb * k / q + k * k) / a0;
    filter.a1 = 2.0 * (k * k - 1.0) / a0;
    filter.a2 = (1.0 - k / q + k * k) / a0;

    return filter;
}

biquad_filter make_high_pass(
    double sample_rate,
    double frequency,
    double q
) {
    biquad_filter filter;

    const double k = std::tan(kPi * frequency / sample_rate);
    const double a0 = 1.0 + k / q + k * k;

    filter.b0 = 1.0 / a0;
    filter.b1 = -2.0 / a0;
    filter.b2 = 1.0 / a0;
    filter.a1 = 2.0 * (k * k - 1.0) / a0;
    filter.a2 = (1.0 - k / q + k * k) / a0;

    return filter;
}

struct channel_filter_state {
    biquad_filter pre_filter;
    biquad_filter rlb_filter;
};

struct three_band_split_state {
    double low_state = 0.0;
    double high_lowpass_state = 0.0;

    void reset() {
        low_state = 0.0;
        high_lowpass_state = 0.0;
    }
};

struct peak_queue_node {
    unsigned long long frame_id = 0;
    double peak = 0.0;
};

double sinc(double value) {
    if (std::fabs(value) < 1.0e-12) {
        return 1.0;
    }

    const double argument = kPi * value;
    return std::sin(argument) / argument;
}

} // namespace

class dsp_r128_normalizer : public dsp_impl_base {
public:
    explicit dsp_r128_normalizer(const dsp_preset& preset)
        : m_settings(parse_preset(preset)) {
        m_last_measurement_reset_request =
            g_measurement_reset_request.load(
                std::memory_order_relaxed
            );
    }

    static GUID g_get_guid() {
        return guid_r128_normalizer;
    }

    static void g_get_name(pfc::string_base& out) {
        out = "R128 \xE9\x9F\xB3\xE9\x87\x8F\xE3\x83\x8E\xE3\x83\xBC\xE3\x83\x9E\xE3\x83\xA9\xE3\x82\xA4\xE3\x82\xB6\xE3\x83\xBC";
    }

    static bool g_get_default_preset(dsp_preset& out) {
        make_preset(default_settings(), out);
        return true;
    }

    static bool g_have_config_popup() {
        return true;
    }

    static void g_show_config_popup(
        const dsp_preset& preset,
        HWND parent,
        dsp_preset_edit_callback& callback
    ) {
        dialog_context context;
        context.value = parse_preset(preset);
        context.callback = &callback;

        DialogBoxParamW(
            core_api::get_my_instance(),
            MAKEINTRESOURCEW(IDD_R128_CONFIG),
            parent,
            config_dialog_proc,
            reinterpret_cast<LPARAM>(&context)
        );
    }

    bool on_chunk(audio_chunk* chunk, abort_callback& abort) override {
        (void)abort;

        if (chunk == nullptr) {
            return true;
        }

        LARGE_INTEGER processing_start = {};
        QueryPerformanceCounter(&processing_start);

        const unsigned sample_rate = chunk->get_srate();
        const unsigned channels = chunk->get_channels();
        const unsigned channel_mask = chunk->get_channel_config();
        const t_size sample_count = chunk->get_sample_count();

        if (sample_rate == 0 || channels == 0 || sample_count == 0) {
            return true;
        }

        if (m_sample_rate != 0 &&
            (sample_rate != m_sample_rate ||
             channels != m_channels ||
             channel_mask != m_channel_mask)) {
            drain_delay_to_inserted_chunk();
        }

        if (sample_rate != m_sample_rate ||
            channels != m_channels ||
            channel_mask != m_channel_mask) {
            configure_stream(sample_rate, channels, channel_mask);
        }

        const unsigned long long reset_request =
            g_measurement_reset_request.load(
                std::memory_order_relaxed
            );

        if (reset_request != m_last_measurement_reset_request) {
            reset_measurement_state_only();
            m_last_measurement_reset_request = reset_request;
        }

        const audio_sample* source_data = chunk->get_data();
        if (source_data == nullptr) {
            return true;
        }

        const t_size input_value_count =
            sample_count * static_cast<t_size>(channels);

        std::vector<audio_sample> input(
            source_data,
            source_data + input_value_count
        );

        for (audio_sample& sample : input) {
            const double value = static_cast<double>(sample);

            if (!std::isfinite(value)) {
                sample = static_cast<audio_sample>(0.0);
                ++m_recovered_sample_count;
            }
            else if (std::fabs(value) > kSafeAudioMagnitudeLimit) {
                sample = static_cast<audio_sample>(
                    clamp_value(
                        value,
                        -kSafeAudioMagnitudeLimit,
                        kSafeAudioMagnitudeLimit
                    )
                );
                ++m_recovered_sample_count;
            }
        }

        std::vector<audio_sample> output;
        output.reserve(input_value_count);

        g_diagnostic_stream_active.store(1, std::memory_order_relaxed);
        g_diagnostic_peak_guard_state.store(
            m_settings.enable_peak_guard ? 1 : 0,
            std::memory_order_relaxed
        );

        const int audition_compare_mode =
            g_original_compare_request.load(
                std::memory_order_relaxed
            );
        const bool audition_bypass = audition_compare_mode != 0;
        update_compare_request_state(audition_compare_mode);

        double chunk_true_peak = 0.0;
        double last_applied_gain_db = m_current_gain_db;
        m_modern_clipper_reduction_db = 0.0;
        m_modern_frame.resize(channels);

        for (t_size frame = 0; frame < sample_count; ++frame) {
            const t_size frame_offset =
                frame * static_cast<t_size>(channels);

            double frame_energy = 0.0;
            double frame_true_peak = 0.0;
            double bypass_frame_true_peak = 0.0;
            double frame_clipper_reduction_db = 0.0;

            for (unsigned channel = 0; channel < channels; ++channel) {
                const double input_value = static_cast<double>(
                    input[frame_offset + channel]
                );

                double filtered =
                    m_filters[channel].pre_filter.process(input_value);
                filtered =
                    m_filters[channel].rlb_filter.process(filtered);

                frame_energy +=
                    m_channel_energy_weights[channel] *
                    filtered * filtered;

                if (audition_bypass) {
                    const double compare_peak =
                        push_and_measure_compare_true_peak(
                            channel,
                            input_value
                        );
                    bypass_frame_true_peak = std::max(
                        bypass_frame_true_peak,
                        compare_peak
                    );
                }

                if (!m_settings.enable_modern_boost) {
                    const double processed_peak =
                        push_and_measure_true_peak(
                            channel,
                            input_value
                        );
                    frame_true_peak = std::max(
                        frame_true_peak,
                        processed_peak
                    );
                }
            }

            push_energy(frame_energy);
            ++m_processed_frames;

            ++m_frames_since_block_update;
            if (m_energy_count >= m_block_window_frames &&
                m_frames_since_block_update >= m_block_hop_frames) {
                m_frames_since_block_update = 0;
                add_loudness_block_and_update_gain();
            }

            // Continue the processed branch while A/B comparison is held.
            // This keeps compressor, adaptive and clipper states continuous,
            // so returning to the processed sound does not restart them.
            update_adaptive_master(false);

            const audio_sample* processed_frame =
                input.data() + frame_offset;

            if (m_settings.enable_modern_boost) {
                frame_true_peak = 0.0;
                smooth_gain_toward(m_target_gain_db);

                const double normalization_gain =
                    db_to_linear(
                        m_current_gain_db +
                        m_safety_reduction_db
                    );

                if (m_settings.enable_three_band_master) {
                    double low_detector_peak = 0.0;
                    double mid_detector_peak = 0.0;
                    double high_detector_peak = 0.0;

                    for (unsigned channel = 0;
                         channel < channels;
                         ++channel) {
                        const double normalized =
                            static_cast<double>(
                                input[frame_offset + channel]
                            ) * normalization_gain;

                        double low = 0.0;
                        double mid = 0.0;
                        double high = 0.0;

                        split_three_band(
                            channel,
                            normalized,
                            low,
                            mid,
                            high
                        );

                        m_three_band_low_frame[channel] = low;
                        m_three_band_mid_frame[channel] = mid;
                        m_three_band_high_frame[channel] = high;

                        low_detector_peak = std::max(
                            low_detector_peak,
                            std::fabs(low)
                        );
                        mid_detector_peak = std::max(
                            mid_detector_peak,
                            std::fabs(mid)
                        );
                        high_detector_peak = std::max(
                            high_detector_peak,
                            std::fabs(high)
                        );
                    }

                    update_three_band_compressors(
                        low_detector_peak,
                        mid_detector_peak,
                        high_detector_peak
                    );

                    const double low_gain =
                        db_to_linear(m_three_band_low_gain_db);
                    const double mid_gain =
                        db_to_linear(m_three_band_mid_gain_db);
                    const double high_gain =
                        db_to_linear(m_three_band_high_gain_db);

                    for (unsigned channel = 0;
                         channel < channels;
                         ++channel) {
                        m_modern_frame[channel] =
                            static_cast<audio_sample>(
                                m_three_band_low_frame[channel] *
                                    low_gain +
                                m_three_band_mid_frame[channel] *
                                    mid_gain +
                                m_three_band_high_frame[channel] *
                                    high_gain
                            );
                    }
                }
                else {
                    double detector_peak = 0.0;

                    for (unsigned channel = 0;
                         channel < channels;
                         ++channel) {
                        const double normalized =
                            static_cast<double>(
                                input[frame_offset + channel]
                            ) * normalization_gain;

                        detector_peak = std::max(
                            detector_peak,
                            std::fabs(normalized)
                        );
                    }

                    update_modern_compressor(detector_peak);

                    const double compressor_gain =
                        db_to_linear(m_modern_compressor_gain_db);

                    for (unsigned channel = 0;
                         channel < channels;
                         ++channel) {
                        m_modern_frame[channel] =
                            static_cast<audio_sample>(
                                static_cast<double>(
                                    input[frame_offset + channel]
                                ) * normalization_gain * compressor_gain
                            );
                    }
                }

                for (unsigned channel = 0; channel < channels; ++channel) {
                    const double preclip =
                        static_cast<double>(m_modern_frame[channel]);

                    double channel_clipper_reduction_db = 0.0;
                    const double processed =
                        apply_modern_soft_clipper_4x(
                            channel,
                            preclip,
                            channel_clipper_reduction_db
                        );

                    m_modern_frame[channel] =
                        static_cast<audio_sample>(processed);

                    frame_clipper_reduction_db = std::max(
                        frame_clipper_reduction_db,
                        channel_clipper_reduction_db
                    );
                    m_modern_clipper_reduction_db = std::max(
                        m_modern_clipper_reduction_db,
                        channel_clipper_reduction_db
                    );

                    frame_true_peak = std::max(
                        frame_true_peak,
                        std::fabs(processed)
                    );

                    const double interpolated_peak =
                        push_and_measure_true_peak(
                            channel,
                            processed
                        );

                    frame_true_peak = std::max(
                        frame_true_peak,
                        interpolated_peak
                    );
                }

                processed_frame = m_modern_frame.data();
            }

            if (audition_bypass) {
                push_delay_frame(
                    input.data() + frame_offset,
                    bypass_frame_true_peak,
                    audition_compare_mode,
                    audition_compare_mode == 2
                        ? m_compare_match_gain_db
                        : 0.0
                );
                chunk_true_peak = std::max(
                    chunk_true_peak,
                    bypass_frame_true_peak
                );
            }
            else {
                push_delay_frame(
                    processed_frame,
                    frame_true_peak,
                    0,
                    0.0
                );
                chunk_true_peak = std::max(
                    chunk_true_peak,
                    frame_true_peak
                );
            }

            if (delay_frame_count() > m_lookahead_frames) {
                emit_one_delayed_frame(output, last_applied_gain_db);
            }

            update_auto_safety(
                frame_clipper_reduction_db,
                false
            );
            update_processing_metrics();
        }

        update_cpu_load(
            processing_start,
            sample_count,
            sample_rate
        );
        publish_diagnostics(chunk_true_peak, last_applied_gain_db);

        if (output.empty()) {
            return false;
        }

        const t_size output_frames =
            output.size() / static_cast<t_size>(channels);

        chunk->set_data_size(output.size());
        audio_sample* output_data = chunk->get_data();
        std::copy(output.begin(), output.end(), output_data);
        chunk->set_srate(sample_rate);
        chunk->set_channels(channels, channel_mask);
        chunk->set_sample_count(output_frames);

        return true;
    }

    void on_endoftrack(abort_callback& abort) override {
        (void)abort;

        if (m_settings.reset_each_track) {
            drain_delay_to_inserted_chunk();
            publish_final_track_summary();
            reset_runtime_state();
        }
        else {
            publish_final_track_summary();
        }
    }

    void on_endofplayback(abort_callback& abort) override {
        (void)abort;
        drain_delay_to_inserted_chunk();
        publish_final_track_summary();
        reset_runtime_state();
    }

    void flush() override {
        reset_runtime_state();
    }

    double get_latency() override {
        if (m_sample_rate == 0) {
            return 0.0;
        }

        return static_cast<double>(m_lookahead_frames) /
            static_cast<double>(m_sample_rate);
    }

    bool need_track_change_mark() override {
        return m_settings.reset_each_track;
    }

private:
    void build_channel_energy_weights(
        unsigned channels,
        unsigned channel_mask
    ) {
        m_channel_energy_weights.assign(channels, 1.0);
        m_layout_has_lfe = false;

        if (channel_mask == 0 ||
            count_channel_flags(channel_mask) != channels) {
            return;
        }

        unsigned channel_index = 0;

        for (unsigned bit_index = 0;
             bit_index < 32 && channel_index < channels;
             ++bit_index) {
            const unsigned channel_flag = 1u << bit_index;

            if ((channel_mask & channel_flag) == 0) {
                continue;
            }

            if (channel_flag == kChannelLfe) {
                m_channel_energy_weights[channel_index] = 0.0;
                m_layout_has_lfe = true;
            }
            else if (is_surround_channel_flag(channel_flag)) {
                m_channel_energy_weights[channel_index] =
                    kSurroundEnergyWeight;
            }
            else {
                m_channel_energy_weights[channel_index] = 1.0;
            }

            ++channel_index;
        }
    }

    void configure_stream(
        unsigned sample_rate,
        unsigned channels,
        unsigned channel_mask
    ) {
        m_sample_rate = sample_rate;
        m_channels = channels;
        m_channel_mask = channel_mask;

        build_channel_energy_weights(channels, channel_mask);

        g_diagnostic_channel_count.store(
            channels,
            std::memory_order_relaxed
        );
        g_diagnostic_channel_mask.store(
            channel_mask,
            std::memory_order_relaxed
        );
        g_diagnostic_lfe_excluded.store(
            m_layout_has_lfe ? 1 : 0,
            std::memory_order_relaxed
        );

        m_filters.clear();
        m_filters.resize(channels);
        m_output_filters.clear();
        m_output_filters.resize(channels);

        for (unsigned channel = 0; channel < channels; ++channel) {
            m_filters[channel].pre_filter = make_high_shelf(
                static_cast<double>(sample_rate),
                1681.974450955533,
                3.999843853973347,
                0.7071752369554196
            );

            m_filters[channel].rlb_filter = make_high_pass(
                static_cast<double>(sample_rate),
                38.13547087602444,
                0.5003270373238773
            );

            m_output_filters[channel].pre_filter = make_high_shelf(
                static_cast<double>(sample_rate),
                1681.974450955533,
                3.999843853973347,
                0.7071752369554196
            );

            m_output_filters[channel].rlb_filter = make_high_pass(
                static_cast<double>(sample_rate),
                38.13547087602444,
                0.5003270373238773
            );
        }

        m_block_window_frames = std::max<t_size>(
            1,
            static_cast<t_size>(
                std::llround(
                    static_cast<double>(sample_rate) * kBlockSeconds
                )
            )
        );

        m_block_hop_frames = std::max<t_size>(
            1,
            static_cast<t_size>(
                std::llround(
                    static_cast<double>(sample_rate) * kBlockHopSeconds
                )
            )
        );

        m_short_term_window_frames = std::max<t_size>(
            1,
            static_cast<t_size>(
                std::llround(
                    static_cast<double>(sample_rate) * kShortTermSeconds
                )
            )
        );

        m_lookahead_frames = m_settings.enable_peak_guard
            ? static_cast<t_size>(std::llround(
                static_cast<double>(sample_rate) *
                static_cast<double>(m_settings.lookahead_ms) /
                1000.0
            ))
            : 0;

        m_energy_ring.assign(m_block_window_frames, 0.0);
        m_short_term_energy_ring.assign(
            m_short_term_window_frames,
            0.0
        );
        m_output_energy_ring.assign(m_block_window_frames, 0.0);
        m_output_short_term_energy_ring.assign(
            m_short_term_window_frames,
            0.0
        );
        m_output_measure_frame.assign(channels, 0.0f);
        m_clipper_previous_input.assign(channels, 0.0);
        m_clipper_previous_valid.assign(channels, false);

        m_three_band_split_states.assign(
            channels,
            three_band_split_state{}
        );
        m_three_band_low_frame.assign(channels, 0.0);
        m_three_band_mid_frame.assign(channels, 0.0);
        m_three_band_high_frame.assign(channels, 0.0);

        const double sample_rate_value =
            static_cast<double>(sample_rate);
        const double low_crossover = std::min(
            kThreeBandLowCrossoverHz,
            sample_rate_value * 0.20
        );
        const double high_crossover = std::min(
            kThreeBandHighCrossoverHz,
            sample_rate_value * 0.45
        );

        m_three_band_low_coefficient = std::exp(
            -2.0 * kPi * low_crossover / sample_rate_value
        );
        m_three_band_high_coefficient = std::exp(
            -2.0 * kPi * high_crossover / sample_rate_value
        );

        build_true_peak_coefficients();

        m_true_peak_history.clear();
        m_true_peak_history.resize(channels);
        m_output_true_peak_history.clear();
        m_output_true_peak_history.resize(channels);
        m_compare_true_peak_history.clear();
        m_compare_true_peak_history.resize(channels);

        for (unsigned channel = 0; channel < channels; ++channel) {
            m_true_peak_history[channel].assign(
                kTruePeakTapCount,
                0.0
            );
            m_output_true_peak_history[channel].assign(
                kTruePeakTapCount,
                0.0
            );
            m_compare_true_peak_history[channel].assign(
                kTruePeakTapCount,
                0.0
            );
        }

        reset_runtime_state();

        g_diagnostic_latency_ms.store(
            get_latency() * 1000.0,
            std::memory_order_relaxed
        );
    }

    void build_true_peak_coefficients() {
        m_true_peak_coefficients.clear();
        m_true_peak_coefficients.resize(kTruePeakFactor);

        for (unsigned phase = 0; phase < kTruePeakFactor; ++phase) {
            auto& coefficients = m_true_peak_coefficients[phase];
            coefficients.assign(kTruePeakTapCount, 0.0);

            const double fraction =
                static_cast<double>(phase) /
                static_cast<double>(kTruePeakFactor);

            double sum = 0.0;

            for (unsigned tap = 0; tap < kTruePeakTapCount; ++tap) {
                const double position =
                    static_cast<double>(tap) -
                    static_cast<double>(kTruePeakDelay) +
                    fraction;

                const double window =
                    0.42 -
                    0.50 * std::cos(
                        2.0 * kPi * static_cast<double>(tap) /
                        static_cast<double>(kTruePeakTapCount - 1)
                    ) +
                    0.08 * std::cos(
                        4.0 * kPi * static_cast<double>(tap) /
                        static_cast<double>(kTruePeakTapCount - 1)
                    );

                coefficients[tap] = sinc(position) * window;
                sum += coefficients[tap];
            }

            if (std::fabs(sum) > 1.0e-12) {
                for (double& coefficient : coefficients) {
                    coefficient /= sum;
                }
            }
        }
    }

    double push_and_measure_true_peak(
        unsigned channel,
        double input
    ) {
        auto& history = m_true_peak_history[channel];

        for (t_size index = history.size() - 1; index > 0; --index) {
            history[index] = history[index - 1];
        }

        history[0] = input;

        double peak = std::fabs(input);

        for (unsigned phase = 0; phase < kTruePeakFactor; ++phase) {
            const auto& coefficients =
                m_true_peak_coefficients[phase];

            double interpolated = 0.0;

            for (unsigned tap = 0; tap < kTruePeakTapCount; ++tap) {
                interpolated += history[tap] * coefficients[tap];
            }

            peak = std::max(peak, std::fabs(interpolated));
        }

        return peak;
    }

    double push_and_measure_compare_true_peak(
        unsigned channel,
        double input
    ) {
        auto& history = m_compare_true_peak_history[channel];

        for (t_size index = history.size() - 1; index > 0; --index) {
            history[index] = history[index - 1];
        }

        history[0] = input;
        double peak = std::fabs(input);

        for (unsigned phase = 0; phase < kTruePeakFactor; ++phase) {
            const auto& coefficients =
                m_true_peak_coefficients[phase];
            double interpolated = 0.0;

            for (unsigned tap = 0; tap < kTruePeakTapCount; ++tap) {
                interpolated += history[tap] * coefficients[tap];
            }

            peak = std::max(peak, std::fabs(interpolated));
        }

        return peak;
    }

    double push_and_measure_output_true_peak(
        unsigned channel,
        double input
    ) {
        auto& history = m_output_true_peak_history[channel];

        for (t_size index = history.size() - 1; index > 0; --index) {
            history[index] = history[index - 1];
        }

        history[0] = input;
        double peak = std::fabs(input);

        for (unsigned phase = 0; phase < kTruePeakFactor; ++phase) {
            const auto& coefficients =
                m_true_peak_coefficients[phase];
            double interpolated = 0.0;

            for (unsigned tap = 0; tap < kTruePeakTapCount; ++tap) {
                interpolated += history[tap] * coefficients[tap];
            }

            peak = std::max(peak, std::fabs(interpolated));
        }

        return peak;
    }

    double process_three_band_lowpass(
        double input,
        double coefficient,
        double& state
    ) {
        state =
            input +
            coefficient * (state - input);

        if (!std::isfinite(state)) {
            state = 0.0;
        }

        return state;
    }

    void split_three_band(
        unsigned channel,
        double input,
        double& low,
        double& mid,
        double& high
    ) {
        if (channel >= m_three_band_split_states.size()) {
            low = input;
            mid = 0.0;
            high = 0.0;
            return;
        }

        auto& state = m_three_band_split_states[channel];

        const double lowpass_low =
            process_three_band_lowpass(
                input,
                m_three_band_low_coefficient,
                state.low_state
            );

        const double lowpass_high =
            process_three_band_lowpass(
                input,
                m_three_band_high_coefficient,
                state.high_lowpass_state
            );

        low = lowpass_low;
        mid = lowpass_high - lowpass_low;
        high = input - lowpass_high;
    }

    double calculate_three_band_target_gain_db(
        double detector_peak,
        double threshold_db,
        double ratio
    ) const {
        if (detector_peak <= 1.0e-12 || ratio <= 1.0001) {
            return 0.0;
        }

        const double input_db = linear_to_db(detector_peak);
        const double lower_knee =
            threshold_db - kThreeBandKneeDb * 0.5;
        const double upper_knee =
            threshold_db + kThreeBandKneeDb * 0.5;

        double output_db = input_db;

        if (input_db > upper_knee) {
            output_db =
                threshold_db +
                (input_db - threshold_db) / ratio;
        }
        else if (input_db > lower_knee) {
            const double distance = input_db - lower_knee;
            const double knee_compression =
                (1.0 / ratio - 1.0) *
                distance * distance /
                (2.0 * kThreeBandKneeDb);

            output_db = input_db + knee_compression;
        }

        return std::min(0.0, output_db - input_db);
    }

    void smooth_three_band_gain(
        double target_gain_db,
        double attack_seconds,
        double release_seconds,
        double& current_gain_db
    ) {
        const bool increasing_reduction =
            target_gain_db < current_gain_db;
        const double time_seconds = increasing_reduction
            ? attack_seconds
            : release_seconds;

        const double coefficient = std::exp(
            -1.0 /
            (
                std::max(0.001, time_seconds) *
                static_cast<double>(m_sample_rate)
            )
        );

        current_gain_db =
            target_gain_db +
            (
                current_gain_db -
                target_gain_db
            ) * coefficient;

        if (!std::isfinite(current_gain_db)) {
            current_gain_db = 0.0;
        }
    }

    void update_three_band_compressors(
        double low_peak,
        double mid_peak,
        double high_peak
    ) {
        if (!m_settings.enable_three_band_master ||
            m_sample_rate == 0) {
            m_three_band_low_gain_db = 0.0;
            m_three_band_mid_gain_db = 0.0;
            m_three_band_high_gain_db = 0.0;
            return;
        }

        const double strength = modern_strength_fraction();

        const double low_ratio = 1.0 + 1.2 * strength;
        const double mid_ratio = 1.0 + 1.8 * strength;
        const double high_ratio = 1.0 + 0.8 * strength;

        const double low_target =
            calculate_three_band_target_gain_db(
                low_peak,
                kThreeBandLowThresholdDb,
                low_ratio
            );

        const double mid_target =
            calculate_three_band_target_gain_db(
                mid_peak,
                kThreeBandMidThresholdDb,
                mid_ratio
            );

        const double high_target =
            calculate_three_band_target_gain_db(
                high_peak,
                kThreeBandHighThresholdDb,
                high_ratio
            );

        smooth_three_band_gain(
            low_target,
            kThreeBandLowAttackSeconds,
            kThreeBandLowReleaseSeconds,
            m_three_band_low_gain_db
        );

        smooth_three_band_gain(
            mid_target,
            kThreeBandMidAttackSeconds,
            kThreeBandMidReleaseSeconds,
            m_three_band_mid_gain_db
        );

        smooth_three_band_gain(
            high_target,
            kThreeBandHighAttackSeconds,
            kThreeBandHighReleaseSeconds,
            m_three_band_high_gain_db
        );

        m_modern_compressor_gain_db = std::min({
            m_three_band_low_gain_db,
            m_three_band_mid_gain_db,
            m_three_band_high_gain_db
        });
    }

    void update_adaptive_master(bool audition_bypass) {
        const double configured_strength = clamp_value(
            static_cast<double>(m_settings.modern_strength_percent),
            0.0,
            100.0
        );

        if (!m_settings.enable_adaptive_master ||
            !m_settings.enable_modern_boost ||
            m_sample_rate == 0 ||
            audition_bypass) {
            m_effective_modern_strength_percent = configured_strength;
            return;
        }

        double desired_strength = std::min(configured_strength, 45.0);

        if (std::isfinite(m_lra_lu) && m_lra_lu > -190.0) {
            if (m_lra_lu >= 10.0) desired_strength = 30.0;
            else if (m_lra_lu >= 6.0) desired_strength = 45.0;
            else desired_strength = 60.0;
        }

        // Already-loud masters need less additional clipping.
        if (std::isfinite(m_integrated_lufs) &&
            m_integrated_lufs > -190.0) {
            if (m_integrated_lufs >= -10.0) {
                desired_strength = std::min(desired_strength, 28.0);
            }
            else if (m_integrated_lufs >= -13.0) {
                desired_strength = std::min(desired_strength, 40.0);
            }
        }

        if (m_processing_risk_state >= 3) {
            desired_strength = std::min(desired_strength, 25.0);
        }
        else if (m_processing_risk_state == 2) {
            desired_strength = std::min(desired_strength, 40.0);
        }
        if (m_safety_reduction_db <= -1.0) {
            desired_strength = std::min(desired_strength, 35.0);
        }

        desired_strength = clamp_value(
            desired_strength,
            kAdaptiveMinimumStrengthPercent,
            std::min(
                kAdaptiveMaximumStrengthPercent,
                std::max(kAdaptiveMinimumStrengthPercent, configured_strength)
            )
        );

        const double coefficient = std::exp(
            -1.0 /
            (kAdaptiveStrengthResponseSeconds *
             static_cast<double>(m_sample_rate))
        );
        m_effective_modern_strength_percent =
            desired_strength +
            (m_effective_modern_strength_percent - desired_strength) *
            coefficient;
        m_effective_modern_strength_percent = clamp_value(
            m_effective_modern_strength_percent,
            0.0,
            100.0
        );
    }

    double modern_strength_fraction() const {
        const double percent = m_settings.enable_adaptive_master
            ? m_effective_modern_strength_percent
            : static_cast<double>(m_settings.modern_strength_percent);
        return clamp_value(percent / 100.0, 0.0, 1.0);
    }

    double modern_compressor_ratio() const {
        return 1.0 + 2.0 * modern_strength_fraction();
    }

    double modern_clip_drive_db() const {
        return 4.0 * modern_strength_fraction();
    }

    double calculate_modern_compressor_target_db(
        double detector_peak
    ) const {
        if (!m_settings.enable_modern_boost ||
            detector_peak <= 1.0e-12) {
            return 0.0;
        }

        const double ratio = modern_compressor_ratio();
        if (ratio <= 1.0001) {
            return 0.0;
        }

        const double input_db = linear_to_db(detector_peak);
        const double threshold_db = kModernCompressorThresholdDb;
        const double knee_db = kModernCompressorKneeDb;
        const double lower_knee = threshold_db - knee_db * 0.5;
        const double upper_knee = threshold_db + knee_db * 0.5;

        double output_db = input_db;

        if (input_db > upper_knee) {
            output_db =
                threshold_db +
                (input_db - threshold_db) / ratio;
        }
        else if (input_db > lower_knee) {
            const double distance = input_db - lower_knee;
            const double knee_compression =
                (1.0 / ratio - 1.0) *
                distance * distance /
                (2.0 * knee_db);

            output_db = input_db + knee_compression;
        }

        return std::min(0.0, output_db - input_db);
    }

    void update_modern_compressor(double detector_peak) {
        if (!m_settings.enable_modern_boost || m_sample_rate == 0) {
            m_modern_compressor_gain_db = 0.0;
            return;
        }

        const double target_gain_db =
            calculate_modern_compressor_target_db(detector_peak);

        const bool increasing_reduction =
            target_gain_db < m_modern_compressor_gain_db;

        const double time_constant = increasing_reduction
            ? kModernCompressorAttackSeconds
            : kModernCompressorReleaseSeconds;

        const double coefficient = std::exp(
            -1.0 /
            (
                time_constant *
                static_cast<double>(m_sample_rate)
            )
        );

        m_modern_compressor_gain_db =
            target_gain_db +
            (
                m_modern_compressor_gain_db -
                target_gain_db
            ) * coefficient;

        if (!std::isfinite(m_modern_compressor_gain_db)) {
            m_modern_compressor_gain_db = 0.0;
        }
    }

    double shape_modern_soft_clip(double input) const {
        const double magnitude = std::fabs(input);
        constexpr double knee_start = 0.75;

        if (magnitude <= knee_start) {
            return input;
        }

        const double room = 1.0 - knee_start;
        const double excess = magnitude - knee_start;
        const double shaped_magnitude =
            knee_start +
            room * excess / (room + excess);

        return std::copysign(
            std::min(1.0, shaped_magnitude),
            input
        );
    }

    double apply_modern_soft_clipper_4x(
        unsigned channel,
        double input,
        double& reduction_db
    ) {
        reduction_db = 0.0;

        if (!m_settings.enable_modern_boost) {
            return input;
        }

        const double strength = modern_strength_fraction();
        if (strength <= 0.0001 ||
            channel >= m_clipper_previous_input.size()) {
            return input;
        }

        const double driven =
            input * db_to_linear(modern_clip_drive_db());

        double previous = driven;
        if (m_clipper_previous_valid[channel]) {
            previous = m_clipper_previous_input[channel];
        }

        // Four interpolated nonlinear evaluations followed by a compact
        // weighted low-pass average. This substantially reduces the
        // aliasing of the direct-rate clipper without changing latency.
        constexpr double weights[kModernClipperOversampleFactor] = {
            1.0, 2.0, 2.0, 1.0
        };
        constexpr double weight_sum = 6.0;

        double accumulated = 0.0;
        double peak_before = 0.0;
        double peak_after = 0.0;

        for (unsigned phase = 1;
             phase <= kModernClipperOversampleFactor;
             ++phase) {
            const double mix =
                static_cast<double>(phase) /
                static_cast<double>(kModernClipperOversampleFactor);

            const double oversampled =
                previous + (driven - previous) * mix;
            const double shaped =
                shape_modern_soft_clip(oversampled);
            const double weight = weights[phase - 1];

            accumulated += shaped * weight;
            peak_before = std::max(
                peak_before,
                std::fabs(oversampled)
            );
            peak_after = std::max(
                peak_after,
                std::fabs(shaped)
            );
        }

        m_clipper_previous_input[channel] = driven;
        m_clipper_previous_valid[channel] = true;

        if (peak_before > 1.0e-12 &&
            peak_after > 1.0e-12) {
            reduction_db = std::max(
                0.0,
                linear_to_db(peak_before / peak_after)
            );
        }

        return accumulated / weight_sum;
    }

    void update_auto_safety(
        double clipper_reduction_db,
        bool audition_bypass
    ) {
        if (!m_settings.enable_modern_boost ||
            m_sample_rate == 0 ||
            audition_bypass) {
            m_processing_risk_state = 0;
            m_strong_processing_seconds = 0.0;
            m_excessive_processing_seconds = 0.0;

            if (!m_settings.enable_modern_boost) {
                m_safety_reduction_db = 0.0;
            }
            return;
        }

        const double compressor_reduction_db = std::max(
            0.0,
            -m_modern_compressor_gain_db
        );
        const double limiter_reduction_db = std::max(
            0.0,
            -m_limiter_gain_db
        );

        const bool strong_now =
            compressor_reduction_db >= 3.0 ||
            clipper_reduction_db >= 1.5 ||
            limiter_reduction_db >= 1.5;

        const bool excessive_now =
            compressor_reduction_db >= 6.0 ||
            clipper_reduction_db >= 3.0 ||
            limiter_reduction_db >= 3.0;

        const double frame_seconds =
            1.0 / static_cast<double>(m_sample_rate);

        if (strong_now) {
            m_strong_processing_seconds += frame_seconds;
        }
        else {
            m_strong_processing_seconds = std::max(
                0.0,
                m_strong_processing_seconds - 2.0 * frame_seconds
            );
        }

        if (excessive_now) {
            m_excessive_processing_seconds += frame_seconds;
        }
        else {
            m_excessive_processing_seconds = std::max(
                0.0,
                m_excessive_processing_seconds - 2.0 * frame_seconds
            );
        }

        if (m_excessive_processing_seconds >=
            kRiskExcessiveHoldSeconds) {
            m_processing_risk_state = 3;
        }
        else if (m_strong_processing_seconds >=
            kRiskStrongHoldSeconds) {
            m_processing_risk_state = 2;
        }
        else {
            m_processing_risk_state = 1;
        }

        const double processing_load = std::max({
            compressor_reduction_db / 6.0,
            clipper_reduction_db / 3.0,
            limiter_reduction_db / 3.0
        });

        double target_safety_db = 0.0;
        if (processing_load > 1.0) {
            target_safety_db = -std::min(
                kAutoSafetyMaximumReductionDb,
                0.5 + 2.0 * (processing_load - 1.0)
            );
        }

        const bool increasing_protection =
            target_safety_db < m_safety_reduction_db;
        const double time_seconds = increasing_protection
            ? kAutoSafetyAttackSeconds
            : kAutoSafetyReleaseSeconds;

        const double coefficient = std::exp(
            -1.0 /
            (
                time_seconds *
                static_cast<double>(m_sample_rate)
            )
        );

        m_safety_reduction_db =
            target_safety_db +
            (
                m_safety_reduction_db -
                target_safety_db
            ) * coefficient;

        m_safety_reduction_db = clamp_value(
            m_safety_reduction_db,
            -kAutoSafetyMaximumReductionDb,
            0.0
        );
    }

    double estimate_compare_match_gain_db() const {
        double result = 0.0;

        if (std::isfinite(m_output_short_term_lufs) &&
            m_output_short_term_lufs > -190.0 &&
            std::isfinite(m_short_term_lufs) &&
            m_short_term_lufs > -190.0) {
            result = m_output_short_term_lufs - m_short_term_lufs;
        }
        else if (std::isfinite(m_output_integrated_lufs) &&
                 m_output_integrated_lufs > -190.0 &&
                 std::isfinite(m_integrated_lufs) &&
                 m_integrated_lufs > -190.0) {
            result = m_output_integrated_lufs - m_integrated_lufs;
        }
        else {
            result = m_current_gain_db + m_safety_reduction_db;

            if (m_settings.enable_three_band_master) {
                result += (
                    m_three_band_low_gain_db +
                    m_three_band_mid_gain_db +
                    m_three_band_high_gain_db
                ) / 3.0;
            }
            else if (m_settings.enable_modern_boost) {
                result += m_modern_compressor_gain_db;
            }

            result += m_limiter_gain_db;
        }

        if (!std::isfinite(result)) {
            result = 0.0;
        }

        return clamp_value(
            result,
            kCompareMatchMinimumDb,
            kCompareMatchMaximumDb
        );
    }

    void update_compare_request_state(int compare_mode) {
        if (compare_mode == m_last_compare_request_mode) {
            return;
        }

        if (compare_mode == 2) {
            m_compare_match_gain_db = estimate_compare_match_gain_db();
        }
        else if (compare_mode == 0) {
            m_compare_limiter_gain_db = 0.0;
        }

        m_last_compare_request_mode = compare_mode;
    }

    void smooth_compare_limiter_gain_toward(double target_gain_db) {
        if (!m_settings.enable_peak_guard || m_sample_rate == 0) {
            m_compare_limiter_gain_db = 0.0;
            return;
        }

        if (target_gain_db < m_compare_limiter_gain_db) {
            m_compare_limiter_gain_db = target_gain_db;
            return;
        }

        const double release_seconds = std::max(
            0.020,
            static_cast<double>(m_settings.limiter_release_ms) / 1000.0
        );
        const double coefficient = std::exp(
            -1.0 /
            (release_seconds * static_cast<double>(m_sample_rate))
        );

        m_compare_limiter_gain_db =
            target_gain_db +
            (m_compare_limiter_gain_db - target_gain_db) * coefficient;

        if (!std::isfinite(m_compare_limiter_gain_db)) {
            m_compare_limiter_gain_db = 0.0;
        }
    }

    void push_delay_frame(
        const audio_sample* frame,
        double frame_peak,
        int compare_mode,
        double compare_gain_db
    ) {
        const unsigned long long frame_id = m_next_frame_id++;

        for (unsigned channel = 0; channel < m_channels; ++channel) {
            m_delay_audio.push_back(frame[channel]);
        }

        while (!m_peak_max_queue.empty() &&
               m_peak_max_queue.back().peak <= frame_peak) {
            m_peak_max_queue.pop_back();
        }

        peak_queue_node node;
        node.frame_id = frame_id;
        node.peak = frame_peak;
        m_peak_max_queue.push_back(node);
        m_delay_compare_mode.push_back(compare_mode);
        m_delay_compare_gain_db.push_back(compare_gain_db);
    }

    t_size delay_frame_count() const {
        if (m_channels == 0) {
            return 0;
        }

        return m_delay_audio.size() /
            static_cast<t_size>(m_channels);
    }

    double calculate_compare_transition_gain(
        int compare_mode
    ) {
        if (m_sample_rate == 0) {
            return 1.0;
        }

        const t_size fade_frames = std::max<t_size>(
            1,
            static_cast<t_size>(std::llround(
                static_cast<double>(m_sample_rate) *
                kOriginalCompareFadeSeconds
            ))
        );

        if (!m_last_emitted_compare_mode_valid) {
            m_last_emitted_compare_mode = compare_mode;
            m_last_emitted_compare_mode_valid = true;
        }
        else if (compare_mode != m_last_emitted_compare_mode) {
            m_last_emitted_compare_mode = compare_mode;
            m_compare_fade_in_remaining = fade_frames;
        }

        if (m_compare_fade_in_remaining > 0) {
            const double gain = 1.0 -
                static_cast<double>(m_compare_fade_in_remaining) /
                static_cast<double>(fade_frames);
            --m_compare_fade_in_remaining;
            return clamp_value(gain, 0.0, 1.0);
        }

        const t_size available = std::min<t_size>(
            fade_frames,
            m_delay_compare_mode.size()
        );

        for (t_size offset = 1; offset < available; ++offset) {
            if (m_delay_compare_mode[offset] != compare_mode) {
                return clamp_value(
                    static_cast<double>(offset) /
                    static_cast<double>(fade_frames),
                    0.0,
                    1.0
                );
            }
        }

        return 1.0;
    }

    void emit_one_delayed_frame(
        std::vector<audio_sample>& output,
        double& last_applied_gain_db
    ) {
        if (delay_frame_count() == 0) {
            return;
        }

        const int compare_mode =
            !m_delay_compare_mode.empty()
                ? m_delay_compare_mode.front()
                : 0;
        const bool bypass_frame = compare_mode != 0;
        const double requested_compare_gain_db =
            !m_delay_compare_gain_db.empty()
                ? m_delay_compare_gain_db.front()
                : 0.0;

        double output_gain_db = 0.0;

        if (bypass_frame) {
            output_gain_db = requested_compare_gain_db;

            if (compare_mode == 2 && m_settings.enable_peak_guard) {
                const double window_peak = m_peak_max_queue.empty()
                    ? 0.0
                    : m_peak_max_queue.front().peak;

                double target_compare_limiter_db = 0.0;
                if (window_peak > 0.0) {
                    const double peak_limit = db_to_linear(
                        static_cast<double>(
                            m_settings.true_peak_limit_dbtp
                        )
                    );
                    const double required_gain_db =
                        linear_to_db(peak_limit / window_peak);
                    target_compare_limiter_db = std::min(
                        0.0,
                        required_gain_db - output_gain_db
                    );
                }

                smooth_compare_limiter_gain_toward(
                    target_compare_limiter_db
                );
                output_gain_db += m_compare_limiter_gain_db;
            }
            else {
                m_compare_limiter_gain_db = 0.0;
            }

            last_applied_gain_db = output_gain_db;
        }
        else {
            if (!m_settings.enable_modern_boost) {
                smooth_gain_toward(m_target_gain_db);
            }

            const double window_peak = m_peak_max_queue.empty()
                ? 0.0
                : m_peak_max_queue.front().peak;

            double required_total_gain_db =
                std::numeric_limits<double>::infinity();

            if (m_settings.enable_peak_guard && window_peak > 0.0) {
                const double peak_limit = db_to_linear(
                    static_cast<double>(
                        m_settings.true_peak_limit_dbtp
                    )
                );

                required_total_gain_db =
                    linear_to_db(peak_limit / window_peak);
            }

            double target_limiter_gain_db = 0.0;

            if (std::isfinite(required_total_gain_db)) {
                if (m_settings.enable_modern_boost) {
                    target_limiter_gain_db = std::min(
                        0.0,
                        required_total_gain_db
                    );
                }
                else {
                    target_limiter_gain_db = std::min(
                        0.0,
                        required_total_gain_db -
                            m_current_gain_db
                    );
                }
            }

            smooth_limiter_gain_toward(target_limiter_gain_db);

            if (m_settings.enable_modern_boost) {
                output_gain_db = m_limiter_gain_db;

                if (std::isfinite(required_total_gain_db)) {
                    output_gain_db = std::min(
                        output_gain_db,
                        required_total_gain_db
                    );
                }

                last_applied_gain_db =
                    m_current_gain_db +
                    m_safety_reduction_db +
                    m_modern_compressor_gain_db +
                    output_gain_db;
            }
            else {
                output_gain_db =
                    m_current_gain_db + m_limiter_gain_db;

                if (std::isfinite(required_total_gain_db)) {
                    output_gain_db = std::min(
                        output_gain_db,
                        required_total_gain_db
                    );
                }

                last_applied_gain_db = output_gain_db;
            }
        }

        const double gain = db_to_linear(output_gain_db);
        const double transition_gain =
            calculate_compare_transition_gain(compare_mode);

        if (m_output_measure_frame.size() != m_channels) {
            m_output_measure_frame.assign(m_channels, 0.0f);
        }

        double output_frame_true_peak = 0.0;

        for (unsigned channel = 0; channel < m_channels; ++channel) {
            const double value = static_cast<double>(
                m_delay_audio.front()
            );
            m_delay_audio.pop_front();

            double rendered_value =
                value * gain * transition_gain;

            if (!std::isfinite(rendered_value)) {
                rendered_value = 0.0;
                ++m_recovered_sample_count;
            }
            else if (
                std::fabs(rendered_value) > kSafeAudioMagnitudeLimit
            ) {
                rendered_value = clamp_value(
                    rendered_value,
                    -kSafeAudioMagnitudeLimit,
                    kSafeAudioMagnitudeLimit
                );
                ++m_recovered_sample_count;
            }

            const audio_sample output_value =
                static_cast<audio_sample>(rendered_value);

            output.push_back(output_value);
            m_output_measure_frame[channel] = output_value;

            output_frame_true_peak = std::max(
                output_frame_true_peak,
                push_and_measure_output_true_peak(
                    channel,
                    static_cast<double>(output_value)
                )
            );
        }

        if (!bypass_frame && transition_gain >= 0.999) {
            m_track_max_output_true_peak_linear = std::max(
                m_track_max_output_true_peak_linear,
                output_frame_true_peak
            );

            const bool clip_event_now = output_frame_true_peak >= 1.0;
            if (clip_event_now && !m_clip_event_active) {
                ++m_clip_event_count;
            }
            if (output_frame_true_peak < kClipEventReleaseLinear) {
                m_clip_event_active = false;
            }
            else if (clip_event_now) {
                m_clip_event_active = true;
            }

            measure_output_frame(m_output_measure_frame.data());
        }
        else {
            m_clip_event_active = false;
        }

        if (!m_delay_compare_mode.empty()) {
            m_delay_compare_mode.pop_front();
        }
        if (!m_delay_compare_gain_db.empty()) {
            m_delay_compare_gain_db.pop_front();
        }

        const unsigned long long emitted_frame_id = m_oldest_frame_id++;

        if (!m_peak_max_queue.empty() &&
            m_peak_max_queue.front().frame_id == emitted_frame_id) {
            m_peak_max_queue.pop_front();
        }
    }

    void drain_delay_to_inserted_chunk() {
        if (m_sample_rate == 0 ||
            m_channels == 0 ||
            delay_frame_count() == 0) {
            clear_delay_state();
            return;
        }

        std::vector<audio_sample> output;
        output.reserve(m_delay_audio.size());
        double last_applied_gain_db = m_current_gain_db;

        while (delay_frame_count() > 0) {
            emit_one_delayed_frame(output, last_applied_gain_db);
        }

        if (!output.empty()) {
            audio_chunk* tail = insert_chunk(output.size());
            tail->set_data_size(output.size());
            std::copy(output.begin(), output.end(), tail->get_data());
            tail->set_srate(m_sample_rate);
            tail->set_channels(m_channels, m_channel_mask);
            tail->set_sample_count(
                output.size() / static_cast<t_size>(m_channels)
            );
        }

        clear_delay_state();
    }

    void clear_delay_state() {
        m_delay_audio.clear();
        m_delay_compare_mode.clear();
        m_delay_compare_gain_db.clear();
        m_peak_max_queue.clear();
        m_next_frame_id = 0;
        m_oldest_frame_id = 0;
        m_limiter_gain_db = 0.0;
        m_compare_limiter_gain_db = 0.0;
        m_last_emitted_compare_mode_valid = false;
        m_last_emitted_compare_mode = 0;
        m_compare_fade_in_remaining = 0;
    }

    void push_output_energy(double energy) {
        if (m_output_energy_ring.empty()) {
            return;
        }

        if (m_output_energy_count < m_block_window_frames) {
            m_output_energy_ring[m_output_energy_position] = energy;
            m_output_energy_sum += energy;
            ++m_output_energy_count;
        }
        else {
            m_output_energy_sum -=
                m_output_energy_ring[m_output_energy_position];

            m_output_energy_ring[m_output_energy_position] = energy;
            m_output_energy_sum += energy;
        }

        ++m_output_energy_position;
        if (m_output_energy_position >= m_block_window_frames) {
            m_output_energy_position = 0;
        }

        if (!m_output_short_term_energy_ring.empty()) {
            if (m_output_short_term_energy_count <
                m_short_term_window_frames) {
                m_output_short_term_energy_ring[
                    m_output_short_term_energy_position
                ] = energy;
                m_output_short_term_energy_sum += energy;
                ++m_output_short_term_energy_count;
            }
            else {
                m_output_short_term_energy_sum -=
                    m_output_short_term_energy_ring[
                        m_output_short_term_energy_position
                    ];
                m_output_short_term_energy_ring[
                    m_output_short_term_energy_position
                ] = energy;
                m_output_short_term_energy_sum += energy;
            }

            ++m_output_short_term_energy_position;
            if (m_output_short_term_energy_position >=
                m_short_term_window_frames) {
                m_output_short_term_energy_position = 0;
            }
        }

        ++m_output_frames_since_block_update;
        if (m_output_energy_count >= m_block_window_frames &&
            m_output_frames_since_block_update >= m_block_hop_frames) {
            m_output_frames_since_block_update = 0;
            add_output_loudness_block();
        }
    }

    void add_output_loudness_block() {
        if (m_output_energy_count < m_block_window_frames ||
            m_block_window_frames == 0) {
            return;
        }

        const double block_energy =
            m_output_energy_sum /
            static_cast<double>(m_block_window_frames);
        const double block_lufs = energy_to_lufs(block_energy);

        if (block_lufs >= kAbsoluteGateLufs) {
            if (m_output_gated_block_energies.size() >=
                kMaximumStoredBlocks) {
                const t_size erase_count = std::min<t_size>(
                    static_cast<t_size>(3600),
                    m_output_gated_block_energies.size()
                );

                m_output_gated_block_energies.erase(
                    m_output_gated_block_energies.begin(),
                    m_output_gated_block_energies.begin() + erase_count
                );
            }

            m_output_gated_block_energies.push_back(block_energy);
        }

        m_output_integrated_lufs =
            calculate_gated_integrated_lufs(
                m_output_gated_block_energies
            );

        if (m_output_short_term_energy_count >=
                m_short_term_window_frames &&
            m_short_term_window_frames > 0) {
            const double short_term_energy =
                m_output_short_term_energy_sum /
                static_cast<double>(m_short_term_window_frames);
            m_output_short_term_lufs = energy_to_lufs(short_term_energy);
        }
        else {
            m_output_short_term_lufs = -200.0;
        }
    }

    void measure_output_frame(
        const audio_sample* frame
    ) {
        if (frame == nullptr ||
            m_output_filters.size() != m_channels) {
            return;
        }

        double frame_energy = 0.0;

        for (unsigned channel = 0;
             channel < m_channels;
             ++channel) {
            double filtered =
                m_output_filters[channel].pre_filter.process(
                    static_cast<double>(frame[channel])
                );

            filtered =
                m_output_filters[channel].rlb_filter.process(filtered);

            frame_energy +=
                m_channel_energy_weights[channel] *
                filtered * filtered;
        }

        push_output_energy(frame_energy);
    }

    void push_energy(double energy) {
        if (!m_energy_ring.empty()) {
            if (m_energy_count < m_block_window_frames) {
                m_energy_ring[m_energy_position] = energy;
                m_energy_sum += energy;
                ++m_energy_count;
            }
            else {
                m_energy_sum -= m_energy_ring[m_energy_position];
                m_energy_ring[m_energy_position] = energy;
                m_energy_sum += energy;
            }

            ++m_energy_position;

            if (m_energy_position >= m_block_window_frames) {
                m_energy_position = 0;
            }
        }

        if (!m_short_term_energy_ring.empty()) {
            if (m_short_term_energy_count < m_short_term_window_frames) {
                m_short_term_energy_ring[m_short_term_energy_position] =
                    energy;
                m_short_term_energy_sum += energy;
                ++m_short_term_energy_count;
            }
            else {
                m_short_term_energy_sum -=
                    m_short_term_energy_ring[
                        m_short_term_energy_position
                    ];

                m_short_term_energy_ring[
                    m_short_term_energy_position
                ] = energy;

                m_short_term_energy_sum += energy;
            }

            ++m_short_term_energy_position;

            if (m_short_term_energy_position >=
                m_short_term_window_frames) {
                m_short_term_energy_position = 0;
            }
        }
    }

    void add_loudness_block_and_update_gain() {
        if (m_energy_count < m_block_window_frames ||
            m_block_window_frames == 0) {
            return;
        }

        const double block_energy =
            m_energy_sum /
            static_cast<double>(m_block_window_frames);

        const double block_lufs = energy_to_lufs(block_energy);
        m_momentary_lufs = block_lufs;

        if (m_short_term_energy_count >= m_short_term_window_frames &&
            m_short_term_window_frames > 0) {
            const double short_term_energy =
                m_short_term_energy_sum /
                static_cast<double>(m_short_term_window_frames);

            m_short_term_lufs = energy_to_lufs(short_term_energy);
        }
        else {
            m_short_term_lufs = -200.0;
        }

        if (block_lufs >= kAbsoluteGateLufs) {
            if (m_gated_block_energies.size() >= kMaximumStoredBlocks) {
                const t_size erase_count =
                    std::min<t_size>(
                        static_cast<t_size>(3600),
                        m_gated_block_energies.size()
                    );

                m_gated_block_energies.erase(
                    m_gated_block_energies.begin(),
                    m_gated_block_energies.begin() + erase_count
                );
            }

            m_gated_block_energies.push_back(block_energy);
        }

        update_integrated_loudness_and_gain();

        if (std::isfinite(m_short_term_lufs) &&
            m_short_term_lufs > -190.0) {
            ++m_blocks_since_lra_sample;

            if (m_blocks_since_lra_sample >= kLraSampleHopBlocks) {
                m_blocks_since_lra_sample = 0;

                if (m_short_term_lufs >= kAbsoluteGateLufs) {
                    if (m_lra_short_term_samples.size() >=
                        kMaximumStoredLraSamples) {
                        const t_size erase_count =
                            std::min<t_size>(
                                static_cast<t_size>(600),
                                m_lra_short_term_samples.size()
                            );

                        m_lra_short_term_samples.erase(
                            m_lra_short_term_samples.begin(),
                            m_lra_short_term_samples.begin() +
                                erase_count
                        );
                    }

                    m_lra_short_term_samples.push_back(
                        m_short_term_lufs
                    );
                }

                update_loudness_range();
            }
        }
    }

    void update_loudness_range() {
        if (!std::isfinite(m_integrated_lufs) ||
            m_integrated_lufs <= -190.0 ||
            m_lra_short_term_samples.size() < 2) {
            m_lra_lu = -200.0;
            return;
        }

        const double gate_lufs = std::max(
            kAbsoluteGateLufs,
            m_integrated_lufs + kLraRelativeGateOffsetLu
        );

        std::vector<double> gated_samples;
        gated_samples.reserve(m_lra_short_term_samples.size());

        for (double value : m_lra_short_term_samples) {
            if (value >= gate_lufs) {
                gated_samples.push_back(value);
            }
        }

        if (gated_samples.size() < 2) {
            m_lra_lu = -200.0;
            return;
        }

        std::sort(gated_samples.begin(), gated_samples.end());

        const auto percentile = [&gated_samples](double fraction) {
            const double position =
                fraction *
                static_cast<double>(gated_samples.size() - 1);

            const t_size lower = static_cast<t_size>(
                std::floor(position)
            );
            const t_size upper = static_cast<t_size>(
                std::ceil(position)
            );
            const double mix =
                position - static_cast<double>(lower);

            return gated_samples[lower] * (1.0 - mix) +
                gated_samples[upper] * mix;
        };

        const double low = percentile(0.10);
        const double high = percentile(0.95);
        m_lra_lu = std::max(0.0, high - low);
    }

    bool startup_analysis_ready() const {
        if (m_sample_rate == 0) {
            return false;
        }

        const double required_frames =
            static_cast<double>(m_sample_rate) *
            static_cast<double>(m_settings.startup_analysis_seconds);

        return static_cast<double>(m_processed_frames) >= required_frames;
    }

    bool silence_guard_holding() const {
        if (!m_settings.enable_silence_guard) {
            return false;
        }

        if (!std::isfinite(m_short_term_lufs) ||
            m_short_term_lufs <= -190.0) {
            return true;
        }

        return m_short_term_lufs <
            static_cast<double>(m_settings.silence_guard_lufs);
    }

    void set_neutral_gain_and_state() {
        m_target_gain_db = 0.0;
        pause_gain_lock_tracking();

        if (!startup_analysis_ready() &&
            m_settings.startup_analysis_seconds > 0.0f) {
            m_normalization_state = 1;
        }
        else if (silence_guard_holding()) {
            m_normalization_state = 2;
        }
        else {
            m_normalization_state = 3;
        }
    }

    void pause_gain_lock_tracking() {
        if (!m_settings.enable_gain_lock) {
            m_gain_lock_state = 0;
            m_gain_lock_remaining_seconds = 0.0;
            return;
        }

        if (!m_gain_locked) {
            m_gain_lock_reference_valid = false;
            m_gain_lock_stable_seconds = 0.0;
            m_gain_lock_remaining_seconds =
                static_cast<double>(m_settings.gain_lock_seconds);
            m_gain_lock_state = 1;
        }
    }

    double apply_gain_lock(
        double candidate_gain_db,
        bool allow_tracking
    ) {
        if (!m_settings.enable_gain_lock) {
            m_gain_lock_state = 0;
            m_gain_lock_remaining_seconds = 0.0;
            return candidate_gain_db;
        }

        if (m_gain_locked) {
            m_gain_lock_remaining_seconds = 0.0;

            // Once locked, never increase beyond the locked value.
            // A lower gain value always wins: less boost or more attenuation.
            if (candidate_gain_db < m_locked_gain_db - 0.01) {
                m_gain_lock_state = 3;
                return candidate_gain_db;
            }

            m_gain_lock_state = 2;
            return m_locked_gain_db;
        }

        if (!allow_tracking) {
            pause_gain_lock_tracking();
            return candidate_gain_db;
        }

        const double tolerance = std::max(
            0.1,
            static_cast<double>(m_settings.gain_lock_tolerance_lu)
        );

        if (!m_gain_lock_reference_valid ||
            std::fabs(candidate_gain_db - m_gain_lock_reference_db) >
                tolerance) {
            m_gain_lock_reference_db = candidate_gain_db;
            m_gain_lock_reference_valid = true;
            m_gain_lock_stable_seconds = 0.0;
        }
        else {
            // Slowly follow small estimation changes while judging stability.
            m_gain_lock_reference_db =
                0.90 * m_gain_lock_reference_db +
                0.10 * candidate_gain_db;

            m_gain_lock_stable_seconds += kBlockHopSeconds;
        }

        const double required_seconds = std::max(
            0.0,
            static_cast<double>(m_settings.gain_lock_seconds)
        );

        m_gain_lock_remaining_seconds = std::max(
            0.0,
            required_seconds - m_gain_lock_stable_seconds
        );

        if (required_seconds <= 0.0 ||
            m_gain_lock_stable_seconds >= required_seconds) {
            m_gain_locked = true;
            m_locked_gain_db = m_gain_lock_reference_db;
            m_gain_lock_remaining_seconds = 0.0;
            m_gain_lock_state = 2;

            return std::min(candidate_gain_db, m_locked_gain_db);
        }

        m_gain_lock_state = 1;
        return candidate_gain_db;
    }

    void update_integrated_loudness_and_gain() {
        if (m_gated_block_energies.empty()) {
            m_integrated_lufs = -200.0;
            set_neutral_gain_and_state();
            return;
        }

        const double ungated_sum = std::accumulate(
            m_gated_block_energies.begin(),
            m_gated_block_energies.end(),
            0.0
        );

        const double ungated_mean =
            ungated_sum /
            static_cast<double>(m_gated_block_energies.size());

        const double ungated_lufs = energy_to_lufs(ungated_mean);
        const double relative_gate_lufs =
            ungated_lufs + kRelativeGateOffsetLu;

        const double effective_gate_lufs = std::max(
            kAbsoluteGateLufs,
            relative_gate_lufs
        );

        double gated_sum = 0.0;
        t_size gated_count = 0;

        for (double energy : m_gated_block_energies) {
            if (energy_to_lufs(energy) >= effective_gate_lufs) {
                gated_sum += energy;
                ++gated_count;
            }
        }

        if (gated_count == 0) {
            m_integrated_lufs = -200.0;
            set_neutral_gain_and_state();
            return;
        }

        const double gated_mean =
            gated_sum / static_cast<double>(gated_count);

        m_integrated_lufs = energy_to_lufs(gated_mean);

        if (!std::isfinite(m_integrated_lufs)) {
            m_integrated_lufs = -200.0;
            set_neutral_gain_and_state();
            return;
        }

        const double desired_gain_db =
            static_cast<double>(m_settings.target_lufs) -
            m_integrated_lufs;

        const double limited_gain_db = clamp_value(
            desired_gain_db,
            -static_cast<double>(m_settings.max_attenuation_db),
            static_cast<double>(m_settings.max_boost_db)
        );

        // Safety attenuation is never delayed. Only positive gain waits for
        // a stable measurement and, when enabled, a non-silent short-term
        // loudness value.
        if (limited_gain_db <= 0.0) {
            const bool allow_lock_tracking =
                startup_analysis_ready() &&
                !silence_guard_holding();

            m_target_gain_db = apply_gain_lock(
                limited_gain_db,
                allow_lock_tracking
            );
            m_normalization_state =
                limited_gain_db < -0.01 ? 4 : 3;
            return;
        }

        if (!startup_analysis_ready() &&
            m_settings.startup_analysis_seconds > 0.0f) {
            m_target_gain_db = 0.0;
            pause_gain_lock_tracking();
            m_normalization_state = 1;
            return;
        }

        if (silence_guard_holding()) {
            m_target_gain_db = 0.0;
            pause_gain_lock_tracking();
            m_normalization_state = 2;
            return;
        }

        m_target_gain_db = apply_gain_lock(
            limited_gain_db,
            true
        );
        m_normalization_state = 3;
    }

    void smooth_gain_toward(double target_gain_db) {
        if (m_sample_rate == 0) {
            m_current_gain_db = target_gain_db;
            return;
        }

        const bool needs_fast_attenuation =
            target_gain_db < m_current_gain_db;

        const double time_constant = needs_fast_attenuation
            ? kFastAttenuationSeconds
            : kSlowBoostSeconds;

        const double coefficient = std::exp(
            -1.0 /
            (time_constant * static_cast<double>(m_sample_rate))
        );

        m_current_gain_db =
            target_gain_db +
            (m_current_gain_db - target_gain_db) * coefficient;

        if (!std::isfinite(m_current_gain_db)) {
            m_current_gain_db = 0.0;
        }
    }

    void smooth_limiter_gain_toward(double target_gain_db) {
        if (!m_settings.enable_peak_guard || m_sample_rate == 0) {
            m_limiter_gain_db = 0.0;
            return;
        }

        if (target_gain_db < m_limiter_gain_db) {
            m_limiter_gain_db = target_gain_db;
            return;
        }

        const double release_seconds = std::max(
            0.020,
            static_cast<double>(m_settings.limiter_release_ms) / 1000.0
        );

        const double coefficient = std::exp(
            -1.0 /
            (release_seconds * static_cast<double>(m_sample_rate))
        );

        m_limiter_gain_db =
            target_gain_db +
            (m_limiter_gain_db - target_gain_db) * coefficient;

        if (!std::isfinite(m_limiter_gain_db)) {
            m_limiter_gain_db = 0.0;
        }
    }

    int calculate_track_evaluation_state() const {
        const double max_true_peak_dbtp =
            linear_to_db(m_track_max_output_true_peak_linear);

        if (m_clip_event_count > 0 ||
            max_true_peak_dbtp > 0.01 ||
            m_track_max_clipper_reduction_db >= 3.0 ||
            m_track_max_limiter_reduction_db >= 3.0) {
            return 3;
        }

        if (m_track_max_compressor_reduction_db >= 6.0 ||
            m_track_max_clipper_reduction_db >= 1.5 ||
            m_track_max_limiter_reduction_db >= 1.5) {
            return 2;
        }

        return m_processed_frames > 0 ? 1 : 0;
    }

    void update_processing_metrics() {
        m_track_max_compressor_reduction_db = std::max(
            m_track_max_compressor_reduction_db,
            std::max(0.0, -m_modern_compressor_gain_db)
        );

        if (m_settings.enable_three_band_master) {
            m_track_max_three_band_low_reduction_db = std::max(
                m_track_max_three_band_low_reduction_db,
                std::max(0.0, -m_three_band_low_gain_db)
            );
            m_track_max_three_band_mid_reduction_db = std::max(
                m_track_max_three_band_mid_reduction_db,
                std::max(0.0, -m_three_band_mid_gain_db)
            );
            m_track_max_three_band_high_reduction_db = std::max(
                m_track_max_three_band_high_reduction_db,
                std::max(0.0, -m_three_band_high_gain_db)
            );
        }

        m_track_max_clipper_reduction_db = std::max(
            m_track_max_clipper_reduction_db,
            m_modern_clipper_reduction_db
        );
        m_track_max_limiter_reduction_db = std::max(
            m_track_max_limiter_reduction_db,
            std::max(0.0, -m_limiter_gain_db)
        );
        m_track_evaluation_state =
            calculate_track_evaluation_state();
    }

    void update_cpu_load(
        const LARGE_INTEGER& processing_start,
        t_size frame_count,
        unsigned sample_rate
    ) {
        LARGE_INTEGER processing_end = {};
        LARGE_INTEGER frequency = {};

        if (!QueryPerformanceCounter(&processing_end) ||
            !QueryPerformanceFrequency(&frequency) ||
            frequency.QuadPart <= 0 ||
            sample_rate == 0 ||
            frame_count == 0) {
            return;
        }

        const double processing_seconds =
            static_cast<double>(
                processing_end.QuadPart - processing_start.QuadPart
            ) /
            static_cast<double>(frequency.QuadPart);
        const double audio_seconds =
            static_cast<double>(frame_count) /
            static_cast<double>(sample_rate);

        if (audio_seconds <= 0.0) {
            return;
        }

        const double instant_percent = clamp_value(
            100.0 * processing_seconds / audio_seconds,
            0.0,
            999.0
        );

        if (!m_cpu_load_initialized) {
            m_cpu_load_percent = instant_percent;
            m_cpu_load_initialized = true;
        }
        else {
            m_cpu_load_percent =
                0.90 * m_cpu_load_percent +
                0.10 * instant_percent;
        }
    }

    void publish_final_track_summary() {
        if (m_processed_frames == 0 ||
            !std::isfinite(m_integrated_lufs) ||
            m_integrated_lufs <= -190.0) {
            return;
        }

        double target_difference_lu = -200.0;
        if (std::isfinite(m_output_integrated_lufs) &&
            m_output_integrated_lufs > -190.0) {
            target_difference_lu =
                m_output_integrated_lufs -
                static_cast<double>(m_settings.target_lufs);
        }

        g_diagnostic_final_input_integrated_lufs.store(
            m_integrated_lufs,
            std::memory_order_relaxed
        );
        g_diagnostic_final_output_integrated_lufs.store(
            m_output_integrated_lufs,
            std::memory_order_relaxed
        );
        g_diagnostic_final_target_difference_lu.store(
            target_difference_lu,
            std::memory_order_relaxed
        );
        g_diagnostic_final_lra_lu.store(
            m_lra_lu,
            std::memory_order_relaxed
        );
        g_diagnostic_final_max_true_peak_dbtp.store(
            linear_to_db(m_track_max_output_true_peak_linear),
            std::memory_order_relaxed
        );
        g_diagnostic_final_max_compressor_reduction_db.store(
            m_track_max_compressor_reduction_db,
            std::memory_order_relaxed
        );
        g_diagnostic_final_max_clipper_reduction_db.store(
            m_track_max_clipper_reduction_db,
            std::memory_order_relaxed
        );
        g_diagnostic_final_max_limiter_reduction_db.store(
            m_track_max_limiter_reduction_db,
            std::memory_order_relaxed
        );
        g_diagnostic_final_three_band_master_state.store(
            m_settings.enable_three_band_master ? 1 : 0,
            std::memory_order_relaxed
        );
        g_diagnostic_final_max_three_band_low_reduction_db.store(
            m_track_max_three_band_low_reduction_db,
            std::memory_order_relaxed
        );
        g_diagnostic_final_max_three_band_mid_reduction_db.store(
            m_track_max_three_band_mid_reduction_db,
            std::memory_order_relaxed
        );
        g_diagnostic_final_max_three_band_high_reduction_db.store(
            m_track_max_three_band_high_reduction_db,
            std::memory_order_relaxed
        );
        g_diagnostic_final_clip_event_count.store(
            m_clip_event_count,
            std::memory_order_relaxed
        );
        g_diagnostic_final_recovered_sample_count.store(
            m_recovered_sample_count,
            std::memory_order_relaxed
        );
        g_diagnostic_final_evaluation_state.store(
            calculate_track_evaluation_state(),
            std::memory_order_relaxed
        );
        g_diagnostic_final_sample_rate_hz.store(
            m_sample_rate,
            std::memory_order_relaxed
        );
        g_diagnostic_final_cpu_load_percent.store(
            m_cpu_load_percent,
            std::memory_order_relaxed
        );
        g_diagnostic_final_summary_valid.store(
            1,
            std::memory_order_relaxed
        );
    }

    void publish_diagnostics(
        double chunk_true_peak,
        double last_applied_gain_db
    ) {
        const bool limiter_active =
            m_settings.enable_peak_guard &&
            m_limiter_gain_db < -0.01;

        g_diagnostic_momentary_lufs.store(
            m_momentary_lufs,
            std::memory_order_relaxed
        );
        g_diagnostic_short_term_lufs.store(
            m_short_term_lufs,
            std::memory_order_relaxed
        );
        g_diagnostic_integrated_lufs.store(
            m_integrated_lufs,
            std::memory_order_relaxed
        );
        g_diagnostic_output_integrated_lufs.store(
            m_output_integrated_lufs,
            std::memory_order_relaxed
        );

        double target_difference_lu = -200.0;
        if (std::isfinite(m_output_integrated_lufs) &&
            m_output_integrated_lufs > -190.0) {
            target_difference_lu =
                m_output_integrated_lufs -
                static_cast<double>(m_settings.target_lufs);
        }

        g_diagnostic_target_difference_lu.store(
            target_difference_lu,
            std::memory_order_relaxed
        );
        g_diagnostic_lra_lu.store(
            m_lra_lu,
            std::memory_order_relaxed
        );
        g_diagnostic_normalization_gain_db.store(
            m_current_gain_db,
            std::memory_order_relaxed
        );
        g_diagnostic_applied_gain_db.store(
            last_applied_gain_db,
            std::memory_order_relaxed
        );
        g_diagnostic_limiter_reduction_db.store(
            m_limiter_gain_db,
            std::memory_order_relaxed
        );
        g_diagnostic_compressor_reduction_db.store(
            m_settings.enable_modern_boost
                ? m_modern_compressor_gain_db
                : 0.0,
            std::memory_order_relaxed
        );
        g_diagnostic_clipper_reduction_db.store(
            m_settings.enable_modern_boost
                ? -m_modern_clipper_reduction_db
                : 0.0,
            std::memory_order_relaxed
        );
        g_diagnostic_modern_boost_state.store(
            m_settings.enable_modern_boost ? 1 : 0,
            std::memory_order_relaxed
        );
        g_diagnostic_processing_risk_state.store(
            m_settings.enable_modern_boost
                ? m_processing_risk_state
                : 0,
            std::memory_order_relaxed
        );
        g_diagnostic_safety_reduction_db.store(
            m_settings.enable_modern_boost
                ? m_safety_reduction_db
                : 0.0,
            std::memory_order_relaxed
        );
        g_diagnostic_original_compare_state.store(
            g_original_compare_request.load(
                std::memory_order_relaxed
            ),
            std::memory_order_relaxed
        );
        g_diagnostic_compare_match_gain_db.store(
            m_compare_match_gain_db,
            std::memory_order_relaxed
        );
        g_diagnostic_adaptive_master_state.store(
            m_settings.enable_adaptive_master ? 1 : 0,
            std::memory_order_relaxed
        );
        g_diagnostic_three_band_master_state.store(
            m_settings.enable_three_band_master ? 1 : 0,
            std::memory_order_relaxed
        );
        g_diagnostic_effective_strength_percent.store(
            m_settings.enable_adaptive_master
                ? m_effective_modern_strength_percent
                : static_cast<double>(m_settings.modern_strength_percent),
            std::memory_order_relaxed
        );
        g_diagnostic_three_band_low_reduction_db.store(
            m_settings.enable_three_band_master
                ? -m_three_band_low_gain_db
                : 0.0,
            std::memory_order_relaxed
        );
        g_diagnostic_three_band_mid_reduction_db.store(
            m_settings.enable_three_band_master
                ? -m_three_band_mid_gain_db
                : 0.0,
            std::memory_order_relaxed
        );
        g_diagnostic_three_band_high_reduction_db.store(
            m_settings.enable_three_band_master
                ? -m_three_band_high_gain_db
                : 0.0,
            std::memory_order_relaxed
        );
        g_diagnostic_track_max_true_peak_dbtp.store(
            linear_to_db(m_track_max_output_true_peak_linear),
            std::memory_order_relaxed
        );
        g_diagnostic_track_max_compressor_reduction_db.store(
            m_track_max_compressor_reduction_db,
            std::memory_order_relaxed
        );
        g_diagnostic_track_max_clipper_reduction_db.store(
            m_track_max_clipper_reduction_db,
            std::memory_order_relaxed
        );
        g_diagnostic_track_max_limiter_reduction_db.store(
            m_track_max_limiter_reduction_db,
            std::memory_order_relaxed
        );
        g_diagnostic_track_max_three_band_low_reduction_db.store(
            m_track_max_three_band_low_reduction_db,
            std::memory_order_relaxed
        );
        g_diagnostic_track_max_three_band_mid_reduction_db.store(
            m_track_max_three_band_mid_reduction_db,
            std::memory_order_relaxed
        );
        g_diagnostic_track_max_three_band_high_reduction_db.store(
            m_track_max_three_band_high_reduction_db,
            std::memory_order_relaxed
        );
        g_diagnostic_clip_event_count.store(
            m_clip_event_count,
            std::memory_order_relaxed
        );
        g_diagnostic_recovered_sample_count.store(
            m_recovered_sample_count,
            std::memory_order_relaxed
        );
        g_diagnostic_track_evaluation_state.store(
            m_track_evaluation_state,
            std::memory_order_relaxed
        );
        g_diagnostic_sample_rate_hz.store(
            m_sample_rate,
            std::memory_order_relaxed
        );
        g_diagnostic_cpu_load_percent.store(
            m_cpu_load_percent,
            std::memory_order_relaxed
        );
        g_diagnostic_true_peak_dbtp.store(
            linear_to_db(chunk_true_peak),
            std::memory_order_relaxed
        );
        g_diagnostic_peak_guard_state.store(
            m_settings.enable_peak_guard
                ? (limiter_active ? 2 : 1)
                : 0,
            std::memory_order_relaxed
        );
        g_diagnostic_latency_ms.store(
            get_latency() * 1000.0,
            std::memory_order_relaxed
        );
        g_diagnostic_normalization_state.store(
            m_normalization_state,
            std::memory_order_relaxed
        );
        g_diagnostic_gain_lock_state.store(
            m_gain_lock_state,
            std::memory_order_relaxed
        );
        g_diagnostic_gain_lock_remaining_seconds.store(
            m_gain_lock_remaining_seconds,
            std::memory_order_relaxed
        );
        g_diagnostic_locked_gain_db.store(
            m_locked_gain_db,
            std::memory_order_relaxed
        );
        g_diagnostic_last_update_tick.store(
            static_cast<unsigned long long>(GetTickCount64()),
            std::memory_order_relaxed
        );
    }

    void reset_measurement_state_only() {
        for (auto& filter : m_filters) {
            filter.pre_filter.reset();
            filter.rlb_filter.reset();
        }

        for (auto& filter : m_output_filters) {
            filter.pre_filter.reset();
            filter.rlb_filter.reset();
        }

        std::fill(m_energy_ring.begin(), m_energy_ring.end(), 0.0);
        std::fill(
            m_short_term_energy_ring.begin(),
            m_short_term_energy_ring.end(),
            0.0
        );
        std::fill(
            m_output_energy_ring.begin(),
            m_output_energy_ring.end(),
            0.0
        );
        std::fill(
            m_output_short_term_energy_ring.begin(),
            m_output_short_term_energy_ring.end(),
            0.0
        );

        m_gated_block_energies.clear();
        m_output_gated_block_energies.clear();
        m_lra_short_term_samples.clear();

        m_energy_position = 0;
        m_energy_count = 0;
        m_energy_sum = 0.0;
        m_frames_since_block_update = 0;
        m_processed_frames = 0;

        m_output_energy_position = 0;
        m_output_energy_count = 0;
        m_output_energy_sum = 0.0;
        m_output_frames_since_block_update = 0;
        m_output_integrated_lufs = -200.0;
        m_output_short_term_energy_position = 0;
        m_output_short_term_energy_count = 0;
        m_output_short_term_energy_sum = 0.0;
        m_output_short_term_lufs = -200.0;

        m_short_term_energy_position = 0;
        m_short_term_energy_count = 0;
        m_short_term_energy_sum = 0.0;
        m_blocks_since_lra_sample = 0;

        m_momentary_lufs = -200.0;
        m_short_term_lufs = -200.0;
        m_integrated_lufs = -200.0;
        m_lra_lu = -200.0;

        for (auto& history : m_output_true_peak_history) {
            std::fill(history.begin(), history.end(), 0.0);
        }
        for (auto& history : m_compare_true_peak_history) {
            std::fill(history.begin(), history.end(), 0.0);
        }
        m_track_max_output_true_peak_linear = 0.0;
        m_track_max_compressor_reduction_db = 0.0;
        m_track_max_clipper_reduction_db = 0.0;
        m_track_max_limiter_reduction_db = 0.0;
        m_track_max_three_band_low_reduction_db = 0.0;
        m_track_max_three_band_mid_reduction_db = 0.0;
        m_track_max_three_band_high_reduction_db = 0.0;
        m_clip_event_count = 0;
        m_recovered_sample_count = 0;
        m_clip_event_active = false;
        m_track_evaluation_state = 0;
        m_cpu_load_percent = 0.0;
        m_cpu_load_initialized = false;

        // Preserve the currently audible gain to avoid a sudden jump.
        m_target_gain_db = m_current_gain_db;
        m_normalization_state =
            m_settings.startup_analysis_seconds > 0.0f ? 1 : 3;

        m_gain_locked = false;
        m_gain_lock_reference_valid = false;
        m_gain_lock_reference_db = 0.0;
        m_locked_gain_db = 0.0;
        m_gain_lock_stable_seconds = 0.0;
        m_gain_lock_remaining_seconds =
            m_settings.enable_gain_lock
                ? static_cast<double>(m_settings.gain_lock_seconds)
                : 0.0;
        m_gain_lock_state = m_settings.enable_gain_lock ? 1 : 0;

        g_diagnostic_momentary_lufs.store(
            -200.0,
            std::memory_order_relaxed
        );
        g_diagnostic_short_term_lufs.store(
            -200.0,
            std::memory_order_relaxed
        );
        g_diagnostic_integrated_lufs.store(
            -200.0,
            std::memory_order_relaxed
        );
        g_diagnostic_output_integrated_lufs.store(
            -200.0,
            std::memory_order_relaxed
        );
        g_diagnostic_target_difference_lu.store(
            -200.0,
            std::memory_order_relaxed
        );
        g_diagnostic_lra_lu.store(
            -200.0,
            std::memory_order_relaxed
        );
        g_diagnostic_track_max_true_peak_dbtp.store(
            -200.0,
            std::memory_order_relaxed
        );
        g_diagnostic_track_max_compressor_reduction_db.store(
            0.0,
            std::memory_order_relaxed
        );
        g_diagnostic_track_max_clipper_reduction_db.store(
            0.0,
            std::memory_order_relaxed
        );
        g_diagnostic_track_max_limiter_reduction_db.store(
            0.0,
            std::memory_order_relaxed
        );
        g_diagnostic_track_max_three_band_low_reduction_db.store(
            0.0,
            std::memory_order_relaxed
        );
        g_diagnostic_track_max_three_band_mid_reduction_db.store(
            0.0,
            std::memory_order_relaxed
        );
        g_diagnostic_track_max_three_band_high_reduction_db.store(
            0.0,
            std::memory_order_relaxed
        );
        g_diagnostic_clip_event_count.store(
            0,
            std::memory_order_relaxed
        );
        g_diagnostic_recovered_sample_count.store(
            0,
            std::memory_order_relaxed
        );
        g_diagnostic_track_evaluation_state.store(
            0,
            std::memory_order_relaxed
        );
        g_diagnostic_cpu_load_percent.store(
            0.0,
            std::memory_order_relaxed
        );
        g_diagnostic_normalization_state.store(
            m_normalization_state,
            std::memory_order_relaxed
        );
        g_diagnostic_gain_lock_state.store(
            m_gain_lock_state,
            std::memory_order_relaxed
        );
        g_diagnostic_gain_lock_remaining_seconds.store(
            m_gain_lock_remaining_seconds,
            std::memory_order_relaxed
        );
        g_diagnostic_locked_gain_db.store(
            0.0,
            std::memory_order_relaxed
        );
    }

    void reset_runtime_state() {
        for (auto& filter : m_filters) {
            filter.pre_filter.reset();
            filter.rlb_filter.reset();
        }

        for (auto& filter : m_output_filters) {
            filter.pre_filter.reset();
            filter.rlb_filter.reset();
        }

        std::fill(m_energy_ring.begin(), m_energy_ring.end(), 0.0);
        std::fill(
            m_short_term_energy_ring.begin(),
            m_short_term_energy_ring.end(),
            0.0
        );
        std::fill(
            m_output_energy_ring.begin(),
            m_output_energy_ring.end(),
            0.0
        );
        std::fill(
            m_output_short_term_energy_ring.begin(),
            m_output_short_term_energy_ring.end(),
            0.0
        );

        for (auto& history : m_true_peak_history) {
            std::fill(history.begin(), history.end(), 0.0);
        }
        for (auto& history : m_output_true_peak_history) {
            std::fill(history.begin(), history.end(), 0.0);
        }
        for (auto& history : m_compare_true_peak_history) {
            std::fill(history.begin(), history.end(), 0.0);
        }

        std::fill(
            m_clipper_previous_input.begin(),
            m_clipper_previous_input.end(),
            0.0
        );
        std::fill(
            m_clipper_previous_valid.begin(),
            m_clipper_previous_valid.end(),
            false
        );

        for (auto& state : m_three_band_split_states) {
            state.reset();
        }

        m_gated_block_energies.clear();
        m_output_gated_block_energies.clear();
        m_lra_short_term_samples.clear();
        clear_delay_state();

        m_energy_position = 0;
        m_energy_count = 0;
        m_energy_sum = 0.0;
        m_frames_since_block_update = 0;
        m_processed_frames = 0;

        m_output_energy_position = 0;
        m_output_energy_count = 0;
        m_output_energy_sum = 0.0;
        m_output_frames_since_block_update = 0;
        m_output_integrated_lufs = -200.0;
        m_output_short_term_energy_position = 0;
        m_output_short_term_energy_count = 0;
        m_output_short_term_energy_sum = 0.0;
        m_output_short_term_lufs = -200.0;

        m_short_term_energy_position = 0;
        m_short_term_energy_count = 0;
        m_short_term_energy_sum = 0.0;
        m_blocks_since_lra_sample = 0;

        m_momentary_lufs = -200.0;
        m_short_term_lufs = -200.0;
        m_integrated_lufs = -200.0;
        m_lra_lu = -200.0;
        m_target_gain_db = 0.0;
        m_current_gain_db = 0.0;
        m_limiter_gain_db = 0.0;
        m_compare_limiter_gain_db = 0.0;
        m_compare_match_gain_db = 0.0;
        m_last_compare_request_mode = 0;
        m_modern_compressor_gain_db = 0.0;
        m_three_band_low_gain_db = 0.0;
        m_three_band_mid_gain_db = 0.0;
        m_three_band_high_gain_db = 0.0;
        m_modern_clipper_reduction_db = 0.0;
        m_safety_reduction_db = 0.0;
        m_strong_processing_seconds = 0.0;
        m_excessive_processing_seconds = 0.0;
        m_processing_risk_state =
            m_settings.enable_modern_boost ? 1 : 0;
        m_effective_modern_strength_percent =
            static_cast<double>(m_settings.modern_strength_percent);
        m_track_max_output_true_peak_linear = 0.0;
        m_track_max_compressor_reduction_db = 0.0;
        m_track_max_clipper_reduction_db = 0.0;
        m_track_max_limiter_reduction_db = 0.0;
        m_track_max_three_band_low_reduction_db = 0.0;
        m_track_max_three_band_mid_reduction_db = 0.0;
        m_track_max_three_band_high_reduction_db = 0.0;
        m_clip_event_count = 0;
        m_recovered_sample_count = 0;
        m_clip_event_active = false;
        m_track_evaluation_state = 0;
        m_cpu_load_percent = 0.0;
        m_cpu_load_initialized = false;
        m_last_emitted_compare_mode_valid = false;
        m_compare_fade_in_remaining = 0;
        m_normalization_state =
            m_settings.startup_analysis_seconds > 0.0f ? 1 : 3;

        m_gain_locked = false;
        m_gain_lock_reference_valid = false;
        m_gain_lock_reference_db = 0.0;
        m_locked_gain_db = 0.0;
        m_gain_lock_stable_seconds = 0.0;
        m_gain_lock_remaining_seconds =
            m_settings.enable_gain_lock
                ? static_cast<double>(m_settings.gain_lock_seconds)
                : 0.0;
        m_gain_lock_state = m_settings.enable_gain_lock ? 1 : 0;

        g_diagnostic_momentary_lufs.store(
            -200.0,
            std::memory_order_relaxed
        );
        g_diagnostic_short_term_lufs.store(
            -200.0,
            std::memory_order_relaxed
        );
        g_diagnostic_integrated_lufs.store(
            -200.0,
            std::memory_order_relaxed
        );
        g_diagnostic_output_integrated_lufs.store(
            -200.0,
            std::memory_order_relaxed
        );
        g_diagnostic_target_difference_lu.store(
            -200.0,
            std::memory_order_relaxed
        );
        g_diagnostic_lra_lu.store(
            -200.0,
            std::memory_order_relaxed
        );
        g_diagnostic_normalization_gain_db.store(
            0.0,
            std::memory_order_relaxed
        );
        g_diagnostic_applied_gain_db.store(
            0.0,
            std::memory_order_relaxed
        );
        g_diagnostic_limiter_reduction_db.store(
            0.0,
            std::memory_order_relaxed
        );
        g_diagnostic_compressor_reduction_db.store(
            0.0,
            std::memory_order_relaxed
        );
        g_diagnostic_clipper_reduction_db.store(
            0.0,
            std::memory_order_relaxed
        );
        g_diagnostic_modern_boost_state.store(
            m_settings.enable_modern_boost ? 1 : 0,
            std::memory_order_relaxed
        );
        g_diagnostic_processing_risk_state.store(
            m_processing_risk_state,
            std::memory_order_relaxed
        );
        g_diagnostic_safety_reduction_db.store(
            0.0,
            std::memory_order_relaxed
        );
        g_diagnostic_original_compare_state.store(
            g_original_compare_request.load(
                std::memory_order_relaxed
            ),
            std::memory_order_relaxed
        );
        g_diagnostic_compare_match_gain_db.store(
            m_compare_match_gain_db,
            std::memory_order_relaxed
        );
        g_diagnostic_adaptive_master_state.store(
            m_settings.enable_adaptive_master ? 1 : 0,
            std::memory_order_relaxed
        );
        g_diagnostic_three_band_master_state.store(
            m_settings.enable_three_band_master ? 1 : 0,
            std::memory_order_relaxed
        );
        g_diagnostic_effective_strength_percent.store(
            static_cast<double>(m_settings.modern_strength_percent),
            std::memory_order_relaxed
        );
        g_diagnostic_three_band_low_reduction_db.store(
            0.0,
            std::memory_order_relaxed
        );
        g_diagnostic_three_band_mid_reduction_db.store(
            0.0,
            std::memory_order_relaxed
        );
        g_diagnostic_three_band_high_reduction_db.store(
            0.0,
            std::memory_order_relaxed
        );
        g_diagnostic_track_max_true_peak_dbtp.store(
            -200.0,
            std::memory_order_relaxed
        );
        g_diagnostic_track_max_compressor_reduction_db.store(
            0.0,
            std::memory_order_relaxed
        );
        g_diagnostic_track_max_clipper_reduction_db.store(
            0.0,
            std::memory_order_relaxed
        );
        g_diagnostic_track_max_limiter_reduction_db.store(
            0.0,
            std::memory_order_relaxed
        );
        g_diagnostic_track_max_three_band_low_reduction_db.store(
            0.0,
            std::memory_order_relaxed
        );
        g_diagnostic_track_max_three_band_mid_reduction_db.store(
            0.0,
            std::memory_order_relaxed
        );
        g_diagnostic_track_max_three_band_high_reduction_db.store(
            0.0,
            std::memory_order_relaxed
        );
        g_diagnostic_clip_event_count.store(
            0,
            std::memory_order_relaxed
        );
        g_diagnostic_recovered_sample_count.store(
            0,
            std::memory_order_relaxed
        );
        g_diagnostic_track_evaluation_state.store(
            0,
            std::memory_order_relaxed
        );
        g_diagnostic_sample_rate_hz.store(
            m_sample_rate,
            std::memory_order_relaxed
        );
        g_diagnostic_cpu_load_percent.store(
            0.0,
            std::memory_order_relaxed
        );
        g_diagnostic_true_peak_dbtp.store(
            -200.0,
            std::memory_order_relaxed
        );
        g_diagnostic_peak_guard_state.store(
            m_settings.enable_peak_guard ? 1 : 0,
            std::memory_order_relaxed
        );
        g_diagnostic_latency_ms.store(
            get_latency() * 1000.0,
            std::memory_order_relaxed
        );
        g_diagnostic_normalization_state.store(
            0,
            std::memory_order_relaxed
        );
        g_diagnostic_gain_lock_state.store(
            m_settings.enable_gain_lock ? 1 : 0,
            std::memory_order_relaxed
        );
        g_diagnostic_gain_lock_remaining_seconds.store(
            m_gain_lock_remaining_seconds,
            std::memory_order_relaxed
        );
        g_diagnostic_locked_gain_db.store(
            0.0,
            std::memory_order_relaxed
        );
        g_diagnostic_stream_active.store(
            0,
            std::memory_order_relaxed
        );
        g_diagnostic_last_update_tick.store(
            0,
            std::memory_order_relaxed
        );
    }

    r128_settings m_settings;

    unsigned m_sample_rate = 0;
    unsigned m_channels = 0;
    unsigned m_channel_mask = 0;
    bool m_layout_has_lfe = false;

    std::vector<double> m_channel_energy_weights;
    std::vector<channel_filter_state> m_filters;
    std::vector<channel_filter_state> m_output_filters;

    std::vector<double> m_energy_ring;
    t_size m_block_window_frames = 0;
    t_size m_block_hop_frames = 1;
    t_size m_energy_position = 0;
    t_size m_energy_count = 0;
    t_size m_frames_since_block_update = 0;
    unsigned long long m_processed_frames = 0;
    double m_energy_sum = 0.0;

    std::vector<double> m_output_energy_ring;
    std::vector<double> m_output_short_term_energy_ring;
    t_size m_output_energy_position = 0;
    t_size m_output_energy_count = 0;
    t_size m_output_frames_since_block_update = 0;
    double m_output_energy_sum = 0.0;
    t_size m_output_short_term_energy_position = 0;
    t_size m_output_short_term_energy_count = 0;
    double m_output_short_term_energy_sum = 0.0;
    std::vector<double> m_output_gated_block_energies;
    double m_output_integrated_lufs = -200.0;
    double m_output_short_term_lufs = -200.0;

    std::vector<double> m_short_term_energy_ring;
    t_size m_short_term_window_frames = 0;
    t_size m_short_term_energy_position = 0;
    t_size m_short_term_energy_count = 0;
    double m_short_term_energy_sum = 0.0;

    std::vector<double> m_gated_block_energies;
    std::vector<double> m_lra_short_term_samples;
    t_size m_blocks_since_lra_sample = 0;
    double m_momentary_lufs = -200.0;
    double m_short_term_lufs = -200.0;
    double m_integrated_lufs = -200.0;
    double m_lra_lu = -200.0;

    std::vector<std::vector<double>> m_true_peak_coefficients;
    std::vector<std::vector<double>> m_true_peak_history;
    std::vector<std::vector<double>> m_output_true_peak_history;
    std::vector<std::vector<double>> m_compare_true_peak_history;
    std::vector<audio_sample> m_modern_frame;
    std::vector<audio_sample> m_output_measure_frame;
    std::vector<three_band_split_state> m_three_band_split_states;
    std::vector<double> m_three_band_low_frame;
    std::vector<double> m_three_band_mid_frame;
    std::vector<double> m_three_band_high_frame;
    std::vector<double> m_clipper_previous_input;
    std::vector<bool> m_clipper_previous_valid;

    t_size m_lookahead_frames = 0;
    std::deque<audio_sample> m_delay_audio;
    std::deque<int> m_delay_compare_mode;
    std::deque<double> m_delay_compare_gain_db;
    std::deque<peak_queue_node> m_peak_max_queue;
    unsigned long long m_next_frame_id = 0;
    unsigned long long m_oldest_frame_id = 0;

    double m_target_gain_db = 0.0;
    double m_current_gain_db = 0.0;
    double m_limiter_gain_db = 0.0;
    double m_compare_limiter_gain_db = 0.0;
    double m_compare_match_gain_db = 0.0;
    int m_last_compare_request_mode = 0;
    double m_modern_compressor_gain_db = 0.0;
    double m_three_band_low_gain_db = 0.0;
    double m_three_band_mid_gain_db = 0.0;
    double m_three_band_high_gain_db = 0.0;
    double m_three_band_low_coefficient = 0.0;
    double m_three_band_high_coefficient = 0.0;
    double m_modern_clipper_reduction_db = 0.0;
    double m_safety_reduction_db = 0.0;
    double m_effective_modern_strength_percent = 50.0;
    double m_track_max_output_true_peak_linear = 0.0;
    double m_track_max_compressor_reduction_db = 0.0;
    double m_track_max_clipper_reduction_db = 0.0;
    double m_track_max_limiter_reduction_db = 0.0;
    double m_track_max_three_band_low_reduction_db = 0.0;
    double m_track_max_three_band_mid_reduction_db = 0.0;
    double m_track_max_three_band_high_reduction_db = 0.0;
    unsigned long long m_clip_event_count = 0;
    unsigned long long m_recovered_sample_count = 0;
    bool m_clip_event_active = false;
    int m_track_evaluation_state = 0;
    double m_cpu_load_percent = 0.0;
    bool m_cpu_load_initialized = false;
    int m_last_emitted_compare_mode = 0;
    bool m_last_emitted_compare_mode_valid = false;
    t_size m_compare_fade_in_remaining = 0;
    double m_strong_processing_seconds = 0.0;
    double m_excessive_processing_seconds = 0.0;
    int m_processing_risk_state = 0;
    int m_normalization_state = 0;

    bool m_gain_locked = false;
    bool m_gain_lock_reference_valid = false;
    double m_gain_lock_reference_db = 0.0;
    double m_locked_gain_db = 0.0;
    double m_gain_lock_stable_seconds = 0.0;
    double m_gain_lock_remaining_seconds = 0.0;
    int m_gain_lock_state = 0;
    unsigned long long m_last_measurement_reset_request = 0;
};

static dsp_factory_t<dsp_r128_normalizer> g_dsp_r128_normalizer_factory;

namespace {

// Main-menu command:
// Playback -> R128 音量ノーマライザーの設定...
static const GUID guid_mainmenu_open_r128_settings =
{ 0xd824be70, 0x4953, 0x464b, { 0xbc, 0x67, 0xce, 0x36, 0x7d, 0x79, 0x03, 0x83 } };

class direct_r128_preset_callback final
    : public dsp_preset_edit_callback {
public:
    void set_dialog(HWND dialog) {
        m_dialog = dialog;
    }

    void on_preset_changed(
        const dsp_preset& new_preset
    ) override {
        static_api_ptr_t<dsp_config_manager> manager;
        dsp_chain_config_impl chain;
        manager->get_core_settings(chain);

        t_size match_count = 0;
        t_size match_index = 0;

        for (t_size index = 0;
             index < chain.get_count();
             ++index) {
            if (chain.get_item(index).get_owner() ==
                guid_r128_normalizer) {
                match_index = index;
                ++match_count;
            }
        }

        if (match_count != 1) {
            MessageBoxW(
                m_dialog,
                L"設定画面を開いている間にDSPチェーンが"
                L"変更されたため、設定を適用できませんでした。\n\n"
                L"DSPの登録状態を確認して、もう一度お試しください。",
                L"R128 音量ノーマライザー",
                MB_OK | MB_ICONWARNING
            );
            return;
        }

        chain.replace_item(new_preset, match_index);
        manager->set_core_settings(chain);
    }

private:
    HWND m_dialog = nullptr;
};

HWND g_direct_r128_settings_window = nullptr;

void cleanup_direct_r128_dialog(
    dialog_context* context
) {
    if (context == nullptr) {
        return;
    }

    delete static_cast<direct_r128_preset_callback*>(
        context->cleanup_state
    );
    delete context;
}

void activate_existing_direct_r128_dialog() {
    if (!IsWindow(g_direct_r128_settings_window)) {
        g_direct_r128_settings_window = nullptr;
        return;
    }

    if (IsIconic(g_direct_r128_settings_window)) {
        ShowWindow(
            g_direct_r128_settings_window,
            SW_RESTORE
        );
    }
    else {
        ShowWindow(
            g_direct_r128_settings_window,
            SW_SHOW
        );
    }

    SetForegroundWindow(g_direct_r128_settings_window);
}

void show_r128_settings_from_main_menu() {
    if (IsWindow(g_direct_r128_settings_window)) {
        activate_existing_direct_r128_dialog();
        return;
    }

    static_api_ptr_t<dsp_config_manager> manager;
    dsp_chain_config_impl chain;
    manager->get_core_settings(chain);

    t_size match_count = 0;
    t_size match_index = 0;

    for (t_size index = 0;
         index < chain.get_count();
         ++index) {
        if (chain.get_item(index).get_owner() ==
            guid_r128_normalizer) {
            match_index = index;
            ++match_count;
        }
    }

    const HWND owner = core_api::get_main_window();

    if (match_count == 0) {
        MessageBoxW(
            owner,
            L"R128 音量ノーマライザーは、現在のDSPチェーンに"
            L"追加されていません。\n\n"
            L"Playback → DSP Managerで追加してから、"
            L"もう一度このメニューを開いてください。",
            L"R128 音量ノーマライザー",
            MB_OK | MB_ICONINFORMATION
        );
        return;
    }

    if (match_count > 1) {
        MessageBoxW(
            owner,
            L"現在のDSPチェーンにR128 音量ノーマライザーが"
            L"複数登録されています。\n\n"
            L"誤った設定を変更しないため、DSP Managerから"
            L"対象を選んで設定してください。",
            L"R128 音量ノーマライザー",
            MB_OK | MB_ICONWARNING
        );
        return;
    }

    const dsp_preset_impl preset(
        chain.get_item(match_index)
    );

    auto* callback =
        new direct_r128_preset_callback();
    auto* context =
        new dialog_context();

    context->value = parse_preset(preset);
    context->callback = callback;
    context->modeless = true;
    context->tracked_window =
        &g_direct_r128_settings_window;
    context->cleanup_state = callback;
    context->cleanup =
        cleanup_direct_r128_dialog;

    // Owned modeless dialog:
    // stays above foobar2000 without disabling it,
    // but is not topmost over unrelated applications.
    const HWND dialog = CreateDialogParamW(
        core_api::get_my_instance(),
        MAKEINTRESOURCEW(IDD_R128_CONFIG),
        owner,
        config_dialog_proc,
        reinterpret_cast<LPARAM>(context)
    );

    if (dialog == nullptr) {
        cleanup_direct_r128_dialog(context);

        MessageBoxW(
            owner,
            L"設定画面を作成できませんでした。",
            L"R128 音量ノーマライザー",
            MB_OK | MB_ICONERROR
        );
        return;
    }

    g_direct_r128_settings_window = dialog;
    callback->set_dialog(dialog);

    ShowWindow(dialog, SW_SHOW);
    SetForegroundWindow(dialog);
}

class mainmenu_commands_r128_settings
    : public mainmenu_commands {
public:
    t_uint32 get_command_count() override {
        return 1;
    }

    GUID get_command(t_uint32 index) override {
        if (index != 0) {
            uBugCheck();
        }

        return guid_mainmenu_open_r128_settings;
    }

    void get_name(
        t_uint32 index,
        pfc::string_base& out
    ) override {
        if (index != 0) {
            uBugCheck();
        }

        out =
            "R128 \xE9\x9F\xB3\xE9\x87\x8F"
            "\xE3\x83\x8E\xE3\x83\xBC\xE3\x83\x9E"
            "\xE3\x83\xA9\xE3\x82\xA4\xE3\x82\xB6"
            "\xE3\x83\xBC\xE3\x81\xAE\xE8\xA8\xAD"
            "\xE5\xAE\x9A...";
    }

    bool get_description(
        t_uint32 index,
        pfc::string_base& out
    ) override {
        if (index != 0) {
            uBugCheck();
        }

        out =
            "R128 \xE9\x9F\xB3\xE9\x87\x8F"
            "\xE3\x83\x8E\xE3\x83\xBC\xE3\x83\x9E"
            "\xE3\x83\xA9\xE3\x82\xA4\xE3\x82\xB6"
            "\xE3\x83\xBC\xE3\x81\xAE\xE8\xA8\xAD"
            "\xE5\xAE\x9A\xE7\x94\xBB\xE9\x9D\xA2"
            "\xE3\x82\x92\xE7\x9B\xB4\xE6\x8E\xA5"
            "\xE9\x96\x8B\xE3\x81\x8D\xE3\x81\xBE"
            "\xE3\x81\x99\xE3\x80\x82";
        return true;
    }

    GUID get_parent() override {
        return mainmenu_groups::playback;
    }

    void execute(
        t_uint32 index,
        service_ptr_t<service_base> callback
    ) override {
        (void)callback;

        if (index != 0) {
            uBugCheck();
        }

        show_r128_settings_from_main_menu();
    }
};

static mainmenu_commands_factory_t<
    mainmenu_commands_r128_settings
> g_mainmenu_commands_r128_settings_factory;

} // namespace
