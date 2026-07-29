# v1.9.0 — 自動制御推移グラフ

## 変更内容

- 診断タブに［推移グラフ］を追加
- 再生中の曲について次の4項目を約1秒ごとに記録
  - Short-termラウドネス
  - 適用中の総ゲイン
  - 自動減衰量
  - 処理後True Peak
- 4項目を時間軸に沿った4段グラフで表示
- 自動制御の発動位置を赤い縦線で表示
- 曲が変わるとグラフを自動的にリセット
- 日本語／英語表示、ライト／ダーク表示へ対応

グラフデータは現在の1曲だけをメモリに保持し、終了時には保存しません。
描画はグラフ画面を開いている間だけ行います。値の記録は再生コールバック側で
行い、リアルタイム音声処理スレッドにはグラフ用処理を追加していません。

## 互換性

音声処理、発動しきい値、自動減衰の最大6 dB、プリセット値、
設定形式v7、DSP GUID、メニューGUID、DLL名は変更していません。

## 動作環境

- foobar2000 2.x
- Windows x64

## English

v1.9.0 adds a lightweight Automatic-Control Trend Graph for the currently
playing track. It records Short-term loudness, total applied gain, automatic
attenuation, and processed True Peak about once per second. Red vertical lines
mark automatic-control activation.

Only the current track is kept in memory, and the data resets when the track
changes. Drawing runs only while the graph window is open. The real-time audio
thread performs no graph recording or drawing work.

Audio processing, trigger thresholds, the 6 dB limit, preset values, preset
format v7, DSP and menu GUIDs, and the DLL name remain unchanged.
