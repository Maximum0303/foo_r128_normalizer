param(
    [string]$SdkRoot = "C:\foobar2000-dev\SDK-2025-03-07"
)

$ErrorActionPreference = "Stop"

$toolRoot = Split-Path -Parent $PSScriptRoot
$errorLog = Join-Path $toolRoot "package_error.txt"
$logPath = Join-Path $toolRoot "package_log.txt"

Remove-Item $errorLog, $logPath `
    -Force `
    -ErrorAction SilentlyContinue

function Get-PeMachine {
    param([string]$Path)

    [byte[]]$bytes = [System.IO.File]::ReadAllBytes($Path)

    if (
        $bytes.Length -lt 256 -or
        $bytes[0] -ne 0x4D -or
        $bytes[1] -ne 0x5A
    ) {
        throw "DLLのMZヘッダーが正しくありません: $Path"
    }

    $peOffset = [BitConverter]::ToInt32($bytes, 0x3C)

    if (
        $peOffset -lt 0 -or
        ($peOffset + 6) -gt $bytes.Length
    ) {
        throw "DLLのPEオフセットが正しくありません: $Path"
    }

    $signature = [BitConverter]::ToUInt32($bytes, $peOffset)

    if ($signature -ne 0x00004550) {
        throw "DLLにPE署名がありません: $Path"
    }

    return [BitConverter]::ToUInt16($bytes, $peOffset + 4)
}

function Test-BinaryContainsAscii {
    param(
        [string]$Path,
        [string]$Text
    )

    $binaryText = [Text.Encoding]::ASCII.GetString(
        [System.IO.File]::ReadAllBytes($Path)
    )

    return $binaryText.Contains($Text)
}

try {
    $dllPath = Join-Path $SdkRoot `
        "foobar2000\foo_r128_normalizer\x64\Release\foo_r128_normalizer.dll"

    $licensePath = Join-Path $toolRoot "license.txt"
    $noticesPath = Join-Path $toolRoot "THIRD-PARTY-NOTICES.txt"
    $readmePath = Join-Path $toolRoot "README.txt"
    $glossaryPath = Join-Path $toolRoot "GLOSSARY.txt"
    $checklistPath = Join-Path $toolRoot "RELEASE_CHECKLIST.txt"
    $shaSumsPath = Join-Path $toolRoot "SHA256SUMS.txt"
    $changelogPath = Join-Path $toolRoot "CHANGELOG.txt"
    $quickStartPath = Join-Path $toolRoot "QUICK_START.txt"

    $requiredPaths = @(
        $dllPath,
        $licensePath,
        $noticesPath,
        $readmePath,
        $glossaryPath,
        $changelogPath,
        $quickStartPath
    )

    foreach ($requiredPath in $requiredPaths) {
        if (-not (Test-Path $requiredPath)) {
            throw "パッケージ必須ファイルが見つかりません: $requiredPath"
        }
    }

    $dll = Get-Item $dllPath

    if ($dll.Length -lt 4096) {
        throw "DLLのサイズが小さすぎます: $($dll.Length) bytes"
    }

    $machine = Get-PeMachine -Path $dllPath

    if ($machine -ne 0x8664) {
        throw (
            "DLLがx64ではありません。PE Machine=0x{0:X4}" -f
            $machine
        )
    }

    if (
        -not (
            Test-BinaryContainsAscii `
                -Path $dllPath `
                -Text "1.5.4"
        )
    ) {
        throw (
            "DLL内にバージョン1.5.4がありません。" +
            "古いDLLを梱包しようとしている可能性があります。"
        )
    }

    $dllHash = (Get-FileHash $dllPath -Algorithm SHA256).Hash

    Add-Type -AssemblyName System.IO.Compression
    Add-Type -AssemblyName System.IO.Compression.FileSystem

    $componentName = "foo_r128_normalizer_v1.5.4.fb2k-component"
    $componentPath = Join-Path $toolRoot $componentName

    Remove-Item $componentPath `
        -Force `
        -ErrorAction SilentlyContinue

    $projectDir = Join-Path $SdkRoot "foobar2000\foo_r128_normalizer"
    $searchRoots = @(
        (Join-Path $SdkRoot "foobar2000\SDK"),
        (Join-Path $SdkRoot "pfc"),
        (Join-Path $SdkRoot "libPPUI")
    )

    $sdkNoticeFiles = New-Object `
        System.Collections.Generic.List[object]

    foreach ($root in $searchRoots) {
        if (-not (Test-Path $root)) {
            continue
        }

        $found = Get-ChildItem `
            $root `
            -Recurse `
            -File `
            -ErrorAction SilentlyContinue |
            Where-Object {
                $_.FullName -notlike "$projectDir*" -and
                $_.Length -le 1048576 -and
                (
                    $_.Name -match '(?i)license' -or
                    $_.Name -match '(?i)copying' -or
                    $_.Name -match '(?i)copyright'
                )
            }

        foreach ($file in $found) {
            if (
                -not (
                    $sdkNoticeFiles.FullName -contains $file.FullName
                )
            ) {
                $sdkNoticeFiles.Add($file)
            }
        }
    }

    $stream = [System.IO.File]::Open(
        $componentPath,
        [System.IO.FileMode]::CreateNew
    )

    try {
        $archive = New-Object System.IO.Compression.ZipArchive(
            $stream,
            [System.IO.Compression.ZipArchiveMode]::Create,
            $false
        )

        try {
            $fixedEntries = @(
                @($dllPath, "foo_r128_normalizer.dll"),
                @($licensePath, "license.txt"),
                @($noticesPath, "THIRD-PARTY-NOTICES.txt"),
                @($readmePath, "README.txt"),
                @($glossaryPath, "GLOSSARY.txt"),
                @($changelogPath, "CHANGELOG.txt"),
                @($quickStartPath, "QUICK_START.txt")
            )

            foreach ($item in $fixedEntries) {
                [System.IO.Compression.ZipFileExtensions]::CreateEntryFromFile(
                    $archive,
                    $item[0],
                    $item[1],
                    [System.IO.Compression.CompressionLevel]::Optimal
                ) | Out-Null
            }

            foreach ($file in $sdkNoticeFiles) {
                $relative = $file.FullName.Substring(
                    $SdkRoot.Length
                ).TrimStart([char[]]"\/")

                $safeRelative = $relative -replace '\\', '/'
                $entryName = "licenses/$safeRelative"

                [System.IO.Compression.ZipFileExtensions]::CreateEntryFromFile(
                    $archive,
                    $file.FullName,
                    $entryName,
                    [System.IO.Compression.CompressionLevel]::Optimal
                ) | Out-Null
            }
        }
        finally {
            $archive.Dispose()
        }
    }
    finally {
        $stream.Dispose()
    }

    $check = [System.IO.Compression.ZipFile]::OpenRead(
        $componentPath
    )

    try {
        $entries = @(
            $check.Entries |
                ForEach-Object {
                    $_.FullName
                }
        )
    }
    finally {
        $check.Dispose()
    }

    $requiredEntries = @(
        "foo_r128_normalizer.dll",
        "license.txt",
        "THIRD-PARTY-NOTICES.txt",
        "README.txt",
        "GLOSSARY.txt",
        "CHANGELOG.txt",
        "QUICK_START.txt"
    )

    foreach ($entry in $requiredEntries) {
        if ($entries -notcontains $entry) {
            throw "パッケージ内容の検証に失敗しました。欠落: $entry"
        }
    }

    $componentHash = (
        Get-FileHash $componentPath -Algorithm SHA256
    ).Hash

    (
        $componentHash.ToLowerInvariant() +
        "  " +
        $componentName
    ) |
        Set-Content $shaSumsPath -Encoding ASCII

    $created = New-Object System.Collections.Generic.List[string]
    $created.Add($componentPath)
    $created.Add($shaSumsPath)

    $releaseDirectories = New-Object `
        System.Collections.Generic.List[string]

    $desktop = [Environment]::GetFolderPath("Desktop")

    if (-not [string]::IsNullOrWhiteSpace($desktop)) {
        $releaseDirectories.Add($desktop)
    }

    if (-not [string]::IsNullOrWhiteSpace($env:OneDrive)) {
        $oneDriveDesktop = Join-Path $env:OneDrive "デスクトップ"

        if (
            (Test-Path $oneDriveDesktop) -and
            -not $releaseDirectories.Contains($oneDriveDesktop)
        ) {
            $releaseDirectories.Add($oneDriveDesktop)
        }
    }

    $copyWarnings = New-Object `
        System.Collections.Generic.List[string]

    foreach ($directory in $releaseDirectories) {
        foreach ($sourceFile in @($componentPath, $shaSumsPath)) {
            try {
                $target = Join-Path `
                    $directory `
                    ([System.IO.Path]::GetFileName($sourceFile))

                if ($target -ne $sourceFile) {
                    Copy-Item $sourceFile $target -Force
                }

                if (-not $created.Contains($target)) {
                    $created.Add($target)
                }
            }
            catch {
                $copyWarnings.Add(
                    "コピーできませんでした: $sourceFile → $directory / " +
                    $_.Exception.Message
                )
            }
        }
    }

    $logLines = New-Object System.Collections.Generic.List[string]
    $logLines.Add("作成成功")
    $logLines.Add("元DLL: $dllPath")
    $logLines.Add("DLLバージョン検証: 1.5.4")
    $logLines.Add("DLL形式: x64 (PE Machine 0x8664)")
    $logLines.Add("DLLサイズ: $($dll.Length) bytes")
    $logLines.Add("DLL SHA256: $dllHash")
    $logLines.Add("パッケージ SHA256: $componentHash")
    $logLines.Add("")
    $logLines.Add("必須アーカイブ内容:")

    foreach ($entry in $requiredEntries) {
        $logLines.Add("  $entry")
    }

    if ($sdkNoticeFiles.Count -gt 0) {
        $logLines.Add("")
        $logLines.Add(
            "SDKから同梱したライセンス・著作権通知:"
        )

        foreach ($file in $sdkNoticeFiles) {
            $logLines.Add("  $($file.FullName)")
        }
    }
    else {
        $logLines.Add("")
        $logLines.Add(
            "SDK内の追加ライセンスファイルは自動検出されませんでした。"
        )
        $logLines.Add(
            "THIRD-PARTY-NOTICES.txtのSDKクレジットは同梱済みです。"
        )
    }

    $logLines.Add("")
    $logLines.Add("作成場所:")

    foreach ($path in $created) {
        $logLines.Add($path)
    }

    if ($copyWarnings.Count -gt 0) {
        $logLines.Add("")
        $logLines.Add("コピー時の警告:")

        foreach ($warning in $copyWarnings) {
            $logLines.Add($warning)
        }
    }

    $logLines |
        Set-Content $logPath -Encoding UTF8

    Write-Host ""
    Write-Host "v1.5.4のインストール用ファイルを作成しました。" `
        -ForegroundColor Green

    foreach ($path in $created) {
        Write-Host $path -ForegroundColor Green
    }

    Write-Host ""
    Write-Host "DLLの版数・x64形式・SHA256を確認しました。"
    Write-Host "必須ファイルの収録検証にも成功しています。"
    Write-Host "GitHub Release用のSHA256SUMS.txtも作成しました。"

    $showPath = $created[$created.Count - 1]
    Start-Process explorer.exe `
        -ArgumentList "/select,`"$showPath`""

    exit 0
}
catch {
    $message = $_ | Format-List * -Force | Out-String
    $message |
        Set-Content $errorLog -Encoding UTF8

    Write-Host ""
    Write-Host "エラー: $($_.Exception.Message)" `
        -ForegroundColor Red
    exit 1
}
