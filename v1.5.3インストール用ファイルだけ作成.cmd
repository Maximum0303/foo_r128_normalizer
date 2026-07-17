@echo off
setlocal
chcp 65001 >nul
cd /d "%~dp0"

echo R128 Normalizer v1.5.3 - package only
echo.
powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0scripts\create_v153_package.ps1"

set EXITCODE=%ERRORLEVEL%
echo.
if not "%EXITCODE%"=="0" (
    echo Package creation failed.
    if exist "%~dp0package_error.txt" start "" notepad.exe "%~dp0package_error.txt"
) else (
    echo Package creation completed successfully.
)
echo.
pause
exit /b %EXITCODE%
