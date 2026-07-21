@echo off
setlocal
chcp 65001 >nul
cd /d "%~dp0"

echo R128 Normalizer v1.5.4
echo Direct settings menu release
echo.

powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0scripts\build_v154.ps1"
if errorlevel 1 goto BUILD_FAILED

echo.
echo ビルド成功。インストール用ファイルを作成します。
echo.

powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0scripts\create_v154_package.ps1"
if errorlevel 1 goto PACKAGE_FAILED

echo.
echo v1.5.4の準備がすべて完了しました。
echo.
pause
exit /b 0

:BUILD_FAILED
echo.
echo ビルドに失敗しました。
if exist "%~dp0build_errors.txt" start "" notepad.exe "%~dp0build_errors.txt"
echo.
pause
exit /b 1

:PACKAGE_FAILED
echo.
echo DLLのビルドは成功しましたが、インストール用ファイルの作成に失敗しました。
echo 「v1.5.4インストール用ファイルだけ作成.cmd」を実行し直せます。
if exist "%~dp0package_error.txt" start "" notepad.exe "%~dp0package_error.txt"
echo.
pause
exit /b 1
