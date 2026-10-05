# R128 Monitor — v1.12.0-dev.6

Development source only. This is not a stable release. The stable baseline
remains v1.11.0 / Source of Truth document revision 1.1.

開発・テスト用ソースです。正式版はv1.11.0、基準資料は文書版1.1のままです。
本ファイルは開発差分の記録であり、正式Source of Truthを置き換えません。

## v1.12.0 release preparation / v1.12.0正式版の準備

The working source is prepared for v1.12.0 from the tested dev.6 implementation.
The dev.6 results below describe the tested development binary; they do not
establish that the final v1.12.0 binary has been built or installed.
Changes for preparation are version text, distribution documentation, script
names/outputs and exact null-terminated DLL version checks. Audio calculations,
monitor telemetry, window layout and saved configuration are retained.
Final build/install checks are listed in RELEASE_CHECKLIST.txt.
Stable remains v1.11.0 until publication is explicitly directed and verified.

テスト済みdev.6からv1.12.0用ソースを準備しています。以下の実機結果は
dev.6の結果であり、正式版バイナリのビルド・導入完了を示すものではありません。
版数、配布文書、スクリプト名と生成ファイル名、DLL版数の完全一致検査を更新し、
音声計算・表示データ・画面配置・設定保存は引き継ぎます。
最終ビルド・導入確認はRELEASE_CHECKLIST.txtを参照してください。
公開指示と公開確認が完了するまで、正式基準はv1.11.0です。

## dev.6 validation record / dev.6検証記録 (2026-10-05 JST)

Status: release candidate for preparation, not a formal release. Stable remains
v1.11.0. The results below update earlier pending statements for the checks
explicitly listed here; they do not mark the entire Windows checklist complete.

正式リリースの準備候補です。正式版はv1.11.0のままです。
以下に記載した項目の未確認状態を更新しますが、実機確認予定の全項目を
合格とするものではありません。

### Runtime environment / 実機環境

- User-reported current player: foobar2000 v2.26, updated before this report.
- Installed component: foo_r128_normalizer v1.12.0-dev.6, built and installed
  by the user on Windows x64.
- One monitor; normal resolution 1920 x 1080; normal Windows scaling 100%.
- The exact player-update point in the earlier test sequence was not recorded.
  Only checks performed after the update are attributable to v2.26; older
  confirmations are not retroactively assigned a player version.

現在のfoobar2000はv2.26（本人申告）。画面は1台、通常1920 x 1080、
表示倍率100%です。差し替え時点と過去の各確認の対応は記録されていないため、
差し替え後の確認をv2.26での結果とし、過去の確認すべてをv2.26扱いにはしません。

### User-confirmed dev.6 results / dev.6実機確認結果

- Windows build and overwrite installation succeeded. Comparison-button
  behavior and the monitor's original-comparison display were confirmed.
- Saved position and close/reopen worked, including placement at the bottom
  right and recovery after reducing and restoring the screen resolution.
- Leaving the monitor open restored it at startup; manually closing it
  retained the closed state across restart.
- Always on Top ON/OFF worked; both states persisted across restart.
- Repeating the menu command did not create a second monitor window.
- Pause and stop cleared current values to unavailable and emptied the bars.
  Track change and seek reset the display and measurement resumed afterward.
- One minute of continuous use with the monitor open was reported stable.
  This is a short runtime check, not a long-duration stability result.

ビルド・上書き導入、比較ボタンと比較表示、位置保存・画面内復帰、起動時再表示、
閉じた状態の保存、最前面ON/OFFと再起動後の保持、多重起動防止を確認済みです。
一時停止・停止では値とバーがクリアされ、曲送り・シーク後はリセット後に
測定が再開しました。開いた状態での1分間の安定動作を確認しています。

### Subsequent monitor checks / モニターの追加確認

After the v2.26 environment report, Maximum confirmed each of the following
checks one at a time:

- All seven built-in presets: Natural -18, Power Boost -14, Relaxed -23,
  Night Safe -22, Modern Boost -9, 1-Band Adaptive -10 and 3-Band Adaptive -10.
  Each was applied during playback and observed for about 10 seconds with
  no reported interruption or abnormal sound and continued monitor updates.
- Modern Boost's Adaptive field was OFF as expected for fixed processing;
  the two Adaptive presets displayed a percentage.
- Saved the current 3-Band Adaptive settings as `モニター確認 dev6`, switched
  to Natural, then recalled/applied the user preset. Playback continued and
  the Adaptive percentage display returned.
- Exported that single preset to `.r128preset`, deleted only the test preset,
  imported it and confirmed its name was restored. After switching to Natural,
  recalling/applying the restored preset returned the Adaptive percentage.
  This verifies the selected-preset monitor workflow, not equality of every
  serialized setting or all import/export conflict paths.
- Maximum reported completion of 30 minutes of continuous playback with the
  monitor open, no abnormalities, and foobar2000 CPU usage of approximately 4%.
  Duration and CPU are user-reported observations, not an instrumented log.
  The 4% is the player process observation, not the monitor's incremental cost.

v2.26の環境申告後、内蔵7プリセットを1件ずつ適用し、約10秒の再生と
モニター更新に問題がないことを確認しました。固定モダンでは適応強度OFF、
1バンド・3バンドでは％表示を確認しています。
テスト用ユーザープリセットの保存・呼出、単体書き出し・削除・読み込み、
復元後の呼出でも適応強度の％表示が戻ることを確認しました。
全設定値の一致や同名処理等の全経路を検証したものではありません。

本人から「30分完了・異常なし・CPU約4％」の報告を受けました。
これは本人による観察結果であり、計測ログではありません。
CPU約4％はfoobar2000全体の観察値で、モニター単独の追加負荷ではありません。
今回の目的はモニターの確認です。既存機能全般への追加実機テストは広げません。

### Local source checks / ソース側の確認

- `tests/verify_monitor_source.py`: passed. Original CPP matched the v1.11.0
  tag after removing the explicitly named monitor/help/compare-button additions
  and version text. Configuration v7, resource IDs, timer, eight build inputs,
  seven bars and context menu checks passed.
- `tests/verify_compare_hold.py`: passed the native-button contract mock for
  mouse/Space hold, dragging, reentry, cancellation and destruction.
- `tests/monitor_meter_test.cpp`: compiled with C++17 and
  `-Wall -Wextra -Werror`; unit/range, gain sign/magnitude, clipping,
  unavailable values and monotonicity tests passed.
- `tests/monitor_telemetry_test.cpp`: compiled with the same options;
  closed-window demand, peak transfer, stale acknowledgement, reset, lifetime,
  capacity and coherent concurrent snapshot tests passed.
- Existing GUID preservation was checked at source level. No binary audio
  equivalence or Windows runtime result is inferred from these local tests.

ソース比較、設定形式v7・既存GUID・リソースの整合性、比較ボタンの模擬テスト、
メーターの単位・符号・範囲、表示データの並行読み取り等の検査は合格しました。
これらは出力音声の完全一致を証明するものではありません。

### Remaining scope / 未確認として残す範囲

- Multiple monitors and mixed-DPI setups.
- Runtime behavior on foobar2000 2.0 and other older player versions.
- Bit-for-bit output audio comparison against v1.11.0 with identical input
  and configuration, including concurrent conversion.
- Measurement accuracy against an independent reference for approximate
  True Peak, limiter attenuation and automatic-protection attenuation.
- Full setting-value regression, bulk import/export and conflict/error paths;
  only the seven-preset playback/monitor checks and selected-preset workflow
  above have been confirmed in dev.6.
- Continuous operation beyond the reported 30 minutes and quantified
  open/closed CPU overhead.

複数画面・混在DPI、旧foobar2000の実機動作、v1.11.0との出力完全一致、
独立した測定器との精度比較、全設定値・一括入出力・同名処理・エラー経路、
報告された30分を超える連続動作・開閉時の定量的なCPU負荷比較は未確認です。
True Peakは引き続き近似値です。正式タグ・Release・Assetsの更新は行っていません。

## dev.6 compare-button repair / 比較ボタンの押下検知修正

- User testing found that holding Compare neither changed the settings status
  nor put the monitor into original comparison. The legacy v1.11.0 handler
  depended on BN_HILITE / BN_UNHILITE, compatibility notifications for old Windows.
- Subclass only the existing native Compare button and read BM_GETSTATE /
  BST_PUSHED after native handling. Mouse hold and Space hold forward transitions
  to the existing comparison command handler. No owner drawing or audio change.
- Dragging out releases comparison; dragging back while captured starts it again.
  Capture loss, cancellation, focus loss and disable clear the native pressed
  state and release owned capture. Dialog deactivation cancels the button;
  destruction removes the subclass. Existing dialog-destruction reset remains.
- DSP GUIDs, configuration v7, presets, telemetry and monitor layout are unchanged.
- `tests/verify_compare_hold.py` compiles the actual subclass body against a
  native-button message contract mock and exercises mouse/Space, drag, reentry,
  cancellation, disabled input and destruction. This is not a Windows runtime test.
- Existing-source verification explicitly removes the named UI repair blocks
  and then compares all remaining original CPP against the v1.11.0 tag.
- Windows build and comparison audio/monitor behavior were pending at source
  archive preparation; subsequent confirmations are recorded above.

References: Microsoft Learn BN_HILITE, BM_GETSTATE and SetWindowSubclass.
https://learn.microsoft.com/en-us/windows/win32/controls/bn-hilite
https://learn.microsoft.com/en-us/windows/win32/controls/bm-getstate
https://learn.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-setwindowsubclass

### Subsequent dev.5 user results / dev.5の追加実機確認

Confirmed: build/install; all four Japanese/English monitor glossary entries
readable through the end; direct monitor-help entry selects the right item;
closing help preserves playback and live monitor updates; settings glossary
opens at the original first entry; Japanese/English/Automatic display; seek;
single window; topmost ON/OFF and persistence; saved position; startup restoration
when left open; closed state retained after manual close; light/dark transitions.

Failed: holding Compare did not change either the settings status or monitor.
No original-comparison pass is claimed. Other pending runtime checks remain open.

## dev.5 monitor help / R128専用解説

- Adds `MONITOR_GUIDE.md`, a bilingual guide specific to R128's seven values.
- Appends four matching entries to the existing Japanese/English glossary:
  values, units/bars, display states, and reading the monitor/DSP order.
- Right-click > How to read the monitor... / モニターの見方... opens the existing
  glossary at the first monitor entry. The settings glossary still starts at
  its original first entry. The glossary remains scrollable and follows the
  existing language/theme behavior.
- README.md and both packaged READMEs link or point to the guide. Both packaged
  glossary text files contain the same content; MONITOR_GUIDE.md is also packaged.
- No changes to telemetry, audio processing, bars, window dimensions or GUIDs.
- Sonic Refiner's specialized guide integration is a separate future step.

### Verified user results / 実機確認済み範囲

dev.4 build and overwrite installation passed. The user confirmed matching
window/bar widths, readable seven rows, updating values during playback,
pause clearing/resume, stop clearing/restart, and track-change updates.
Seek was requested but not confirmed. Topmost, restoration, language/theme/DPI,
comparison, source filtering, processing regressions and CPU checks remain pending.
dev.5 Windows build and the new help entry point were subsequently confirmed;
see the added dev.5 user results above.

## dev.4 matching bars / バーの位置・横幅統一

- All seven progress bars use x=99, width=49, height=8 dialog units, matching
  Sonic Refiner v0.9.0's four bars. Right edges remain at x=148.
- Numeric fields use x=51, width=44; labels remain x=8, width=42.
  A gap remains between labels, numbers and bars. Units are retained.
- Window width, font, height, seven values, meter ranges, UI behavior and saved
  state remain unchanged from dev.3. Only control rectangles change at runtime.
- dev.3 build and installation were reported successful. dev.4 build,
  installation and compact numeric-text fit were subsequently confirmed.
  This does not establish a full monitor runtime pass.

Sonic Refinerとバーの開始位置・横幅・高さを統一しました。
数値欄の幅を少し調整し、数値の単位と7項目を維持しています。

## dev.3 matching width / 横幅の統一

- Resource width is 156 dialog units, matching Sonic Refiner v0.9.0's monitor.
- Font is the same Yu Gothic UI 9 pt / weight 400 / SHIFTJIS charset.
- Height remains 163 dialog units to retain all seven rows and the output note.
- Labels are shortened in both languages. Numeric values, units, bar scales,
  saved UI state and runtime behavior are unchanged from dev.2.
- Identical width is intended at the same DPI. Windows font rendering and
  label/value fit still require the user's runtime check.

| Japanese label | English label | Value |
|---|---|---|
| 入力 3秒 | Input 3 s | Input Short-term loudness, LUFS |
| ゲイン | Gain | Current normalization gain, signed dB |
| 出力 3秒 | Output 3 s | Output Short-term loudness, LUFS |
| True Peak | True Peak | Approximate output peak, dBTP |
| リミッター | Limiter | Positive limiter attenuation, dB |
| 自動保護 | Protect. | Positive auto-protection attenuation, dB |
| 適応強度 | Adaptive | Effective Adaptive strength, % / OFF |

Sonic Refinerと同じ横幅・フォントへ統一しました。同じ表示倍率で横幅を揃える
設計です。7項目のため高さは維持し、数値・単位を残して項目名を短くしました。

## dev.2 UI alignment / 画面・操作の統一

Reference: Sonic Refiner formal v0.9.0 source and
foo_sonic_refiner_project_source_of_truth_2026-10-05_v0.9.0_FINAL.md.

- Seven rows of label / value / horizontal bar, in a fixed compact tool window.
- Right-click (or keyboard context menu) toggles Always on Top, checked when on.
  The visible checkbox from dev.1 is removed; its saved GUID remains unchanged.
- Playback menu: R128 Processing Monitor / R128 補正モニター.
- First opening is placed beside the main window; subsequent openings restore
  the saved position. Position validation, work-area clamp and DPI scaling remain.
- Basic status wording follows Active / Waiting / Paused / Analyzing, with
  additional R128-specific explanations for source selection and original comparison.
- Unavailable values clear their bars. Paused values remain hidden instead of
  presenting a last sample as current. Original comparison shows only input.
- R128 retains its seven measured items and 2.1+ playback-only source guard.
  Sonic Refiner's four values and its telemetry implementation are not copied.
- The display-only meter header is the eighth copied build input.

Sonic Refiner v0.9.0に合わせ、項目名・数値・横バーの7行構成、固定小窓、
右クリックの「常に手前に表示／Always on Top」、基本状態表記を統一しました。
保存済みの位置・開閉・最前面設定はdev.1から引き継ぎます。

### Bar ranges / 横バーの目盛り

| Value | Bar scale | Meaning |
|---|---|---|
| Input / output Short-term | -60 to 0 LUFS | Current measured loudness |
| Normalization gain | Absolute magnitude 0 to 24 dB | Direction is shown by the numeric +/- sign |
| Output True Peak | -60 to +3 dBTP | Approximate peak at this DSP's output |
| Limiter / automatic protection | 0 to 12 dB | Positive attenuation amount |
| Adaptive strength | 0 to 100% | Effective strength; zero bar when OFF |

Bars clamp at their endpoints. Numeric values retain the actual measurement,
including values beyond a bar endpoint. No bar scale changes processing limits.
数値は実測値です。ゲインバーは補正量の大きさを表し、増幅／減衰の方向は
数値の＋／－で確認します。バーの範囲外でも数値はそのまま表示します。

dev.1, dev.2 and dev.3 builds and installations were reported successful by the
user. The requests after opening dev.2 and dev.3 concerned monitor and bar width.
Full telemetry, theme, right-click and persistence testing is still pending.
dev.4 build and compact numeric-text fit were subsequently confirmed; see above.

## Approved behavior / 承認済み仕様

- Playback > R128 Processing Monitor / R128 補正モニター
- A small independent, modeless window; one window per foobar2000 process.
- About 100 ms UI refresh, only while the window exists.
- Input/output Short-term (3-second) loudness in LUFS.
- Current normalization gain, not a claimed sum of all nonlinear processing.
- Output True Peak (4x / 17-tap approximation), collected from already-measured
  rendered frames at this DSP's output, not the final player output.
- Separate positive limiter and automatic-protection reductions in dB.
- Adaptive effective strength in percent; OFF when disabled.
- Position, visibility and Always on Top retained separately from DSP format v7.
- Always on Top defaults OFF. A manual close clears saved visibility; quitting
  foobar2000 with the monitor open preserves it for the next startup.
- Follow existing Automatic (Windows) / Japanese / English setting and the
  foobar2000 light/dark theme. No separate monitor language setting.
- Off-screen positions are clamped to the current monitor work area.
- No current correction values while stopped, paused, comparing original, or
  with an absent / multiple / ambiguous DSP source.
- Input remains visible during original comparison. Output measurement resumes
  when the normal processed branch is being emitted again.

## Compatibility / 対応条件

Monitor: foobar2000 2.1 or newer, Windows x64.
Existing audio processing: foobar2000 2.0 compatibility retained.

SDK 2025-03-07 dsp_entry_v4 supplies playback/conversion instantiation flags.
Only explicitly classified playback instances publish monitor telemetry.
Conversion and legacy/unclassified instances never publish monitor values.
On 2.0, the window explains the 2.1 requirement instead of showing uncertain data.

The existing settings popup falls back to the original v2 popup. DSP GUID,
all existing setting/menu GUIDs, format v7, old format readers, seven built-in
preset values, user presets, import/export, and audio calculations are retained.
Existing diagnostics, history and trend telemetry have not been replaced.

## Display data transfer / 表示専用データ

- source/r128_monitor_state.h: fixed 16-slot registry for playback publishers,
  per-instance lifetime, atomic fields and sequence-checked UI snapshots.
- source/r128_monitor_ui.h: Win32 modeless UI, timer, lifecycle and saved state.
- source/r128_monitor_meter.h: UI-only numeric-to-bar mapping.
- UI retries a snapshot at most three times; audio never waits for UI.
- All payload accesses are atomic. Lock-free 64-bit/double atomics are required
  by static assertions on x64.
- Audio publishes once per chunk and accumulates peaks from the existing output
  True Peak calculation. It does not recalculate loudness or alter audio samples.
- Publication and peak capture are disabled while the monitor is closed.
  Display epochs reject values from before reopening, resume or comparison
  transitions, instead of showing a whole-track peak as a current interval.
- UI acknowledges the exact snapshot it read. A stale acknowledgement cannot
  clear a newer unseen peak; in that race, the peak is conservatively retained.
- Displayed peak interval is approximately the UI interval, but can be longer
  under scheduling delays, output buffering or acknowledgement races.
- Stale data expires after 1.5 seconds. Playback state checks hide stopped or
  paused values immediately on the next UI tick. DSP chain checks are UI-only.
- Loudness startup needs approximately 3 seconds of measured audio. Original
  comparison holds normal-output measurement; it is not presented as current.
- Rendering, SDK chain inspection, text formatting, config saving and window
  operations run only on the main UI thread.
- Added display work is small but not literally zero CPU cost. Windows testing
  must verify CPU, playback continuity, and monitor open/closed behavior.

## New IDs / 追加識別子

Existing GUIDs are unchanged. New, independent monitor GUIDs:

| Purpose | GUID |
|---|---|
| Playback command | 6B40FE4D-9F8A-45C9-B89F-62A637A5E37B |
| Saved visibility | B25B9EF0-6316-4BBD-8C91-55706DADFA4E |
| Always on Top | 9FB00C64-5A41-405D-9440-A3BEBDF09A91 |
| Saved position | C39050CA-89A1-49BB-A9B2-6A6DC7BECAF9 |

Dialog ID 109; control IDs 1300–1336 (only defined IDs are used).
1321 is retained as a reserved, unused former checkbox ID.
Build script copies three headers along with the original five source files.

## Build / ビルド

Use the existing SDK project. This source archive is not a standalone .vcxproj.

`v1.12.0-dev.6へ更新・ビルド・梱包.cmd`

Expected output: `foo_r128_normalizer_v1.12.0-dev.6.fb2k-component`

Build backup suffix: `.before_v1.12.0-dev.6`. Existing backups are not overwritten.
On failure, report `build_errors.txt`; if needed, `build_full_log.txt`.
This managed environment cannot run Visual Studio/MSBuild or foobar2000.
Local static checks and portable telemetry tests do NOT constitute a Windows
build, UI/DPI test, or proof of binary audio equivalence.

Portable tests: `tests/monitor_telemetry_test.cpp` builds with C++17 and tests
coherent concurrent snapshots, classified/unclassified publishers, stale peak
acknowledgements, closed-window demand, display epochs, reset, slot lifetime and
capacity. AddressSanitizer/UndefinedBehaviorSanitizer checks pass with leak
detection disabled because this environment cannot inspect threads under ptrace.
No leak-check success is claimed.

`tests/verify_monitor_source.py` compares the original CPP against the v1.11.0
Git tag after removing only the named display hooks and version strings. It
also checks resource ID uniqueness, format v7, the timer, eight build inputs,
seven bars and the right-click menu. `tests/monitor_meter_test.cpp` checks unit
ranges, gain magnitude, clipping and unavailable/non-finite values.
This comparison requires a Git checkout containing the v1.11.0 tag; the plain
source ZIP does not contain Git metadata. These tests are development checks,
not a substitute for the Windows checklist below.

## Windows test checklist / 実機確認予定

Follow the one-step-at-a-time workflow and wait for Maximum's OK at every step.

1. Release x64 build, dev version in Components, overwrite installation.
2. Playback menu, seven label/value/bar rows, values after 3 seconds, gain and reductions.
3. Repeat menu command, close/reopen, keyboard navigation, ESC.
4. Position and open-state restoration after restart; closed-state persistence.
5. Right-click / keyboard context menu, Always on Top OFF/ON and persistence;
   monitor help opens at the first of four entries, all entries readable,
   closing help returns to the monitor, and settings glossary starts normally.
6. Japanese/English/Automatic switching while the monitor stays open.
7. Light/dark switching while open, 100–250% DPI and mixed-DPI monitors.
8. Monitor removal / disconnected screen / saved off-screen position recovery.
9. Stop, pause, resume, seek, track change, measurement reset and silence.
10. DSP absent, multiple instances, removal/re-addition and settings changes.
11. Original comparison and loudness-matched comparison, including transitions.
12. Concurrent conversion using this DSP: no conversion values in monitor.
13. All seven built-in presets, user presets, .r128preset round trip, diagnostics,
    automatic-control history and trend graph regression.
14. CPU and continuous playback with monitor open versus closed; compare audio
    output against v1.11.0 using the same configuration and input.
15. foobar2000 2.0: audio and original settings work, monitor requirement shown.

This is the historical full development checklist, not a claim that every item
passed. The monitor-focused confirmed scope and residual limitations are recorded
above. For release preparation, use RELEASE_CHECKLIST.txt; do not create a stable
tag or publish Assets until final artifact checks and Maximum's instructions.
