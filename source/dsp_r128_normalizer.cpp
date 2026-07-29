#include "stdafx.h"

namespace {

static const GUID guid_r128_normalizer =
{ 0x4d3f8b85, 0x61d2, 0x47f4, { 0xa8, 0x51, 0x7c, 0x1b, 0x3d, 0xd7, 0x92, 0x46 } };

// Separate component-level setting. This deliberately does not change the
// DSP preset serialization format (kPresetVersion remains v7).
static const GUID guid_cfg_display_language =
{ 0x13f3c74b, 0xf11d, 0x4db5, { 0xa9, 0x6d, 0xe7, 0x7f, 0xc8, 0x90, 0x6c, 0x52 } };

static const GUID guid_cfg_auto_control_history =
{ 0x8f971df3, 0xe9c8, 0x4b37, { 0xb5, 0x38, 0x2f, 0x65, 0xe3, 0x8a, 0x54, 0x91 } };

enum class display_language : int {
    automatic = 0,
    japanese = 1,
    english = 2
};

cfg_int g_cfg_display_language(
    guid_cfg_display_language,
    static_cast<int>(display_language::automatic)
);

cfg_string g_cfg_auto_control_history(
    guid_cfg_auto_control_history,
    ""
);

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
constexpr double kAutoSafetyMaximumReductionDb = 6.0;
constexpr double kAutoSafetyAttackSeconds = 0.250;
constexpr double kAutoSafetyReleaseSeconds = 4.000;
constexpr double kAutoSafetyTriggerSeconds = 0.750;
constexpr double kAutoSafetySafeHoldSeconds = 3.000;
constexpr double kAutoSafetyLimitHoldSeconds = 0.750;
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
constexpr UINT_PTR kTrendDialogTimerId = 3;
constexpr UINT kTrendDialogRefreshMilliseconds = 500;
constexpr t_size kMaximumTrendSamples = 21600;
constexpr t_size kMaximumAutoControlHistoryEntries = 100;
constexpr t_size kMaximumHistoryMetadataCharacters = 512;

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
std::atomic<int> g_diagnostic_current_processing_state(0);
std::atomic<int> g_diagnostic_auto_control_reason_mask(0);
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
std::atomic<unsigned> g_history_auto_control_trigger_count(0);
std::atomic<int> g_history_auto_control_reason_mask(0);
std::atomic<double> g_history_max_auto_attenuation_db(0.0);
std::atomic<int> g_history_adjustment_limit_reached(0);
std::atomic<int> g_history_recovered(0);
std::atomic<int> g_history_profile_id(-1);
std::atomic<int> g_history_latest_auto_control_reason_mask(0);
std::atomic<unsigned> g_history_final_auto_control_trigger_count(0);
std::atomic<int> g_history_final_auto_control_reason_mask(0);
std::atomic<double> g_history_final_max_auto_attenuation_db(0.0);
std::atomic<int> g_history_final_adjustment_limit_reached(0);
std::atomic<int> g_history_final_recovered(0);
std::atomic<int> g_history_final_profile_id(-1);
std::atomic<unsigned long long> g_history_final_session_id(0);
std::atomic<unsigned long long> g_history_active_session_id(0);
std::atomic<unsigned long long> g_history_metrics_session_id(0);
std::atomic<unsigned long long> g_history_track_reset_request(0);

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

display_language configured_display_language() {
    const int value = static_cast<int>(
        g_cfg_display_language.get()
    );

    if (value == static_cast<int>(display_language::japanese)) {
        return display_language::japanese;
    }
    if (value == static_cast<int>(display_language::english)) {
        return display_language::english;
    }

    return display_language::automatic;
}

bool ui_uses_english() {
    const display_language configured =
        configured_display_language();

    if (configured == display_language::english) {
        return true;
    }
    if (configured == display_language::japanese) {
        return false;
    }

    return PRIMARYLANGID(GetUserDefaultUILanguage()) !=
        LANG_JAPANESE;
}

const wchar_t* ui_text(
    const wchar_t* japanese,
    const wchar_t* english
) {
    return ui_uses_english() ? english : japanese;
}

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
    const bool english = ui_uses_english();

    switch (profile) {
    case recognized_profile::standard:
        return english ? L"Natural -18" : L"ナチュラル -18";
    case recognized_profile::streaming:
        return english
            ? L"Power Boost -14"
            : L"パワーブースト -14";
    case recognized_profile::broadcast:
        return english ? L"Relaxed -23" : L"リラックス -23";
    case recognized_profile::night:
        return english ? L"Night Safe -22" : L"ナイトセーフ -22";
    case recognized_profile::modern:
        return english
            ? L"Modern Boost -9"
            : L"モダンブースト -9";
    case recognized_profile::adaptive:
        return english
            ? L"1-Band Adaptive -10"
            : L"1バンド・アダプティブ -10";
    case recognized_profile::three_band:
        return english
            ? L"3-Band Adaptive -10"
            : L"3バンド・アダプティブ -10";
    default:
        return english ? L"Custom Settings" : L"カスタム設定";
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
        pending
            ? ui_text(L"選択: %s", L"Selected: %s")
            : ui_text(L"現在: %s", L"Current: %s"),
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
        return ui_text(L"適正", L"Normal");
    case 2:
        return ui_text(L"強め", L"Strong");
    case 3:
        return ui_text(
            L"過剰・自動保護中",
            L"Excessive / auto protection"
        );
    default:
        return ui_text(L"無効", L"Disabled");
    }
}

const wchar_t* track_evaluation_to_text(int state) {
    switch (state) {
    case 1:
        return ui_text(L"安全", L"Safe");
    case 2:
        return ui_text(L"強め", L"Strong");
    case 3:
        return ui_text(L"要調整", L"Needs adjustment");
    default:
        return ui_text(L"未測定", L"Not measured");
    }
}

const wchar_t* current_processing_state_to_text(int state) {
    switch (state) {
    case 1:
        return ui_text(L"正常", L"Normal");
    case 2:
        return ui_text(L"自動調整中", L"Auto-adjusting");
    case 3:
        return ui_text(L"調整上限", L"Adjustment limit");
    case 4:
        return ui_text(L"監視中", L"Monitoring");
    default:
        return ui_text(L"未測定", L"Not measured");
    }
}

const wchar_t* auto_control_reason_to_text(int reason_mask) {
    switch (reason_mask) {
    case 1:
        return ui_text(L"True Peak超過", L"True Peak exceedance");
    case 2:
        return ui_text(L"リミッター過多", L"Excessive limiting");
    case 4:
        return ui_text(L"クリッパー過多", L"Excessive clipping");
    case 0:
        return ui_text(L"なし", L"None");
    default:
        return ui_text(L"複数要因", L"Multiple factors");
    }
}

std::wstring diagnostic_auto_control_reason_text(
    int current_reason_mask,
    int latest_reason_mask,
    int processing_state,
    unsigned trigger_count,
    bool latest_recovered
) {
    if (trigger_count == 0) {
        if (processing_state == 4 &&
            current_reason_mask != 0) {
            return auto_control_reason_to_text(
                current_reason_mask
            );
        }

        return ui_text(
            L"この曲では未発動",
            L"Not activated for this track"
        );
    }

    const int displayed_reason =
        latest_reason_mask != 0
            ? latest_reason_mask
            : current_reason_mask;

    if (processing_state == 1 && latest_recovered) {
        wchar_t text[160] = {};
        swprintf_s(
            text,
            ui_text(
                L"直近：%s（復帰済み）",
                L"Latest: %s (recovered)"
            ),
            auto_control_reason_to_text(displayed_reason)
        );
        return text;
    }

    return auto_control_reason_to_text(displayed_reason);
}

struct auto_control_history_metrics {
    unsigned trigger_count = 0;
    int reason_mask = 0;
    double maximum_attenuation_db = 0.0;
    bool adjustment_limit_reached = false;
    bool recovered = false;
    int profile_id = -1;
};

struct auto_control_history_entry {
    unsigned long long timestamp = 0;
    unsigned long long session_id = 0;
    std::wstring artist;
    std::wstring title;
    int profile_id = -1;
    int reason_mask = 0;
    double maximum_attenuation_db = 0.0;
    unsigned trigger_count = 0;
    bool adjustment_limit_reached = false;
    bool recovered = false;
};

struct auto_control_trend_sample {
    double playback_seconds = 0.0;
    double short_term_lufs = -200.0;
    double applied_gain_db = 0.0;
    double automatic_attenuation_db = 0.0;
    double true_peak_dbtp = -200.0;
    int processing_state = 0;
};

struct auto_control_trend_snapshot {
    std::wstring artist;
    std::wstring title;
    std::vector<auto_control_trend_sample> samples;
    unsigned long long revision = 0;
};

std::vector<auto_control_history_entry> g_auto_control_history_entries;
bool g_auto_control_history_loaded = false;
unsigned long long g_auto_control_history_next_session_id = 1;
unsigned long long g_auto_control_history_active_session_id = 0;
unsigned long long g_auto_control_history_suppressed_session_id = 0;
unsigned long long g_auto_control_history_revision = 0;

std::mutex g_auto_control_trend_mutex;
std::wstring g_auto_control_trend_artist;
std::wstring g_auto_control_trend_title;
std::vector<auto_control_trend_sample> g_auto_control_trend_samples;
unsigned long long g_auto_control_trend_revision = 0;

void reset_auto_control_trend(
    const std::wstring& artist,
    const std::wstring& title
) {
    std::lock_guard<std::mutex> lock(g_auto_control_trend_mutex);
    g_auto_control_trend_artist = artist;
    g_auto_control_trend_title = title;
    g_auto_control_trend_samples.clear();
    ++g_auto_control_trend_revision;
}

void capture_auto_control_trend_sample(double playback_seconds) {
    if (!std::isfinite(playback_seconds) || playback_seconds < 0.0) {
        return;
    }

    auto_control_trend_sample sample;
    sample.playback_seconds = playback_seconds;
    sample.short_term_lufs =
        g_diagnostic_short_term_lufs.load(std::memory_order_relaxed);
    sample.applied_gain_db =
        g_diagnostic_applied_gain_db.load(std::memory_order_relaxed);
    sample.automatic_attenuation_db = std::max(
        0.0,
        -g_diagnostic_safety_reduction_db.load(
            std::memory_order_relaxed
        )
    );
    sample.true_peak_dbtp =
        g_diagnostic_true_peak_dbtp.load(std::memory_order_relaxed);
    sample.processing_state =
        g_diagnostic_current_processing_state.load(
            std::memory_order_relaxed
        );

    std::lock_guard<std::mutex> lock(g_auto_control_trend_mutex);

    if (!g_auto_control_trend_samples.empty()) {
        const double previous_seconds =
            g_auto_control_trend_samples.back().playback_seconds;

        if (playback_seconds + 0.5 < previous_seconds) {
            g_auto_control_trend_samples.clear();
        }
        else if (playback_seconds - previous_seconds < 0.45) {
            return;
        }
    }

    if (g_auto_control_trend_samples.size() >= kMaximumTrendSamples) {
        const t_size erase_count = std::min<t_size>(
            3600,
            g_auto_control_trend_samples.size()
        );
        g_auto_control_trend_samples.erase(
            g_auto_control_trend_samples.begin(),
            g_auto_control_trend_samples.begin() + erase_count
        );
    }

    g_auto_control_trend_samples.push_back(sample);
    ++g_auto_control_trend_revision;
}

auto_control_trend_snapshot current_auto_control_trend_snapshot() {
    std::lock_guard<std::mutex> lock(g_auto_control_trend_mutex);

    auto_control_trend_snapshot snapshot;
    snapshot.artist = g_auto_control_trend_artist;
    snapshot.title = g_auto_control_trend_title;
    snapshot.samples = g_auto_control_trend_samples;
    snapshot.revision = g_auto_control_trend_revision;
    return snapshot;
}

std::wstring utf8_to_wide(const char* text) {
    if (text == nullptr || *text == '\0') {
        return {};
    }

    const int count = MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        text,
        -1,
        nullptr,
        0
    );

    if (count <= 1) {
        return {};
    }

    std::wstring result(
        static_cast<t_size>(count),
        L'\0'
    );
    MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        text,
        -1,
        result.data(),
        count
    );
    result.resize(static_cast<t_size>(count - 1));
    return result;
}

std::string wide_to_utf8(const std::wstring& text) {
    if (text.empty()) {
        return {};
    }

    const int count = WideCharToMultiByte(
        CP_UTF8,
        WC_ERR_INVALID_CHARS,
        text.c_str(),
        static_cast<int>(text.size()),
        nullptr,
        0,
        nullptr,
        nullptr
    );

    if (count <= 0) {
        return {};
    }

    std::string result(
        static_cast<t_size>(count),
        '\0'
    );
    WideCharToMultiByte(
        CP_UTF8,
        WC_ERR_INVALID_CHARS,
        text.c_str(),
        static_cast<int>(text.size()),
        result.data(),
        count,
        nullptr,
        nullptr
    );
    return result;
}

std::string hex_encode(const std::string& value) {
    static constexpr char kHexDigits[] =
        "0123456789ABCDEF";

    std::string result;
    result.reserve(value.size() * 2);

    for (unsigned char character : value) {
        result.push_back(kHexDigits[(character >> 4) & 0x0f]);
        result.push_back(kHexDigits[character & 0x0f]);
    }

    return result;
}

int hex_digit_value(char character) {
    if (character >= '0' && character <= '9') {
        return character - '0';
    }
    if (character >= 'a' && character <= 'f') {
        return 10 + character - 'a';
    }
    if (character >= 'A' && character <= 'F') {
        return 10 + character - 'A';
    }
    return -1;
}

bool hex_decode(
    const std::string& value,
    std::string& result
) {
    if ((value.size() % 2) != 0) {
        return false;
    }

    result.clear();
    result.reserve(value.size() / 2);

    for (t_size index = 0;
         index < value.size();
         index += 2) {
        const int high = hex_digit_value(value[index]);
        const int low = hex_digit_value(value[index + 1]);

        if (high < 0 || low < 0) {
            result.clear();
            return false;
        }

        result.push_back(
            static_cast<char>((high << 4) | low)
        );
    }

    return true;
}

std::vector<std::string> split_history_fields(
    const std::string& line
) {
    std::vector<std::string> fields;
    t_size start = 0;

    for (;;) {
        const t_size separator = line.find('\t', start);

        if (separator == std::string::npos) {
            fields.push_back(line.substr(start));
            break;
        }

        fields.push_back(line.substr(start, separator - start));
        start = separator + 1;
    }

    return fields;
}

void limit_history_metadata_length(std::wstring& value);

void save_auto_control_history() {
    std::ostringstream output;
    output << "R128H1\n";

    for (const auto& entry : g_auto_control_history_entries) {
        output
            << entry.timestamp << '\t'
            << entry.profile_id << '\t'
            << entry.reason_mask << '\t'
            << static_cast<long long>(std::llround(
                entry.maximum_attenuation_db * 1000.0
            )) << '\t'
            << entry.trigger_count << '\t'
            << (entry.adjustment_limit_reached ? 1 : 0) << '\t'
            << (entry.recovered ? 1 : 0) << '\t'
            << hex_encode(wide_to_utf8(entry.artist)) << '\t'
            << hex_encode(wide_to_utf8(entry.title))
            << '\n';
    }

    g_cfg_auto_control_history = output.str().c_str();
    ++g_auto_control_history_revision;
}

void ensure_auto_control_history_loaded() {
    if (g_auto_control_history_loaded) {
        return;
    }

    g_auto_control_history_loaded = true;
    g_auto_control_history_entries.clear();

    std::istringstream input(
        g_cfg_auto_control_history.get_ptr()
    );
    std::string line;

    if (!std::getline(input, line) || line != "R128H1") {
        return;
    }

    while (std::getline(input, line) &&
           g_auto_control_history_entries.size() <
               kMaximumAutoControlHistoryEntries) {
        const auto fields = split_history_fields(line);

        if (fields.size() != 9) {
            continue;
        }

        try {
            auto_control_history_entry entry;
            entry.timestamp = std::stoull(fields[0]);
            entry.profile_id = std::stoi(fields[1]);
            entry.reason_mask = std::stoi(fields[2]);
            entry.maximum_attenuation_db =
                static_cast<double>(std::stoll(fields[3])) /
                1000.0;
            entry.trigger_count = static_cast<unsigned>(
                std::stoul(fields[4])
            );
            entry.adjustment_limit_reached =
                std::stoi(fields[5]) != 0;
            entry.recovered = std::stoi(fields[6]) != 0;

            std::string artist_utf8;
            std::string title_utf8;

            if (!hex_decode(fields[7], artist_utf8) ||
                !hex_decode(fields[8], title_utf8)) {
                continue;
            }

            entry.artist = utf8_to_wide(artist_utf8.c_str());
            entry.title = utf8_to_wide(title_utf8.c_str());
            limit_history_metadata_length(entry.artist);
            limit_history_metadata_length(entry.title);

            if (entry.timestamp == 0 ||
                entry.trigger_count == 0) {
                continue;
            }

            g_auto_control_history_entries.push_back(
                std::move(entry)
            );
        }
        catch (...) {
            continue;
        }
    }
}

unsigned long long current_filetime_value() {
    FILETIME file_time = {};
    GetSystemTimeAsFileTime(&file_time);

    ULARGE_INTEGER value = {};
    value.LowPart = file_time.dwLowDateTime;
    value.HighPart = file_time.dwHighDateTime;
    return value.QuadPart;
}

auto_control_history_metrics current_auto_control_history_metrics() {
    auto_control_history_metrics metrics;

    if (g_history_metrics_session_id.load(
            std::memory_order_relaxed
        ) !=
        g_history_active_session_id.load(
            std::memory_order_relaxed
        )) {
        return metrics;
    }

    metrics.trigger_count =
        g_history_auto_control_trigger_count.load(
            std::memory_order_relaxed
        );
    metrics.reason_mask =
        g_history_auto_control_reason_mask.load(
            std::memory_order_relaxed
        );
    metrics.maximum_attenuation_db =
        g_history_max_auto_attenuation_db.load(
            std::memory_order_relaxed
        );
    metrics.adjustment_limit_reached =
        g_history_adjustment_limit_reached.load(
            std::memory_order_relaxed
        ) != 0;
    metrics.recovered =
        g_history_recovered.load(
            std::memory_order_relaxed
        ) != 0;
    metrics.profile_id =
        g_history_profile_id.load(
            std::memory_order_relaxed
        );
    return metrics;
}

auto_control_history_metrics final_auto_control_history_metrics() {
    auto_control_history_metrics metrics;
    metrics.trigger_count =
        g_history_final_auto_control_trigger_count.load(
            std::memory_order_relaxed
        );
    metrics.reason_mask =
        g_history_final_auto_control_reason_mask.load(
            std::memory_order_relaxed
        );
    metrics.maximum_attenuation_db =
        g_history_final_max_auto_attenuation_db.load(
            std::memory_order_relaxed
        );
    metrics.adjustment_limit_reached =
        g_history_final_adjustment_limit_reached.load(
            std::memory_order_relaxed
        ) != 0;
    metrics.recovered =
        g_history_final_recovered.load(
            std::memory_order_relaxed
        ) != 0;
    metrics.profile_id =
        g_history_final_profile_id.load(
            std::memory_order_relaxed
        );
    return metrics;
}

bool valid_recognized_profile_id(int profile_id) {
    return profile_id >=
            static_cast<int>(recognized_profile::standard) &&
        profile_id <=
            static_cast<int>(recognized_profile::custom);
}

void update_auto_control_history_entry(
    unsigned long long session_id,
    unsigned long long timestamp,
    const std::wstring& artist,
    const std::wstring& title,
    const auto_control_history_metrics& metrics
) {
    if (session_id == 0 ||
        session_id ==
            g_auto_control_history_suppressed_session_id ||
        metrics.trigger_count == 0) {
        return;
    }

    ensure_auto_control_history_loaded();

    auto iterator = std::find_if(
        g_auto_control_history_entries.begin(),
        g_auto_control_history_entries.end(),
        [session_id](const auto_control_history_entry& entry) {
            return entry.session_id == session_id;
        }
    );

    bool changed = false;

    if (iterator == g_auto_control_history_entries.end()) {
        auto_control_history_entry entry;
        entry.timestamp = timestamp;
        entry.session_id = session_id;
        entry.artist = artist;
        entry.title = title;
        entry.profile_id = valid_recognized_profile_id(
            metrics.profile_id
        )
            ? metrics.profile_id
            : static_cast<int>(recognized_profile::custom);

        g_auto_control_history_entries.insert(
            g_auto_control_history_entries.begin(),
            std::move(entry)
        );
        iterator = g_auto_control_history_entries.begin();
        changed = true;
    }

    auto& entry = *iterator;

    if (!artist.empty() && entry.artist != artist) {
        entry.artist = artist;
        changed = true;
    }
    if (!title.empty() && entry.title != title) {
        entry.title = title;
        changed = true;
    }

    const int profile_id = valid_recognized_profile_id(
        metrics.profile_id
    )
        ? metrics.profile_id
        : static_cast<int>(recognized_profile::custom);

    if (entry.trigger_count == 0) {
        if (entry.profile_id != profile_id) {
            entry.profile_id = profile_id;
            changed = true;
        }
    }
    else if (entry.profile_id != profile_id) {
        const int custom_profile_id =
            static_cast<int>(recognized_profile::custom);
        if (entry.profile_id != custom_profile_id) {
            entry.profile_id = custom_profile_id;
            changed = true;
        }
    }

    const int combined_reason =
        entry.reason_mask | metrics.reason_mask;
    if (entry.reason_mask != combined_reason) {
        entry.reason_mask = combined_reason;
        changed = true;
    }

    const double maximum_attenuation = std::max(
        entry.maximum_attenuation_db,
        metrics.maximum_attenuation_db
    );
    if (std::fabs(
            entry.maximum_attenuation_db -
            maximum_attenuation
        ) > 0.0005) {
        entry.maximum_attenuation_db = maximum_attenuation;
        changed = true;
    }

    if (entry.trigger_count <= metrics.trigger_count &&
        entry.recovered != metrics.recovered) {
        entry.recovered = metrics.recovered;
        changed = true;
    }
    if (entry.trigger_count < metrics.trigger_count) {
        entry.trigger_count = metrics.trigger_count;
        changed = true;
    }
    if (!entry.adjustment_limit_reached &&
        metrics.adjustment_limit_reached) {
        entry.adjustment_limit_reached = true;
        changed = true;
    }
    if (g_auto_control_history_entries.size() >
        kMaximumAutoControlHistoryEntries) {
        g_auto_control_history_entries.resize(
            kMaximumAutoControlHistoryEntries
        );
        changed = true;
    }

    if (changed) {
        save_auto_control_history();
    }
}

std::wstring history_title_fallback_from_path(
    const char* path_utf8
) {
    std::wstring path = utf8_to_wide(path_utf8);

    if (path.empty()) {
        return {};
    }

    const t_size separator = path.find_last_of(L"\\/");
    if (separator != std::wstring::npos) {
        path.erase(0, separator + 1);
    }

    const t_size query = path.find_first_of(L"?#");
    if (query != std::wstring::npos) {
        path.erase(query);
    }

    const t_size extension = path.find_last_of(L'.');
    if (extension != std::wstring::npos && extension > 0) {
        path.erase(extension);
    }

    return path;
}

void limit_history_metadata_length(std::wstring& value) {
    if (value.size() > kMaximumHistoryMetadataCharacters) {
        value.resize(kMaximumHistoryMetadataCharacters);

        if (!value.empty() &&
            value.back() >= 0xd800 &&
            value.back() <= 0xdbff) {
            value.pop_back();
        }
    }
}

void history_metadata_from_info(
    const file_info& info,
    std::wstring& artist,
    std::wstring& title
) {
    const char* artist_value = info.meta_get("artist", 0);
    const char* title_value = info.meta_get("title", 0);

    if (artist_value != nullptr) {
        artist = utf8_to_wide(artist_value);
    }
    if (title_value != nullptr) {
        title = utf8_to_wide(title_value);
    }

    limit_history_metadata_length(artist);
    limit_history_metadata_length(title);
}

void history_metadata_from_handle(
    const metadb_handle_ptr& handle,
    std::wstring& artist,
    std::wstring& title
) {
    artist.clear();
    title.clear();

    if (handle.is_empty()) {
        return;
    }

    try {
        const auto info = handle->get_info_ref();
        history_metadata_from_info(
            info->info(),
            artist,
            title
        );
    }
    catch (...) {
    }

    if (title.empty()) {
        title = history_title_fallback_from_path(
            handle->get_path()
        );
    }

    limit_history_metadata_length(artist);
    limit_history_metadata_length(title);
}

class auto_control_history_play_callback
    : public play_callback_static {
public:
    uint32_t get_flags() override {
        return
            flag_on_playback_new_track |
            flag_on_playback_stop |
            flag_on_playback_time |
            flag_on_playback_seek |
            flag_on_playback_edited |
            flag_on_playback_dynamic_info_track;
    }

    void on_playback_new_track(
        metadb_handle_ptr track
    ) override {
        finish_current_track();
        start_new_track(track);
    }

    void on_playback_stop(
        play_control::t_stop_reason
    ) override {
        finish_current_track();
    }

    void on_playback_time(double playback_seconds) override {
        capture_auto_control_trend_sample(playback_seconds);
        capture_current_track(
            current_auto_control_history_metrics()
        );
    }

    void on_playback_edited(
        metadb_handle_ptr track
    ) override {
        if (!m_active) {
            return;
        }

        std::wstring artist;
        std::wstring title;
        history_metadata_from_handle(track, artist, title);

        if (!artist.empty()) {
            m_artist = std::move(artist);
        }
        if (!title.empty()) {
            m_title = std::move(title);
        }
    }

    void on_playback_dynamic_info_track(
        const file_info& info
    ) override {
        if (!m_active) {
            return;
        }

        std::wstring artist = m_artist;
        std::wstring title = m_title;
        history_metadata_from_info(info, artist, title);

        if (artist == m_artist && title == m_title) {
            return;
        }

        finish_current_track();
        start_new_track(artist, title);
    }

    void on_playback_dynamic_info(
        const file_info&
    ) override {
    }

    void on_playback_pause(bool) override {
    }

    void on_playback_seek(double) override {
        reset_auto_control_trend(m_artist, m_title);
    }

    void on_playback_starting(
        play_control::t_track_command,
        bool
    ) override {
    }

    void on_volume_change(float) override {
    }

private:
    void start_new_track(
        const metadb_handle_ptr& track
    ) {
        std::wstring artist;
        std::wstring title;
        history_metadata_from_handle(track, artist, title);
        start_new_track(artist, title);
    }

    void start_new_track(
        const std::wstring& artist,
        const std::wstring& title
    ) {
        m_artist = artist;
        m_title = title;
        reset_auto_control_trend(m_artist, m_title);
        m_timestamp = current_filetime_value();
        m_session_id =
            g_auto_control_history_next_session_id++;
        g_auto_control_history_active_session_id = m_session_id;
        g_history_active_session_id.store(
            m_session_id,
            std::memory_order_relaxed
        );
        m_active = true;

        g_history_track_reset_request.fetch_add(
            1,
            std::memory_order_relaxed
        );
    }

    void capture_current_track(
        const auto_control_history_metrics& metrics
    ) {
        if (!m_active) {
            return;
        }

        update_auto_control_history_entry(
            m_session_id,
            m_timestamp,
            m_artist,
            m_title,
            metrics
        );
    }

    void finish_current_track() {
        if (!m_active) {
            return;
        }

        const unsigned long long final_session_id =
            g_history_final_session_id.load(
                std::memory_order_relaxed
            );

        if (final_session_id == m_session_id) {
            capture_current_track(
                final_auto_control_history_metrics()
            );
        }
        else {
            capture_current_track(
                current_auto_control_history_metrics()
            );
        }

        m_active = false;
        m_artist.clear();
        m_title.clear();
        m_timestamp = 0;
        m_session_id = 0;
        g_auto_control_history_active_session_id = 0;
        g_history_active_session_id.store(
            0,
            std::memory_order_relaxed
        );
    }

    bool m_active = false;
    std::wstring m_artist;
    std::wstring m_title;
    unsigned long long m_timestamp = 0;
    unsigned long long m_session_id = 0;
};

FB2K_SERVICE_FACTORY(auto_control_history_play_callback);

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
    IDC_BASIC_RIGHT_HEADER,
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
    IDC_ADV_GROUP_MODERN,
    IDC_ADV_GROUP_ADAPTIVE,
    IDC_ADV_GROUP_THREE_BAND,
    IDC_ADV_GROUP_STRENGTH,
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
    IDC_DIAG_LEFT_HEADER,
    IDC_DIAG_RIGHT_HEADER,
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
    IDC_LABEL_DIAG_AUTO_REASON,
    IDC_DIAG_AUTO_REASON,
    IDC_LABEL_DIAG_MAX_TRUE_PEAK,
    IDC_DIAG_MAX_TRUE_PEAK,
    IDC_LABEL_DIAG_MAX_COMPRESSOR,
    IDC_DIAG_MAX_COMPRESSOR_REDUCTION,
    IDC_LABEL_DIAG_MAX_CLIPPER,
    IDC_DIAG_MAX_CLIPPER_REDUCTION,
    IDC_LABEL_DIAG_MAX_LIMITER,
    IDC_DIAG_MAX_LIMITER_REDUCTION,
    IDC_SHOW_TREND_GRAPH
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

    int selected_page = TabCtrl_GetCurSel(tabs);
    if (selected_page < 0 || selected_page > 2) {
        selected_page = 0;
    }

    TabCtrl_DeleteAllItems(tabs);

    const wchar_t* titles[] = {
        ui_text(L"基本設定", L"Basic"),
        ui_text(L"追加処理", L"Processing"),
        ui_text(L"診断", L"Diagnostics")
    };

    for (int index = 0; index < 3; ++index) {
        TCITEMW item = {};
        item.mask = TCIF_TEXT;
        item.pszText = const_cast<LPWSTR>(titles[index]);
        TabCtrl_InsertItem(tabs, index, &item);
    }

    TabCtrl_SetCurSel(tabs, selected_page);
    update_config_tab_page(wnd, selected_page);
}

struct localized_control_text {
    int control_id;
    const wchar_t* japanese;
    const wchar_t* english;
};

constexpr localized_control_text kPrimaryUiText[] = {
    { IDC_BASIC_HEADER, L"左列：R128補正・測定", L"R128 normalization and measurement" },
    { IDC_BASIC_RIGHT_HEADER, L"右列：True Peak保護・ゲイン固定", L"True Peak protection and gain lock" },
    { IDC_LABEL_TARGET_LUFS, L"目標ラウドネス：", L"Target loudness:" },
    { IDC_LABEL_MAX_BOOST, L"R128補正ゲイン最大増幅量：", L"Maximum R128 boost:" },
    { IDC_LABEL_MAX_ATTENUATION, L"R128補正ゲイン最大減衰量：", L"Maximum R128 attenuation:" },
    { IDC_LABEL_TRUE_PEAK, L"True Peak上限：", L"True Peak limit:" },
    { IDC_LABEL_LOOKAHEAD, L"True Peakリミッター先読み時間：", L"Limiter look-ahead:" },
    { IDC_LABEL_LIMITER_RELEASE, L"True Peakリミッター解放時間：", L"Limiter release:" },
    { IDC_LABEL_STARTUP, L"曲頭安定化解析時間：", L"Startup analysis time:" },
    { IDC_UNIT_STARTUP, L"秒", L"sec" },
    { IDC_LABEL_SILENCE_THRESHOLD, L"静音保護しきい値：", L"Silence guard threshold:" },
    { IDC_LABEL_GAIN_LOCK_SECONDS, L"補正ゲイン固定判定時間：", L"Gain-lock detection time:" },
    { IDC_UNIT_GAIN_LOCK_SECONDS, L"秒", L"sec" },
    { IDC_LABEL_GAIN_LOCK_TOLERANCE, L"補正ゲイン固定許容幅：", L"Gain-lock tolerance:" },
    { IDC_RESET_EACH_TRACK, L"トラック変更時に測定値をリセット", L"Reset measurements on track change" },
    { IDC_ENABLE_SILENCE_GUARD, L"静音区間では増幅を保留", L"Hold boost during silent passages" },
    { IDC_ENABLE_PEAK_GUARD, L"True Peak保護（先読みリミッター）を有効", L"Enable True Peak protection (look-ahead limiter)" },
    { IDC_ENABLE_GAIN_LOCK, L"安定後に曲中の補正ゲインを固定", L"Lock normalization gain after stabilization" },

    { IDC_ADV_HEADER, L"追加マスタリング処理（任意）", L"Additional mastering processing (optional)" },
    { IDC_ADV_GROUP_MODERN, L"モダン処理", L"Modern Processing" },
    { IDC_ADV_GROUP_ADAPTIVE, L"1バンド・アダプティブ", L"1-Band Adaptive" },
    { IDC_ADV_GROUP_THREE_BAND, L"3バンド・アダプティブ", L"3-Band Adaptive" },
    { IDC_ADV_GROUP_STRENGTH, L"共通の処理強度", L"Shared Processing Strength" },
    { IDC_ENABLE_MODERN_BOOST, L"有効にする", L"Enable" },
    { IDC_ENABLE_ADAPTIVE_MASTER, L"有効にする", L"Enable" },
    { IDC_ENABLE_THREE_BAND_MASTER, L"有効にする", L"Enable" },
    { IDC_ADV_INFO_1, L"コンプレッサー、ソフトクリッパー、True Peakリミッターを組み合わせます。", L"Combines compression, soft clipping, and True Peak limiting." },
    { IDC_ADV_INFO_2, L"曲の音量とLRAに応じて、モダン処理の強度を自動調整します。", L"Adapts Modern Processing strength to loudness and LRA." },
    { IDC_ADV_INFO_3, L"低域・中域・高域に分け、それぞれの処理量を個別に制御します。", L"Controls low, mid, and high bands independently." },
    { IDC_LABEL_MODERN_STRENGTH, L"モダン処理強度／アダプティブ上限：", L"Modern strength / adaptive maximum:" },
    { IDC_UNIT_MODERN_STRENGTH, L"%（0～100）", L"% (0-100)" },

    { IDC_DIAG_LEFT_HEADER, L"現在値・処理状態（上から下へ）", L"Current values and processing state" },
    { IDC_DIAG_RIGHT_HEADER, L"ピーク・環境・最大値（上から下へ）", L"Peaks, environment, and maxima" },
    { IDC_LABEL_DIAG_NORMALIZATION, L"R128ノーマライズ状態：", L"R128 normalization:" },
    { IDC_LABEL_DIAG_GAIN_LOCK, L"補正ゲイン固定状態：", L"Gain lock:" },
    { IDC_LABEL_DIAG_MOMENTARY, L"Momentaryラウドネス：", L"Momentary loudness:" },
    { IDC_LABEL_DIAG_SHORT_TERM, L"Short-termラウドネス：", L"Short-term loudness:" },
    { IDC_LABEL_DIAG_INPUT_INT, L"入力Integratedラウドネス：", L"Input integrated loudness:" },
    { IDC_LABEL_DIAG_OUTPUT_INT, L"出力Integratedラウドネス：", L"Output integrated loudness:" },
    { IDC_LABEL_DIAG_TARGET_DIFF, L"目標LUFSとの差：", L"Difference from target:" },
    { IDC_LABEL_DIAG_LRA, L"ラウドネスレンジ（LRA）：", L"Loudness range (LRA):" },
    { IDC_LABEL_DIAG_GAIN, L"適用中の総ゲイン：", L"Total applied gain:" },
    { IDC_LABEL_DIAG_PROCESSING, L"追加処理の状態：", L"Additional processing:" },
    { IDC_LABEL_DIAG_SAFETY, L"自動減衰量：", L"Automatic attenuation:" },
    { IDC_LABEL_DIAG_COMPRESSOR, L"コンプレッサー減衰量：", L"Compressor reduction:" },
    { IDC_LABEL_DIAG_CLIPPER, L"ソフトクリッパー減衰量：", L"Soft clipper reduction:" },
    { IDC_LABEL_DIAG_LIMITER, L"True Peakリミッター減衰量：", L"True Peak limiter reduction:" },
    { IDC_LABEL_DIAG_TRUE_PEAK, L"処理後True Peak：", L"Processed True Peak:" },
    { IDC_LABEL_DIAG_PEAK_GUARD, L"True Peak保護状態：", L"True Peak protection:" },
    { IDC_LABEL_DIAG_LATENCY, L"処理遅延：", L"Processing latency:" },
    { IDC_LABEL_DIAG_SAMPLE_RATE, L"サンプルレート：", L"Sample rate:" },
    { IDC_LABEL_DIAG_CHANNEL_LAYOUT, L"チャンネル構成：", L"Channel layout:" },
    { IDC_LABEL_DIAG_CPU, L"CPU負荷：", L"CPU load:" },
    { IDC_LABEL_DIAG_CLIP_EVENTS, L"0 dBTP超過イベント：", L"0 dBTP exceedance events:" },
    { IDC_LABEL_DIAG_THREE_BAND, L"3バンド減衰量：", L"3-band reduction:" },
    { IDC_LABEL_DIAG_MAX_TRUE_PEAK, L"最大True Peak：", L"Maximum True Peak:" },
    { IDC_LABEL_DIAG_MAX_COMPRESSOR, L"最大コンプレッサー減衰量：", L"Maximum compressor reduction:" },
    { IDC_LABEL_DIAG_MAX_CLIPPER, L"最大ソフトクリッパー減衰量：", L"Maximum soft clipper reduction:" },
    { IDC_LABEL_DIAG_MAX_LIMITER, L"最大True Peakリミッター減衰量：", L"Maximum limiter reduction:" },
    { IDC_LABEL_DIAG_EVALUATION, L"現在の処理状態：", L"Current processing state:" },
    { IDC_LABEL_DIAG_AUTO_REASON, L"自動制御の理由：", L"Automatic-control reason:" },

    { IDC_PRESET_GROUP, L"プリセット（上段：標準4種／下段：追加処理3種）", L"Presets (standard: top / additional processing: bottom)" },
    { IDC_PROFILE_STANDARD, L"ナチュラル -18", L"Natural -18" },
    { IDC_PROFILE_STREAMING, L"パワーブースト -14", L"Power Boost -14" },
    { IDC_PROFILE_BROADCAST, L"リラックス -23", L"Relaxed -23" },
    { IDC_PROFILE_NIGHT, L"ナイトセーフ -22", L"Night Safe -22" },
    { IDC_PROFILE_MODERN, L"モダンブースト -9", L"Modern Boost -9" },
    { IDC_PROFILE_ADAPTIVE, L"1バンド・アダプティブ -10", L"1-Band Adaptive -10" },
    { IDC_PROFILE_THREE_BAND, L"3バンド・アダプティブ -10", L"3-Band Adaptive -10" },
    { IDC_COMPARE_GROUP, L"選択中のプリセット／比較", L"Selected preset / comparison" },
    { IDC_COMPARE_LOUDNESS_MATCH, L"音量一致", L"Loudness match" },
    { IDC_ORIGINAL_COMPARE, L"比較（押している間）", L"Compare (hold)" },
    { IDC_SHOW_LICENSE, L"ライセンス", L"License" },
    { IDC_LANGUAGE_LABEL, L"表示言語：", L"Display language:" },
    { IDC_RESET_MEASUREMENT, L"測定リセット", L"Reset measurement" },
    { IDC_COPY_DIAGNOSTICS, L"診断コピー", L"Copy diagnostics" },
    { IDC_SHOW_AUTO_HISTORY, L"自動制御履歴", L"Automatic-Control History" },
    { IDC_SHOW_TREND_GRAPH, L"推移グラフ", L"Trend Graph" },
    { IDC_SHOW_DIAGNOSTIC_HELP, L"用語集", L"Glossary" },
    { IDC_DEFAULTS, L"初期設定", L"Defaults" },
    { IDC_APPLY_SETTINGS, L"適用", L"Apply" },
    { IDOK, L"OK", L"OK" },
    { IDCANCEL, L"取消", L"Cancel" }
};

void initialize_language_combo(HWND wnd) {
    const HWND combo = GetDlgItem(wnd, IDC_DISPLAY_LANGUAGE);
    if (combo == nullptr) {
        return;
    }

    SendMessageW(combo, CB_RESETCONTENT, 0, 0);

    const wchar_t* choices[] = {
        ui_text(L"自動（Windows）", L"Automatic (Windows)"),
        ui_text(L"日本語", L"Japanese"),
        L"English"
    };

    for (const wchar_t* choice : choices) {
        SendMessageW(
            combo,
            CB_ADDSTRING,
            0,
            reinterpret_cast<LPARAM>(choice)
        );
    }

    SendMessageW(
        combo,
        CB_SETCURSEL,
        static_cast<WPARAM>(configured_display_language()),
        0
    );
}

void apply_primary_ui_language(
    HWND wnd,
    const r128_settings* value,
    bool pending
) {
    SetWindowTextW(
        wnd,
        ui_text(
            L"R128 音量ノーマライザー",
            L"R128 Loudness Normalizer"
        )
    );

    for (const auto& entry : kPrimaryUiText) {
        SetDlgItemTextW(
            wnd,
            entry.control_id,
            ui_text(entry.japanese, entry.english)
        );
    }

    setup_config_tabs(wnd);
    initialize_language_combo(wnd);

    if (value != nullptr) {
        update_profile_indicator(wnd, *value, pending);
    }
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

struct dialog_scroll_state {
    bool initialized = false;
    int position_x = 0;
    int position_y = 0;
    int content_width = 0;
    int content_height = 0;
    int full_window_width = 0;
    int full_window_height = 0;
    UINT dpi = 96;
};

UINT dialog_dpi_for_window(HWND wnd) {
    using get_dpi_for_window_fn = UINT(WINAPI*)(HWND);

    const HMODULE user32 = GetModuleHandleW(L"user32.dll");
    if (user32 != nullptr) {
        const auto get_dpi_for_window =
            reinterpret_cast<get_dpi_for_window_fn>(
                GetProcAddress(user32, "GetDpiForWindow")
            );

        if (get_dpi_for_window != nullptr) {
            const UINT dpi = get_dpi_for_window(wnd);
            if (dpi != 0) {
                return dpi;
            }
        }
    }

    return 96;
}

int dialog_system_metric_for_dpi(int metric, UINT dpi) {
    using get_system_metrics_for_dpi_fn = int(WINAPI*)(int, UINT);

    const HMODULE user32 = GetModuleHandleW(L"user32.dll");
    if (user32 != nullptr) {
        const auto get_system_metrics_for_dpi =
            reinterpret_cast<get_system_metrics_for_dpi_fn>(
                GetProcAddress(user32, "GetSystemMetricsForDpi")
            );

        if (get_system_metrics_for_dpi != nullptr) {
            return get_system_metrics_for_dpi(
                metric,
                dpi == 0 ? 96 : dpi
            );
        }
    }

    return GetSystemMetrics(metric);
}

void offset_dialog_children(HWND wnd, int offset_x, int offset_y) {
    if (offset_x == 0 && offset_y == 0) {
        return;
    }

    for (HWND child = GetWindow(wnd, GW_CHILD);
         child != nullptr;
         child = GetWindow(child, GW_HWNDNEXT)) {
        RECT child_rect = {};
        if (!GetWindowRect(child, &child_rect)) {
            continue;
        }

        MapWindowPoints(
            nullptr,
            wnd,
            reinterpret_cast<POINT*>(&child_rect),
            2
        );

        SetWindowPos(
            child,
            nullptr,
            child_rect.left + offset_x,
            child_rect.top + offset_y,
            0,
            0,
            SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE
        );
    }
}

void reset_dialog_scroll_position(
    HWND wnd,
    dialog_scroll_state& state
) {
    offset_dialog_children(
        wnd,
        state.position_x,
        state.position_y
    );
    state.position_x = 0;
    state.position_y = 0;
}

void update_dialog_scroll_dpi(
    dialog_scroll_state& state,
    UINT new_dpi
) {
    if (!state.initialized || new_dpi == 0 || new_dpi == state.dpi) {
        return;
    }

    const int old_dpi = static_cast<int>(
        state.dpi == 0 ? 96 : state.dpi
    );
    const int target_dpi = static_cast<int>(new_dpi);

    state.content_width = MulDiv(
        state.content_width,
        target_dpi,
        old_dpi
    );
    state.content_height = MulDiv(
        state.content_height,
        target_dpi,
        old_dpi
    );
    state.full_window_width = MulDiv(
        state.full_window_width,
        target_dpi,
        old_dpi
    );
    state.full_window_height = MulDiv(
        state.full_window_height,
        target_dpi,
        old_dpi
    );
    state.dpi = new_dpi;
}

void configure_dialog_scroll_bar(
    HWND wnd,
    int bar,
    int content_extent,
    int page_extent,
    int position
) {
    SCROLLINFO info = {};
    info.cbSize = sizeof(info);
    info.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
    info.nMin = 0;
    info.nMax = std::max(content_extent - 1, 0);
    info.nPage = static_cast<UINT>(std::max(page_extent, 0));
    info.nPos = std::max(position, 0);
    SetScrollInfo(wnd, bar, &info, TRUE);
}

void fit_dialog_to_monitor_work_area(
    HWND wnd,
    dialog_scroll_state* scroll_state
) {
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

    const LONG current_width =
        window_rect.right - window_rect.left;
    const LONG current_height =
        window_rect.bottom - window_rect.top;
    const RECT& work = monitor_info.rcWork;
    const LONG work_width = work.right - work.left;
    const LONG work_height = work.bottom - work.top;

    if (scroll_state != nullptr && !scroll_state->initialized) {
        RECT client_rect = {};
        GetClientRect(wnd, &client_rect);

        scroll_state->initialized = true;
        scroll_state->content_width =
            client_rect.right - client_rect.left;
        scroll_state->content_height =
            client_rect.bottom - client_rect.top;
        scroll_state->full_window_width =
            static_cast<int>(current_width);
        scroll_state->full_window_height =
            static_cast<int>(current_height);
        scroll_state->dpi = dialog_dpi_for_window(wnd);
    }

    LONG target_width = current_width;
    LONG target_height = current_height;
    bool horizontal_scroll = false;
    bool vertical_scroll = false;

    if (scroll_state != nullptr) {
        reset_dialog_scroll_position(wnd, *scroll_state);

        horizontal_scroll =
            scroll_state->full_window_width > work_width;
        vertical_scroll =
            scroll_state->full_window_height > work_height;

        const int scroll_width = dialog_system_metric_for_dpi(
            SM_CXVSCROLL,
            scroll_state->dpi
        );
        const int scroll_height = dialog_system_metric_for_dpi(
            SM_CYHSCROLL,
            scroll_state->dpi
        );

        // Adding one scrollbar reduces the other client dimension.
        // Reserve its non-client space when the monitor has room, then
        // re-evaluate the perpendicular scrollbar.
        for (int iteration = 0; iteration < 2; ++iteration) {
            const LONG desired_width =
                scroll_state->full_window_width +
                (vertical_scroll ? scroll_width : 0);
            const LONG desired_height =
                scroll_state->full_window_height +
                (horizontal_scroll ? scroll_height : 0);

            horizontal_scroll =
                horizontal_scroll || desired_width > work_width;
            vertical_scroll =
                vertical_scroll || desired_height > work_height;
        }

        const LONG desired_width =
            scroll_state->full_window_width +
            (vertical_scroll ? scroll_width : 0);
        const LONG desired_height =
            scroll_state->full_window_height +
            (horizontal_scroll ? scroll_height : 0);

        target_width = std::min(desired_width, work_width);
        target_height = std::min(desired_height, work_height);

        LONG_PTR style = GetWindowLongPtrW(wnd, GWL_STYLE);
        const LONG_PTR original_style = style;

        if (horizontal_scroll) {
            style |= WS_HSCROLL;
        }
        else {
            style &= ~static_cast<LONG_PTR>(WS_HSCROLL);
        }

        if (vertical_scroll) {
            style |= WS_VSCROLL;
        }
        else {
            style &= ~static_cast<LONG_PTR>(WS_VSCROLL);
        }

        if (style != original_style) {
            SetWindowLongPtrW(wnd, GWL_STYLE, style);
        }
    }

    LONG x = window_rect.left;
    LONG y = window_rect.top;

    if (target_width <= work_width) {
        x = clamp_value<LONG>(
            x,
            work.left,
            work.right - target_width
        );
    }
    else {
        x = work.left;
    }

    if (target_height <= work_height) {
        y = clamp_value<LONG>(
            y,
            work.top,
            work.bottom - target_height
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
        target_width,
        target_height,
        SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED
    );

    if (scroll_state != nullptr) {
        RECT client_rect = {};
        GetClientRect(wnd, &client_rect);

        const int client_width =
            client_rect.right - client_rect.left;
        const int client_height =
            client_rect.bottom - client_rect.top;

        configure_dialog_scroll_bar(
            wnd,
            SB_HORZ,
            scroll_state->content_width,
            client_width,
            scroll_state->position_x
        );
        configure_dialog_scroll_bar(
            wnd,
            SB_VERT,
            scroll_state->content_height,
            client_height,
            scroll_state->position_y
        );

        ShowScrollBar(
            wnd,
            SB_HORZ,
            horizontal_scroll ? TRUE : FALSE
        );
        ShowScrollBar(
            wnd,
            SB_VERT,
            vertical_scroll ? TRUE : FALSE
        );
    }
}

bool scroll_config_dialog(
    HWND wnd,
    dialog_scroll_state& state,
    int bar,
    UINT request
) {
    SCROLLINFO info = {};
    info.cbSize = sizeof(info);
    info.fMask =
        SIF_ALL;

    if (!GetScrollInfo(wnd, bar, &info)) {
        return false;
    }

    const int old_position = info.nPos;
    int new_position = old_position;
    const int line_step = std::max(
        static_cast<int>(info.nPage) / 12,
        16
    );
    const int page_step = std::max(
        static_cast<int>(info.nPage) - line_step,
        line_step
    );

    switch (request) {
    case SB_LINEUP:
        new_position -= line_step;
        break;
    case SB_LINEDOWN:
        new_position += line_step;
        break;
    case SB_PAGEUP:
        new_position -= page_step;
        break;
    case SB_PAGEDOWN:
        new_position += page_step;
        break;
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
        new_position = info.nTrackPos;
        break;
    case SB_TOP:
        new_position = info.nMin;
        break;
    case SB_BOTTOM:
        new_position = info.nMax;
        break;
    default:
        return false;
    }

    const int maximum_position = std::max(
        info.nMax - static_cast<int>(info.nPage) + 1,
        info.nMin
    );
    new_position = clamp_value(
        new_position,
        info.nMin,
        maximum_position
    );

    if (new_position == old_position) {
        return true;
    }

    info.fMask = SIF_POS;
    info.nPos = new_position;
    SetScrollInfo(wnd, bar, &info, TRUE);

    if (bar == SB_HORZ) {
        offset_dialog_children(
            wnd,
            old_position - new_position,
            0
        );
        state.position_x = new_position;
    }
    else {
        offset_dialog_children(
            wnd,
            0,
            old_position - new_position
        );
        state.position_y = new_position;
    }

    RedrawWindow(
        wnd,
        nullptr,
        nullptr,
        RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN
    );
    return true;
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
            ui_text(
                L"%sの値が正しくありません。\n範囲：%.1f ～ %.1f",
                L"The value for %s is invalid.\nRange: %.1f to %.1f"
            ),
            field_name,
            static_cast<double>(minimum),
            static_cast<double>(maximum)
        );
        MessageBoxW(
            wnd,
            message,
            ui_text(L"入力値の確認", L"Check input value"),
            MB_OK | MB_ICONWARNING
        );
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
        layout_name = ui_text(L"モノラル", L"Mono");
    }
    else if (channels == 2 && channel_mask == kChannelConfigStereo) {
        layout_name = ui_text(L"ステレオ", L"Stereo");
    }
    else if (channels == 6 && channel_mask == kChannelConfig5Point1) {
        layout_name = ui_text(L"5.1（バック）", L"5.1 (back)");
    }
    else if (channels == 6 && channel_mask == kChannelConfig5Point1Side) {
        layout_name = ui_text(L"5.1（サイド）", L"5.1 (side)");
    }
    else if (channels == 8 && channel_mask == kChannelConfig7Point1) {
        layout_name = L"7.1";
    }

    if (layout_name != nullptr) {
        if (lfe_excluded) {
            swprintf_s(
                output,
                output_count,
                ui_text(L"%s / LFE除外", L"%s / LFE excluded"),
                layout_name
            );
        }
        else {
            swprintf_s(output, output_count, L"%s", layout_name);
        }
        return;
    }

    if (channels == 0) {
        swprintf_s(
            output,
            output_count,
            L"%s",
            ui_text(L"未検出", L"Not detected")
        );
        return;
    }

    if (channel_mask == 0) {
        swprintf_s(
            output,
            output_count,
            ui_text(L"%u ch / 配置不明", L"%u ch / unknown layout"),
            channels
        );
        return;
    }

    if (lfe_excluded) {
        swprintf_s(
            output,
            output_count,
            ui_text(L"%u ch / LFE除外", L"%u ch / LFE excluded"),
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
        return ui_text(L"測定中・保留", L"Measuring / held");
    case 2:
        return ui_text(L"静音保護・保留", L"Silence guard / held");
    case 4:
        return ui_text(L"安全減衰中", L"Safety attenuation");
    default:
        return ui_text(L"通常補正中", L"Normalizing");
    }
}

const wchar_t* peak_guard_state_to_text(int state) {
    switch (state) {
    case 0:
        return ui_text(L"無効", L"Disabled");
    case 2:
        return ui_text(L"作動中", L"Active");
    default:
        return ui_text(L"待機", L"Standby");
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

UINT dpi_for_window_or_default(HWND wnd);

constexpr UINT_PTR kHistoryDialogTimerId = 2;
constexpr UINT kHistoryDialogRefreshMilliseconds = 1000;

struct auto_control_history_dialog_context {
    fb2k::CCoreDarkModeHooks dark_mode;
    unsigned long long displayed_revision =
        std::numeric_limits<unsigned long long>::max();
};

std::wstring history_timestamp_to_text(
    unsigned long long timestamp
) {
    ULARGE_INTEGER value = {};
    value.QuadPart = timestamp;

    FILETIME utc = {};
    utc.dwLowDateTime = value.LowPart;
    utc.dwHighDateTime = value.HighPart;

    FILETIME local = {};
    SYSTEMTIME system_time = {};

    if (!FileTimeToLocalFileTime(&utc, &local) ||
        !FileTimeToSystemTime(&local, &system_time)) {
        return ui_text(L"時刻不明", L"Unknown time");
    }

    wchar_t text[32] = {};
    swprintf_s(
        text,
        L"%04u-%02u-%02u %02u:%02u:%02u",
        static_cast<unsigned>(system_time.wYear),
        static_cast<unsigned>(system_time.wMonth),
        static_cast<unsigned>(system_time.wDay),
        static_cast<unsigned>(system_time.wHour),
        static_cast<unsigned>(system_time.wMinute),
        static_cast<unsigned>(system_time.wSecond)
    );
    return text;
}

std::wstring history_single_line_text(
    const std::wstring& value,
    const wchar_t* fallback
) {
    std::wstring result = value.empty()
        ? std::wstring(fallback)
        : value;

    for (wchar_t& character : result) {
        if (character == L'\t' ||
            character == L'\r' ||
            character == L'\n') {
            character = L' ';
        }
    }

    return result;
}

const wchar_t* history_profile_to_text(int profile_id) {
    if (!valid_recognized_profile_id(profile_id)) {
        profile_id =
            static_cast<int>(recognized_profile::custom);
    }

    return recognized_profile_name(
        static_cast<recognized_profile>(profile_id)
    );
}

void initialize_auto_control_history_columns(HWND list) {
    if (list == nullptr) {
        return;
    }

    while (ListView_DeleteColumn(list, 0)) {
    }

    const wchar_t* headers[] = {
        ui_text(L"再生日時", L"Played"),
        ui_text(L"曲名", L"Title"),
        ui_text(L"アーティスト", L"Artist"),
        ui_text(L"プリセット", L"Preset"),
        ui_text(L"発動理由", L"Trigger reason"),
        ui_text(L"最大自動減衰", L"Max attenuation"),
        ui_text(L"発動回数", L"Activations"),
        ui_text(L"上限", L"Limit"),
        ui_text(L"復帰", L"Recovery")
    };

    const int base_widths[] = {
        132, 200, 150, 150, 140, 105, 78, 64, 82
    };
    const UINT dpi = dpi_for_window_or_default(list);

    for (int index = 0;
         index < static_cast<int>(std::size(headers));
         ++index) {
        LVCOLUMNW column = {};
        column.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
        column.pszText = const_cast<LPWSTR>(headers[index]);
        column.cx = MulDiv(base_widths[index], dpi, 96);
        column.fmt = index >= 5
            ? LVCFMT_CENTER
            : LVCFMT_LEFT;
        SendMessageW(
            list,
            LVM_INSERTCOLUMNW,
            static_cast<WPARAM>(index),
            reinterpret_cast<LPARAM>(&column)
        );
    }
}

void set_auto_control_history_cell(
    HWND list,
    int row,
    int column,
    const std::wstring& text
) {
    LVITEMW item = {};
    item.mask = LVIF_TEXT;
    item.iItem = row;
    item.iSubItem = column;
    item.pszText = const_cast<LPWSTR>(text.c_str());

    SendMessageW(
        list,
        LVM_SETITEMW,
        0,
        reinterpret_cast<LPARAM>(&item)
    );
}

void refresh_auto_control_history_list(
    HWND wnd,
    auto_control_history_dialog_context* context,
    bool force
) {
    ensure_auto_control_history_loaded();

    if (context != nullptr &&
        !force &&
        context->displayed_revision ==
            g_auto_control_history_revision) {
        return;
    }

    HWND list = GetDlgItem(wnd, IDC_HISTORY_LIST);
    if (list == nullptr) {
        return;
    }

    const int selected_row = ListView_GetNextItem(
        list,
        -1,
        LVNI_SELECTED
    );

    ListView_DeleteAllItems(list);

    for (t_size index = 0;
         index < g_auto_control_history_entries.size();
         ++index) {
        const auto& entry =
            g_auto_control_history_entries[index];

        const std::wstring timestamp =
            history_timestamp_to_text(entry.timestamp);
        const std::wstring title = history_single_line_text(
            entry.title,
            ui_text(L"不明な曲", L"Unknown track")
        );
        const std::wstring artist = history_single_line_text(
            entry.artist,
            ui_text(L"不明", L"Unknown")
        );
        const std::wstring profile =
            history_profile_to_text(entry.profile_id);
        const std::wstring reason =
            auto_control_reason_to_text(entry.reason_mask);

        wchar_t attenuation[32] = {};
        swprintf_s(
            attenuation,
            L"%.2f dB",
            entry.maximum_attenuation_db
        );

        wchar_t trigger_count[24] = {};
        swprintf_s(
            trigger_count,
            ui_text(L"%u 回", L"%u"),
            entry.trigger_count
        );

        LVITEMW item = {};
        item.mask = LVIF_TEXT;
        item.iItem = static_cast<int>(index);
        item.iSubItem = 0;
        item.pszText = const_cast<LPWSTR>(timestamp.c_str());

        const int row = static_cast<int>(
            SendMessageW(
                list,
                LVM_INSERTITEMW,
                0,
                reinterpret_cast<LPARAM>(&item)
            )
        );
        if (row < 0) {
            continue;
        }

        set_auto_control_history_cell(list, row, 1, title);
        set_auto_control_history_cell(list, row, 2, artist);
        set_auto_control_history_cell(list, row, 3, profile);
        set_auto_control_history_cell(list, row, 4, reason);
        set_auto_control_history_cell(
            list,
            row,
            5,
            attenuation
        );
        set_auto_control_history_cell(
            list,
            row,
            6,
            trigger_count
        );
        set_auto_control_history_cell(
            list,
            row,
            7,
            entry.adjustment_limit_reached
                ? ui_text(L"到達", L"Reached")
                : ui_text(L"なし", L"No")
        );
        set_auto_control_history_cell(
            list,
            row,
            8,
            entry.recovered
                ? ui_text(L"復帰済み", L"Recovered")
                : ui_text(L"未復帰", L"Not yet")
        );
    }

    if (selected_row >= 0 &&
        selected_row <
            static_cast<int>(
                g_auto_control_history_entries.size()
            )) {
        ListView_SetItemState(
            list,
            selected_row,
            LVIS_SELECTED | LVIS_FOCUSED,
            LVIS_SELECTED | LVIS_FOCUSED
        );
    }

    wchar_t summary[160] = {};
    if (g_auto_control_history_entries.empty()) {
        swprintf_s(
            summary,
            L"%s",
            ui_text(
                L"自動制御が発動した履歴はまだありません。",
                L"No automatic-control activations have been recorded yet."
            )
        );
    }
    else {
        swprintf_s(
            summary,
            ui_text(
                L"自動制御が発動した最新100曲を表示します（現在 %zu 件）。",
                L"Shows up to 100 tracks with automatic-control activity "
                L"(%zu currently)."
            ),
            g_auto_control_history_entries.size()
        );
    }
    SetDlgItemTextW(wnd, IDC_HISTORY_SUMMARY, summary);

    EnableWindow(
        GetDlgItem(wnd, IDC_HISTORY_DELETE_SELECTED),
        ListView_GetNextItem(list, -1, LVNI_SELECTED) >= 0
    );
    EnableWindow(
        GetDlgItem(wnd, IDC_HISTORY_DELETE_ALL),
        !g_auto_control_history_entries.empty()
    );
    EnableWindow(
        GetDlgItem(wnd, IDC_HISTORY_COPY),
        !g_auto_control_history_entries.empty()
    );

    if (context != nullptr) {
        context->displayed_revision =
            g_auto_control_history_revision;
    }
}

std::wstring build_auto_control_history_report() {
    ensure_auto_control_history_loaded();

    std::wostringstream output;
    output
        << ui_text(
            L"再生日時\t曲名\tアーティスト\tプリセット\t"
            L"発動理由\t最大自動減衰量\t発動回数\t調整上限\t安全復帰\r\n",
            L"Played\tTitle\tArtist\tPreset\tTrigger reason\t"
            L"Maximum automatic attenuation\tActivations\t"
            L"Adjustment limit\tSafe recovery\r\n"
        );

    for (const auto& entry : g_auto_control_history_entries) {
        const std::wstring title = history_single_line_text(
            entry.title,
            ui_text(L"不明な曲", L"Unknown track")
        );
        const std::wstring artist = history_single_line_text(
            entry.artist,
            ui_text(L"不明", L"Unknown")
        );

        output
            << history_timestamp_to_text(entry.timestamp) << L'\t'
            << title << L'\t'
            << artist << L'\t'
            << history_profile_to_text(entry.profile_id) << L'\t'
            << auto_control_reason_to_text(entry.reason_mask) << L'\t';

        wchar_t attenuation[32] = {};
        swprintf_s(
            attenuation,
            L"%.2f dB",
            entry.maximum_attenuation_db
        );

        output
            << attenuation << L'\t'
            << entry.trigger_count << L'\t'
            << (entry.adjustment_limit_reached
                ? ui_text(L"到達", L"Reached")
                : ui_text(L"なし", L"No")) << L'\t'
            << (entry.recovered
                ? ui_text(L"復帰済み", L"Recovered")
                : ui_text(L"未復帰", L"Not yet"))
            << L"\r\n";
    }

    return output.str();
}

void delete_selected_auto_control_history_entry(
    HWND wnd,
    auto_control_history_dialog_context* context
) {
    HWND list = GetDlgItem(wnd, IDC_HISTORY_LIST);
    const int selected = list != nullptr
        ? ListView_GetNextItem(list, -1, LVNI_SELECTED)
        : -1;

    if (selected < 0 ||
        selected >= static_cast<int>(
            g_auto_control_history_entries.size()
        )) {
        return;
    }

    const auto& entry =
        g_auto_control_history_entries[
            static_cast<t_size>(selected)
        ];
    if (entry.session_id != 0 &&
        entry.session_id ==
            g_auto_control_history_active_session_id) {
        g_auto_control_history_suppressed_session_id =
            entry.session_id;
    }

    g_auto_control_history_entries.erase(
        g_auto_control_history_entries.begin() + selected
    );
    save_auto_control_history();
    refresh_auto_control_history_list(wnd, context, true);
}

void delete_all_auto_control_history_entries(
    HWND wnd,
    auto_control_history_dialog_context* context
) {
    if (g_auto_control_history_entries.empty()) {
        return;
    }

    if (MessageBoxW(
            wnd,
            ui_text(
                L"自動制御履歴をすべて削除します。\n"
                L"この操作は元に戻せません。続けますか？",
                L"Delete all automatic-control history?\n"
                L"This action cannot be undone."
            ),
            ui_text(
                L"自動制御履歴の削除",
                L"Delete automatic-control history"
            ),
            MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2
        ) != IDYES) {
        return;
    }

    g_auto_control_history_suppressed_session_id =
        g_auto_control_history_active_session_id;
    g_auto_control_history_entries.clear();
    save_auto_control_history();
    refresh_auto_control_history_list(wnd, context, true);
}

struct auto_control_trend_dialog_context {
    fb2k::CCoreDarkModeHooks dark_mode;
    auto_control_trend_snapshot snapshot;
    RECT initial_client = {};
    RECT track_rect = {};
    RECT legend_rect = {};
    RECT graph_rect = {};
    RECT close_button_rect = {};
    int minimum_window_width = 0;
    int minimum_window_height = 0;
    bool layout_ready = false;
};

RECT dialog_child_rect(HWND wnd, int control_id) {
    RECT rect = {};
    HWND control = GetDlgItem(wnd, control_id);

    if (control == nullptr || !GetWindowRect(control, &rect)) {
        return rect;
    }

    MapWindowPoints(
        HWND_DESKTOP,
        wnd,
        reinterpret_cast<POINT*>(&rect),
        2
    );
    return rect;
}

void initialize_auto_control_trend_layout(
    HWND wnd,
    auto_control_trend_dialog_context* context
) {
    if (context == nullptr) {
        return;
    }

    GetClientRect(wnd, &context->initial_client);
    context->track_rect = dialog_child_rect(wnd, IDC_TREND_TRACK);
    context->legend_rect = dialog_child_rect(wnd, IDC_TREND_LEGEND);
    context->graph_rect = dialog_child_rect(wnd, IDC_TREND_GRAPH);
    context->close_button_rect = dialog_child_rect(wnd, IDOK);

    RECT window_rect = {};
    GetWindowRect(wnd, &window_rect);
    context->minimum_window_width = std::max<int>(
        1,
        static_cast<int>(window_rect.right - window_rect.left) * 2 / 3
    );
    context->minimum_window_height = std::max<int>(
        1,
        static_cast<int>(window_rect.bottom - window_rect.top) * 2 / 3
    );
    context->layout_ready = true;
}

void resize_auto_control_trend_dialog(
    HWND wnd,
    auto_control_trend_dialog_context* context,
    int client_width,
    int client_height
) {
    if (context == nullptr ||
        !context->layout_ready ||
        client_width <= 0 ||
        client_height <= 0) {
        return;
    }

    const int initial_width =
        context->initial_client.right - context->initial_client.left;
    const int initial_height =
        context->initial_client.bottom - context->initial_client.top;

    const auto move_control = [wnd](
        int control_id,
        int x,
        int y,
        int width,
        int height
    ) {
        SetWindowPos(
            GetDlgItem(wnd, control_id),
            nullptr,
            x,
            y,
            std::max(1, width),
            std::max(1, height),
            SWP_NOACTIVATE | SWP_NOZORDER
        );
    };

    const int track_right_margin =
        initial_width - context->track_rect.right;
    move_control(
        IDC_TREND_TRACK,
        context->track_rect.left,
        context->track_rect.top,
        client_width - context->track_rect.left - track_right_margin,
        context->track_rect.bottom - context->track_rect.top
    );

    const int legend_right_margin =
        initial_width - context->legend_rect.right;
    move_control(
        IDC_TREND_LEGEND,
        context->legend_rect.left,
        context->legend_rect.top,
        client_width - context->legend_rect.left - legend_right_margin,
        context->legend_rect.bottom - context->legend_rect.top
    );

    const int graph_right_margin =
        initial_width - context->graph_rect.right;
    const int graph_bottom_margin =
        initial_height - context->graph_rect.bottom;
    move_control(
        IDC_TREND_GRAPH,
        context->graph_rect.left,
        context->graph_rect.top,
        client_width - context->graph_rect.left - graph_right_margin,
        client_height - context->graph_rect.top - graph_bottom_margin
    );

    const int close_right_margin =
        initial_width - context->close_button_rect.right;
    const int close_bottom_margin =
        initial_height - context->close_button_rect.bottom;
    const int close_width =
        context->close_button_rect.right -
        context->close_button_rect.left;
    const int close_height =
        context->close_button_rect.bottom -
        context->close_button_rect.top;
    move_control(
        IDOK,
        client_width - close_right_margin - close_width,
        client_height - close_bottom_margin - close_height,
        close_width,
        close_height
    );

    InvalidateRect(GetDlgItem(wnd, IDC_TREND_GRAPH), nullptr, FALSE);
}

std::wstring auto_control_trend_track_text(
    const auto_control_trend_snapshot& snapshot
) {
    if (!snapshot.artist.empty() && !snapshot.title.empty()) {
        return snapshot.artist + L" - " + snapshot.title;
    }
    if (!snapshot.title.empty()) {
        return snapshot.title;
    }
    if (!snapshot.artist.empty()) {
        return snapshot.artist;
    }

    return ui_text(
        L"再生中の曲を待っています",
        L"Waiting for a playing track"
    );
}

void refresh_auto_control_trend_dialog(
    HWND wnd,
    auto_control_trend_dialog_context* context,
    bool force
) {
    if (context == nullptr) {
        return;
    }

    auto_control_trend_snapshot snapshot =
        current_auto_control_trend_snapshot();

    if (!force &&
        snapshot.revision == context->snapshot.revision) {
        return;
    }

    context->snapshot = std::move(snapshot);
    SetDlgItemTextW(
        wnd,
        IDC_TREND_TRACK,
        auto_control_trend_track_text(context->snapshot).c_str()
    );
    InvalidateRect(
        GetDlgItem(wnd, IDC_TREND_GRAPH),
        nullptr,
        FALSE
    );
}

double trend_lane_value(
    const auto_control_trend_sample& sample,
    int lane
) {
    switch (lane) {
    case 0:
        return sample.short_term_lufs;
    case 1:
        return sample.applied_gain_db;
    case 2:
        return sample.automatic_attenuation_db;
    default:
        return sample.true_peak_dbtp;
    }
}

void draw_auto_control_trend_graph(
    const DRAWITEMSTRUCT& item,
    const auto_control_trend_snapshot& snapshot,
    bool dark_mode
) {
    const HDC dc = item.hDC;
    const RECT bounds = item.rcItem;
    const COLORREF background =
        dark_mode ? RGB(32, 32, 32) : RGB(255, 255, 255);
    const COLORREF text_color =
        dark_mode ? RGB(235, 235, 235) : RGB(32, 32, 32);
    const COLORREF grid_color =
        dark_mode ? RGB(74, 74, 74) : RGB(218, 218, 218);
    const COLORREF event_color = RGB(220, 65, 65);

    HBRUSH background_brush = CreateSolidBrush(background);
    FillRect(dc, &bounds, background_brush);
    DeleteObject(background_brush);

    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, text_color);

    if (snapshot.samples.empty()) {
        RECT message_bounds = bounds;
        DrawTextW(
            dc,
            ui_text(
                L"再生を開始すると、約1秒ごとに推移を記録します。",
                L"Start playback to record the trend about once per second."
            ),
            -1,
            &message_bounds,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX
        );
        return;
    }

    const int left_margin = 118;
    const int right_margin = 46;
    const int top_margin = 8;
    const int bottom_margin = 24;
    const int plot_left = bounds.left + left_margin;
    const int plot_right = std::max(
        plot_left + 1,
        static_cast<int>(bounds.right) - right_margin
    );
    const int plot_top = bounds.top + top_margin;
    const int plot_bottom = std::max(
        plot_top + 4,
        static_cast<int>(bounds.bottom) - bottom_margin
    );
    const int plot_width = std::max(1, plot_right - plot_left);
    const int plot_height = std::max(4, plot_bottom - plot_top);

    const wchar_t* lane_names[] = {
        ui_text(L"Short-term", L"Short-term"),
        ui_text(L"総ゲイン", L"Total gain"),
        ui_text(L"自動減衰", L"Auto attenuation"),
        ui_text(L"True Peak", L"True Peak")
    };
    const wchar_t* lane_units[] = {
        L"LUFS",
        L"dB",
        L"dB",
        L"dBTP"
    };
    const double lane_minimums[] = {
        -60.0,
        -24.0,
        0.0,
        -12.0
    };
    const double lane_maximums[] = {
        0.0,
        24.0,
        kAutoSafetyMaximumReductionDb,
        0.0
    };
    const COLORREF lane_colors[] = {
        RGB(48, 140, 230),
        RGB(45, 175, 105),
        RGB(235, 145, 35),
        RGB(155, 100, 220)
    };

    HPEN grid_pen = CreatePen(PS_SOLID, 1, grid_color);
    const HGDIOBJ previous_pen = SelectObject(dc, grid_pen);

    for (int index = 0; index <= 4; ++index) {
        const int x =
            plot_left + plot_width * index / 4;
        MoveToEx(dc, x, plot_top, nullptr);
        LineTo(dc, x, plot_bottom);
    }

    for (int lane = 0; lane <= 4; ++lane) {
        const int y =
            plot_top + plot_height * lane / 4;
        MoveToEx(dc, plot_left, y, nullptr);
        LineTo(dc, plot_right, y);
    }

    SelectObject(dc, previous_pen);
    DeleteObject(grid_pen);

    const double first_seconds =
        snapshot.samples.front().playback_seconds;
    const double last_seconds = std::max(
        first_seconds + 1.0,
        snapshot.samples.back().playback_seconds
    );
    const double duration_seconds =
        std::max(1.0, last_seconds - first_seconds);

    HPEN event_pen = CreatePen(PS_SOLID, 1, event_color);
    const HGDIOBJ old_event_pen = SelectObject(dc, event_pen);

    for (t_size index = 0; index < snapshot.samples.size(); ++index) {
        const int state = snapshot.samples[index].processing_state;
        const int previous_state =
            index > 0
                ? snapshot.samples[index - 1].processing_state
                : 0;
        const bool activated =
            (state == 2 || state == 3) &&
            previous_state != 2 &&
            previous_state != 3;

        if (!activated) {
            continue;
        }

        const double position =
            (snapshot.samples[index].playback_seconds - first_seconds) /
            duration_seconds;
        const int x = plot_left + static_cast<int>(
            position * static_cast<double>(plot_width)
        );

        MoveToEx(dc, x, plot_top, nullptr);
        LineTo(dc, x, plot_bottom);
    }

    SelectObject(dc, old_event_pen);
    DeleteObject(event_pen);

    for (int lane = 0; lane < 4; ++lane) {
        const int lane_top =
            plot_top + plot_height * lane / 4;
        const int lane_bottom =
            plot_top + plot_height * (lane + 1) / 4;

        RECT name_bounds = {
            bounds.left + 6,
            lane_top + 4,
            plot_left - 8,
            lane_top + 22
        };
        DrawTextW(
            dc,
            lane_names[lane],
            -1,
            &name_bounds,
            DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX
        );

        wchar_t range_text[64] = {};
        swprintf_s(
            range_text,
            L"%.0f ... %.0f %s",
            lane_minimums[lane],
            lane_maximums[lane],
            lane_units[lane]
        );
        RECT range_bounds = {
            bounds.left + 6,
            lane_top + 22,
            plot_left - 8,
            lane_bottom - 2
        };
        DrawTextW(
            dc,
            range_text,
            -1,
            &range_bounds,
            DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS |
                DT_NOPREFIX
        );

        HPEN series_pen =
            CreatePen(PS_SOLID, 2, lane_colors[lane]);
        const HGDIOBJ old_pen = SelectObject(dc, series_pen);
        bool previous_valid = false;

        for (const auto& sample : snapshot.samples) {
            const double value = trend_lane_value(sample, lane);
            const bool valid =
                std::isfinite(value) &&
                (lane == 1 || lane == 2 || value > -190.0);

            if (!valid) {
                previous_valid = false;
                continue;
            }

            const double position =
                (sample.playback_seconds - first_seconds) /
                duration_seconds;
            const double normalized = clamp_value(
                (value - lane_minimums[lane]) /
                    (lane_maximums[lane] - lane_minimums[lane]),
                0.0,
                1.0
            );
            const int x = plot_left + static_cast<int>(
                position * static_cast<double>(plot_width)
            );
            const int y = lane_bottom - 2 - static_cast<int>(
                normalized *
                static_cast<double>(
                    std::max(1, lane_bottom - lane_top - 4)
                )
            );

            if (previous_valid) {
                LineTo(dc, x, y);
            }
            else {
                MoveToEx(dc, x, y, nullptr);
            }
            previous_valid = true;
        }

        SelectObject(dc, old_pen);
        DeleteObject(series_pen);
    }

    const auto format_time = [](double seconds) {
        const unsigned total = static_cast<unsigned>(
            std::max(0.0, seconds)
        );
        wchar_t text[32] = {};
        swprintf_s(
            text,
            L"%u:%02u",
            total / 60,
            total % 60
        );
        return std::wstring(text);
    };

    const double time_values[] = {
        first_seconds,
        first_seconds + duration_seconds * 0.5,
        last_seconds
    };
    const int time_positions[] = {
        plot_left,
        plot_left + plot_width / 2,
        plot_right
    };
    const UINT time_alignments[] = {
        DT_LEFT,
        DT_CENTER,
        DT_RIGHT
    };

    for (int index = 0; index < 3; ++index) {
        RECT time_bounds = {
            time_positions[index] - 44,
            plot_bottom + 4,
            time_positions[index] + 44,
            bounds.bottom - 2
        };
        const std::wstring time_text =
            format_time(time_values[index]);
        DrawTextW(
            dc,
            time_text.c_str(),
            -1,
            &time_bounds,
            time_alignments[index] | DT_SINGLELINE | DT_NOPREFIX
        );
    }
}

INT_PTR CALLBACK auto_control_trend_dialog_proc(
    HWND wnd,
    UINT message,
    WPARAM wp,
    LPARAM lp
) {
    auto* context =
        reinterpret_cast<auto_control_trend_dialog_context*>(
            GetWindowLongPtrW(wnd, GWLP_USERDATA)
        );

    switch (message) {
    case WM_INITDIALOG:
        context = new auto_control_trend_dialog_context();
        SetWindowLongPtrW(
            wnd,
            GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(context)
        );
        context->dark_mode.AddDialogWithControls(wnd);
        initialize_auto_control_trend_layout(wnd, context);

        SetWindowTextW(
            wnd,
            ui_text(
                L"R128 音量ノーマライザー 自動制御推移グラフ",
                L"R128 Loudness Normalizer - Automatic-Control Trend"
            )
        );
        SetDlgItemTextW(
            wnd,
            IDC_TREND_LEGEND,
            ui_text(
                L"赤い縦線：自動制御の発動位置",
                L"Red vertical line: automatic-control activation"
            )
        );
        SetDlgItemTextW(
            wnd,
            IDOK,
            ui_text(L"閉じる", L"Close")
        );
        refresh_auto_control_trend_dialog(wnd, context, true);
        SetTimer(
            wnd,
            kTrendDialogTimerId,
            kTrendDialogRefreshMilliseconds,
            nullptr
        );
        return TRUE;

    case WM_GETMINMAXINFO:
        if (context != nullptr && context->layout_ready && lp != 0) {
            auto* size_info = reinterpret_cast<MINMAXINFO*>(lp);
            size_info->ptMinTrackSize.x =
                context->minimum_window_width;
            size_info->ptMinTrackSize.y =
                context->minimum_window_height;
            return TRUE;
        }
        break;

    case WM_SIZE:
        resize_auto_control_trend_dialog(
            wnd,
            context,
            LOWORD(lp),
            HIWORD(lp)
        );
        return TRUE;

    case WM_DRAWITEM:
        if (wp == IDC_TREND_GRAPH && lp != 0 && context != nullptr) {
            draw_auto_control_trend_graph(
                *reinterpret_cast<const DRAWITEMSTRUCT*>(lp),
                context->snapshot,
                static_cast<bool>(context->dark_mode)
            );
            return TRUE;
        }
        break;

    case WM_TIMER:
        if (wp == kTrendDialogTimerId) {
            refresh_auto_control_trend_dialog(
                wnd,
                context,
                false
            );
            return TRUE;
        }
        break;

    case WM_THEMECHANGED:
    case WM_SYSCOLORCHANGE:
    case WM_SETTINGCHANGE:
        InvalidateRect(
            GetDlgItem(wnd, IDC_TREND_GRAPH),
            nullptr,
            FALSE
        );
        return TRUE;

    case WM_COMMAND:
        if (LOWORD(wp) == IDOK || LOWORD(wp) == IDCANCEL) {
            EndDialog(wnd, LOWORD(wp));
            return TRUE;
        }
        break;

    case WM_CLOSE:
        EndDialog(wnd, IDCANCEL);
        return TRUE;

    case WM_DESTROY:
        KillTimer(wnd, kTrendDialogTimerId);
        return TRUE;

    case WM_NCDESTROY:
        SetWindowLongPtrW(wnd, GWLP_USERDATA, 0);
        delete context;
        return FALSE;
    }

    return FALSE;
}

INT_PTR CALLBACK auto_control_history_dialog_proc(
    HWND wnd,
    UINT message,
    WPARAM wp,
    LPARAM lp
) {
    auto* context =
        reinterpret_cast<auto_control_history_dialog_context*>(
            GetWindowLongPtrW(wnd, GWLP_USERDATA)
        );

    switch (message) {
    case WM_INITDIALOG: {
        INITCOMMONCONTROLSEX controls = {};
        controls.dwSize = sizeof(controls);
        controls.dwICC = ICC_LISTVIEW_CLASSES;
        InitCommonControlsEx(&controls);

        context = new auto_control_history_dialog_context();
        SetWindowLongPtrW(
            wnd,
            GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(context)
        );
        context->dark_mode.AddDialogWithControls(wnd);

        SetWindowTextW(
            wnd,
            ui_text(
                L"R128 音量ノーマライザー 自動制御履歴",
                L"R128 Loudness Normalizer - Automatic-Control History"
            )
        );
        SetDlgItemTextW(
            wnd,
            IDC_HISTORY_COPY,
            ui_text(L"履歴コピー", L"Copy History")
        );
        SetDlgItemTextW(
            wnd,
            IDC_HISTORY_DELETE_SELECTED,
            ui_text(L"選択削除", L"Delete Selected")
        );
        SetDlgItemTextW(
            wnd,
            IDC_HISTORY_DELETE_ALL,
            ui_text(L"すべて削除", L"Delete All")
        );
        SetDlgItemTextW(
            wnd,
            IDOK,
            ui_text(L"閉じる", L"Close")
        );

        HWND list = GetDlgItem(wnd, IDC_HISTORY_LIST);
        ListView_SetExtendedListViewStyle(
            list,
            LVS_EX_FULLROWSELECT |
            LVS_EX_GRIDLINES |
            LVS_EX_DOUBLEBUFFER
        );
        initialize_auto_control_history_columns(list);
        refresh_auto_control_history_list(
            wnd,
            context,
            true
        );
        SetTimer(
            wnd,
            kHistoryDialogTimerId,
            kHistoryDialogRefreshMilliseconds,
            nullptr
        );
        return TRUE;
    }

    case WM_TIMER:
        if (wp == kHistoryDialogTimerId) {
            refresh_auto_control_history_list(
                wnd,
                context,
                false
            );
            return TRUE;
        }
        break;

    case WM_NOTIFY:
        if (lp != 0) {
            const auto* header =
                reinterpret_cast<const NMHDR*>(lp);

            if (header->idFrom == IDC_HISTORY_LIST &&
                header->code == LVN_ITEMCHANGED) {
                HWND list = GetDlgItem(wnd, IDC_HISTORY_LIST);
                EnableWindow(
                    GetDlgItem(
                        wnd,
                        IDC_HISTORY_DELETE_SELECTED
                    ),
                    ListView_GetNextItem(
                        list,
                        -1,
                        LVNI_SELECTED
                    ) >= 0
                );
                return TRUE;
            }
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wp)) {
        case IDC_HISTORY_COPY:
            if (copy_unicode_text_to_clipboard(
                    wnd,
                    build_auto_control_history_report()
                )) {
                MessageBoxW(
                    wnd,
                    ui_text(
                        L"自動制御履歴をクリップボードへコピーしました。",
                        L"Automatic-control history copied to the clipboard."
                    ),
                    ui_text(
                        L"履歴コピー",
                        L"Copy History"
                    ),
                    MB_OK | MB_ICONINFORMATION
                );
            }
            return TRUE;

        case IDC_HISTORY_DELETE_SELECTED:
            delete_selected_auto_control_history_entry(
                wnd,
                context
            );
            return TRUE;

        case IDC_HISTORY_DELETE_ALL:
            delete_all_auto_control_history_entries(
                wnd,
                context
            );
            return TRUE;

        case IDOK:
        case IDCANCEL:
            EndDialog(wnd, LOWORD(wp));
            return TRUE;
        }
        break;

    case WM_CLOSE:
        EndDialog(wnd, IDCANCEL);
        return TRUE;

    case WM_DESTROY:
        KillTimer(wnd, kHistoryDialogTimerId);
        return TRUE;

    case WM_NCDESTROY:
        SetWindowLongPtrW(wnd, GWLP_USERDATA, 0);
        delete context;
        return FALSE;
    }

    return FALSE;
}

std::wstring format_diagnostic_number(
    double value,
    const wchar_t* unit,
    int decimals
) {
    wchar_t buffer[64] = {};

    if (!std::isfinite(value) || value <= -190.0) {
        return ui_text(L"未測定", L"Not measured");
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
    const int current_processing_state =
        g_diagnostic_current_processing_state.load(
            std::memory_order_relaxed
        );
    const int auto_control_reason_mask =
        g_diagnostic_auto_control_reason_mask.load(
            std::memory_order_relaxed
        );
    const unsigned auto_control_trigger_count =
        g_history_auto_control_trigger_count.load(
            std::memory_order_relaxed
        );
    const int latest_auto_control_reason_mask =
        g_history_latest_auto_control_reason_mask.load(
            std::memory_order_relaxed
        );
    const bool latest_auto_control_recovered =
        g_history_recovered.load(
            std::memory_order_relaxed
        ) != 0;
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
        swprintf_s(
            gain_lock_text,
            L"%s",
            ui_text(L"無効", L"Disabled")
        );
        break;
    case 2:
        swprintf_s(
            gain_lock_text,
            ui_text(L"固定済み %+.2f dB", L"Locked at %+.2f dB"),
            locked_gain_db
        );
        break;
    case 3:
        if (normalization_gain_db < -0.01) {
            swprintf_s(
                gain_lock_text,
                ui_text(
                    L"固定 %+.2f → 現在 %+.2f dB（安全減衰）",
                    L"Locked %+.2f -> current %+.2f dB (safety attenuation)"
                ),
                locked_gain_db,
                normalization_gain_db
            );
        }
        else {
            swprintf_s(
                gain_lock_text,
                ui_text(
                    L"固定 %+.2f → 現在 %+.2f dB（増幅抑制）",
                    L"Locked %+.2f -> current %+.2f dB (boost limited)"
                ),
                locked_gain_db,
                normalization_gain_db
            );
        }
        break;
    default:
        swprintf_s(
            gain_lock_text,
            ui_text(L"残り %.1f秒", L"%.1f sec remaining"),
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
    const std::wstring auto_control_reason =
        diagnostic_auto_control_reason_text(
            auto_control_reason_mask,
            latest_auto_control_reason_mask,
            current_processing_state,
            auto_control_trigger_count,
            latest_auto_control_recovered
        );

    wchar_t report[6656] = {};
    swprintf_s(
        report,
        ui_text(
        L"R128 音量ノーマライザー 1.9.0\r\n"
        L"再生状態: %s\r\n"
        L"補正状態: %s\r\n"
        L"補正ゲイン固定: %s\r\n"
        L"モダンブースト: %s\r\n"
        L"A/B比較: %s\r\n"
        L"A/B一致ゲイン: %+.2f dB（15msフェード）\r\n"
        L"1バンド・アダプティブ: %s\r\n"
        L"3バンド・アダプティブ: %s\r\n"
        L"実効モダン強度: %.1f %%\r\n"
        L"3バンド減衰（現在・低／中／高）: %.2f / %.2f / %.2f dB\r\n"
        L"3バンド最大減衰（低／中／高）: %.2f / %.2f / %.2f dB\r\n"
        L"Momentary（入力）: %s\r\n"
        L"Short-term（入力）: %s\r\n"
        L"Integrated（入力）: %s\r\n"
        L"Integrated（出力）: %s\r\n"
        L"目標との差: %s\r\n"
        L"LRA推定: %s\r\n"
        L"追加処理状態: %s\r\n"
        L"自動減衰量: %.2f dB\r\n"
        L"現在の処理状態: %s\r\n"
        L"自動制御の理由: %s\r\n"
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
        L"3バンド・アダプティブ使用: %s\r\n"
        L"3バンド最大減衰（低／中／高）: %.2f / %.2f / %.2f dB\r\n"
        L"0 dBTP超過イベント: %llu 回\r\n"
        L"異常値保護作動: %llu サンプル\r\n"
        L"処理評価: %s\r\n"
        L"サンプルレート: %u Hz\r\n"
        L"推定CPU負荷: %.2f %%\r\n",
        L"R128 Loudness Normalizer 1.9.0\r\n"
        L"Playback state: %s\r\n"
        L"Normalization state: %s\r\n"
        L"Gain lock: %s\r\n"
        L"Modern Boost: %s\r\n"
        L"A/B comparison: %s\r\n"
        L"A/B match gain: %+.2f dB (15 ms fade)\r\n"
        L"1-Band Adaptive: %s\r\n"
        L"3-Band Adaptive: %s\r\n"
        L"Effective Modern strength: %.1f %%\r\n"
        L"Current 3-band reduction (L/M/H): %.2f / %.2f / %.2f dB\r\n"
        L"Maximum 3-band reduction (L/M/H): %.2f / %.2f / %.2f dB\r\n"
        L"Momentary (input): %s\r\n"
        L"Short-term (input): %s\r\n"
        L"Integrated (input): %s\r\n"
        L"Integrated (output): %s\r\n"
        L"Difference from target: %s\r\n"
        L"Estimated LRA: %s\r\n"
        L"Additional processing: %s\r\n"
        L"Automatic attenuation: %.2f dB\r\n"
        L"Current processing state: %s\r\n"
        L"Automatic-control reason: %s\r\n"
        L"Current normalization gain: %+.2f dB\r\n"
        L"Total applied gain: %+.2f dB\r\n"
        L"Compressor reduction: %.2f dB\r\n"
        L"Clipper reduction: %.2f dB\r\n"
        L"Limiter reduction: %.2f dB\r\n"
        L"Detected True Peak: %s\r\n"
        L"Peak protection: %s\r\n"
        L"Effective latency: %.1f ms\r\n"
        L"Sample rate: %u Hz\r\n"
        L"Estimated CPU load: %.2f %%\r\n"
        L"Track maximum True Peak: %s\r\n"
        L"Maximum compressor reduction: %.2f dB\r\n"
        L"Maximum clipper reduction: %.2f dB\r\n"
        L"Maximum limiter reduction: %.2f dB\r\n"
        L"0 dBTP exceedance events: %llu\r\n"
        L"Invalid samples recovered: %llu\r\n"
        L"Track evaluation: %s\r\n"
        L"Channel layout: %s\r\n"
        L"\r\n"
        L"Previous finalized track: %s\r\n"
        L"Input Integrated: %s\r\n"
        L"Output Integrated: %s\r\n"
        L"Difference from target: %s\r\n"
        L"Estimated LRA: %s\r\n"
        L"Maximum True Peak: %s\r\n"
        L"Maximum compressor reduction: %.2f dB\r\n"
        L"Maximum clipper reduction: %.2f dB\r\n"
        L"Maximum limiter reduction: %.2f dB\r\n"
        L"3-Band Adaptive used: %s\r\n"
        L"Maximum 3-band reduction (L/M/H): %.2f / %.2f / %.2f dB\r\n"
        L"0 dBTP exceedance events: %llu\r\n"
        L"Invalid samples recovered: %llu\r\n"
        L"Processing evaluation: %s\r\n"
        L"Sample rate: %u Hz\r\n"
        L"Estimated CPU load: %.2f %%\r\n"
        ),
        stream_active
            ? ui_text(L"再生中", L"Playing")
            : ui_text(L"待機中", L"Standby"),
        normalization_state_to_text(normalization_state),
        gain_lock_text,
        modern_boost_enabled
            ? ui_text(L"有効", L"Enabled")
            : ui_text(L"無効", L"Disabled"),
        original_compare_mode == 2
            ? ui_text(L"音量一致", L"Loudness matched")
            : (original_compare_mode == 1
                ? ui_text(L"完全バイパス", L"Full bypass")
                : ui_text(L"通常", L"Normal")),
        compare_match_gain_db,
        adaptive_master_enabled
            ? ui_text(L"有効", L"Enabled")
            : ui_text(L"無効", L"Disabled"),
        three_band_master_enabled
            ? ui_text(L"有効", L"Enabled")
            : ui_text(L"無効", L"Disabled"),
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
        std::max(0.0, -safety_reduction_db),
        stream_active
            ? current_processing_state_to_text(current_processing_state)
            : ui_text(L"待機中", L"Standby"),
        stream_active
            ? auto_control_reason.c_str()
            : ui_text(L"待機中", L"Standby"),
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
        final_summary_valid
            ? ui_text(L"あり", L"Available")
            : ui_text(L"なし", L"None"),
        final_input_integrated.c_str(),
        final_output_integrated.c_str(),
        final_target_difference.c_str(),
        final_lra.c_str(),
        final_max_true_peak.c_str(),
        final_max_compressor_reduction_db,
        final_max_clipper_reduction_db,
        final_max_limiter_reduction_db,
        final_three_band_master_enabled
            ? ui_text(L"有効", L"Enabled")
            : ui_text(L"無効", L"Disabled"),
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
    const int current_processing_state =
        g_diagnostic_current_processing_state.load(
            std::memory_order_relaxed
        );
    const int auto_control_reason_mask =
        g_diagnostic_auto_control_reason_mask.load(
            std::memory_order_relaxed
        );
    const unsigned auto_control_trigger_count =
        g_history_auto_control_trigger_count.load(
            std::memory_order_relaxed
        );
    const int latest_auto_control_reason_mask =
        g_history_latest_auto_control_reason_mask.load(
            std::memory_order_relaxed
        );
    const bool latest_auto_control_recovered =
        g_history_recovered.load(
            std::memory_order_relaxed
        ) != 0;
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
    const int displayed_current_processing_state =
        stream_active ? current_processing_state : 0;
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
        set_control_text(
            wnd,
            IDC_DIAG_MAX_TRUE_PEAK,
            ui_text(L"未測定", L"Not measured")
        );
    }

    swprintf_s(text, L"%.2f dB", displayed_max_compressor);
    set_control_text(wnd, IDC_DIAG_MAX_COMPRESSOR_REDUCTION, text);
    swprintf_s(text, L"%.2f dB", displayed_max_clipper);
    set_control_text(wnd, IDC_DIAG_MAX_CLIPPER_REDUCTION, text);
    swprintf_s(text, L"%.2f dB", displayed_max_limiter);
    set_control_text(wnd, IDC_DIAG_MAX_LIMITER_REDUCTION, text);
    swprintf_s(
        text,
        ui_text(L"%llu 回", L"%llu events"),
        displayed_clip_events
    );
    set_control_text(wnd, IDC_DIAG_CLIP_EVENT_COUNT, text);
    set_control_text(
        wnd,
        IDC_DIAG_PROCESSING_EVALUATION,
        stream_active
            ? current_processing_state_to_text(
                displayed_current_processing_state
            )
            : ui_text(L"待機中", L"Standby")
    );
    const std::wstring displayed_auto_control_reason =
        stream_active
            ? diagnostic_auto_control_reason_text(
                auto_control_reason_mask,
                latest_auto_control_reason_mask,
                displayed_current_processing_state,
                auto_control_trigger_count,
                latest_auto_control_recovered
            )
            : std::wstring(ui_text(L"待機中", L"Standby"));
    set_control_text(
        wnd,
        IDC_DIAG_AUTO_REASON,
        displayed_auto_control_reason.c_str()
    );
    if (displayed_sample_rate > 0) {
        swprintf_s(text, L"%u Hz", displayed_sample_rate);
        set_control_text(wnd, IDC_DIAG_SAMPLE_RATE, text);
    }
    else {
        set_control_text(
            wnd,
            IDC_DIAG_SAMPLE_RATE,
            ui_text(L"未検出", L"Not detected")
        );
    }
    swprintf_s(text, L"%.2f %%", displayed_cpu_load);
    set_control_text(wnd, IDC_DIAG_CPU_LOAD, text);

    if (!stream_active) {
        set_control_text(wnd, IDC_DIAG_NORMALIZATION_STATE, ui_text(L"待機中", L"Standby"));
        set_control_text(wnd, IDC_DIAG_GAIN_LOCK, ui_text(L"待機中", L"Standby"));
        set_control_text(wnd, IDC_DIAG_MOMENTARY, ui_text(L"待機中", L"Standby"));
        set_control_text(wnd, IDC_DIAG_SHORT_TERM, ui_text(L"待機中", L"Standby"));
        if (final_summary_valid) {
            swprintf_s(text, ui_text(L"前回 %.1f LUFS", L"Previous %.1f LUFS"), final_input_integrated_lufs);
            set_control_text(wnd, IDC_DIAG_INTEGRATED, text);
            swprintf_s(text, ui_text(L"前回 %.1f LUFS", L"Previous %.1f LUFS"), final_output_integrated_lufs);
            set_control_text(wnd, IDC_DIAG_OUTPUT_INTEGRATED, text);
            swprintf_s(text, ui_text(L"前回 %+.1f LU", L"Previous %+.1f LU"), final_target_difference_lu);
            set_control_text(wnd, IDC_DIAG_TARGET_DIFFERENCE, text);
            swprintf_s(text, ui_text(L"前回 %.1f LU", L"Previous %.1f LU"), final_lra_lu);
            set_control_text(wnd, IDC_DIAG_LRA, text);
        }
        else {
            set_control_text(wnd, IDC_DIAG_INTEGRATED, ui_text(L"待機中", L"Standby"));
            set_control_text(wnd, IDC_DIAG_OUTPUT_INTEGRATED, ui_text(L"待機中", L"Standby"));
            set_control_text(wnd, IDC_DIAG_TARGET_DIFFERENCE, ui_text(L"待機中", L"Standby"));
            set_control_text(wnd, IDC_DIAG_LRA, ui_text(L"待機中", L"Standby"));
        }
        set_control_text(
            wnd,
            IDC_DIAG_PROCESSING_RISK,
            original_compare_mode == 2
                ? ui_text(L"A/B音量一致", L"A/B loudness match")
                : (original_compare_mode == 1
                    ? ui_text(L"原音比較中", L"Comparing original")
                    : (modern_boost_enabled
                        ? ui_text(L"待機中", L"Standby")
                        : ui_text(L"無効", L"Disabled")))
        );
        set_control_text(
            wnd,
            IDC_DIAG_SAFETY_REDUCTION,
            L"0.00 dB"
        );
        set_control_text(wnd, IDC_DIAG_GAIN, ui_text(L"待機中", L"Standby"));
        set_control_text(wnd, IDC_DIAG_TRUE_PEAK, ui_text(L"待機中", L"Standby"));
        set_control_text(
            wnd,
            IDC_DIAG_COMPRESSOR_REDUCTION,
            modern_boost_enabled
                ? ui_text(L"待機中", L"Standby")
                : ui_text(L"無効", L"Disabled")
        );
        set_control_text(
            wnd,
            IDC_DIAG_CLIPPER_REDUCTION,
            modern_boost_enabled
                ? ui_text(L"待機中", L"Standby")
                : ui_text(L"無効", L"Disabled")
        );
        set_control_text(
            wnd,
            IDC_DIAG_LIMITER_REDUCTION,
            ui_text(L"待機中", L"Standby")
        );
        if (final_summary_valid && final_three_band_master_enabled) {
            swprintf_s(
                text,
                ui_text(
                    L"前回 低 %.1f / 中 %.1f / 高 %.1f dB",
                    L"Previous L %.1f / M %.1f / H %.1f dB"
                ),
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
                three_band_master_enabled
                    ? ui_text(L"待機中", L"Standby")
                    : ui_text(L"無効", L"Disabled")
            );
        }
        swprintf_s(text, L"%.1f ms", latency_ms);
        set_control_text(wnd, IDC_DIAG_LATENCY, text);

        if (peak_guard_state == 0) {
            set_control_text(wnd, IDC_DIAG_PEAK_GUARD, ui_text(L"無効", L"Disabled"));
        }
        else {
            set_control_text(wnd, IDC_DIAG_PEAK_GUARD, ui_text(L"待機中", L"Standby"));
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
        set_control_text(wnd, IDC_DIAG_GAIN_LOCK, ui_text(L"無効", L"Disabled"));
        break;
    case 2:
        swprintf_s(
            text,
            ui_text(L"%+.2f dB 固定", L"Locked at %+.2f dB"),
            locked_gain_db
        );
        set_control_text(wnd, IDC_DIAG_GAIN_LOCK, text);
        break;
    case 3:
        if (normalization_gain_db < -0.01) {
            swprintf_s(
                text,
                ui_text(
                    L"%+.2f→%+.2f 減衰",
                    L"%+.2f to %+.2f attenuation"
                ),
                locked_gain_db,
                normalization_gain_db
            );
        }
        else {
            swprintf_s(
                text,
                ui_text(
                    L"%+.2f→%+.2f 抑制",
                    L"%+.2f to %+.2f boost limited"
                ),
                locked_gain_db,
                normalization_gain_db
            );
        }
        set_control_text(wnd, IDC_DIAG_GAIN_LOCK, text);
        break;
    default:
        swprintf_s(
            text,
            ui_text(L"残り %.1f秒", L"%.1f sec remaining"),
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
        set_control_text(wnd, IDC_DIAG_MOMENTARY, ui_text(L"測定中…", L"Measuring..."));
    }

    if (std::isfinite(short_term_lufs) && short_term_lufs > -190.0) {
        swprintf_s(text, L"%.1f LUFS", short_term_lufs);
        set_control_text(wnd, IDC_DIAG_SHORT_TERM, text);
    }
    else {
        set_control_text(wnd, IDC_DIAG_SHORT_TERM, ui_text(L"測定中…", L"Measuring..."));
    }

    if (std::isfinite(integrated_lufs) && integrated_lufs > -190.0) {
        swprintf_s(text, L"%.1f LUFS", integrated_lufs);
        set_control_text(wnd, IDC_DIAG_INTEGRATED, text);
    }
    else {
        set_control_text(wnd, IDC_DIAG_INTEGRATED, ui_text(L"測定中…", L"Measuring..."));
    }

    if (std::isfinite(output_integrated_lufs) &&
        output_integrated_lufs > -190.0) {
        swprintf_s(text, L"%.1f LUFS", output_integrated_lufs);
        set_control_text(wnd, IDC_DIAG_OUTPUT_INTEGRATED, text);
    }
    else {
        set_control_text(wnd, IDC_DIAG_OUTPUT_INTEGRATED, ui_text(L"測定中…", L"Measuring..."));
    }

    if (std::isfinite(target_difference_lu) &&
        target_difference_lu > -190.0) {
        swprintf_s(text, L"%+.1f LU", target_difference_lu);
        set_control_text(wnd, IDC_DIAG_TARGET_DIFFERENCE, text);
    }
    else {
        set_control_text(wnd, IDC_DIAG_TARGET_DIFFERENCE, ui_text(L"測定中…", L"Measuring..."));
    }

    if (std::isfinite(lra_lu) && lra_lu > -190.0) {
        swprintf_s(text, L"%.1f LU", lra_lu);
        set_control_text(wnd, IDC_DIAG_LRA, text);
    }
    else {
        set_control_text(wnd, IDC_DIAG_LRA, ui_text(L"測定中…", L"Measuring..."));
    }

    if (original_compare_active) {
        if (original_compare_mode == 2) {
            swprintf_s(text, L"A/B %+.1f dB", compare_match_gain_db);
            set_control_text(wnd, IDC_DIAG_PROCESSING_RISK, text);
        }
        else {
            set_control_text(
                wnd,
                IDC_DIAG_PROCESSING_RISK,
                ui_text(L"原音比較中", L"Comparing original")
            );
        }
    }
    else if (adaptive_master_enabled) {
        swprintf_s(
            text,
            ui_text(L"%s・%.1f%%", L"%s / %.1f%%"),
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

    if (stream_active && std::isfinite(safety_reduction_db)) {
        swprintf_s(text, L"%.2f dB", std::max(0.0, -safety_reduction_db));
        set_control_text(wnd, IDC_DIAG_SAFETY_REDUCTION, text);
    }
    else {
        set_control_text(wnd, IDC_DIAG_SAFETY_REDUCTION, ui_text(L"待機中", L"Standby"));
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
        set_control_text(wnd, IDC_DIAG_COMPRESSOR_REDUCTION, ui_text(L"無効", L"Disabled"));
    }

    if (three_band_master_enabled) {
        swprintf_s(
            text,
            ui_text(
                L"低 %.1f / 中 %.1f / 高 %.1f dB",
                L"L %.1f / M %.1f / H %.1f dB"
            ),
            three_band_low_reduction_db,
            three_band_mid_reduction_db,
            three_band_high_reduction_db
        );
        set_control_text(wnd, IDC_DIAG_THREE_BAND_REDUCTION, text);
    }
    else {
        set_control_text(wnd, IDC_DIAG_THREE_BAND_REDUCTION, ui_text(L"無効", L"Disabled"));
    }

    if (modern_boost_enabled && std::isfinite(clipper_reduction_db)) {
        swprintf_s(text, L"%.2f dB", clipper_reduction_db);
        set_control_text(wnd, IDC_DIAG_CLIPPER_REDUCTION, text);
    }
    else {
        set_control_text(wnd, IDC_DIAG_CLIPPER_REDUCTION, ui_text(L"無効", L"Disabled"));
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
        set_control_text(wnd, IDC_DIAG_TRUE_PEAK, ui_text(L"未検出", L"Not detected"));
    }

    switch (peak_guard_state) {
    case 0:
        set_control_text(wnd, IDC_DIAG_PEAK_GUARD, ui_text(L"無効", L"Disabled"));
        break;
    case 2:
        set_control_text(wnd, IDC_DIAG_PEAK_GUARD, ui_text(L"作動中", L"Active"));
        break;
    default:
        set_control_text(wnd, IDC_DIAG_PEAK_GUARD, ui_text(L"待機", L"Standby"));
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
        L"現在の処理状態は「正常／監視中／自動調整中／調整上限」で"
        L"表示します。自動制御の理由と自動減衰量も"
        L"同じ診断ページで確認できます。\r\n"
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
        L"Momentaryラウドネス",
        L"約400ミリ秒の短い区間で測ったラウドネスです。\r\n\r\n"
        L"瞬間的な音量変化を確認できます。ドラムや歌声の一音ごとに"
        L"大きく動くため、曲全体の音量判断には使いません。"
    },
    {
        L"Short-termラウドネス",
        L"直近約3秒間のラウドネスです。\r\n\r\n"
        L"Momentaryより安定し、現在聞いている部分の音量感を"
        L"確認できます。音量一致A/B比較にも使用します。"
    },
    {
        L"Integratedラウドネス",
        L"測定開始から現在までをまとめた平均ラウドネスです。\r\n\r\n"
        L"入力Integratedラウドネスは処理前、出力Integratedラウドネスは処理後を示します。"
        L"曲全体の音量を判断する中心的な値です。"
    },
    {
        L"ラウドネスレンジ（LRA）",
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
        L"目標ラウドネス",
        L"処理後に近づけたいラウドネスです。\r\n\r\n"
        L"値を0に近づけるほど音量感は大きくなりますが、"
        L"コンプレッサーやクリッパーの処理量も増えやすくなります。"
    },
    {
        L"R128補正ゲイン最大増幅量 / 最大減衰量",
        L"R128補正ゲインが動ける範囲です。\r\n\r\n"
        L"最大増幅は小さい音源を持ち上げる上限、最大減衰は"
        L"大きい音源を下げる上限です。安全用の制限として働きます。"
    },
    {
        L"True Peakリミッター先読み時間",
        L"ピークが来る少し前にリミッターを動かすための時間です。\r\n\r\n"
        L"ピークを確実に抑えやすくなりますが、その分だけDSPの"
        L"遅延が増えます。既定値は5 msです。"
    },
    {
        L"True Peakリミッター解放時間",
        L"リミッターが音量を抑えたあと、通常のゲインへ戻る速さです。\r\n\r\n"
        L"短すぎると揺れや歪みが出やすく、長すぎると抑えた状態が"
        L"長く残ります。"
    },
    {
        L"曲頭安定化解析時間",
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
        L"補正ゲイン固定",
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
        L"True Peakリミッター",
        L"最終段でTrue Peak上限を超えないように音量を抑えます。\r\n\r\n"
        L"最大リミッターが3 dB以上になる場合は、目標LUFSや"
        L"モダン強度を下げることを推奨します。"
    },
    {
        L"自動セーフティ",
        L"処理が強くなりすぎたとき、全プリセットで最大6 dBまで"
        L"全体を安全側へ下げる機能です。\r\n\r\n"
        L"診断の「自動減衰量」に現在の減衰量が正の値で表示され、"
        L"「自動制御の理由」で発動要因を確認できます。"
    },
    {
        L"自動制御履歴",
        L"自動制御が実際に発動した最新100曲を保存します。\r\n\r\n"
        L"再生日時、曲名／アーティスト、プリセット、発動理由、"
        L"最大自動減衰量、発動回数、調整上限、安全復帰を確認できます。"
        L"短い監視だけで自動調整へ入らなかった曲は記録しません。"
    },
    {
        L"モダンブースト",
        L"R128補正にコンプレッサー、ソフトクリッパー、"
        L"True Peakリミッターを組み合わせる高密度モードです。\r\n\r\n"
        L"通常のノーマライズより音色やダイナミクスが変化します。"
    },
    {
        L"1バンド・アダプティブ",
        L"入力ラウドネス、LRA、処理負荷を見ながら、モダン処理の"
        L"強度を自動調整するモードです。\r\n\r\n"
        L"設定したモダン強度は、自動調整の上限として働きます。"
    },
    {
        L"3バンド・アダプティブ",
        L"音を低域・中域・高域に分け、それぞれを別の"
        L"コンプレッサーで制御します。\r\n\r\n"
        L"約160 Hzと約4 kHzがクロスオーバーの目安です。"
        L"固定EQではありませんが、処理量によって音色は変化します。"
    },
    {
        L"クロスオーバー周波数",
        L"3バンド・アダプティブで低域・中域・高域を分ける境界周波数です。\r\n\r\n"
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
        L"最大True Peak",
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
        L"0 dBTP超過イベント",
        L"処理後のTrue Peakが0 dBTPを超えたイベント回数です。\r\n\r\n"
        L"通常は0回になることを想定しています。1回以上なら"
        L"True Peak設定と処理強度を見直してください。"
    },
    {
        L"CPU負荷",
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

constexpr glossary_entry kGlossaryEntriesEnglish[] = {
    {
        L"Reading the diagnostics",
        L"Current processing state is shown as Normal, Monitoring, "
        L"Auto-adjusting, or Adjustment limit.\r\n\r\n"
        L"Monitoring is a short confirmation period. If the condition "
        L"continues, automatic safety control adds up to 6 dB of attenuation. "
        L"After safe recovery, the state returns to Normal. The diagnostics "
        L"also show the last trigger reason and automatic attenuation amount."
    },
    {
        L"EBU R128",
        L"A loudness-measurement method designed to make the perceived "
        L"level of programs and music more consistent.\r\n\r\n"
        L"This component uses R128 principles to adjust playback loudness "
        L"in real time."
    },
    {
        L"LUFS",
        L"Short for Loudness Units relative to Full Scale. It represents "
        L"perceived loudness; values closer to 0 sound louder. "
        L"For example, -10 LUFS is louder than -18 LUFS."
    },
    {
        L"LU",
        L"Short for Loudness Unit. It expresses a loudness difference. "
        L"A difference of 1 LU is numerically equivalent to 1 dB."
    },
    {
        L"Momentary loudness",
        L"Loudness measured over approximately 400 milliseconds. "
        L"It follows short events closely and is not intended to represent "
        L"the loudness of an entire track."
    },
    {
        L"Short-term loudness",
        L"Loudness measured over approximately three seconds. "
        L"It is steadier than Momentary loudness and is also used for "
        L"loudness-matched A/B comparison."
    },
    {
        L"Integrated loudness",
        L"Average loudness from the start of measurement. Input Integrated "
        L"is measured before processing and Output Integrated after processing."
    },
    {
        L"Loudness range (LRA)",
        L"An estimate of the difference between quieter and louder parts "
        L"of a track. A larger value generally indicates greater dynamics."
    },
    {
        L"True Peak / dBTP",
        L"An estimate of the actual peak between digital samples. dBTP is "
        L"the True Peak unit. A ceiling near -1.0 dBTP commonly leaves "
        L"headroom for playback and conversion."
    },
    {
        L"Target loudness",
        L"The loudness level the output approaches. Targets closer to 0 "
        L"sound louder but usually require more compression and peak control."
    },
    {
        L"Maximum R128 boost / attenuation",
        L"The allowed range of normalization gain. Maximum boost limits "
        L"how far quiet material can be raised; maximum attenuation limits "
        L"how far loud material can be reduced."
    },
    {
        L"True Peak limiter look-ahead",
        L"How far the limiter anticipates a peak. More look-ahead improves "
        L"peak control but increases DSP latency. The default is 5 ms."
    },
    {
        L"True Peak limiter release",
        L"How quickly the limiter returns to normal gain after reducing a peak. "
        L"Very short values can sound unstable; very long values keep gain "
        L"reduced for longer."
    },
    {
        L"Startup analysis time",
        L"Time used to measure the beginning of a track and stabilize boost "
        L"decisions. A temporary Measuring / held state is normal."
    },
    {
        L"Silence guard / boost hold",
        L"Prevents very quiet passages or silence from being raised excessively. "
        L"A boost hold is a normal safety action, not an error."
    },
    {
        L"Gain lock",
        L"Locks normalization gain after loudness stabilizes to reduce movement "
        L"within the track. Safety attenuation and peak limiting can still apply."
    },
    {
        L"Compressor",
        L"Automatically reduces sections that are too loud. Higher reduction "
        L"increases density but can reduce dynamics."
    },
    {
        L"Soft clipper",
        L"Rounds sharp peaks to preserve loudness. Clipper reduction is the "
        L"amount of peak shaping, not a count of digital clipping events."
    },
    {
        L"True Peak limiter",
        L"The final stage that keeps output below the True Peak ceiling. "
        L"If maximum reduction reaches 3 dB or more, consider lowering the "
        L"target or Modern strength."
    },
    {
        L"Automatic safety control",
        L"Adds up to 6 dB of attenuation for every preset when processing "
        L"becomes too strong. The current positive amount appears as Automatic "
        L"attenuation, and the trigger appears as Automatic-control reason."
    },
    {
        L"Automatic-Control History",
        L"Stores up to 100 recent tracks where automatic control actually "
        L"activated. Each entry includes playback time, title and artist, "
        L"preset, trigger reason, maximum automatic attenuation, activation "
        L"count, adjustment-limit status, and safe recovery. A brief "
        L"Monitoring state without Auto-adjusting is not recorded."
    },
    {
        L"Modern Boost",
        L"A high-density mode combining R128 normalization, compression, "
        L"soft clipping, and True Peak limiting."
    },
    {
        L"1-Band Adaptive",
        L"Adjusts Modern Processing strength from input loudness, LRA, and "
        L"processing load. The selected strength acts as the automatic maximum."
    },
    {
        L"3-Band Adaptive",
        L"Splits audio into low, mid, and high bands and controls each band "
        L"separately. Approximate crossovers are 160 Hz and 4 kHz."
    },
    {
        L"Crossover frequencies",
        L"The boundaries used to split low, mid, and high bands in 3-Band "
        L"Adaptive mode. This component uses approximately 160 Hz and 4 kHz."
    },
    {
        L"Loudness-matched A/B comparison",
        L"Matches the Short-term loudness of processed and original audio "
        L"to reduce level bias when comparing tone and dynamics."
    },
    {
        L"Full-bypass comparison",
        L"With Loudness match off, holding Compare bypasses all processing "
        L"in this component. Other DSPs earlier in the chain remain active."
    },
    {
        L"Maximum True Peak",
        L"The highest processed True Peak measured in the track. A value "
        L"above +0.01 dBTP or any 0 dBTP exceedance needs attention."
    },
    {
        L"Maximum compressor / clipper / limiter",
        L"The greatest reduction applied by each stage in the track. "
        L"Compressor reduction of 6 dB is a strong-processing guide; "
        L"clipper or limiter reduction of 3 dB needs attention."
    },
    {
        L"0 dBTP exceedance events",
        L"Number of events where processed True Peak exceeded 0 dBTP. "
        L"Normally this should remain at zero."
    },
    {
        L"CPU load",
        L"Estimated processing load of this DSP. It varies with the computer, "
        L"sample rate, and channel count and is not used for state evaluation."
    },
    {
        L"Invalid-sample protection",
        L"Replaces NaN, infinite, or extreme samples received from a decoder "
        L"or earlier DSP with safe values. Normal finite audio is unaffected; "
        L"the activation count appears in the copied diagnostics."
    }
};

static_assert(
    std::size(kGlossaryEntriesEnglish) == std::size(kGlossaryEntries),
    "Japanese and English glossary entry counts must match"
);

struct tooltip_entry {
    int control_id;
    const wchar_t* japanese;
    const wchar_t* english;
};

constexpr tooltip_entry kPresetTooltips[] = {
    {
        IDC_PROFILE_STANDARD,
        L"ナチュラル -18：普段の音楽再生向け。"
        L"自然な音量感を保ってそろえます。",
        L"Natural -18: For everyday listening. "
        L"Balances loudness while preserving a natural sound."
    },
    {
        IDC_PROFILE_STREAMING,
        L"パワーブースト -14：小さい音源を強めに持ち上げ、"
        L"迫力のある音量感にします。",
        L"Power Boost -14: Raises quieter sources more strongly "
        L"for a more powerful presentation."
    },
    {
        IDC_PROFILE_BROADCAST,
        L"リラックス -23：全体を控えめにそろえ、"
        L"長時間でも聴きやすくします。",
        L"Relaxed -23: Uses a restrained loudness target "
        L"for comfortable long listening sessions."
    },
    {
        IDC_PROFILE_NIGHT,
        L"ナイトセーフ -22：夜間向け。"
        L"音量とピークを低めに抑えます。",
        L"Night Safe -22: Keeps loudness and peaks lower "
        L"for nighttime listening."
    },
    {
        IDC_PROFILE_MODERN,
        L"モダンブースト -9：圧縮とソフトクリップで"
        L"高密度な音に近づけます。",
        L"Modern Boost -9: Uses compression and soft clipping "
        L"for a denser sound."
    },
    {
        IDC_PROFILE_ADAPTIVE,
        L"1バンド・アダプティブ -10：曲のLRAと入力音量を解析し、"
        L"モダン処理の強度を自動調整します。",
        L"1-Band Adaptive -10: Analyzes LRA and input loudness "
        L"to adjust Modern Processing automatically."
    },
    {
        IDC_PROFILE_THREE_BAND,
        L"3バンド・アダプティブ -10：低・中・高域を個別に制御し、"
        L"低音の潰れと高域のざらつきを抑えます。",
        L"3-Band Adaptive -10: Controls low, mid, and high bands "
        L"independently to reduce bass pumping and harsh treble."
    },
    {
        IDC_COMPARE_LOUDNESS_MATCH,
        L"オン：ラウドネスをそろえた公平なA/B比較。"
        L"オフ：このDSPを外す完全バイパス比較。",
        L"On: Loudness-matched A/B comparison. "
        L"Off: Full bypass comparison for this DSP."
    },
    {
        IDC_ORIGINAL_COMPARE,
        L"押している間だけ比較音へ切り替えます。"
        L"離すと処理音へ戻ります。",
        L"Hold to hear the comparison signal. "
        L"Release to return to processed audio."
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
        L"目標ラウドネス",
        L"処理後に近づけたいラウドネスです。"
        L"0に近づけるほど大きくなりますが、"
        L"圧縮やピーク処理も強くなりやすくなります。"
    },
    {
        IDC_MAX_BOOST,
        L"R128補正ゲイン最大増幅量",
        L"小さい音源を持ち上げられる最大量です。"
        L"静かな音源や無音を過剰に増幅しないための上限です。"
    },
    {
        IDC_MAX_ATTENUATION,
        L"R128補正ゲイン最大減衰量",
        L"大きい音源を下げられる最大量です。"
        L"極端な設定値にならないための安全上限です。"
    },
    {
        IDC_TRUE_PEAK,
        L"True Peak上限",
        L"処理後のTrue Peak上限です。"
        L"一般的な開始値は-1.0 dBTPです。"
    },
    {
        IDC_LOOKAHEAD_MS,
        L"True Peakリミッター先読み時間",
        L"ピークの少し前からリミッターを動かす時間です。"
        L"長くするとピークを抑えやすくなりますが、遅延も増えます。"
    },
    {
        IDC_LIMITER_RELEASE_MS,
        L"True Peakリミッター解放時間",
        L"リミッターが通常状態へ戻る速さです。"
        L"短すぎると揺れ、長すぎると抑制が長く残ることがあります。"
    },
    {
        IDC_STARTUP_ANALYSIS_SECONDS,
        L"曲頭安定化解析時間",
        L"曲の冒頭を測定して増幅判断を安定させる時間です。"
        L"この間の増幅保留は正常な安全動作です。"
    },
    {
        IDC_SILENCE_GUARD_LUFS,
        L"静音保護しきい値",
        L"この値より静かな区間では増幅を保留します。"
        L"無音や小さなノイズの過剰増幅を防ぎます。"
    },
    {
        IDC_GAIN_LOCK_SECONDS,
        L"補正ゲイン固定判定時間",
        L"補正ゲインを固定するまでの安定確認時間です。"
        L"長くすると慎重に、短くすると早く固定します。"
    },
    {
        IDC_GAIN_LOCK_TOLERANCE_LU,
        L"補正ゲイン固定許容幅",
        L"ゲイン固定を判断する際に許容するラウドネス変動幅です。"
        L"小さいほど厳密な安定を求めます。"
    },
    {
        IDC_MODERN_STRENGTH,
        L"モダン強度／上限",
        L"コンプレッサーとクリッパーの処理強度です。"
        L"1バンド・アダプティブでは自動強度の上限になります。"
    },
    {
        IDC_RESET_EACH_TRACK,
        L"トラック変更時に測定値をリセット",
        L"次の曲へ移った際にIntegrated、LRA、最大値などを"
        L"新しい曲用にリセットします。"
    },
    {
        IDC_ENABLE_PEAK_GUARD,
        L"True Peak保護（先読みリミッター）を有効",
        L"先読みリミッターを使い、設定したTP上限を守ります。"
    },
    {
        IDC_ENABLE_SILENCE_GUARD,
        L"静音区間では増幅を保留",
        L"無音や非常に静かな区間を大きく持ち上げない安全機能です。"
    },
    {
        IDC_ENABLE_GAIN_LOCK,
        L"安定後に曲中の補正ゲインを固定",
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
        L"1バンド・アダプティブ",
        L"曲のLRAや入力音量に応じ、モダン処理の強度を自動調整します。"
    },
    {
        IDC_ENABLE_THREE_BAND_MASTER,
        L"3バンド・アダプティブ",
        L"低域・中域・高域を分けて個別に制御します。"
        L"固定EQではなく、帯域別の動的処理です。"
    },
    {
        IDC_DIAG_NORMALIZATION_STATE,
        L"R128ノーマライズ状態",
        L"通常補正、測定中・保留、静音保護・保留など、"
        L"現在のR128補正状態を表示します。"
    },
    {
        IDC_DIAG_GAIN_LOCK,
        L"補正ゲイン固定状態",
        L"ゲイン固定の待ち時間、固定値、安全減衰、"
        L"増幅抑制などの状態を表示します。"
    },
    {
        IDC_DIAG_MOMENTARY,
        L"Momentaryラウドネス",
        L"約400ミリ秒の瞬間的なラウドネスです。"
    },
    {
        IDC_DIAG_SHORT_TERM,
        L"Short-termラウドネス",
        L"直近約3秒間のラウドネスです。"
        L"音量一致A/B比較の中心値にも使用します。"
    },
    {
        IDC_DIAG_INTEGRATED,
        L"入力Integratedラウドネス",
        L"処理前の測定開始から現在までの平均ラウドネスです。"
    },
    {
        IDC_DIAG_OUTPUT_INTEGRATED,
        L"出力Integratedラウドネス",
        L"処理後の測定開始から現在までの平均ラウドネスです。"
    },
    {
        IDC_DIAG_TARGET_DIFFERENCE,
        L"目標LUFSとの差",
        L"出力Integratedと目標LUFSの差です。"
        L"0 LUに近いほど目標へ近づいています。"
    },
    {
        IDC_DIAG_LRA,
        L"ラウドネスレンジ（LRA）",
        L"曲の静かな部分と大きな部分の幅を推定した値です。"
    },
    {
        IDC_DIAG_PROCESSING_RISK,
        L"追加処理の状態",
        L"現在のモダン／1バンド・アダプティブ処理の強さや、"
        L"A/B比較状態を表示します。"
    },
    {
        IDC_DIAG_SAFETY_REDUCTION,
        L"自動減衰量",
        L"処理が強くなりすぎた際に追加された安全減衰量です。"
        L"全プリセットで最大6 dBまで自動調整します。"
    },
    {
        IDC_DIAG_AUTO_REASON,
        L"自動制御の理由",
        L"監視または自動調整を開始した要因です。"
        L"True Peak超過、リミッター過多、クリッパー過多、"
        L"複数要因のいずれかを表示します。"
    },
    {
        IDC_DIAG_GAIN,
        L"適用中の総ゲイン",
        L"R128補正と安全補正を含む、現在の全体ゲインです。"
    },
    {
        IDC_DIAG_COMPRESSOR_REDUCTION,
        L"コンプレッサー減衰量",
        L"現在のコンプレッサー減衰量です。"
    },
    {
        IDC_DIAG_CLIPPER_REDUCTION,
        L"ソフトクリッパー減衰量",
        L"現在のソフトクリッパーによるピーク整形量です。"
    },
    {
        IDC_DIAG_LIMITER_REDUCTION,
        L"True Peakリミッター減衰量",
        L"現在の最終True Peakリミッター減衰量です。"
    },
    {
        IDC_DIAG_TRUE_PEAK,
        L"処理後True Peak",
        L"現在の処理後True Peak推定値です。単位はdBTPです。"
    },
    {
        IDC_DIAG_LATENCY,
        L"処理遅延",
        L"先読み処理などによって発生するDSP遅延です。"
    },
    {
        IDC_DIAG_PEAK_GUARD,
        L"True Peak保護状態",
        L"True Peak保護が無効、待機、作動中のどれかを表示します。"
    },
    {
        IDC_DIAG_CHANNEL_LAYOUT,
        L"チャンネル構成",
        L"検出したチャンネル構成と、LFEをラウドネス計算から"
        L"除外しているかを表示します。"
    },
    {
        IDC_DIAG_THREE_BAND_REDUCTION,
        L"3バンド・アダプティブ減衰量",
        L"再生中は低域・中域・高域の現在の減衰量を表示します。"
        L"停止後は前回トラックで記録した各帯域の最大減衰量を表示します。"
    },
    {
        IDC_DIAG_MAX_TRUE_PEAK,
        L"最大True Peak",
        L"トラック中の処理後最大True Peakです。"
        L"+0.01 dBTPを超えた場合は要調整です。"
    },
    {
        IDC_DIAG_MAX_COMPRESSOR_REDUCTION,
        L"最大コンプレッサー減衰量",
        L"トラック中の最大コンプレッサー減衰量です。"
        L"6 dB以上は「強め」の目安です。"
    },
    {
        IDC_DIAG_MAX_CLIPPER_REDUCTION,
        L"最大ソフトクリッパー減衰量",
        L"トラック中の最大ソフトクリッパー整形量です。"
        L"3 dB以上は「要調整」です。"
    },
    {
        IDC_DIAG_MAX_LIMITER_REDUCTION,
        L"最大True Peakリミッター減衰量",
        L"トラック中の最大リミッター減衰量です。"
        L"3 dB以上は「要調整」です。"
    },
    {
        IDC_DIAG_SAMPLE_RATE,
        L"サンプルレート",
        L"再生中のサンプルレートです。例：44100 Hz、48000 Hz。"
    },
    {
        IDC_DIAG_CPU_LOAD,
        L"CPU負荷",
        L"このDSPの推定処理負荷です。総合評価には使用しません。"
    },
    {
        IDC_DIAG_CLIP_EVENT_COUNT,
        L"0 dBTP超過イベント",
        L"処理後のTrue Peakが0 dBTPを超えたイベント回数です。"
        L"通常は0回を想定しています。"
    },
    {
        IDC_DIAG_PROCESSING_EVALUATION,
        L"現在の処理状態",
        L"リアルタイム監視の状態を「正常／監視中／自動調整中／"
        L"調整上限」で表示します。安全復帰後は「正常」に戻ります。"
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
        IDC_SHOW_AUTO_HISTORY,
        L"自動制御履歴",
        L"自動制御が発動した最新100曲を一覧表示します。"
        L"発動理由、最大自動減衰量、発動回数、調整上限への到達、"
        L"安全復帰を確認できます。"
    },
    {
        IDC_SHOW_TREND_GRAPH,
        L"推移グラフ",
        L"再生中の曲について、Short-termラウドネス、総ゲイン、"
        L"自動減衰量、True Peakを約1秒ごとに記録して表示します。"
        L"赤い縦線は自動制御の発動位置です。"
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
    { L"目標ラウドネス：", IDC_TARGET_LUFS },
    { L"R128補正ゲイン最大増幅量：", IDC_MAX_BOOST },
    { L"R128補正ゲイン最大減衰量：", IDC_MAX_ATTENUATION },
    { L"True Peak上限：", IDC_TRUE_PEAK },
    { L"True Peakリミッター先読み時間：", IDC_LOOKAHEAD_MS },
    { L"True Peakリミッター解放時間：", IDC_LIMITER_RELEASE_MS },
    { L"曲頭安定化解析時間：", IDC_STARTUP_ANALYSIS_SECONDS },
    { L"静音保護しきい値：", IDC_SILENCE_GUARD_LUFS },
    { L"補正ゲイン固定判定時間：", IDC_GAIN_LOCK_SECONDS },
    { L"補正ゲイン固定許容幅：", IDC_GAIN_LOCK_TOLERANCE_LU },
    { L"モダン強度／上限：", IDC_MODERN_STRENGTH },
    { L"R128ノーマライズ状態：", IDC_DIAG_NORMALIZATION_STATE },
    { L"補正ゲイン固定状態：", IDC_DIAG_GAIN_LOCK },
    { L"Momentaryラウドネス：", IDC_DIAG_MOMENTARY },
    { L"Short-termラウドネス：", IDC_DIAG_SHORT_TERM },
    { L"入力Integratedラウドネス：", IDC_DIAG_INTEGRATED },
    { L"出力Integratedラウドネス：", IDC_DIAG_OUTPUT_INTEGRATED },
    { L"目標LUFSとの差：", IDC_DIAG_TARGET_DIFFERENCE },
    { L"ラウドネスレンジ（LRA）：", IDC_DIAG_LRA },
    { L"適用中の総ゲイン：", IDC_DIAG_GAIN },
    { L"追加処理の状態：", IDC_DIAG_PROCESSING_RISK },
    { L"自動減衰量：", IDC_DIAG_SAFETY_REDUCTION },
    { L"コンプレッサー減衰量：", IDC_DIAG_COMPRESSOR_REDUCTION },
    { L"ソフトクリッパー減衰量：", IDC_DIAG_CLIPPER_REDUCTION },
    { L"True Peakリミッター減衰量：", IDC_DIAG_LIMITER_REDUCTION },
    { L"処理後True Peak：", IDC_DIAG_TRUE_PEAK },
    { L"処理遅延：", IDC_DIAG_LATENCY },
    { L"True Peak保護状態：", IDC_DIAG_PEAK_GUARD },
    { L"サンプルレート：", IDC_DIAG_SAMPLE_RATE },
    { L"チャンネル構成：", IDC_DIAG_CHANNEL_LAYOUT },
    { L"CPU負荷：", IDC_DIAG_CPU_LOAD },
    { L"0 dBTP超過イベント：", IDC_DIAG_CLIP_EVENT_COUNT },
    { L"現在の処理状態：", IDC_DIAG_PROCESSING_EVALUATION },
    { L"自動制御の理由：", IDC_DIAG_AUTO_REASON },
    { L"最大True Peak：", IDC_DIAG_MAX_TRUE_PEAK },
    { L"最大コンプレッサー減衰量：", IDC_DIAG_MAX_COMPRESSOR_REDUCTION },
    { L"最大ソフトクリッパー減衰量：", IDC_DIAG_MAX_CLIPPER_REDUCTION },
    { L"最大True Peakリミッター減衰量：", IDC_DIAG_MAX_LIMITER_REDUCTION },
    { L"3バンド・アダプティブ減衰量：", IDC_DIAG_THREE_BAND_REDUCTION }
};

const context_help_entry* find_context_help(int control_id) {
    for (const auto& entry : kContextHelpEntries) {
        if (entry.control_id == control_id) {
            return &entry;
        }
    }

    return nullptr;
}

const wchar_t* english_control_title(
    int control_id,
    const wchar_t* fallback
) {
    switch (control_id) {
    case IDC_TARGET_LUFS: return L"Target loudness";
    case IDC_MAX_BOOST: return L"Maximum R128 boost";
    case IDC_MAX_ATTENUATION: return L"Maximum R128 attenuation";
    case IDC_TRUE_PEAK: return L"True Peak limit";
    case IDC_LOOKAHEAD_MS: return L"Limiter look-ahead";
    case IDC_LIMITER_RELEASE_MS: return L"Limiter release";
    case IDC_STARTUP_ANALYSIS_SECONDS: return L"Startup analysis time";
    case IDC_SILENCE_GUARD_LUFS: return L"Silence guard threshold";
    case IDC_GAIN_LOCK_SECONDS: return L"Gain-lock detection time";
    case IDC_GAIN_LOCK_TOLERANCE_LU: return L"Gain-lock tolerance";
    case IDC_MODERN_STRENGTH: return L"Modern strength / adaptive maximum";
    case IDC_RESET_EACH_TRACK: return L"Reset measurements on track change";
    case IDC_ENABLE_PEAK_GUARD: return L"True Peak protection";
    case IDC_ENABLE_SILENCE_GUARD: return L"Silence guard";
    case IDC_ENABLE_GAIN_LOCK: return L"Gain lock";
    case IDC_ENABLE_MODERN_BOOST: return L"Modern Boost";
    case IDC_ENABLE_ADAPTIVE_MASTER: return L"1-Band Adaptive";
    case IDC_ENABLE_THREE_BAND_MASTER: return L"3-Band Adaptive";
    case IDC_DIAG_NORMALIZATION_STATE: return L"R128 normalization";
    case IDC_DIAG_GAIN_LOCK: return L"Gain lock";
    case IDC_DIAG_MOMENTARY: return L"Momentary loudness";
    case IDC_DIAG_SHORT_TERM: return L"Short-term loudness";
    case IDC_DIAG_INTEGRATED: return L"Input Integrated loudness";
    case IDC_DIAG_OUTPUT_INTEGRATED: return L"Output Integrated loudness";
    case IDC_DIAG_TARGET_DIFFERENCE: return L"Difference from target";
    case IDC_DIAG_LRA: return L"Loudness range (LRA)";
    case IDC_DIAG_PROCESSING_RISK: return L"Additional processing";
    case IDC_DIAG_SAFETY_REDUCTION: return L"Automatic attenuation";
    case IDC_DIAG_GAIN: return L"Total applied gain";
    case IDC_DIAG_COMPRESSOR_REDUCTION: return L"Compressor reduction";
    case IDC_DIAG_CLIPPER_REDUCTION: return L"Soft clipper reduction";
    case IDC_DIAG_LIMITER_REDUCTION: return L"True Peak limiter reduction";
    case IDC_DIAG_TRUE_PEAK: return L"Processed True Peak";
    case IDC_DIAG_LATENCY: return L"Processing latency";
    case IDC_DIAG_PEAK_GUARD: return L"True Peak protection";
    case IDC_DIAG_CHANNEL_LAYOUT: return L"Channel layout";
    case IDC_DIAG_THREE_BAND_REDUCTION: return L"3-band reduction";
    case IDC_DIAG_MAX_TRUE_PEAK: return L"Maximum True Peak";
    case IDC_DIAG_MAX_COMPRESSOR_REDUCTION:
        return L"Maximum compressor reduction";
    case IDC_DIAG_MAX_CLIPPER_REDUCTION:
        return L"Maximum soft clipper reduction";
    case IDC_DIAG_MAX_LIMITER_REDUCTION:
        return L"Maximum True Peak limiter reduction";
    case IDC_DIAG_SAMPLE_RATE: return L"Sample rate";
    case IDC_DIAG_CPU_LOAD: return L"CPU load";
    case IDC_DIAG_CLIP_EVENT_COUNT: return L"0 dBTP exceedance events";
    case IDC_DIAG_PROCESSING_EVALUATION: return L"Current processing state";
    case IDC_DIAG_AUTO_REASON: return L"Automatic-control reason";
    case IDC_COMPARE_LOUDNESS_MATCH: return L"Loudness match";
    case IDC_ORIGINAL_COMPARE: return L"Compare (hold)";
    case IDC_RESET_MEASUREMENT: return L"Reset measurement";
    case IDC_COPY_DIAGNOSTICS: return L"Copy diagnostics";
    case IDC_SHOW_AUTO_HISTORY:
        return L"Automatic-Control History";
    case IDC_SHOW_TREND_GRAPH: return L"Trend Graph";
    case IDC_SHOW_DIAGNOSTIC_HELP: return L"Glossary";
    default:
        break;
    }

    for (const auto& entry : kPrimaryUiText) {
        if (entry.control_id == control_id) {
            return entry.english;
        }
    }

    return fallback;
}

const wchar_t* english_context_help_description(int control_id) {
    switch (control_id) {
    case IDC_TARGET_LUFS:
        return L"The loudness target for processed audio. "
            L"Targets closer to 0 sound louder and can require more peak control.";
    case IDC_MAX_BOOST:
        return L"The maximum gain available for raising quiet sources. "
            L"It also prevents excessive amplification of very quiet material.";
    case IDC_MAX_ATTENUATION:
        return L"The maximum amount by which loud sources may be reduced.";
    case IDC_TRUE_PEAK:
        return L"The True Peak ceiling after processing. "
            L"-1.0 dBTP is a common starting point.";
    case IDC_LOOKAHEAD_MS:
        return L"How far the limiter looks ahead. More look-ahead improves "
            L"peak control but adds latency.";
    case IDC_LIMITER_RELEASE_MS:
        return L"How quickly the limiter returns to normal gain after a peak.";
    case IDC_STARTUP_ANALYSIS_SECONDS:
        return L"Initial analysis time used to stabilize gain decisions. "
            L"Holding boost during this period is normal.";
    case IDC_SILENCE_GUARD_LUFS:
        return L"Boost is held when loudness falls below this threshold, "
            L"preventing silence and low-level noise from being over-amplified.";
    case IDC_GAIN_LOCK_SECONDS:
        return L"How long loudness must remain stable before normalization "
            L"gain is locked.";
    case IDC_GAIN_LOCK_TOLERANCE_LU:
        return L"The allowed loudness variation when deciding whether gain "
            L"is stable enough to lock.";
    case IDC_MODERN_STRENGTH:
        return L"Controls compressor and soft-clipper intensity. "
            L"For adaptive modes, it acts as the maximum automatic strength.";
    case IDC_RESET_EACH_TRACK:
        return L"Resets Integrated loudness, LRA, and maximum values "
            L"when the next track starts.";
    case IDC_ENABLE_PEAK_GUARD:
        return L"Enables the look-ahead limiter to keep output below "
            L"the selected True Peak limit.";
    case IDC_ENABLE_SILENCE_GUARD:
        return L"Prevents large boosts during silence or very quiet passages.";
    case IDC_ENABLE_GAIN_LOCK:
        return L"Locks normalization gain after the measurement stabilizes "
            L"to reduce loudness movement within a track.";
    case IDC_ENABLE_MODERN_BOOST:
        return L"Combines compression, soft clipping, and True Peak limiting.";
    case IDC_ENABLE_ADAPTIVE_MASTER:
        return L"Adjusts Modern Processing strength from input loudness and LRA.";
    case IDC_ENABLE_THREE_BAND_MASTER:
        return L"Controls low, mid, and high bands independently. "
            L"This is dynamic multiband processing, not a fixed EQ.";
    case IDC_DIAG_NORMALIZATION_STATE:
        return L"Shows the current R128 normalization state, including "
            L"measurement holds and silence protection.";
    case IDC_DIAG_GAIN_LOCK:
        return L"Shows gain-lock timing, the locked value, and any safety "
            L"attenuation or boost suppression.";
    case IDC_DIAG_MOMENTARY:
        return L"Loudness measured over approximately 400 milliseconds.";
    case IDC_DIAG_SHORT_TERM:
        return L"Loudness measured over approximately three seconds. "
            L"It is also used for loudness-matched A/B comparison.";
    case IDC_DIAG_INTEGRATED:
        return L"Average input loudness from the start of the measurement.";
    case IDC_DIAG_OUTPUT_INTEGRATED:
        return L"Average processed-output loudness from the start of the measurement.";
    case IDC_DIAG_TARGET_DIFFERENCE:
        return L"Difference between output Integrated loudness and the target. "
            L"Values near 0 LU are closest to the target.";
    case IDC_DIAG_LRA:
        return L"Estimated difference between quieter and louder parts of the track.";
    case IDC_DIAG_PROCESSING_RISK:
        return L"Shows Modern or Adaptive processing intensity and A/B compare status.";
    case IDC_DIAG_SAFETY_REDUCTION:
        return L"Additional automatic attenuation applied when processing "
            L"becomes too strong, up to 6 dB for every preset.";
    case IDC_DIAG_AUTO_REASON:
        return L"Shows why monitoring or automatic adjustment started: "
            L"True Peak exceedance, excessive limiting, excessive clipping, "
            L"or multiple factors.";
    case IDC_DIAG_GAIN:
        return L"Current overall gain, including R128 normalization and safety control.";
    case IDC_DIAG_COMPRESSOR_REDUCTION:
        return L"Current compressor gain reduction.";
    case IDC_DIAG_CLIPPER_REDUCTION:
        return L"Current amount of peak shaping by the soft clipper.";
    case IDC_DIAG_LIMITER_REDUCTION:
        return L"Current gain reduction by the final True Peak limiter.";
    case IDC_DIAG_TRUE_PEAK:
        return L"Estimated True Peak after processing, in dBTP.";
    case IDC_DIAG_LATENCY:
        return L"DSP latency introduced by look-ahead and related processing.";
    case IDC_DIAG_PEAK_GUARD:
        return L"Shows whether True Peak protection is disabled, standing by, or active.";
    case IDC_DIAG_CHANNEL_LAYOUT:
        return L"Detected channel layout and whether LFE is excluded "
            L"from loudness measurement.";
    case IDC_DIAG_THREE_BAND_REDUCTION:
        return L"Shows current low, mid, and high-band reduction during playback, "
            L"or the previous track maxima after playback stops.";
    case IDC_DIAG_MAX_TRUE_PEAK:
        return L"Highest processed True Peak measured in the track.";
    case IDC_DIAG_MAX_COMPRESSOR_REDUCTION:
        return L"Highest compressor reduction measured in the track.";
    case IDC_DIAG_MAX_CLIPPER_REDUCTION:
        return L"Highest soft-clipper peak shaping measured in the track.";
    case IDC_DIAG_MAX_LIMITER_REDUCTION:
        return L"Highest True Peak limiter reduction measured in the track.";
    case IDC_DIAG_SAMPLE_RATE:
        return L"Current playback sample rate, such as 44100 Hz or 48000 Hz.";
    case IDC_DIAG_CPU_LOAD:
        return L"Estimated CPU load of this DSP. It is not used for "
            L"the processing-state evaluation.";
    case IDC_DIAG_CLIP_EVENT_COUNT:
        return L"Number of events where processed True Peak exceeded 0 dBTP. "
            L"Normally this should remain at zero.";
    case IDC_DIAG_PROCESSING_EVALUATION:
        return L"Real-time state: Normal, Monitoring, Auto-adjusting, "
            L"or Adjustment limit. It returns to Normal after safe recovery.";
    case IDC_COMPARE_LOUDNESS_MATCH:
        return L"When enabled, processed and original audio are loudness matched. "
            L"When disabled, comparison fully bypasses this DSP.";
    case IDC_ORIGINAL_COMPARE:
        return L"Hold to hear the comparison signal; release to return "
            L"to processed audio.";
    case IDC_RESET_MEASUREMENT:
        return L"Restarts Integrated loudness, LRA, and track-maximum measurements "
            L"from the current playback position.";
    case IDC_COPY_DIAGNOSTICS:
        return L"Copies the current settings and diagnostic results to the clipboard.";
    case IDC_SHOW_AUTO_HISTORY:
        return L"Shows up to 100 recent tracks where automatic control "
            L"activated, including the trigger reason, maximum automatic "
            L"attenuation, activation count, adjustment-limit status, "
            L"and safe recovery.";
    case IDC_SHOW_TREND_GRAPH:
        return L"Shows the current track's Short-term loudness, total gain, "
            L"automatic attenuation, and True Peak about once per second. "
            L"Red vertical lines mark automatic-control activation.";
    case IDC_SHOW_DIAGNOSTIC_HELP:
        return L"Opens definitions and guidance for the terms and values used here.";
    default:
        return L"Shows help for this setting or diagnostic value.";
    }
}

const wchar_t* context_help_title(const context_help_entry& entry) {
    return ui_uses_english()
        ? english_control_title(entry.control_id, L"Item Help")
        : entry.title;
}

const wchar_t* context_help_description(
    const context_help_entry& entry
) {
    return ui_uses_english()
        ? english_context_help_description(entry.control_id)
        : entry.description;
}

int help_control_id_from_label(HWND item) {
    if (item == nullptr) {
        return 0;
    }

    switch (GetDlgCtrlID(item)) {
    case IDC_LABEL_TARGET_LUFS: return IDC_TARGET_LUFS;
    case IDC_LABEL_MAX_BOOST: return IDC_MAX_BOOST;
    case IDC_LABEL_MAX_ATTENUATION: return IDC_MAX_ATTENUATION;
    case IDC_LABEL_TRUE_PEAK: return IDC_TRUE_PEAK;
    case IDC_LABEL_LOOKAHEAD: return IDC_LOOKAHEAD_MS;
    case IDC_LABEL_LIMITER_RELEASE: return IDC_LIMITER_RELEASE_MS;
    case IDC_LABEL_STARTUP: return IDC_STARTUP_ANALYSIS_SECONDS;
    case IDC_LABEL_SILENCE_THRESHOLD: return IDC_SILENCE_GUARD_LUFS;
    case IDC_LABEL_GAIN_LOCK_SECONDS: return IDC_GAIN_LOCK_SECONDS;
    case IDC_LABEL_GAIN_LOCK_TOLERANCE: return IDC_GAIN_LOCK_TOLERANCE_LU;
    case IDC_LABEL_MODERN_STRENGTH: return IDC_MODERN_STRENGTH;
    case IDC_LABEL_DIAG_NORMALIZATION: return IDC_DIAG_NORMALIZATION_STATE;
    case IDC_LABEL_DIAG_GAIN_LOCK: return IDC_DIAG_GAIN_LOCK;
    case IDC_LABEL_DIAG_MOMENTARY: return IDC_DIAG_MOMENTARY;
    case IDC_LABEL_DIAG_SHORT_TERM: return IDC_DIAG_SHORT_TERM;
    case IDC_LABEL_DIAG_INPUT_INT: return IDC_DIAG_INTEGRATED;
    case IDC_LABEL_DIAG_OUTPUT_INT: return IDC_DIAG_OUTPUT_INTEGRATED;
    case IDC_LABEL_DIAG_TARGET_DIFF: return IDC_DIAG_TARGET_DIFFERENCE;
    case IDC_LABEL_DIAG_LRA: return IDC_DIAG_LRA;
    case IDC_LABEL_DIAG_GAIN: return IDC_DIAG_GAIN;
    case IDC_LABEL_DIAG_PROCESSING: return IDC_DIAG_PROCESSING_RISK;
    case IDC_LABEL_DIAG_SAFETY: return IDC_DIAG_SAFETY_REDUCTION;
    case IDC_LABEL_DIAG_COMPRESSOR: return IDC_DIAG_COMPRESSOR_REDUCTION;
    case IDC_LABEL_DIAG_CLIPPER: return IDC_DIAG_CLIPPER_REDUCTION;
    case IDC_LABEL_DIAG_LIMITER: return IDC_DIAG_LIMITER_REDUCTION;
    case IDC_LABEL_DIAG_TRUE_PEAK: return IDC_DIAG_TRUE_PEAK;
    case IDC_LABEL_DIAG_LATENCY: return IDC_DIAG_LATENCY;
    case IDC_LABEL_DIAG_PEAK_GUARD: return IDC_DIAG_PEAK_GUARD;
    case IDC_LABEL_DIAG_SAMPLE_RATE: return IDC_DIAG_SAMPLE_RATE;
    case IDC_LABEL_DIAG_CHANNEL_LAYOUT: return IDC_DIAG_CHANNEL_LAYOUT;
    case IDC_LABEL_DIAG_CPU: return IDC_DIAG_CPU_LOAD;
    case IDC_LABEL_DIAG_CLIP_EVENTS: return IDC_DIAG_CLIP_EVENT_COUNT;
    case IDC_LABEL_DIAG_EVALUATION: return IDC_DIAG_PROCESSING_EVALUATION;
    case IDC_LABEL_DIAG_AUTO_REASON: return IDC_DIAG_AUTO_REASON;
    case IDC_LABEL_DIAG_MAX_TRUE_PEAK: return IDC_DIAG_MAX_TRUE_PEAK;
    case IDC_LABEL_DIAG_MAX_COMPRESSOR:
        return IDC_DIAG_MAX_COMPRESSOR_REDUCTION;
    case IDC_LABEL_DIAG_MAX_CLIPPER:
        return IDC_DIAG_MAX_CLIPPER_REDUCTION;
    case IDC_LABEL_DIAG_MAX_LIMITER:
        return IDC_DIAG_MAX_LIMITER_REDUCTION;
    case IDC_LABEL_DIAG_THREE_BAND:
        return IDC_DIAG_THREE_BAND_REDUCTION;
    default:
        break;
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

struct text_info_dialog_data {
    const wchar_t* title = nullptr;
    const wchar_t* text = nullptr;
};

INT_PTR CALLBACK text_info_dialog_proc(
    HWND wnd,
    UINT message,
    WPARAM wp,
    LPARAM lp
) {
    auto* dark_mode = reinterpret_cast<fb2k::CCoreDarkModeHooks*>(
        GetWindowLongPtrW(wnd, GWLP_USERDATA)
    );

    switch (message) {
    case WM_INITDIALOG: {
        const auto* data =
            reinterpret_cast<const text_info_dialog_data*>(lp);

        dark_mode = new fb2k::CCoreDarkModeHooks();
        SetWindowLongPtrW(
            wnd,
            GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(dark_mode)
        );
        dark_mode->AddDialogWithControls(wnd);
        SetDlgItemTextW(
            wnd,
            IDOK,
            ui_text(L"閉じる", L"Close")
        );

        if (data != nullptr) {
            if (data->title != nullptr) {
                SetWindowTextW(wnd, data->title);
            }
            if (data->text != nullptr) {
                SetDlgItemTextW(
                    wnd,
                    IDC_TEXT_INFO_BODY,
                    data->text
                );
            }
        }

        SendDlgItemMessageW(
            wnd,
            IDC_TEXT_INFO_BODY,
            EM_SETSEL,
            0,
            0
        );
        SetFocus(GetDlgItem(wnd, IDOK));
        return FALSE;
    }

    case WM_COMMAND:
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

void show_text_info_dialog(
    HWND owner,
    const wchar_t* title,
    const wchar_t* text
) {
    const text_info_dialog_data data = {
        title,
        text
    };

    DialogBoxParamW(
        core_api::get_my_instance(),
        MAKEINTRESOURCEW(IDD_R128_TEXT_INFO),
        owner,
        text_info_dialog_proc,
        reinterpret_cast<LPARAM>(&data)
    );
}

INT_PTR CALLBACK confirm_defaults_dialog_proc(
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

        SetWindowTextW(
            wnd,
            ui_text(
                L"初期設定の確認",
                L"Confirm default settings"
            )
        );
        SetDlgItemTextW(
            wnd,
            IDC_CONFIRM_DEFAULTS_TEXT,
            ui_text(
                L"現在の未適用変更を破棄し、すべての設定欄を"
                L"初期設定へ戻します。\r\n"
                L"［初期設定に戻す］のあと［適用］を押すと、"
                L"再生中のDSPへ反映されます。\r\n続けますか？",
                L"This discards current unapplied changes and restores "
                L"every setting to its default value.\r\n"
                L"After selecting Restore Defaults, select Apply to update "
                L"the active DSP.\r\nContinue?"
            )
        );
        SetDlgItemTextW(
            wnd,
            IDOK,
            ui_text(L"初期設定に戻す", L"Restore Defaults")
        );
        SetDlgItemTextW(
            wnd,
            IDCANCEL,
            ui_text(L"取消", L"Cancel")
        );

        SetFocus(GetDlgItem(wnd, IDCANCEL));
        return FALSE;

    case WM_COMMAND:
        if (LOWORD(wp) == IDOK || LOWORD(wp) == IDCANCEL) {
            EndDialog(wnd, LOWORD(wp));
            return TRUE;
        }
        break;

    case WM_CLOSE:
        EndDialog(wnd, IDCANCEL);
        return TRUE;

    case WM_NCDESTROY:
        SetWindowLongPtrW(wnd, GWLP_USERDATA, 0);
        delete dark_mode;
        return FALSE;
    }

    return FALSE;
}

bool confirm_restore_defaults(HWND owner) {
    return DialogBoxParamW(
        core_api::get_my_instance(),
        MAKEINTRESOURCEW(IDD_R128_CONFIRM_DEFAULTS),
        owner,
        confirm_defaults_dialog_proc,
        0
    ) == IDOK;
}

constexpr wchar_t kLicenseCreditsText[] =
    L"R128 リアルタイム音量ノーマライザー 1.9.0\r\n"
    L"\r\n"
    L"作者：Maximum\r\n"
    L"Copyright (c) 2026 Maximum\r\n"
    L"ライセンス：MIT License\r\n"
    L"\r\n"
    L"先行作品への謝辞：\r\n"
    L"EBU R128 Normalizer by mudlord\r\n"
    L"本実装は独立して作成された非公式コンポーネントです。\r\n"
    L"\r\n"
    L"foobar2000 SDKを使用してビルドしています。\r\n"
    L"mudlord氏およびfoobar2000との提携・承認関係はありません。\r\n"
    L"\r\n"
    L"全文はパッケージ内のlicense.txtと\r\n"
    L"THIRD-PARTY-NOTICES.txtをご覧ください。";

constexpr wchar_t kLicenseCreditsTextEnglish[] =
    L"R128 Real-time Loudness Normalizer 1.9.0\r\n"
    L"\r\n"
    L"Author: Maximum\r\n"
    L"Copyright (c) 2026 Maximum\r\n"
    L"License: MIT License\r\n"
    L"\r\n"
    L"Acknowledgment of prior work:\r\n"
    L"EBU R128 Normalizer by mudlord\r\n"
    L"This is an independently developed, unofficial component.\r\n"
    L"\r\n"
    L"Built with the foobar2000 SDK.\r\n"
    L"No affiliation with or endorsement by mudlord or foobar2000 "
    L"is implied.\r\n"
    L"\r\n"
    L"See license.txt and THIRD-PARTY-NOTICES.txt in the package "
    L"for the full notices.";

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
        show_text_info_dialog(
            wnd,
            context_help_title(*entry),
            context_help_description(*entry)
        );
        return;
    }

    show_text_info_dialog(
        wnd,
        ui_text(L"項目ヘルプ", L"Item Help"),
        ui_text(
            L"説明を確認したい設定欄、診断値、チェック項目、"
            L"またはプリセットをクリックしてください。\r\n\r\n"
            L"詳しい用語一覧は画面下部の「用語集」から開けます。",
            L"Select a setting, diagnostic value, check box, or preset "
            L"to view its explanation.\r\n\r\n"
            L"Open Glossary at the bottom of the window for detailed terms."
        )
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

UINT dpi_for_window_or_default(HWND wnd) {
    using get_dpi_for_window_fn = UINT(WINAPI*)(HWND);

    const HMODULE user32 = GetModuleHandleW(L"user32.dll");
    if (user32 != nullptr) {
        const auto get_dpi_for_window =
            reinterpret_cast<get_dpi_for_window_fn>(
                GetProcAddress(user32, "GetDpiForWindow")
            );

        if (get_dpi_for_window != nullptr) {
            const UINT dpi = get_dpi_for_window(wnd);
            if (dpi != 0) {
                return dpi;
            }
        }
    }

    return 96;
}

void update_tooltip_dpi(HWND tooltip, HWND dialog) {
    if (tooltip == nullptr || dialog == nullptr) {
        return;
    }

    const int max_width = MulDiv(
        420,
        static_cast<int>(dpi_for_window_or_default(dialog)),
        96
    );

    SendMessageW(
        tooltip,
        TTM_SETMAXTIPWIDTH,
        0,
        std::max(max_width, 1)
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

    update_tooltip_dpi(tooltip, dialog);
    SendMessageW(tooltip, TTM_SETDELAYTIME, TTDT_INITIAL, 450);
    SendMessageW(tooltip, TTM_SETDELAYTIME, TTDT_AUTOPOP, 12000);

    for (const auto& entry : kPresetTooltips) {
        add_tooltip(
            tooltip,
            dialog,
            entry.control_id,
            ui_text(entry.japanese, entry.english)
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
                context_help_description(entry)
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
            ui_text(
                L"左の用語を選択してください。",
                L"Select a term from the list."
            )
        );
        return;
    }

    const auto& entries = ui_uses_english()
        ? kGlossaryEntriesEnglish
        : kGlossaryEntries;

    SetDlgItemTextW(
        wnd,
        IDC_GLOSSARY_DESCRIPTION,
        entries[static_cast<t_size>(selection)].description
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
    case WM_INITDIALOG: {
        dark_mode = new fb2k::CCoreDarkModeHooks();
        SetWindowLongPtrW(
            wnd,
            GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(dark_mode)
        );
        dark_mode->AddDialogWithControls(wnd);

        SetWindowTextW(
            wnd,
            ui_text(
                L"R128 音量ノーマライザー 用語集",
                L"R128 Loudness Normalizer Glossary"
            )
        );
        SetDlgItemTextW(
            wnd,
            IDC_GLOSSARY_INSTRUCTION,
            ui_text(
                L"左の用語を選ぶと、このコンポーネント内での意味と読み方を表示します。",
                L"Select a term on the left to view its meaning and how it is used in this component."
            )
        );
        SetDlgItemTextW(
            wnd,
            IDOK,
            ui_text(L"閉じる", L"Close")
        );

        const auto& entries = ui_uses_english()
            ? kGlossaryEntriesEnglish
            : kGlossaryEntries;

        for (const auto& entry : entries) {
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
    }

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
    dialog_scroll_state scroll;
    fb2k::CCoreDarkModeHooks dark_mode;
    bool tooltip_dark_mode = false;
    bool tooltip_theme_initialized = false;
    bool updating_controls = false;
    bool has_unapplied_changes = false;

    // Direct main-menu launch only.
    // Modal DSP Manager dialogs leave these at their defaults.
    bool modeless = false;
    HWND* tracked_window = nullptr;
    void* cleanup_state = nullptr;
    void (*cleanup)(dialog_context*) = nullptr;
};

void update_tooltip_theme(
    dialog_context* context,
    bool force
) {
    if (context == nullptr || context->tooltip == nullptr) {
        return;
    }

    const bool dark_mode =
        static_cast<bool>(context->dark_mode);

    if (!force &&
        context->tooltip_theme_initialized &&
        context->tooltip_dark_mode == dark_mode) {
        return;
    }

    SetWindowTheme(
        context->tooltip,
        dark_mode ? L"DarkMode_Explorer" : nullptr,
        nullptr
    );
    RedrawWindow(
        context->tooltip,
        nullptr,
        nullptr,
        RDW_INVALIDATE | RDW_ERASE | RDW_FRAME
    );

    context->tooltip_dark_mode = dark_mode;
    context->tooltip_theme_initialized = true;
}

void recreate_help_tooltips(
    HWND wnd,
    dialog_context* context
) {
    if (context == nullptr) {
        return;
    }

    if (context->tooltip != nullptr) {
        DestroyWindow(context->tooltip);
        context->tooltip = nullptr;
    }

    context->tooltip = create_help_tooltips(wnd);
    context->tooltip_theme_initialized = false;
    update_tooltip_theme(context, true);
}

constexpr int kSettingEditControls[] = {
    IDC_TARGET_LUFS,
    IDC_MAX_BOOST,
    IDC_MAX_ATTENUATION,
    IDC_TRUE_PEAK,
    IDC_LOOKAHEAD_MS,
    IDC_LIMITER_RELEASE_MS,
    IDC_STARTUP_ANALYSIS_SECONDS,
    IDC_SILENCE_GUARD_LUFS,
    IDC_GAIN_LOCK_SECONDS,
    IDC_GAIN_LOCK_TOLERANCE_LU,
    IDC_MODERN_STRENGTH
};

constexpr int kSettingCheckControls[] = {
    IDC_RESET_EACH_TRACK,
    IDC_ENABLE_PEAK_GUARD,
    IDC_ENABLE_SILENCE_GUARD,
    IDC_ENABLE_GAIN_LOCK,
    IDC_ENABLE_MODERN_BOOST,
    IDC_ENABLE_ADAPTIVE_MASTER,
    IDC_ENABLE_THREE_BAND_MASTER
};

template <t_size Count>
bool contains_control_id(
    const int (&controls)[Count],
    int control_id
) {
    for (const int candidate : controls) {
        if (candidate == control_id) {
            return true;
        }
    }

    return false;
}

void set_apply_button_state(
    HWND wnd,
    dialog_context* context,
    bool has_unapplied_changes
) {
    if (context != nullptr) {
        context->has_unapplied_changes = has_unapplied_changes;
    }

    const HWND apply_button =
        GetDlgItem(wnd, IDC_APPLY_SETTINGS);

    if (apply_button != nullptr) {
        EnableWindow(
            apply_button,
            has_unapplied_changes ? TRUE : FALSE
        );
    }
}

void mark_unapplied_changes(
    HWND wnd,
    dialog_context* context
) {
    if (context == nullptr || context->updating_controls) {
        return;
    }

    set_apply_button_state(wnd, context, true);
    set_control_text(
        wnd,
        IDC_APPLY_STATUS,
        ui_text(
            L"未適用の変更があります",
            L"There are unapplied changes"
        )
    );
}

void select_profile_in_dialog(
    HWND wnd,
    dialog_context* context,
    const r128_settings& profile
) {
    if (context == nullptr) {
        return;
    }

    context->value = profile;
    context->updating_controls = true;
    settings_to_dialog(wnd, context->value);
    context->updating_controls = false;

    update_profile_indicator(
        wnd,
        context->value,
        true
    );
    mark_unapplied_changes(wnd, context);
}

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
            value.target_lufs,
            ui_text(L"目標ラウドネス", L"Target loudness"))) {
        return false;
    }
    if (!read_float(
            wnd, IDC_MAX_BOOST, 0.0f, 24.0f,
            value.max_boost_db,
            ui_text(L"最大増幅量", L"Maximum boost"))) {
        return false;
    }
    if (!read_float(
            wnd, IDC_MAX_ATTENUATION, 0.0f, 36.0f,
            value.max_attenuation_db,
            ui_text(L"最大減衰量", L"Maximum attenuation"))) {
        return false;
    }
    if (!read_float(
            wnd, IDC_TRUE_PEAK, -12.0f, 0.0f,
            value.true_peak_limit_dbtp,
            ui_text(L"True Peak上限", L"True Peak limit"))) {
        return false;
    }
    if (!read_float(
            wnd, IDC_LOOKAHEAD_MS, 0.0f, 20.0f,
            value.lookahead_ms,
            ui_text(L"先読み時間", L"Look-ahead time"))) {
        return false;
    }
    if (!read_float(
            wnd, IDC_LIMITER_RELEASE_MS, 20.0f, 1000.0f,
            value.limiter_release_ms,
            ui_text(L"リミッター解放時間", L"Limiter release time"))) {
        return false;
    }
    if (!read_float(
            wnd, IDC_STARTUP_ANALYSIS_SECONDS, 0.0f, 15.0f,
            value.startup_analysis_seconds,
            ui_text(
                L"曲頭の測定安定化時間",
                L"Startup analysis time"
            ))) {
        return false;
    }
    if (!read_float(
            wnd, IDC_SILENCE_GUARD_LUFS, -70.0f, -20.0f,
            value.silence_guard_lufs,
            ui_text(
                L"静音保護しきい値",
                L"Silence guard threshold"
            ))) {
        return false;
    }
    if (!read_float(
            wnd, IDC_GAIN_LOCK_SECONDS, 0.0f, 60.0f,
            value.gain_lock_seconds,
            ui_text(
                L"補正ゲイン固定判定時間",
                L"Gain-lock detection time"
            ))) {
        return false;
    }
    if (!read_float(
            wnd, IDC_GAIN_LOCK_TOLERANCE_LU, 0.1f, 3.0f,
            value.gain_lock_tolerance_lu,
            ui_text(
                L"固定許容変動",
                L"Gain-lock tolerance"
            ))) {
        return false;
    }
    if (!read_float(
            wnd, IDC_MODERN_STRENGTH, 0.0f, 100.0f,
            value.modern_strength_percent,
            ui_text(
                L"モダン強度",
                L"Modern processing strength"
            ))) {
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
    set_apply_button_state(wnd, context, false);
    set_control_text(
        wnd,
        IDC_APPLY_STATUS,
        ui_text(
            L"設定を適用しました",
            L"Settings applied"
        )
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
        fit_dialog_to_monitor_work_area(
            wnd,
            context != nullptr ? &context->scroll : nullptr
        );

        if (context != nullptr) {
            recreate_help_tooltips(wnd, context);
        }
        CheckDlgButton(
            wnd,
            IDC_COMPARE_LOUDNESS_MATCH,
            BST_CHECKED
        );
        if (context != nullptr) {
            context->updating_controls = true;
            settings_to_dialog(wnd, context->value);
            context->updating_controls = false;

            apply_primary_ui_language(
                wnd,
                &context->value,
                false
            );
        }
        else {
            apply_primary_ui_language(wnd, nullptr, false);
        }
        set_apply_button_state(wnd, context, false);
        set_control_text(
            wnd,
            IDC_APPLY_STATUS,
            ui_text(
                L"変更後は［適用］または［OK］",
                L"After changes, select Apply or OK"
            )
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
        if (context != nullptr) {
            reset_dialog_scroll_position(wnd, context->scroll);
            update_dialog_scroll_dpi(
                context->scroll,
                HIWORD(wp)
            );
        }

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

        fit_dialog_to_monitor_work_area(
            wnd,
            context != nullptr ? &context->scroll : nullptr
        );
        if (context != nullptr && context->tooltip != nullptr) {
            update_tooltip_dpi(context->tooltip, wnd);
        }
        return TRUE;

    case WM_HSCROLL:
        if (context != nullptr &&
            reinterpret_cast<HWND>(lp) == nullptr &&
            scroll_config_dialog(
                wnd,
                context->scroll,
                SB_HORZ,
                LOWORD(wp)
            )) {
            return TRUE;
        }
        break;

    case WM_VSCROLL:
        if (context != nullptr &&
            reinterpret_cast<HWND>(lp) == nullptr &&
            scroll_config_dialog(
                wnd,
                context->scroll,
                SB_VERT,
                LOWORD(wp)
            )) {
            return TRUE;
        }
        break;

    case WM_MOUSEWHEEL:
        if (context != nullptr) {
            const int wheel_delta = GET_WHEEL_DELTA_WPARAM(wp);
            const int line_count = std::max(
                std::abs(wheel_delta) / WHEEL_DELTA,
                1
            ) * 3;
            const UINT scroll_request =
                wheel_delta > 0 ? SB_LINEUP : SB_LINEDOWN;

            for (int index = 0; index < line_count; ++index) {
                scroll_config_dialog(
                    wnd,
                    context->scroll,
                    SB_VERT,
                    scroll_request
                );
            }
            return TRUE;
        }
        break;

    case WM_THEMECHANGED:
    case WM_SYSCOLORCHANGE:
    case WM_SETTINGCHANGE:
        update_tooltip_theme(context, true);
        break;

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
            update_tooltip_theme(context, false);
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
        if (LOWORD(wp) == IDC_DISPLAY_LANGUAGE &&
            HIWORD(wp) == CBN_SELCHANGE) {
            const LRESULT selection = SendDlgItemMessageW(
                wnd,
                IDC_DISPLAY_LANGUAGE,
                CB_GETCURSEL,
                0,
                0
            );

            if (selection >= 0 && selection <= 2) {
                g_cfg_display_language =
                    static_cast<t_int32>(selection);

                apply_primary_ui_language(
                    wnd,
                    context != nullptr ? &context->value : nullptr,
                    false
                );
                recreate_help_tooltips(wnd, context);
                refresh_diagnostic_controls(wnd);
                set_control_text(
                    wnd,
                    IDC_APPLY_STATUS,
                    ui_text(
                        L"表示言語を変更しました",
                        L"Display language updated"
                    )
                );
            }
            return TRUE;
        }

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
                        ? ui_text(
                            L"A/B音量一致中",
                            L"A/B loudness matching"
                        )
                        : ui_text(
                            L"原音比較中",
                            L"Comparing original audio"
                        )
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
                    context != nullptr &&
                    context->has_unapplied_changes
                        ? ui_text(
                            L"未適用の変更があります",
                            L"There are unapplied changes"
                        )
                        : ui_text(
                            L"処理音へ戻りました",
                            L"Returned to processed audio"
                        )
                );
                return TRUE;
            }
        }

        if (context != nullptr &&
            !context->updating_controls) {
            const int control_id = LOWORD(wp);
            const WORD notification = HIWORD(wp);

            const bool edited_value =
                notification == EN_CHANGE &&
                contains_control_id(
                    kSettingEditControls,
                    control_id
                );

            const bool clicked_setting =
                notification == BN_CLICKED &&
                contains_control_id(
                    kSettingCheckControls,
                    control_id
                );

            if (edited_value || clicked_setting) {
                mark_unapplied_changes(wnd, context);
            }
        }

        switch (LOWORD(wp)) {
        case IDC_DEFAULTS:
            if (confirm_restore_defaults(wnd)) {
                select_profile_in_dialog(
                    wnd,
                    context,
                    standard_profile()
                );
            }
            return TRUE;

        case IDC_PROFILE_STANDARD:
            select_profile_in_dialog(
                wnd,
                context,
                standard_profile()
            );
            return TRUE;

        case IDC_PROFILE_STREAMING:
            select_profile_in_dialog(
                wnd,
                context,
                streaming_profile()
            );
            return TRUE;

        case IDC_PROFILE_BROADCAST:
            select_profile_in_dialog(
                wnd,
                context,
                broadcast_profile()
            );
            return TRUE;

        case IDC_PROFILE_NIGHT:
            select_profile_in_dialog(
                wnd,
                context,
                night_profile()
            );
            return TRUE;

        case IDC_PROFILE_MODERN:
            select_profile_in_dialog(
                wnd,
                context,
                modern_profile()
            );
            return TRUE;

        case IDC_PROFILE_ADAPTIVE:
            select_profile_in_dialog(
                wnd,
                context,
                adaptive_profile()
            );
            return TRUE;

        case IDC_PROFILE_THREE_BAND:
            select_profile_in_dialog(
                wnd,
                context,
                three_band_profile()
            );
            return TRUE;

        case IDC_RESET_MEASUREMENT:
            g_measurement_reset_request.fetch_add(
                1,
                std::memory_order_relaxed
            );
            set_control_text(
                wnd,
                IDC_DIAG_NORMALIZATION_STATE,
                ui_text(
                    L"測定リセットを要求しました",
                    L"Measurement reset requested"
                )
            );
            return TRUE;

        case IDC_COPY_DIAGNOSTICS:
            if (copy_unicode_text_to_clipboard(
                    wnd,
                    build_diagnostic_report())) {
                MessageBoxW(
                    wnd,
                    ui_text(
                        L"診断結果をクリップボードへコピーしました。",
                        L"Diagnostic results copied to the clipboard."
                    ),
                    ui_text(
                        L"診断結果のコピー",
                        L"Copy diagnostics"
                    ),
                    MB_OK | MB_ICONINFORMATION
                );
            }
            else {
                MessageBoxW(
                    wnd,
                    ui_text(
                        L"クリップボードへコピーできませんでした。",
                        L"Could not copy to the clipboard."
                    ),
                    ui_text(
                        L"診断結果のコピー",
                        L"Copy diagnostics"
                    ),
                    MB_OK | MB_ICONWARNING
                );
            }
            return TRUE;

        case IDC_SHOW_AUTO_HISTORY:
            DialogBoxParamW(
                core_api::get_my_instance(),
                MAKEINTRESOURCEW(IDD_R128_AUTO_HISTORY),
                wnd,
                auto_control_history_dialog_proc,
                0
            );
            return TRUE;

        case IDC_SHOW_TREND_GRAPH:
            DialogBoxParamW(
                core_api::get_my_instance(),
                MAKEINTRESOURCEW(IDD_R128_TREND_GRAPH),
                wnd,
                auto_control_trend_dialog_proc,
                0
            );
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
            show_text_info_dialog(
                wnd,
                ui_text(
                    L"ライセンスとクレジット",
                    L"License and Credits"
                ),
                ui_uses_english()
                    ? kLicenseCreditsTextEnglish
                    : kLicenseCreditsText
            );
            return TRUE;

        case IDC_APPLY_SETTINGS:
            if (context != nullptr &&
                context->has_unapplied_changes) {
                apply_dialog_settings(wnd, context);
            }
            return TRUE;

        case IDOK:
            if (context == nullptr ||
                !context->has_unapplied_changes) {
                close_config_dialog(wnd, IDOK, context);
            }
            else if (apply_dialog_settings(wnd, context)) {
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
        m_last_history_reset_request =
            g_history_track_reset_request.load(
                std::memory_order_relaxed
            );
        m_history_session_id =
            g_history_active_session_id.load(
                std::memory_order_relaxed
            );

        if (m_history_session_id != 0 &&
            g_history_metrics_session_id.load(
                std::memory_order_relaxed
            ) == m_history_session_id) {
            m_auto_control_trigger_count =
                g_history_auto_control_trigger_count.load(
                    std::memory_order_relaxed
                );
            m_auto_control_history_reason_mask =
                g_history_auto_control_reason_mask.load(
                    std::memory_order_relaxed
                );
            m_auto_control_latest_reason_mask =
                g_history_latest_auto_control_reason_mask.load(
                    std::memory_order_relaxed
                );
            m_track_max_auto_safety_reduction_db =
                g_history_max_auto_attenuation_db.load(
                    std::memory_order_relaxed
                );
            m_auto_control_limit_reached =
                g_history_adjustment_limit_reached.load(
                    std::memory_order_relaxed
                ) != 0;
            m_auto_control_latest_recovered =
                g_history_recovered.load(
                    std::memory_order_relaxed
                ) != 0;
        }
    }

    static GUID g_get_guid() {
        return guid_r128_normalizer;
    }

    static void g_get_name(pfc::string_base& out) {
        if (ui_uses_english()) {
            out = "R128 Loudness Normalizer";
        }
        else {
            out = "R128 \xE9\x9F\xB3\xE9\x87\x8F"
                "\xE3\x83\x8E\xE3\x83\xBC\xE3\x83\x9E"
                "\xE3\x83\xA9\xE3\x82\xA4\xE3\x82\xB6"
                "\xE3\x83\xBC";
        }
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

        const unsigned long long history_reset_request =
            g_history_track_reset_request.load(
                std::memory_order_relaxed
            );

        if (history_reset_request !=
            m_last_history_reset_request) {
            reset_history_tracking_state_only();
            m_last_history_reset_request =
                history_reset_request;
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
                audition_bypass
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
        if (m_sample_rate == 0 || audition_bypass) {
            m_current_processing_state = 0;
            return;
        }

        const double compressor_reduction_db =
            m_settings.enable_modern_boost
                ? std::max(0.0, -m_modern_compressor_gain_db)
                : 0.0;
        const double limiter_reduction_db = std::max(
            0.0,
            -m_limiter_gain_db
        );
        const double output_true_peak_dbtp =
            linear_to_db(m_current_output_true_peak_linear);

        const bool strong_now =
            compressor_reduction_db >= 3.0 ||
            clipper_reduction_db >= 1.5 ||
            limiter_reduction_db >= 1.5;

        const bool excessive_now =
            clipper_reduction_db >= 3.0 ||
            limiter_reduction_db >= 3.0 ||
            output_true_peak_dbtp > 0.01;
        const int current_reason_mask =
            (output_true_peak_dbtp > 0.01 ? 1 : 0) |
            (limiter_reduction_db >= 3.0 ? 2 : 0) |
            (clipper_reduction_db >= 3.0 ? 4 : 0);

        if (!m_auto_control_engaged &&
            m_auto_control_recovered &&
            current_reason_mask != 0) {
            m_auto_control_recovered = false;
            m_auto_control_reason_mask = 0;
        }

        if (current_reason_mask != 0) {
            m_auto_control_reason_mask |= current_reason_mask;

            if (m_auto_control_engaged) {
                m_auto_control_history_reason_mask |=
                    current_reason_mask;
                m_auto_control_latest_reason_mask |=
                    current_reason_mask;
            }
        }

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
            m_auto_control_safe_seconds = 0.0;
        }
        else {
            m_excessive_processing_seconds = std::max(
                0.0,
                m_excessive_processing_seconds - 2.0 * frame_seconds
            );

            if (strong_now) {
                m_auto_control_safe_seconds = 0.0;
            }
            else {
                m_auto_control_safe_seconds += frame_seconds;
            }
        }

        if (m_settings.enable_modern_boost) {
            if (m_excessive_processing_seconds >= kRiskExcessiveHoldSeconds) {
                m_processing_risk_state = 3;
            }
            else if (m_strong_processing_seconds >= kRiskStrongHoldSeconds) {
                m_processing_risk_state = 2;
            }
            else {
                m_processing_risk_state = 1;
            }
        }
        else {
            m_processing_risk_state = 0;
        }

        if (!m_auto_control_engaged &&
            m_excessive_processing_seconds >= kAutoSafetyTriggerSeconds) {
            m_auto_control_engaged = true;
            m_auto_control_recovered = false;
            m_auto_control_latest_recovered = false;
            m_auto_control_safe_seconds = 0.0;
            ++m_auto_control_trigger_count;
            m_auto_control_latest_reason_mask =
                m_auto_control_reason_mask;
            m_auto_control_history_reason_mask |=
                m_auto_control_latest_reason_mask;
        }

        if (!m_auto_control_engaged &&
            !m_auto_control_recovered &&
            !excessive_now &&
            m_excessive_processing_seconds <= 0.001) {
            m_auto_control_reason_mask = 0;
        }

        double target_safety_db = 0.0;

        if (m_auto_control_engaged) {
            if (strong_now || excessive_now) {
                double required_reduction_db = 0.0;
                required_reduction_db = std::max(
                    required_reduction_db,
                    limiter_reduction_db - 1.25
                );
                required_reduction_db = std::max(
                    required_reduction_db,
                    clipper_reduction_db - 1.25
                );

                if (output_true_peak_dbtp > -0.10) {
                    required_reduction_db = std::max(
                        required_reduction_db,
                        output_true_peak_dbtp + 0.10
                    );
                }

                const double requested_reduction_db = clamp_value(
                    std::max(0.50, required_reduction_db + 0.25),
                    0.0,
                    kAutoSafetyMaximumReductionDb
                );
                target_safety_db = std::min(
                    m_safety_reduction_db,
                    -requested_reduction_db
                );
            }
            else if (m_auto_control_safe_seconds < kAutoSafetySafeHoldSeconds) {
                target_safety_db = m_safety_reduction_db;
            }
        }

        const bool increasing_protection =
            target_safety_db < m_safety_reduction_db;
        const double time_seconds = increasing_protection
            ? kAutoSafetyAttackSeconds
            : kAutoSafetyReleaseSeconds;
        const double coefficient = std::exp(
            -1.0 /
            (time_seconds * static_cast<double>(m_sample_rate))
        );

        m_safety_reduction_db =
            target_safety_db +
            (m_safety_reduction_db - target_safety_db) * coefficient;
        m_safety_reduction_db = clamp_value(
            m_safety_reduction_db,
            -kAutoSafetyMaximumReductionDb,
            0.0
        );
        m_track_max_auto_safety_reduction_db = std::max(
            m_track_max_auto_safety_reduction_db,
            -m_safety_reduction_db
        );

        const bool at_adjustment_limit =
            m_safety_reduction_db <= -kAutoSafetyMaximumReductionDb + 0.02;

        if (m_auto_control_engaged && at_adjustment_limit && excessive_now) {
            m_auto_control_limit_seconds += frame_seconds;
        }
        else {
            m_auto_control_limit_seconds = std::max(
                0.0,
                m_auto_control_limit_seconds - 2.0 * frame_seconds
            );
        }

        if (!m_auto_control_engaged) {
            m_current_processing_state =
                m_excessive_processing_seconds > 0.001
                ? 4
                : 1;
        }
        else if (m_auto_control_limit_seconds >= kAutoSafetyLimitHoldSeconds) {
            m_current_processing_state = 3;
            m_auto_control_limit_reached = true;
        }
        else if (target_safety_db >= -0.001 &&
                 m_safety_reduction_db > -0.02 &&
                 m_auto_control_safe_seconds >= kAutoSafetySafeHoldSeconds) {
            m_auto_control_engaged = false;
            m_auto_control_safe_seconds = 0.0;
            m_auto_control_limit_seconds = 0.0;
            m_safety_reduction_db = 0.0;
            m_auto_control_recovered = true;
            m_auto_control_latest_recovered = true;
            m_current_processing_state = 1;
        }
        else {
            m_current_processing_state = 2;
        }
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
                            (m_current_gain_db + m_safety_reduction_db)
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
                    m_current_gain_db +
                    m_safety_reduction_db +
                    m_limiter_gain_db;

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
            m_current_output_true_peak_linear = output_frame_true_peak;
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
            m_current_output_true_peak_linear = 0.0;
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
        if (!m_history_final_published &&
            m_history_session_id != 0) {
            g_history_final_auto_control_trigger_count.store(
                m_auto_control_trigger_count,
                std::memory_order_relaxed
            );
            g_history_final_auto_control_reason_mask.store(
                m_auto_control_history_reason_mask,
                std::memory_order_relaxed
            );
            g_history_final_max_auto_attenuation_db.store(
                m_track_max_auto_safety_reduction_db,
                std::memory_order_relaxed
            );
            g_history_final_adjustment_limit_reached.store(
                m_auto_control_limit_reached ? 1 : 0,
                std::memory_order_relaxed
            );
            g_history_final_recovered.store(
                m_auto_control_latest_recovered ? 1 : 0,
                std::memory_order_relaxed
            );
            g_history_final_profile_id.store(
                static_cast<int>(
                    detect_recognized_profile(m_settings)
                ),
                std::memory_order_relaxed
            );
            g_history_final_session_id.store(
                m_history_session_id,
                std::memory_order_relaxed
            );
            m_history_final_published = true;
        }

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
            m_safety_reduction_db,
            std::memory_order_relaxed
        );
        g_diagnostic_current_processing_state.store(
            m_current_processing_state,
            std::memory_order_relaxed
        );
        g_diagnostic_auto_control_reason_mask.store(
            m_auto_control_reason_mask,
            std::memory_order_relaxed
        );
        g_history_auto_control_trigger_count.store(
            m_auto_control_trigger_count,
            std::memory_order_relaxed
        );
        g_history_auto_control_reason_mask.store(
            m_auto_control_history_reason_mask,
            std::memory_order_relaxed
        );
        g_history_latest_auto_control_reason_mask.store(
            m_auto_control_latest_reason_mask,
            std::memory_order_relaxed
        );
        g_history_max_auto_attenuation_db.store(
            m_track_max_auto_safety_reduction_db,
            std::memory_order_relaxed
        );
        g_history_adjustment_limit_reached.store(
            m_auto_control_limit_reached ? 1 : 0,
            std::memory_order_relaxed
        );
        g_history_recovered.store(
            m_auto_control_latest_recovered ? 1 : 0,
            std::memory_order_relaxed
        );
        g_history_profile_id.store(
            static_cast<int>(
                detect_recognized_profile(m_settings)
            ),
            std::memory_order_relaxed
        );
        g_history_metrics_session_id.store(
            m_history_session_id,
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

    void reset_history_tracking_state_only() {
        m_history_session_id =
            g_history_active_session_id.load(
                std::memory_order_relaxed
            );
        m_auto_control_trigger_count = 0;
        m_auto_control_history_reason_mask = 0;
        m_auto_control_latest_reason_mask = 0;
        m_track_max_auto_safety_reduction_db = 0.0;
        m_auto_control_limit_reached = false;
        m_auto_control_latest_recovered = false;
        m_history_final_published = false;

        g_history_auto_control_trigger_count.store(
            0,
            std::memory_order_relaxed
        );
        g_history_auto_control_reason_mask.store(
            0,
            std::memory_order_relaxed
        );
        g_history_latest_auto_control_reason_mask.store(
            0,
            std::memory_order_relaxed
        );
        g_history_max_auto_attenuation_db.store(
            0.0,
            std::memory_order_relaxed
        );
        g_history_adjustment_limit_reached.store(
            0,
            std::memory_order_relaxed
        );
        g_history_recovered.store(
            0,
            std::memory_order_relaxed
        );
        g_history_profile_id.store(
            static_cast<int>(
                detect_recognized_profile(m_settings)
            ),
            std::memory_order_relaxed
        );
        g_history_metrics_session_id.store(
            m_history_session_id,
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
        g_diagnostic_current_processing_state.store(
            0,
            std::memory_order_relaxed
        );
        g_diagnostic_auto_control_reason_mask.store(
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
        m_current_output_true_peak_linear = 0.0;
        m_strong_processing_seconds = 0.0;
        m_excessive_processing_seconds = 0.0;
        m_auto_control_safe_seconds = 0.0;
        m_auto_control_limit_seconds = 0.0;
        m_auto_control_engaged = false;
        m_auto_control_recovered = false;
        m_auto_control_reason_mask = 0;
        m_current_processing_state = 0;
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
        g_diagnostic_current_processing_state.store(
            0,
            std::memory_order_relaxed
        );
        g_diagnostic_auto_control_reason_mask.store(
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
    double m_current_output_true_peak_linear = 0.0;
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
    double m_auto_control_safe_seconds = 0.0;
    double m_auto_control_limit_seconds = 0.0;
    bool m_auto_control_engaged = false;
    bool m_auto_control_recovered = false;
    int m_auto_control_reason_mask = 0;
    unsigned m_auto_control_trigger_count = 0;
    int m_auto_control_history_reason_mask = 0;
    int m_auto_control_latest_reason_mask = 0;
    double m_track_max_auto_safety_reduction_db = 0.0;
    bool m_auto_control_limit_reached = false;
    bool m_auto_control_latest_recovered = false;
    bool m_history_final_published = false;
    unsigned long long m_history_session_id = 0;
    int m_current_processing_state = 0;
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
    unsigned long long m_last_history_reset_request = 0;
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
                ui_text(
                    L"設定画面を開いている間にDSPチェーンが"
                    L"変更されたため、設定を適用できませんでした。\n\n"
                    L"DSPの登録状態を確認して、もう一度お試しください。",
                    L"The DSP chain changed while the settings window "
                    L"was open, so the settings could not be applied.\n\n"
                    L"Check the DSP chain and try again."
                ),
                ui_text(
                    L"R128 音量ノーマライザー",
                    L"R128 Loudness Normalizer"
                ),
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
            ui_text(
                L"R128 音量ノーマライザーは、現在のDSPチェーンに"
                L"追加されていません。\n\n"
                L"Playback → DSP Managerで追加してから、"
                L"もう一度このメニューを開いてください。",
                L"R128 Loudness Normalizer is not in the current DSP chain.\n\n"
                L"Add it in Playback > DSP Manager, then open this command again."
            ),
            ui_text(
                L"R128 音量ノーマライザー",
                L"R128 Loudness Normalizer"
            ),
            MB_OK | MB_ICONINFORMATION
        );
        return;
    }

    if (match_count > 1) {
        MessageBoxW(
            owner,
            ui_text(
                L"現在のDSPチェーンにR128 音量ノーマライザーが"
                L"複数登録されています。\n\n"
                L"誤った設定を変更しないため、DSP Managerから"
                L"対象を選んで設定してください。",
                L"More than one R128 Loudness Normalizer is present "
                L"in the current DSP chain.\n\n"
                L"Open DSP Manager and select the instance you want to configure."
            ),
            ui_text(
                L"R128 音量ノーマライザー",
                L"R128 Loudness Normalizer"
            ),
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
            ui_text(
                L"設定画面を作成できませんでした。",
                L"Could not create the settings window."
            ),
            ui_text(
                L"R128 音量ノーマライザー",
                L"R128 Loudness Normalizer"
            ),
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

        if (ui_uses_english()) {
            out = "R128 Loudness Normalizer Settings...";
        }
        else {
            out =
                "R128 \xE9\x9F\xB3\xE9\x87\x8F"
                "\xE3\x83\x8E\xE3\x83\xBC\xE3\x83\x9E"
                "\xE3\x83\xA9\xE3\x82\xA4\xE3\x82\xB6"
                "\xE3\x83\xBC\xE3\x81\xAE\xE8\xA8\xAD"
                "\xE5\xAE\x9A...";
        }
    }

    bool get_description(
        t_uint32 index,
        pfc::string_base& out
    ) override {
        if (index != 0) {
            uBugCheck();
        }

        if (ui_uses_english()) {
            out =
                "Opens the R128 Loudness Normalizer settings "
                "dialog directly.";
        }
        else {
            out =
                "R128 \xE9\x9F\xB3\xE9\x87\x8F"
                "\xE3\x83\x8E\xE3\x83\xBC\xE3\x83\x9E"
                "\xE3\x83\xA9\xE3\x82\xA4\xE3\x82\xB6"
                "\xE3\x83\xBC\xE3\x81\xAE\xE8\xA8\xAD"
                "\xE5\xAE\x9A\xE7\x94\xBB\xE9\x9D\xA2"
                "\xE3\x82\x92\xE7\x9B\xB4\xE6\x8E\xA5"
                "\xE9\x96\x8B\xE3\x81\x8D\xE3\x81\xBE"
                "\xE3\x81\x99\xE3\x80\x82";
        }
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
