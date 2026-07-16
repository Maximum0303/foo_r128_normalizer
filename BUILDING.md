# ソースからのビルド

この文書は開発者向けです。

一般利用者は、GitHub Releasesで配布される
`foo_r128_normalizer_v1.5.2.fb2k-component`を使用してください。

## 基準環境

- Windows x64
- Visual Studio 2022
- foobar2000 SDK 2025-03-07
- SDKルート：`C:\foobar2000-dev\SDK-2025-03-07`

## 自動ビルド・梱包

1. このソース一式を展開します。
2. `v1.5.2へ更新・ビルド・梱包.cmd`を実行します。
3. Release／x64でビルドされます。
4. DLLの版数、x64形式、更新時刻、SHA256が検査されます。
5. 成功すると次の配布ファイルが作成されます。

   `foo_r128_normalizer_v1.5.2.fb2k-component`

## GitHub Releaseへアップロードするファイル

必須:

- `foo_r128_normalizer_v1.5.2.fb2k-component`
- `SHA256SUMS.txt`

任意:

- ソースコード一式
- `CHANGELOG.txt`

一般利用者向けRelease AssetをZIPで二重圧縮する必要はありません。
`.fb2k-component`をそのままアップロードしてください。

## ビルド失敗時

- `build_errors.txt`
- 必要に応じて`build_full_log.txt`

を確認してください。
