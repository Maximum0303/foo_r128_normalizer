# v1.7.0 — リアルタイム自動制御の見える化

## 変更内容

- 現在の処理状態の「要調整」を「監視中」へ変更
- 自動安全補正から安全に復帰した後は「正常」へ戻る
- 自動制御の理由を次の4種類で表示
  - True Peak超過
  - リミッター過多
  - クリッパー過多
  - 複数要因
- 「自動安全補正ゲイン」を、実施量が分かりやすい「自動減衰量」へ変更
- 自動減衰量を正のdB値で表示
- 診断コピー、項目ヘルプ、日本語／英語用語集を新表示へ対応
- 診断タブ右列の行間隔を調整し、最下段が枠外へはみ出す問題を修正

発動理由は、トラック変更または測定リセットまで保持されます。

## 互換性

音声処理、発動しきい値、自動減衰の最大6 dB、プリセット値、
設定形式v7、DSP GUID、メニューGUID、DLL名は変更していません。

## 動作環境

- foobar2000 2.x
- Windows x64

## English

v1.7.0 makes real-time automatic safety control easier to understand.
The current processing state now uses Normal, Monitoring, Auto-adjusting,
and Adjustment limit, and returns to Normal after safe recovery. Diagnostics
show whether the trigger was a True Peak exceedance, excessive limiting,
excessive clipping, or multiple factors. Automatic attenuation is displayed
as a positive dB amount.

Audio processing, trigger thresholds, the 6 dB limit, preset values,
preset format v7, DSP and menu GUIDs, and the DLL name remain unchanged.
