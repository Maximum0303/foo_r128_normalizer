# v1.6.0 — 日本語／English表示対応

## 変更内容

- 表示言語に「自動（Windows）／日本語／English」を追加
- 設定画面を開いたまま表示言語を即時切り替え
- 設定画面のタブ、主要項目、プリセット、操作ボタンを英語化
- 診断の主要見出しと現在の処理状態を英語化
- PlaybackメニューとDSP名を表示言語に合わせて切り替え
- 言語設定をDSPプリセットとは別のコンポーネント設定として保存
- 診断の動的表示と診断コピーを英語化
- 項目ヘルプと全31項目の用語集を英語化
- プリセット／各設定／診断値のツールチップを英語化
- ライセンス、初期設定確認、入力値エラー、確認メッセージを英語化
- 言語を即時変更した場合にツールチップも同じ言語で再生成
- 用語集上部の案内文を含め、補助画面内の固定表示も英語化

## 互換性

音声処理、設定形式v7、DSP GUID、メニューGUID、プリセット設定値、
DLL名は変更していません。v1.5.4のリアルタイム自動制御も維持しています。

## 動作環境

- foobar2000 2.x
- Windows x64

## English

v1.6.0 adds Automatic (Windows), Japanese, and English display-language
selection. The settings UI, diagnostics, item help, glossary, tooltips,
messages, license information, DSP name, and Playback menu now follow the
selected language immediately.

Language selection is stored separately from DSP presets. Preset format v7,
existing DSP and menu GUIDs, preset values, audio processing, and the v1.5.4
real-time automatic safety-control behavior remain unchanged.
