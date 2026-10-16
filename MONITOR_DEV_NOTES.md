# v1.14.0 — Source and verification record

## Baseline and release state / 基準・公開状態

The sole published baseline is R128 v1.13.0 Source of Truth revision 1.0.
main/tag baseline: d42f85e3c647ea34e11a59437507b882568cb20a
tree: d8b83566774f5662ddcdf87a8b8907bfbffbc33f
This is release-preparation source, not evidence of publication. The v1.14.0
Windows rebuild/install and received-package verification have passed. GitHub
main/tag/Release, published Asset digests and Latest remain unverified.
The canonical Source of Truth is not switched to an unpublished version.

## Changes / 変更

- Loud -9 = streaming_profile(), changing only target_lufs to -9.0f.
- Five standard buttons on the upper row; three additional presets below.
- Existing recognized-profile/history IDs 0..7 remain fixed; Loud -9 uses 8.
- All old seven profile values, DSP audio calculations, measurements, GUIDs,
  DSP v7, history R128H1, user store R128U1 and backup R128PRESET1 are retained.
- Old fourteen reserved names keep their validation. New Loud names are reserved
  for newly entered Save As/Rename names; old stored/imported Loud names remain valid.
- Every user name gets a display-only (User) / （ユーザー） suffix. Raw names and
  settings are retained in storage and exports; distinct user names stay distinct.
- dev.2 shortens only Loud's JP/EN tooltip and removes the compatibility paragraph
  only from its embedded/bundled glossary entries. Detailed usage help remains.
- Shared tooltip/theme code, monitor headers/resources and runtime are unchanged.
- Formalization changes version strings/scripts/file names and distribution docs;
  product processing and UI behavior are identical to tested dev.2.

## Targeted Windows results / 実機報告（2026-10-16 JST）

These results belong to v1.14.0-dev.2, not the future official-version binary.
The user reported OK for each separately guided check:

- Bundled CMD build/package; installation/restart; Components 1.14.0-dev.2.
- Japanese and English Loud tooltips: rounded corners and matching balloon-stem
  color; embedded glossary compatibility paragraph removed with usage retained.
- Built-in Loud values: target -9.0 LUFS, boost 8 dB, attenuation 18 dB, TP -1 dBTP.
- Modern, 1-Band Adaptive and 3-Band Adaptive all off.
- Apply/reopen and application restart retain built-in Loud -9 and target -9.0.
- Previously saved same-name user preset remains separately labeled and recallable.
- Short playback and live monitor updates reported normal.

The dev.1 long tooltip had square corners and a white stem. Shared tooltip code
was already common to all controls. Shortening fixed the reported display on the
tested system; the exact Windows rendering cause has not been established.

No new 30-minute/CPU measurement was requested or performed. Automatic language,
theme switching/all DPI layouts, original compare, backup import, rename/save
rejection, overwrite/export and every old feature were not rechecked on hardware.
Local tests do not substitute for Windows rendering or SDK-persistence tests.
The historical foobar2000 v2.26 / 1920x1080 / 100% / CPU results are not reassigned
to this build; the current environment was not independently re-inventoried.

## Local verification / ローカル検査

The following checks passed for both dev.2 and formalized source:
- verify_loud_preset.py: extracts actual profile/history/storage/import/UI code,
  compiles C++17 with -Wall -Wextra -Werror using SDK/Win32 adapters; tests preset
  values, old IDs, JP/EN names, same-name storage/backup round trips, invalid files,
  name rejection, current-preset indicator and combo item/index handling.
- verify_monitor_source.py: protected-source comparison against published bases;
  resource uniqueness, DSP v7, timer/playback flags, eight inputs and seven bars.
- verify_compare_hold.py: actual hold-button code with native-push-state mock.
- verify_monitor_minimize.py: actual owner/menu and lifecycle code with mocks.
- dev.2 versus formalized product sources must be byte-identical after version
  normalization; scripts/CMDs differ only by version/path references.
- Final ZIP entry count, CRC and every entry's bytes are compared with source.

The local environment has g++ but no MSBuild. Windows build/installation was
reported separately and recorded below. See RELEASE_CHECKLIST.txt.

## Formal build and received-package verification / 正式番号の確認

On 2026-10-16 JST, the user separately reported OK for the bundled Windows
build/package, overwrite installation/restart, Components version 1.14.0,
retained built-in Loud -9 and target -9.0 LUFS, normal tooltip rendering,
and short playback with live monitor updates.

The received .fb2k-component and checksum file were checked directly:

- Component: 210162 bytes; SHA-256
  `a7eb5b707ec4904aa535a6fdbd7f7b4890dcf36e95309ea7f921274e1d6bfe42`.
- Checksum file: 110 bytes; SHA-256
  `c54c36bd2d514dd722002108d6ca96c4de6d0278c26cf003af05b930c899cbc8`.
- Internal DLL: 429568 bytes; SHA-256
  `715a24bc6e0f12bbee77325b4dd3cefc6b2ae009a14989f29032d0dc289d91e3`.
- ZIP CRC passed; all 12 entries were unique with safe relative paths.
- DLL PE Machine 0x8664, PE32+ 0x20b, DLL flag present, exact ASCII
  null-terminated version 1.14.0 present, no 1.14.0-dev version string.
- Required DLL and nine bundled documents (including the license) were present; all nine
  documents matched the source byte-for-byte. Two SDK license notices were included.
- Checksum content named foo_r128_normalizer_v1.14.0.fb2k-component and matched
  its actual SHA-256. Use SHA256SUMS.txt as the public Release asset name.

This final source-ZIP update changes only README.md, RELEASE_NOTES.md,
BUILDING.md, RELEASE_CHECKLIST.txt, GITHUB_RELEASE_GUIDE.txt and this record.
Code, scripts/CMDs and the nine packaged documents are unchanged from the
tested source. The received binary and its checksum remain the release candidates;
no rebuild or repeated runtime test is needed for these source-only records.
No GitHub upload/publication is claimed. Update the sole Source of Truth after
the main/tag/Release/Assets check; the v1.13.0 document remains the published baseline.

## Earlier monitor results / 旧モニター確認

v1.13.0-dev.1 covered minimize ON/OFF, updates while minimized, saved settings,
manual close, independence from Always on Top and JP/EN/Automatic menus.
v1.12.0 covered the earlier 30-minute monitor playback test. Full historical
evidence and official v1.13.0 hashes are in the sole published R128 Source of Truth.
Those results are not new endurance or CPU measurements for v1.14.0.

## Rollback / 旧版への復帰

Formats and GUIDs are retained. v1.13.0 reads the numerical settings and user
presets, but treats the new -9 built-in/history ID 8 as Custom Settings.
Keep manual upload and one-step-at-a-time OK workflow. Update the R128 Source
of Truth only after checking the official main/tag/Release/Assets.
