# ソースからのビルド

v1.13.0のソースです。既存のSDK内プロジェクトが必要です。
スクリプトはソース8ファイルを更新し、
バックアップを残したうえでC++17以上・Release x64でビルドします。
モニターはfoobar2000 2.1以降対応、従来の音声処理は2.0対応を維持します。
モニター実装の検証記録と未確認範囲はMONITOR_DEV_NOTES.mdに記載しています。
テスト済みv1.13.0-dev.1からの正式版準備ソースです。版数更新後の
Windowsでの再ビルド・導入確認は、RELEASE_CHECKLIST.txtに従って行います。

この文書は開発者向けです。

本ZIPは完成ソース一式です。既存SDK内のvcxprojが必要です。
一般利用者向けの配布物は`.fb2k-component`です。

## 基準環境

- Windows x64
- Visual Studio 2022またはBuild Tools 2022
- C++によるデスクトップ開発、MSVC v143 x64/x86、Windows SDK、C++ ATL x86/x64
- foobar2000 SDK 2025-03-07
- SDKルート：`F:\foobar2000-dev\SDK-2025-03-07`

プロジェクトは`F:\foobar2000-dev\SDK-2025-03-07\foobar2000\foo_r128_normalizer\foo_r128_normalizer.vcxproj`。
同梱CMDはFドライブを既定値とするスクリプトを実行します。別のSDK位置を使う場合は、
ビルド・梱包の両方へ`-SdkRoot`を指定してください。

## 自動ビルド・梱包

1. このソース一式を展開します。
2. `v1.13.0へ更新・ビルド・梱包.cmd`を実行します。
3. Release／x64でビルドされます。
4. DLLの版数、x64形式、更新時刻、SHA256が検査されます。
5. 成功すると次の配布ファイルが作成されます。

   `foo_r128_normalizer_v1.13.0.fb2k-component`

## 生成物

必須:

- `foo_r128_normalizer_v1.13.0.fb2k-component`
- `SHA256SUMS.txt`

生成したコンポーネントを上書きインストールして再起動し、
RELEASE_CHECKLIST.txtの対象項目を1工程ずつ確認します。
GitHub公開は最終確認後の別工程です。

## ビルド失敗時

- `build_errors.txt`
- 必要に応じて`build_full_log.txt`

を確認してください。
