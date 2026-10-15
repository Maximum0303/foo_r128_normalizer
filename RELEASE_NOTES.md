# v1.13.0 — Minimize with foobar2000 / foobar2000と一緒に最小化

## English

### Changes

- Adds **Minimize with foobar2000** to the Processing Monitor right-click menu, matching Sonic Refiner v1.1.0.
- The option defaults to **ON**.
- ON: minimizing foobar2000 hides the still-open monitor; restoring foobar2000 brings it back.
- OFF: the monitor stays visible and its live values continue updating about every 100 ms during playback.
- Saves the option across restarts, independently of **Always on Top**.
- A manually closed monitor is not reopened by main-window minimize/restore.
- Supports Japanese / English / Automatic (Windows).
- Updates the built-in monitor help, packaged glossaries and `MONITOR_GUIDE.md`.

### Download

`foo_r128_normalizer_v1.13.0.fb2k-component`

Visual Studio and the foobar2000 SDK are not required for normal use.

### Compatibility and verification scope

- UI/window-behavior change only. Existing audio calculations, measurements, seven built-in presets, DSP format v7, existing GUIDs and user-preset formats are retained.
- The new preference is stored separately from DSP presets; existing monitor position, visibility and topmost settings are preserved.
- The v1.13.0-dev.1 Windows build was checked for default ON, ON/OFF minimize behavior, live updates with the option OFF, saved ON/OFF settings, manual-close behavior, Always on Top independence, localized menus and the Japanese built-in help.
- The final v1.13.0 build and packaging completed on Windows. Overwrite installation, restart, the Components version display and live monitor updates while foobar2000 is minimized with the option OFF were confirmed.
- Monitor: foobar2000 2.1+ / Windows x64. Existing audio processing retains 2.0 compatibility; runtime behavior on older players has not been verified.
- Multiple monitors/mixed DPI, all window-state combinations, a new 30-minute endurance run and a full legacy-feature regression were not part of this change's targeted checks.
- True Peak is approximate. Independent-meter accuracy and bit-for-bit output equivalence are unverified.

## 日本語

### 変更内容

- Sonic Refiner v1.1.0と同じ［foobar2000と一緒に最小化］を補正モニターの右クリックへ追加。
- 初期値はオン。
- オン：本体の最小化で開いたままのモニターも消え、本体の復元で戻る。
- オフ：本体最小化中もモニターを表示し、再生中の値を約100ms更新。
- 設定を再起動後も保存。［常に手前に表示］とは独立。
- 手動で閉じたモニターを本体の最小化・復元で勝手に再表示しない。
- 日本語／English／Automatic (Windows)に対応。
- 内蔵解説、配布用用語集、`MONITOR_GUIDE.md`を更新。

### ダウンロード

`foo_r128_normalizer_v1.13.0.fb2k-component`

一般利用ではVisual Studioやfoobar2000 SDKは不要。

### 互換性と確認範囲

- UI／ウィンドウ動作のみの変更。音声計算、測定、既存7プリセット、DSP設定形式v7、既存GUID、ユーザープリセット形式は維持。
- 新設定はDSPプリセットとは別に保存。既存のモニター位置・開閉・最前面設定を引き継ぐ。
- v1.13.0-dev.1のWindowsビルドで、初期値オン、最小化連動オン／オフ、オフ時の更新、オン／オフ設定保存、手動終了、最前面設定との独立性、言語メニュー、日本語の内蔵解説を確認。
- 正式版v1.13.0のWindowsビルド・梱包が完了。上書き導入・再起動、Componentsの版数表示、最小化連動オフで本体を最小化した際のモニター表示・更新を確認。
- モニターはfoobar2000 2.1以降／Windows x64対応。既存音声処理は2.0対応を維持。旧バージョンでの実機動作は未確認。
- 複数画面・混在DPI、全ウィンドウ状態の組合せ、新版での30分耐久、全既存機能の回帰は今回の限定した確認の対象外。
- True Peakは近似。独立した測定器との精度比較、音声出力のビット単位一致は未確認。
