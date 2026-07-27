# v1.6.1 — 表示品質と安定性の改善

## 変更内容

- 英語表示で処理強度の範囲が日本語記号のまま残る問題を修正
- Windows表示倍率に合わせてツールチップの最大幅を調整
- DPI変更時にツールチップ幅を再計算
- 200～250%表示で設定画面が作業領域を超える場合、画面内へ収めて
  必要な方向だけスクロールできるように改善
- 設定画面を開いたままfoobar2000のテーマを変更した場合も、
  ツールチップのライト／ダーク配色を更新
- 100～250%表示倍率、マルチモニター、曲変更・停止・終了時の
  品質確認項目を追加

## 互換性

音声処理、設定形式v7、DSP GUID、メニューGUID、プリセット設定値、
DLL名、v1.6.0の日本語／English表示、v1.5.4のリアルタイム自動制御は
変更していません。

## 動作環境

- foobar2000 2.x
- Windows x64

## English

v1.6.1 keeps the settings window within the monitor work area and adds
scrolling when 200-250% display scaling would otherwise hide controls. It also
improves tooltip sizing at high DPI, updates tooltip colors when the foobar2000
theme changes while the settings window remains open, and fixes the
processing-strength range text that retained Japanese punctuation in the
English interface.

Preset format v7, existing DSP and menu GUIDs, preset values, audio processing,
v1.6.0 language support, and the v1.5.4 real-time automatic safety-control
behavior remain unchanged.
