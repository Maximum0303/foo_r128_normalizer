# v1.8.0 — 自動制御履歴

## 変更内容

- 診断タブに［自動制御履歴］を追加
- 自動制御が実際に発動した最新100曲を再起動後も保持
- 各トラックについて次を一覧表示
  - 再生日時
  - 曲名／アーティスト
  - 使用プリセット
  - 発動理由
  - 最大自動減衰量
  - 発動回数
  - 調整上限への到達
  - 安全復帰
- ［履歴コピー］［選択削除］［すべて削除］を追加
- 短い「監視中」だけで自動調整へ入らなかった曲は履歴へ追加しない
- 未発動時の診断表示を「この曲では未発動」へ変更
- 安全復帰後は「直近：発動理由（復帰済み）」と表示

履歴は音声処理とは別の再生コールバックで更新します。
リアルタイム音声処理スレッドから履歴のディスク書き込みは行いません。

## 互換性

音声処理、発動しきい値、自動減衰の最大6 dB、プリセット値、
設定形式v7、DSP GUID、メニューGUID、DLL名は変更していません。
履歴設定はDSPプリセットとは別に保存されます。

## 動作環境

- foobar2000 2.x
- Windows x64

## English

v1.8.0 adds a persistent Automatic-Control History for up to 100 tracks where
automatic control actually activated. Each entry records playback time, title
and artist, preset, trigger reason, maximum automatic attenuation, activation
count, adjustment-limit status, and safe recovery.

The history can be copied, selectively deleted, or cleared. Brief Monitoring
states that never reach Auto-adjusting are not recorded. History updates are
performed outside the real-time audio thread.

Audio processing, trigger thresholds, the 6 dB limit, preset values, preset
format v7, DSP and menu GUIDs, and the DLL name remain unchanged.
