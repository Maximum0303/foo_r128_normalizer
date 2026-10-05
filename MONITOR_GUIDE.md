# R128 Processing Monitor / R128 補正モニター

R128 Real-time Loudness Normalizer (`foo_r128_normalizer`)

Added in **v1.12.0**. Monitor: foobar2000 2.1+ / Windows x64.

v1.12.0で追加された補正モニターの解説です。
モニターはfoobar2000 2.1以降／Windows x64に対応します。

[English](#english) | [日本語](#日本語)

## English

This guide describes the R128 monitor. Sonic Refiner's guide will be integrated
separately and cover its different monitored values. The two use the same
explanations for units, bar interpretation and DSP order where applicable.

### The seven values

From top to bottom, the monitor shows these values. It refreshes about every 100 ms.

#### 1. Input 3 s (LUFS)

Short-term loudness of audio entering this DSP over approximately three seconds. It includes earlier DSP processing and is not a whole-file average.

#### 2. Gain (dB)

Current R128 normalization gain. Positive means amplification, negative means attenuation, and zero means no gain change. This is not the combined final gain of normalization, limiting, protection and additional processing.

#### 3. Output 3 s (LUFS)

Short-term loudness at this DSP output over approximately three seconds. Later DSPs and player volume changes are excluded. Compare it with the input value.

#### 4. True Peak (dBTP)

Approximate peak at this DSP output, estimated with 4x interpolation and 17 taps. Shows the maximum over the recent display interval, not the whole-track maximum. Zero dBTP is the full-scale reference. Scheduling delays can make the collection interval longer.

#### 5. Limiter (dB)

Current limiter attenuation, shown as a positive amount. For example, 2 dB means 2 dB of reduction; zero means none. This is not the track maximum.

#### 6. Protect. (dB)

Current additional attenuation applied by automatic protection in response to excessive peaks, limiting, clipping or related conditions. It is a positive amount; zero means no additional attenuation. It is separate from the limiter value. See diagnostics and history for details.

#### 7. Adaptive (% / OFF)

Current effective Modern Processing strength adjusted by Adaptive. It is not a volume increase percentage or the strength of all R128 processing. 50% does not mean 50% louder. OFF means Adaptive is inactive, not that fixed Modern Processing is necessarily disabled.

### Units and bars

LUFS measures loudness with perceptual weighting. -18 LUFS is louder than -23 LUFS.

dB measures gain change. Gain is signed; Limiter and Protect. show positive attenuation amounts.

dBTP measures True Peak relative to full scale. Values above 0 dBTP warrant checking peak settings and processing strength.

| Value | Bar range |
|---|---|
| Input / Output | -60 to 0 LUFS |
| Gain | Magnitude 0 to 24 dB; direction is in the numeric sign |
| True Peak | -60 to +3 dBTP |
| Limiter / Protect. | 0 to 12 dB |
| Adaptive | 0 to 100% |

Bars clamp at their endpoints; numbers retain the actual values. A full bar is not a quality or safety rating. Display ranges do not change processing limits or settings.

### Display states

Active: current values are available from the playback R128 DSP.

Analyzing / silence: loudness is unavailable while measuring or during silence. Three-second loudness needs approximately three seconds of measured audio after playback starts. The 100 ms refresh interval is separate from the measurement window.

Paused / Waiting: current values are hidden during pause or stop and update after playback resumes.

An em dash means no reliable current value. It can also indicate an absent DSP, multiple instances, an ambiguous source or stale data. Unavailable bars clear to zero. An em dash differs from a numeric zero or OFF.

Original comparison: only input is shown; processed values are hidden. Output measurement resumes on return to normal processing.

Conversion DSP values are excluded. The monitor requires foobar2000 2.1+. Language follows this component's Automatic (Windows), Japanese or English selection.

### Reading it and DSP order

Check the state first, then input/output three-second loudness, the gain sign, True Peak, the two attenuation values and Adaptive strength. Output need not always equal the target: source audio, measurement windows, gain limits, additional processing and protection all affect it.

If high True Peak or substantial attenuation persists, consult diagnostics and history to review settings. Momentary readings alone cannot rate sound quality.

Example: Sonic Refiner > R128 > Output

R128 input is audio after Sonic Refiner; R128 output is audio after R128. Later DSPs and player volume changes are excluded. Measurements from the two monitors use different methods and intervals and cannot simply be added.

Open: Playback > R128 Processing Monitor

Right-click > How to read the monitor... opens this guide.

Right-click > Always on Top toggles topmost display.

Position, visibility and topmost state are saved. Leaving the monitor open when quitting foobar2000 restores it on next startup. Closing it manually prevents that restoration.

This is a display window. Opening, closing or reading its help does not change DSP audio settings.

## 日本語

この解説はR128専用です。Sonic Refinerには、そちらの表示項目に合わせた
専用解説を別途用意します。単位・バー・DSP順序の説明は共通の考え方です。

### 7項目の意味

上から順に、次の値を表示します。約100ms更新です。

#### 1. 入力 3秒（LUFS）

このDSPへ入った音の直近約3秒のラウドネスです。前段DSPの処理を含みます。ファイル全体の平均ではありません。

#### 2. ゲイン（dB）

現在のR128ノーマライズによる補正ゲインです。＋は増幅、－は減衰、0は補正なしです。リミッター、自動保護、追加処理を合算した最終ゲインではありません。

#### 3. 出力 3秒（LUFS）

このDSPが出力した音の直近約3秒のラウドネスです。後段DSPやプレーヤーの音量操作は含みません。入力との比較に使います。

#### 4. True Peak（dBTP）

このDSPの出力ピークの近似値です。4倍補間・17タップで推定し、最近の表示区間の最大値を表示します。曲全体の最大値ではありません。0 dBTPはフルスケールの基準です。表示更新が遅れた場合、集計区間は長くなります。

#### 5. リミッター（dB）

現在リミッターが抑えている量です。正の数値で表示します。例：2 dBは2 dBの減衰、0は減衰なしです。トラック最大値ではありません。

#### 6. 自動保護（dB）

過度なピーク、リミッター、クリップ等を受け、自動保護が追加した現在の減衰量です。正の数値で表示し、0は追加減衰なしです。リミッターとは別の値です。詳細は診断・履歴を確認してください。

#### 7. 適応強度（%／OFF）

Adaptiveが自動調整する、モダン処理の現在の実効強度です。音量の増加率や全R128処理の強さではありません。50%は音量が50%増える意味ではありません。無効時はオフ／OFFです。固定モダン処理の有効・無効を示す値でもありません。

### 単位とバー

LUFS：聴感に近いラウドネスの単位です。－18 LUFSは－23 LUFSより大きい値です。

dB：ゲインの変化量です。ゲイン欄は符号付き、リミッター・自動保護欄は正の減衰量です。

dBTP：フルスケールを基準にしたTrue Peakです。0 dBTPを超える場合はピーク設定や処理強度を確認します。

| 項目 | バーの範囲 |
|---|---|
| 入力／出力 | －60～0 LUFS |
| ゲイン | 絶対値0～24 dB（方向は数値の＋／－） |
| True Peak | －60～＋3 dBTP |
| リミッター／自動保護 | 0～12 dB |
| 適応強度 | 0～100% |

バーの両端を超える値は、バーだけ端で止まり、数値は実際の値を表示します。バーの満杯は音質・安全性の評価ではありません。目盛りは処理の上限や設定値を変更しません。

### 表示状態

動作中／Active：再生用のR128 DSPから現在の値を表示しています。

解析中／静音・Analyzing / silence：測定待ちや静音でラウドネスを表示できない状態です。再生開始後、3秒ラウドネスには約3秒の測定が必要です。約100ms更新は測定時間とは別です。

一時停止中／Paused・待機中／Waiting：一時停止や停止では現在値を隠します。再開後に更新されます。

—：現在値を確定できません。DSP未登録、複数登録、表示対象の曖昧さ、古い測定値等でも表示されます。無効なバーは0になります。数値の0やOFFとは意味が異なります。

原音比較中：入力だけを表示し、処理後の値は隠します。通常の処理へ戻ると出力測定も再開します。

変換用DSPの値は表示しません。モニターはfoobar2000 2.1以降が必要です。言語は本コンポーネントの「自動（Windows）／日本語／English」に追従します。

### 読み方とDSP順序

まず状態を確認し、入力と出力の3秒ラウドネス、ゲインの符号、True Peak、2種類の減衰量、適応強度の順に見ます。出力値が常に目標値と一致するとは限りません。音源・測定窓・ゲイン上限・追加処理・保護等の影響を受けます。

True Peakが高い状態や大きな減衰が続く場合は、診断・履歴も確認し、設定を見直す材料にしてください。瞬間値だけで音質の良し悪しは判断できません。

例：Sonic Refiner → R128 → 出力

R128の入力値はSonic Refiner処理後の音です。R128の出力値はR128処理後の音です。後段DSPやプレーヤー音量による変化は含みません。モニター間の数値は測定方法・表示区間が異なるため、単純に加算できません。

開く：Playback → R128 補正モニター

右クリック → モニターの見方...：この解説を開きます。

右クリック → 常に手前に表示：最前面を切り替えます。

位置・開閉状態・最前面設定を保存します。開いたままfoobar2000を終了すると次回起動時に再表示します。手動で閉じた場合は再表示しません。

この窓は表示用です。開く・閉じる・解説を読む操作でDSPの音声設定を変更しません。
