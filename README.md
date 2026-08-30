# R128 Real-time Loudness Normalizer

[English](#english) | [日本語](#日本語)

## English

**R128 Real-time Loudness Normalizer** is an R128-based real-time loudness
normalizer for foobar2000 2.x on Windows x64.

The technical identifier, repository name, and DLL name remain
`foo_r128_normalizer`.

In addition to standard loudness normalization, optional Modern Processing,
1-Band Adaptive, and 3-Band Adaptive processing are available.

### Download

Download the following file from GitHub Releases:

`foo_r128_normalizer_v1.11.0.fb2k-component`

Visual Studio and the foobar2000 SDK are not required for normal use.

### Installation

1. Download `foo_r128_normalizer_v1.11.0.fb2k-component`.
2. Open the file and follow the foobar2000 installation prompt.
3. Restart foobar2000.
4. Add **R128 Real-time Loudness Normalizer** to the active DSP chain in DSP Manager.
5. **Natural -18** is recommended as a starting preset.

### Display language

Choose a display language at the bottom of the settings window:

- Automatic (Windows)
- Japanese
- English

The selection is immediately applied to settings, presets, diagnostics, help,
the glossary, license information, the Playback menu, and the DSP name.
Language selection is stored separately from DSP presets and does not affect
configuration format v7, built-in presets, audio processing, or GUIDs.

### Automatic safety control

The current state is shown as **Normal**, **Monitoring**, **Auto-adjusting**, or
**Adjustment limit**. After safe recovery, the current processing state returns
to **Normal**. Diagnostics identify whether control was triggered by True Peak
exceedance, excessive limiting, excessive clipping, or multiple factors, and
show the automatic attenuation amount.

### Automatic-control history

Open **Automatic-Control History** from the Diagnostics page to review up to
100 recent tracks where automatic control actually activated.

- Playback date and time
- Track title and artist
- Preset
- Trigger reason
- Maximum automatic attenuation
- Activation count
- Adjustment-limit status
- Safe recovery status

History persists across restarts. Entries can be copied, selectively deleted,
or cleared. A brief **Monitoring** state that never reaches **Auto-adjusting**
is not added to the history.

### Automatic-control trend graph

Open **Trend Graph** from the Diagnostics page to view the following values for
the currently playing track over time:

- Short-term loudness
- Total applied gain
- Automatic attenuation
- Processed True Peak

Values are recorded about once per second. Red vertical lines mark
automatic-control activation. Only the current track is kept in memory, and
the graph resets when the track changes. Drawing runs only while the graph
window is open, and the real-time audio thread performs no graph recording or
drawing work.

### Opening settings directly

After adding the DSP, open its settings without opening DSP Manager:

`Playback > R128 Real-time Loudness Normalizer Settings...`

This command can also be assigned to a foobar2000 keyboard shortcut. The
settings window stays above foobar2000 while allowing the player behind it to
remain operable. It is not system-wide always-on-top.

To prevent unintended changes, settings are not opened when the DSP is absent
from the active chain or is registered more than once.

### Presets

#### Standard normalization

- Natural -18
- Power Boost -14
- Relaxed -23
- Night Safe -22

#### Additional mastering processing

- Modern Boost -9
- 1-Band Adaptive -10
- 3-Band Adaptive -10

Additional mastering processing affects tone and dynamics as well as loudness.
Use one of the four standard presets when transparent loudness matching is the
priority.

#### User presets

Version 1.10.0 and later can save all current Basic and Processing settings
under a custom name. Use **Save As**, **Load**, **Overwrite**, **Rename**, and
**Delete** to manage them.

Names may contain Japanese or English text and must be 1–64 characters long.
Up to 100 user presets can be stored. The seven built-in presets remain
read-only. User presets are stored only in the foobar2000 configuration and
are not transmitted externally.

Version 1.11.0 adds backup and migration features:

- **Export Selected** saves one selected preset to an `.r128preset` file.
- **Export All** saves up to 100 presets in a single file.
- **Import from File** restores presets from an exported file.
- Duplicate names can be overwritten, saved under another name, or skipped.
- The entire file is validated before import; invalid files do not change
  existing data.

Exported files contain only user-preset names and settings. They do not contain
track or artist names, audio, file paths, Windows user names, PC names, or IP
addresses.

### Main features

- Automatic light/dark theme matching with foobar2000
- R128-based real-time loudness normalization
- Direct settings access from the Playback menu or a keyboard shortcut
- Momentary, Short-term, Integrated, and LRA displays
- True Peak estimation and look-ahead limiter
- Track-start stabilization, silence protection, and gain locking
- Modern, 1-Band Adaptive, and 3-Band Adaptive processing
- Loudness-matched A/B comparison and complete bypass comparison
- Diagnostic results retained after track completion
- Real-time automatic-control state, trigger reason, and attenuation display
- Persistent automatic-control history for up to 100 activated tracks
- Lightweight trend graph for the current track
- Save, load, overwrite, rename, and delete user presets
- Export selected or all user presets and import `.r128preset` files
- Safety protection against invalid audio values

### Verified display environment

- Resolution: 1920 × 1080
- Windows display scaling: 150%

Results may differ with other resolutions, scaling levels, taskbar layouts, or
multi-monitor configurations.

### System requirements

- foobar2000 2.x
- Windows x64

### Known limitations

- Direct settings access targets the active playback DSP chain.
- Direct settings access is blocked if the DSP is registered more than once.
- 3-band processing uses a simplified design optimized for low latency and low
  processing cost.
- It is not a substitute for professional, material-specific mastering.
- True Peak is approximated using 4× oversampling.
- Correct display cannot be guaranteed at every resolution and DPI level.
- Actual output quality is affected by the entire playback chain.

### Building from source

See [BUILDING.md](BUILDING.md) for developer instructions. Normal users do not
need to build from source.

### License

MIT License

Author: Maximum

This implementation was developed independently while referring to the prior
concept of `EBU R128 Normalizer by mudlord`. See
[`THIRD-PARTY-NOTICES.txt`](THIRD-PARTY-NOTICES.txt) for details.

---

## 日本語

**R128 リアルタイム音量ノーマライザー**は、foobar2000 2.x
（Windows x64）向けのR128ベース・リアルタイム音量ノーマライザーです。

技術識別名、リポジトリ名、DLL名は従来どおり `foo_r128_normalizer` です。

通常のラウドネス補正に加え、任意でモダン処理、1バンド・アダプティブ、3バンド・アダプティブを利用できます。

### ダウンロード

GitHub Releasesから、次のファイルをダウンロードしてください。

`foo_r128_normalizer_v1.11.0.fb2k-component`

一般利用では、Visual Studioやfoobar2000 SDKは必要ありません。

### インストール

1. `foo_r128_normalizer_v1.11.0.fb2k-component`をダウンロードします。
2. ファイルを開き、foobar2000の確認画面に従ってインストールします。
3. foobar2000を再起動します。
4. DSP Managerで「R128 音量ノーマライザー」を使用中のDSPへ追加します。
5. 最初は「ナチュラル -18」を選ぶことをおすすめします。

### 表示言語

設定画面下部の「表示言語」で、次から選択できます。

- 自動（Windows）
- 日本語
- English

設定画面、プリセット、診断、ヘルプ、用語集、ライセンス、Playbackメニュー、
DSP名へすぐ反映されます。言語設定はDSPプリセットと別に保存されるため、
設定形式v7、既存プリセット、音声処理、GUIDには影響しません。

### リアルタイム自動制御

リアルタイム自動制御の状態を
「正常／監視中／自動調整中／調整上限」で表示します。
安全復帰後の現在の処理状態は「正常」に戻ります。
True Peak超過、リミッター過多、クリッパー過多、複数要因のいずれで
制御が必要になったかと、実施した自動減衰量も診断画面で確認できます。

### 自動制御履歴

v1.8.0では、診断タブの［自動制御履歴］から、
自動制御が実際に発動した最新100曲を確認できます。

- 再生日時
- 曲名／アーティスト
- 使用プリセット
- 発動理由
- 最大自動減衰量
- 発動回数
- 調整上限への到達
- 安全復帰

履歴は再起動後も保持され、一覧をクリップボードへコピーできます。
選択した履歴または全履歴の削除にも対応します。
短い「監視中」だけで自動調整へ入らなかった曲は履歴へ追加しません。

### 自動制御推移グラフ

v1.9.0では、診断タブの［推移グラフ］から、再生中の曲について
次の4項目を時間軸に沿って確認できます。

- Short-termラウドネス
- 適用中の総ゲイン
- 自動減衰量
- 処理後True Peak

値は約1秒ごとに記録し、自動制御が発動した位置を赤い縦線で表示します。
保持するのは現在の1曲だけで、曲が変わると自動的にリセットされます。
グラフ画面を閉じている間は描画しません。音声処理スレッドでは
グラフ用の記録・描画を行わないため、再生への影響を抑えています。

### 設定画面を直接開く

DSPへ追加した後は、DSP Managerを開かずに次のメニューから設定できます。

`Playback → R128 音量ノーマライザーの設定...`

このメニューコマンドは、foobar2000のキーボードショートカット設定にも割り当てられます。

直接開いた設定画面はfoobar2000より常に手前に表示されます。
設定画面を開いたまま、背後のfoobar2000を操作できます。
Windows全体の「常に最前面」ではないため、ほかのアプリは通常どおり前へ出せます。

安全のため、次の場合は設定を変更せず案内を表示します。

- 現在のDSPチェーンに本DSPが登録されていない
- 現在のDSPチェーンに本DSPが複数登録されている

### プリセット

#### 標準ノーマライズ

- ナチュラル -18
- パワーブースト -14
- リラックス -23
- ナイトセーフ -22

#### 追加マスタリング処理

- モダンブースト -9
- 1バンド・アダプティブ -10
- 3バンド・アダプティブ -10

追加マスタリング処理は、音量だけでなく音色やダイナミクスにも影響します。
純粋な音量統一を優先する場合は、標準4プリセットを使用してください。

#### ユーザープリセット

v1.10.0以降では、現在の基本設定と追加処理設定を任意の名前で保存できます。

- ［名前を付けて保存］：現在の設定欄を新しい名前で保存
- ［呼出］：選択したユーザープリセットを設定欄へ読み込み
- ［上書き］：選択したユーザープリセットを現在の設定で更新
- ［名前変更］：保存済みの名前だけを変更
- ［削除］：保存済みのユーザープリセットを削除

日本語・英語を含む1～64文字の名前を使用でき、最大100件まで保存できます。
既存7プリセットは変更・削除されません。既存プリセットを基に調整し、
［名前を付けて保存］で別のユーザープリセットとして保存できます。
ユーザープリセットはfoobar2000の設定内だけに保存され、外部へ送信されません。

v1.11.0では、バックアップや別PCへの移行用に次の操作を追加しました。

- ［選択を書き出し］：選択した1件だけを `.r128preset` へ保存
- ［すべて書き出し］：最大100件を1ファイルへ一括保存
- ［ファイルから読み込み］：書き出したファイルから復元
- 同名時は［上書き］［別名保存］［スキップ］から選択
- 読み込み前にファイル全体を検証し、壊れている場合は既存データを変更しない

書き出しファイルに含まれるのは、ユーザープリセット名と設定値だけです。
曲名、アーティスト名、音声、ファイルパス、Windowsユーザー名、PC名、IPアドレスは含みません。

### 主な機能

- foobar2000本体のライト／ダーク表示へ自動追従
- 設定画面、各コントロール、タブ、ツールチップ、用語集、項目ヘルプ、ライセンス・クレジット表示のダークモード対応
- R128ベースのリアルタイム音量補正
- Playbackメニューから設定画面を直接表示
- キーボードショートカットから設定画面を起動可能
- Momentary／Short-term／Integrated／LRA表示
- True Peak推定と先読みリミッター
- 曲頭安定化、静音保護、ゲイン固定
- モダン、1バンド・アダプティブ、3バンド・アダプティブ
- 音量一致A/B比較と完全バイパス比較
- トラック終了後の診断結果保持
- 診断コピー、用語集、項目別ヘルプ
- 自動制御の状態、発動理由、自動減衰量をリアルタイム表示
- 自動制御が発動した最新100曲の履歴、コピー、選択削除、全削除
- 現在の1曲だけを対象とする軽量な自動制御推移グラフ
- 現在の全設定を保存・呼出・上書き・名前変更・削除できるユーザープリセット
- ユーザープリセットの選択書き出し、一括書き出し、読み込み
- 異常な音声数値に対する安全保護
- 基本設定／追加処理／診断の3タブ画面
- 基本設定と診断ページの各項目を、省略しない正式名称で表示
- 用語集の用語名も、できる範囲で正式名称へ統一
- 追加処理タブを横3列カード表示にし、3つの処理を比較しやすく配置
- 共通の処理強度設定を追加処理タブ下段へ横一杯に配置
- 診断ページの上下余白を均等に調整
- 基本設定と診断は、左列を上から下へ読んでから右列へ移る構成
- 診断ページを2列表示にし、各項目を正式名称で表示
- 再起動後も現在のプリセット名を表示
- 手動調整した設定は「カスタム設定」と表示
- ［初期設定］は確認ダイアログを経てから設定欄へ反映され、［適用］を押すと再生中のDSPへ反映
- ［適用］は設定変更がある時だけ有効になり、適用後は無効へ戻る

### 確認済み表示環境

- 画面解像度：1920 × 1080
- Windows表示倍率：150%
- 設定画面全体が作業領域内に収まることを実機確認

その他の解像度、表示倍率、タスクバー構成、マルチモニター構成では、
表示結果が異なる場合があります。

### 動作環境

- foobar2000 2.x
- Windows x64

### 既知の制限

- 直接設定メニューは現在の再生用DSPチェーンを対象にします。
- 同じDSPを複数登録している場合は、誤変更防止のため直接設定を開きません。
- 3バンド処理は、軽量・低遅延を優先した簡易構成です。
- 素材別に調整された業務用マスタリングの代替ではありません。
- True Peakは4倍オーバーサンプリングによる近似です。
- すべての画面解像度とDPI倍率での表示を保証するものではありません。
- 実際の出力品質は再生チェーン全体の影響を受けます。

### ソースからビルドする場合

開発者向けの手順は [BUILDING.md](BUILDING.md) を参照してください。

一般利用者がソースからビルドする必要はありません。

### ライセンス

MIT License

Author: Maximum

本実装は `EBU R128 Normalizer by mudlord` の先行概念を参考にしつつ、
独立して実装されたものです。詳細は `THIRD-PARTY-NOTICES.txt` を参照してください。
