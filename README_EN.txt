R128 Real-time Loudness Normalizer v1.9.0

For foobar2000 2.x / Windows x64

INSTALLATION

1. Open foo_r128_normalizer_v1.9.0.fb2k-component.
2. Follow the foobar2000 installation prompt.
3. Restart foobar2000.

Existing installations can be upgraded in place.

OPENING SETTINGS

Open:

Playback > R128 Loudness Normalizer Settings...

The DSP must be present exactly once in the active DSP chain. If it is not
present, add it from Playback > DSP Manager.

DISPLAY LANGUAGE

Choose a language at the bottom of the settings window:

- Automatic (Windows)
- Japanese
- English

The selection immediately updates the settings, diagnostics, help, glossary,
tooltips, license information, messages, DSP name, and Playback menu.
Language selection is stored separately from DSP presets.

AUTOMATIC SAFETY CONTROL

v1.8.0 shows Normal, Monitoring, Auto-adjusting, and Adjustment limit states.
After safe recovery, the current processing state returns to Normal.
Diagnostics also identify True Peak exceedance, excessive limiting, excessive
clipping, or multiple factors, and show the automatic attenuation amount as a
positive dB value.

AUTOMATIC-CONTROL HISTORY

Open Automatic-Control History from the Diagnostics page. It retains up to
100 recent tracks where automatic control actually activated. Each entry
contains playback time, title and artist, preset, trigger reason, maximum
automatic attenuation, activation count, adjustment-limit status, and safe
recovery. The list persists across restarts and can be copied, selectively
deleted, or cleared. A brief Monitoring state that never reaches
Auto-adjusting is not added.

AUTOMATIC-CONTROL TREND GRAPH

Open Trend Graph from the Diagnostics page. For the currently playing track,
it records Short-term loudness, total applied gain, automatic attenuation,
and processed True Peak about once per second. Red vertical lines mark
automatic-control activation.

Only the current track is kept in memory, and the graph resets when the track
changes. Drawing runs only while the graph window is open. The real-time audio
thread performs no graph recording or drawing work.

COMPATIBILITY

v1.9.0 retains preset format v7, existing DSP and menu GUIDs, preset values,
audio-processing thresholds, and the 6 dB automatic-attenuation limit. History
storage is separate from DSP presets.

LICENSE

MIT License. See license.txt and THIRD-PARTY-NOTICES.txt.
