# v1.11.0 — ユーザープリセットの書き出し／読み込み

**R128 Real-time Loudness Normalizer** (`foo_r128_normalizer`) は、
foobar2000 2.x（Windows x64）向けのR128ベース・リアルタイム音量ノーマライザーです。

## ダウンロード

一般利用者は、次のファイルをダウンロードしてください。

`foo_r128_normalizer_v1.11.0.fb2k-component`

Visual Studioやfoobar2000 SDKは必要ありません。

## 変更内容

- 選択したユーザープリセット1件を `.r128preset` ファイルへ書き出し
- 最大100件の全ユーザープリセットを1ファイルへ一括書き出し
- `.r128preset` ファイルからユーザープリセットを読み込み
- 同名プリセットは［上書き］［別名保存］［スキップ］から選択
- 読み込み前にファイル全体、件数、名前、全設定値を検証
- 不正・破損ファイルでは既存ユーザープリセットを変更しない
- 書き出しファイルにはプリセット名と設定値だけを収録
- 曲名、アーティスト名、音声、ファイルパス、Windowsユーザー名、PC名、IPアドレスは収録しない
- 公開上の正式名称を **R128 Real-time Loudness Normalizer** へ統一
- 日本語表記は **R128 リアルタイム音量ノーマライザー** を維持

## 互換性

- v1.10.0以前のDSP設定とユーザープリセットをそのまま引き継げます
- 音声処理、既存7プリセット値、判定基準に変更はありません
- コンポーネントGUID、メニューGUID、設定形式v7に変更はありません
- リポジトリ名、DLL名、配布ファイルの接頭辞は `foo_r128_normalizer` のままです
- 現在のDSP登録を解除せず、そのまま上書き更新できます

## 動作環境

- foobar2000 2.x
- Windows x64

## English

**R128 Real-time Loudness Normalizer** (`foo_r128_normalizer`) v1.11.0 adds
privacy-safe export and import for user presets.

### Download

Download the following file:

`foo_r128_normalizer_v1.11.0.fb2k-component`

Visual Studio and the foobar2000 SDK are not required.

### Changes

- Export the selected user preset to a `.r128preset` file
- Export all user presets, up to 100, to one file
- Import user presets from a `.r128preset` file
- Choose Overwrite, Save As, or Skip for duplicate names
- Validate the complete file, count, names, and settings before import
- Leave existing presets unchanged when a file is invalid or damaged
- Include only user-preset names and settings in exported files
- Never include track metadata, audio, file paths, Windows user names, PC names, or IP addresses
- Use **R128 Real-time Loudness Normalizer** as the public English name

### Compatibility

- Existing DSP settings and user presets from v1.10.0 and earlier are preserved
- No changes to audio processing, built-in preset values, or control criteria
- No changes to the component GUID, menu GUID, or configuration format v7
- Repository, DLL, and package filename prefixes remain `foo_r128_normalizer`
- The component can be updated without removing it from the current DSP chain

### System requirements

- foobar2000 2.x
- Windows x64
