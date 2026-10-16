# v1.14.0 — Loud -9 / ラウド -9

## English

**R128 Real-time Loudness Normalizer** adds **Loud -9** as the fifth standard
normalization preset. There are now eight built-in presets: five standard and
three additional-processing presets.

### Download

`foo_r128_normalizer_v1.14.0.fb2k-component`

Visual Studio and the foobar2000 SDK are not required for normal use.

### Loud -9

- Based on Power Boost, with only its target changed from -14 to **-9 LUFS**.
- Retains maximum boost 8 dB, maximum attenuation 18 dB, True Peak limit -1 dBTP,
  look-ahead 5 ms and limiter release 120 ms.
- Modern Processing, 1-Band Adaptive and 3-Band Adaptive remain off.
- Can be placed after Sonic Refiner to adjust playback loudness.
- Source audio and protection may prevent reaching the target. Lower the target
  if peak reduction remains strong.
- Includes Japanese/English labels, concise tooltips, context help and glossary.

### Compatibility and checks

- Original seven presets, audio calculations, measurements, GUIDs and DSP format v7 are retained.
- Existing same-name user presets and backups keep their names and settings.
  User presets have a display-only `(User)` suffix; saved/exported names are unchanged.
  New Save As/Rename names must differ from built-in names.
- History IDs 0–7 and preset storage/backup formats are retained; Loud -9 uses new ID 8.
- The v1.14.0-dev.2 Windows build passed targeted checks for preset values,
  extra-processing-off state, Apply/reopen/restart persistence, existing same-name
  user-preset recall, Japanese/English tooltips and glossary, and short playback
  with monitor updates.
- The 1.14.0 build was then checked for installation/version, retained Loud settings,
  normal tooltips and short playback with monitor updates. The received package passed
  CRC, x64/version, SHA-256 and bundled-document checks.
- Local source comparison and extracted-code tests checked the original presets,
  protected processing/storage code and same-name backup compatibility.
- No new 30-minute endurance or CPU test was performed. These checks do not
  certify every theme, DPI, monitor configuration or foobar2000 version.

### Requirements

- Windows x64; foobar2000 2.x for audio processing.
- foobar2000 2.1 or later for the independent processing monitor.

## 日本語

**R128 リアルタイム音量ノーマライザー**に、標準ノーマライズの5種目として
**「ラウド -9」**を追加。標準5種＋追加処理3種の計8内蔵プリセットとなる。

### ダウンロード

`foo_r128_normalizer_v1.14.0.fb2k-component`

一般利用ではVisual Studioやfoobar2000 SDKは不要。

### ラウド -9

- パワーブーストから目標だけを−14から**−9 LUFS**へ変更。
- 最大増幅8 dB、最大減衰18 dB、True Peak上限−1 dBTP、先読み5 ms、
  リミッター解放120 msを維持。
- モダン処理・1バンド／3バンドAdaptiveはオフ。
- Sonic Refinerの後段でも再生音量の調整に使用できる。
- 音源や保護動作で目標に達しない場合がある。ピーク抑制が強く続く場合は目標を下げる。
- 日英のラベル、簡潔な吹き出し、項目ヘルプ、用語集を収録。

### 互換性と確認範囲

- 既存7プリセット値、音声計算、測定、GUID、DSP設定形式v7を維持。
- 既存の同名ユーザープリセットとバックアップの名前・設定を維持。
  ユーザープリセットは画面上だけ「（ユーザー）」を付け、保存・書き出し名は変更しない。
  新規保存・別名変更では内蔵名以外を使用する。
- 既存履歴ID 0～7とプリセット保存形式を維持。ラウド -9は新番号8を使用。
- v1.14.0-dev.2のWindowsビルドで、設定値、追加処理オフ、適用後の再表示・再起動保持、
  既存の同名ユーザープリセット呼出、日英の吹き出し・用語集、短時間再生とモニター更新を確認。
- 正式番号1.14.0でも導入・版数・ラウド設定保持・吹き出し・短時間再生・モニター更新を確認。
  受領配布物のCRC・x64 DLL・版数・SHA-256・同梱文書を検査。
- ローカルのソース比較と実コード抽出テストで、旧プリセット・保護対象処理・保存処理・
  同名バックアップの互換性を検査。
- 今回は30分の耐久試験・CPU測定を再実施していない。
  全テーマ・DPI・画面構成・foobar2000バージョンの動作を保証する試験ではない。

### 動作環境

- Windows x64。音声処理はfoobar2000 2.xに対応。
- 独立補正モニターはfoobar2000 2.1以降に対応。
