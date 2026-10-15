# R128 Monitor — v1.13.0 preparation / 正式版の準備

## Current status / 現在の状態

The v1.13.0 source follows the tested v1.13.0-dev.1 implementation.
The final Windows build, packaging, installation and targeted monitor check
were confirmed on 2026-10-16 JST. Publication is pending.
Until publication is directed and verified, the sole stable baseline remains
R128 v1.12.0 / Source of Truth document revision 1.1 (updated 2026-10-06).
Sonic Refiner v1.1.0 is a reference for monitor window behavior only.

テスト済みv1.13.0-dev.1を正式版数へ更新したソース。
正式版のWindowsビルド・梱包・導入と限定したモニター確認は2026-10-16 JSTに完了。
公開は未完了。
公開確認までは、R128 v1.12.0専用Source of Truth文書版1.1を唯一の正式基準とする。
Sonic Refinerの資料をR128の設定・音声処理の基準へ置き換えない。

## Source and changes / ソースと変更範囲

- Base main: a85c1976dea5a07cc290688b7466f8a5a51bc349.
- Formal v1.12.0 tag: 8c45a9228fafd551201a4f297c1f7b6fb5b3a436.
- New separate cfg_bool: F3D5A340-8A5E-45DE-9F69-A0961D6D4F1D, default true.
- Match Sonic Refiner v1.1.0's GWLP_HWNDPARENT owner selection on creation
  and option change; reapply the separately stored Always on Top setting.
- Retain the 100 ms refresh, audio telemetry, manual close and startup lifecycle.
- Preserve all existing GUIDs, DSP format v7/readers, seven presets and audio math.
- Formal preparation changes only version text, documentation, script/output
  names, and the SDK default from C: to the confirmed F: installation.
- Monitor UI/state/meter headers and dialog resources are identical to tested dev.1.
- Exact null-terminated DLL version checks reject stale development binaries.

## Windows development-build results / dev.1実機結果 (2026-10-16 JST)

These are the specific v1.13.0-dev.1 checks confirmed in the development session,
not proof of a final v1.13.0 binary or completion of every checklist entry.

- Release/x64 build and packaging succeeded after restoring Visual Studio 2022
  Build Tools with Desktop development with C++ and x86/x64 v143 ATL.
- Actual SDK: F:\foobar2000-dev\SDK-2025-03-07.
  Dev.1 used -SdkRoot explicitly because its default was C:.
- Overwrite installation and foobar2000 restart succeeded.
- Japanese menu item present, initially checked (ON).
- ON: main-window minimize hides the monitor; restore brings it back.
- OFF: monitor remains visible with numeric/bar updates while playback continues.
- OFF persists across restart; a monitor left open returns at startup.
- ON also persists across restart.
- After X closes the monitor, main-window minimize/restore does not reopen it.
- OFF with Always on Top ON and OFF: monitor remains visible and updates.
- English menu displays Minimize with foobar2000.
- Automatic (Windows) returns to Japanese in the tested environment.
- Japanese built-in monitor reading/DSP-order help describes the ON/OFF option.

Windows結果は開発セッションで確認された操作結果。
プレイヤー版数・解像度・倍率のこの試験時点での再申告は取得していないため、
過去の環境情報を新しい試験へ無条件に割り当てない。
過去のv1.12.0の30分試験とCPU約4%は、新バイナリの試験結果ではない。

## Final Windows results / 正式版実機結果 (2026-10-16 JST)

- The bundled v1.13.0 build/package CMD completed successfully.
- Overwrite installation and foobar2000 restart completed.
- Components displays version 1.13.0.
- With playback active and Minimize with foobar2000 OFF, minimizing the main
  window leaves the monitor visible with continuing numeric/bar updates.

上記は各工程への「OK」で確認した正式版の操作結果。
ビルドログ、配布物の実バイト、SHA256SUMS.txtはこの時点で未受領。
ログの個別項目や配布物ハッシュを独立に照合したとの意味ではない。
開発版で確認済みの全項目を正式版で再実施したとの意味でもない。

## Local verification / ローカル検証

- verify_monitor_source.py: original audio code matches after excluding the
  existing named monitor/help/compare hooks and version text.
- verify_compare_hold.py: actual native-button subclass body with an API contract mock.
- verify_monitor_minimize.py: actual owner/menu functions compiled with C++17,
  -Wall -Wextra -Werror. Covers default ON, routing, owner selection, independent
  topmost settings, Japanese/English labels, checked states, cancellation and help.
- Direct dev.1/formal comparison protects monitor code, telemetry, resources,
  existing GUIDs, presets and DSP settings from unrelated changes.
- Script changes are limited to version/path/name updates; UTF-8 BOM/CRLF retained.

The mocks do not emulate Windows minimize/restore or prove persisted SDK settings.
Local verification does not replace the Windows results above. MSBuild is not
available in the agent environment; the final binary was built on Windows.

## Remaining scope / 未確認の範囲

- Independent inspection of final build logs, package bytes and SHA256SUMS.txt.
- All topmost/minimize combinations, closed-state startup and English help viewing.
- New endurance testing, mixed DPI/multiple monitors and older foobar2000 versions.
- Full legacy-feature regression, independent-meter precision and bit-for-bit audio comparison.

Use RELEASE_CHECKLIST.txt for the remaining release checks. Do not repeat every old
feature test for this UI-only change without a specific reason. Publish only
following the separate publication instruction, then create the new formal SoT.
