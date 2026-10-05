# v1.12.0 — R128 Processing Monitor / R128 補正モニター

## English

**R128 Real-time Loudness Normalizer** adds a small independent processing
monitor opened from **Playback > R128 Processing Monitor**.

### Download

`foo_r128_normalizer_v1.12.0.fb2k-component`

Visual Studio and the foobar2000 SDK are not required for normal use.

### Changes

- Seven values with numeric units and bars, refreshed about every 100 ms:
  input/output 3-second loudness, normalization gain, approximate output
  True Peak, limiter attenuation, automatic-protection attenuation and
  effective Adaptive strength.
- Match Sonic Refiner's monitor width, font, bar position and bar width.
- Save position, open/closed state and Always on Top; restore at startup
  when the monitor was left open at exit. Prevent duplicate windows.
- Follow Automatic (Windows), Japanese and English display settings and
  foobar2000's light/dark theme.
- Clear unavailable values while paused/stopped or without a unique playback
  DSP source. Original comparison displays input only. Conversion DSP values
  are excluded.
- Right-click **How to read the monitor...** opens the R128-specific guide
  in the built-in glossary. Include the bilingual guide and glossary text
  in the component package.
- Repair the native Compare (hold) button's mouse/Space hold detection and
  cancellation handling.

### Compatibility and scope

- Monitor: foobar2000 2.1+ / Windows x64. Existing audio processing retains
  2.0 compatibility; runtime behavior on older players has not been verified.
- Existing audio calculations, seven built-in presets, DSP format v7,
  existing GUIDs and user-preset format are retained.
- Monitor implementation tested on foobar2000 v2.26, one 1920 x 1080 display
  at 100% scaling. The user reported 30 minutes of playback without
  abnormalities and approximately 4% CPU for the entire player process.
- Multiple monitors/mixed DPI, accuracy against an independent meter and
  bit-for-bit output equivalence against v1.11.0 remain unverified.
- True Peak is approximate and describes this DSP's output, excluding later
  DSP processing and player volume. The monitor does not change audio settings.

## 日本語

**R128 リアルタイム音量ノーマライザー**に、Playbackメニューから開ける
独立した小型の補正モニターを追加しました。

### ダウンロード

`foo_r128_normalizer_v1.12.0.fb2k-component`

一般利用ではVisual Studioやfoobar2000 SDKは必要ありません。

### 変更内容

- 入力／出力の3秒ラウドネス、補正ゲイン、出力True Peak近似値、リミッター減衰、
  自動保護減衰、Adaptive実効強度の7項目を、単位付き数値と横バーで約100ms更新。
- Sonic Refiner補正モニターと横幅・フォント・バーの位置と横幅を統一。
- 位置・開閉状態・常に手前に表示を保存。開いたまま終了した場合は起動時に再表示。
  同じモニターの多重起動を防止。
- 自動（Windows）／日本語／Englishと、ライト／ダーク表示へ追従。
- 一時停止・停止や一意の再生用DSPがない場合は値をクリア。原音比較中は入力のみ表示。
  変換用DSPの値は表示しない。
- 右クリック→［モニターの見方...］からR128専用の用語集解説を表示。
  英日の専用解説と用語集テキストを配布パッケージへ収録。
- ［比較（押している間）］のマウス／Spaceによる押下検知と取消処理を修正。

### 互換性と確認範囲

- モニターはfoobar2000 2.1以降／Windows x64対応。従来の音声処理は2.0対応を維持。
  旧バージョンでの実機動作は未確認。
- 音声計算、既存7プリセット、DSP設定形式v7、既存GUID、ユーザープリセット形式は維持。
- モニター実装はfoobar2000 v2.26、1画面、1920 x 1080、表示倍率100%で確認。
  本人から30分の連続再生は異常なし、foobar2000全体のCPU使用率は約4%との報告。
- 複数画面・混在DPI、独立した測定器との精度比較、v1.11.0との出力完全一致は未確認。
- True PeakはこのDSPの出力に対する近似値。後段DSPやプレーヤー音量は含まない。
  モニターを開閉しても音声設定は変更しない。
