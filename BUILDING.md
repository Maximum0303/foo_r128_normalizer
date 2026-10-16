# v1.14.0のビルド

「ラウド -9」を追加した正式版向け完成ソース。
v1.14.0-dev.2の確認済み処理を維持し、版数・ビルド用ファイル名・配布文書を正式版向けに整理した。
正式番号1.14.0でのWindowsビルド・梱包・導入・再起動・版数・設定保持・吹き出し・短時間再生を確認済み。
受領した配布物のCRC・x64 DLL・版数・SHA-256・同梱文書も検査済み。GitHubへの手動公開は未実施。
公開済みの基準はR128 v1.13.0 Source of Truth文書版1.0のままとする。

## 環境

- Windows x64、Visual Studio 2022 Build Tools
- C++によるデスクトップ開発、MSVC v143 x86/x64、Windows SDK、C++ ATL x86/x64
- foobar2000 SDK 2025-03-07
- SDK：`F:\foobar2000-dev\SDK-2025-03-07`
- 既存プロジェクト：`F:\foobar2000-dev\SDK-2025-03-07\foobar2000\foo_r128_normalizer\foo_r128_normalizer.vcxproj`

vcxproj／slnは収録しない。既存SDKプロジェクトへ8ソースをコピーし、C++17・UTF-8を設定して
Release/x64をRebuildする。バックアップは.before_v1.14.0。MSBuildはvswhereで検出する。
生成DLLはx64と版数の完全一致を検査し、古いDLLの誤梱包を防ぐ。

## 実行

1. ZIPをdev版とは別の新しいフォルダーへ展開する。
2. `v1.14.0へ更新・ビルド・梱包.cmd`をダブルクリックする。
3. 成功すると`foo_r128_normalizer_v1.14.0.fb2k-component`と`SHA256SUMS.txt`を生成する。

別のSDK位置ではビルド・梱包の両方に-SdkRootを指定する。

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\build_v1140.ps1 -SdkRoot "F:\foobar2000-dev\SDK-2025-03-07"
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\create_v1140_package.ps1 -SdkRoot "F:\foobar2000-dev\SDK-2025-03-07"
```

## 失敗時

ビルド失敗は`build_errors.txt`、必要なら`build_full_log.txt`、梱包失敗は`package_error.txt`を確認する。
ログ・生成DLL・配布物・バックアップはGitHubソースへ含めない。
ビルド後の導入・確認・手動公開は1工程ずつ「OK」を待って進める。
