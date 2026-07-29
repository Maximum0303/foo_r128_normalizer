# v1.10.0 — ユーザープリセット対応

foobar2000 2.x（Windows x64）向けの、R128ベース・リアルタイム音量ノーマライザーです。

## ダウンロード

一般利用者は、次のファイルをダウンロードしてください。

`foo_r128_normalizer_v1.10.0.fb2k-component`

Visual Studioやfoobar2000 SDKは必要ありません。

## 変更内容

- 現在の基本設定と追加処理設定を任意の名前で保存できるユーザープリセットを追加
- ユーザープリセットの呼出、上書き、名前変更、削除に対応
- 日本語・英語を含む1～64文字の名前に対応
- 最大100件まで保存
- 既存7プリセットは変更・削除不可のまま維持
- 既存プリセットを基に調整した設定を別名保存可能
- ユーザープリセットはfoobar2000の設定内だけに保存し、外部送信しない
- 日本語／英語表示、ライト／ダーク表示に対応

## 互換性

- v1.9.0以前の設定をそのまま引き継げます
- 音声処理、既存プリセット値、判定基準に変更はありません
- コンポーネントGUID、メニューGUID、設定形式v7に変更はありません
- 現在のDSP登録を解除せず、そのまま上書き更新できます

## 動作環境

- foobar2000 2.x
- Windows x64

## English

foo_r128_normalizer v1.10.0 adds user presets for all Basic and Processing
settings.

### Download

Download the following file:

`foo_r128_normalizer_v1.10.0.fb2k-component`

Visual Studio and the foobar2000 SDK are not required.

### Changes

- Save all current settings under a custom name
- Load, overwrite, rename, and delete user presets
- Supports Japanese and English names from 1 to 64 characters
- Stores up to 100 user presets
- Keeps all seven built-in presets read-only
- Allows a modified built-in preset to be saved separately
- Stores user presets only in the foobar2000 configuration
- Supports Japanese and English UI, light mode, and dark mode

### Compatibility

- Existing settings from v1.9.0 and earlier are preserved
- No changes to audio processing, built-in preset values, or control criteria
- No changes to the component GUID, menu GUID, or configuration format v7
- The component can be updated without removing it from the current DSP chain

### System requirements

- foobar2000 2.x
- Windows x64
