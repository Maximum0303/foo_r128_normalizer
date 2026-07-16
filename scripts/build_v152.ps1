param(
    [string]$SdkRoot = "C:\foobar2000-dev\SDK-2025-03-07"
)

$ErrorActionPreference = "Stop"

$toolRoot = Split-Path -Parent $PSScriptRoot
$sourceRoot = Join-Path $toolRoot "source"
$projectDir = Join-Path $SdkRoot "foobar2000\foo_r128_normalizer"
$projectPath = Join-Path $projectDir "foo_r128_normalizer.vcxproj"

$fullLog = Join-Path $toolRoot "build_full_log.txt"
$errorsLog = Join-Path $toolRoot "build_errors.txt"
$updateLog = Join-Path $toolRoot "v1.5.2_update_log.txt"
$validationLog = Join-Path $toolRoot "build_validation.txt"
$binLog = Join-Path $toolRoot "build.binlog"

Remove-Item `
    $fullLog, `
    $errorsLog, `
    $updateLog, `
    $validationLog, `
    $binLog `
    -Force `
    -ErrorAction SilentlyContinue

function Write-UpdateLog {
    param([string]$Message)

    $Message |
        Tee-Object -FilePath $updateLog -Append
}

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

function Write-ErrorSummary {
    param([string]$Path)

    if (-not (Test-Path $Path)) {
        return
    }

    $lines = @(Get-Content $Path -Encoding UTF8)
    $contextLines = New-Object System.Collections.Generic.List[string]

    for ($index = 0; $index -lt $lines.Count; ++$index) {
        $line = [string]$lines[$index]

        $isError =
            $line -match '(^|\s)error\s+[A-Z]+\d+' -or
            $line -match ': error ' -or
            $line -match 'fatal error' -or
            $line -match '\bLNK\d{4}\b' -or
            $line -match '\bRC\d{4}\b' -or
            $line -match '\bMSB\d{4}\b'

        if (-not $isError) {
            continue
        }

        for ($offset = -2; $offset -le 2; ++$offset) {
            $candidate = $index + $offset

            if (
                $candidate -ge 0 -and
                $candidate -lt $lines.Count
            ) {
                $contextLines.Add([string]$lines[$candidate])
            }
        }

        $contextLines.Add("")
    }

    if ($contextLines.Count -gt 0) {
        @(
            "R128 Normalizer v1.5.2 build error summary"
            "重要行の前後2行を抽出しています。"
            ""
            $contextLines
        ) |
            Set-Content $errorsLog -Encoding UTF8
    }
}

try {
    if (-not (Test-Path $projectPath)) {
        throw "プロジェクトが見つかりません: $projectPath"
    }

    $files = @(
        "stdafx.h",
        "main.cpp",
        "resource.h",
        "foo_r128_normalizer.rc",
        "dsp_r128_normalizer.cpp"
    )

    foreach ($file in $files) {
        $source = Join-Path $sourceRoot $file
        $target = Join-Path $projectDir $file

        if (-not (Test-Path $source)) {
            throw "更新用ソースが見つかりません: $source"
        }

        $backup = "$target.before_v1.5.2"

        if (
            (Test-Path $target) -and
            -not (Test-Path $backup)
        ) {
            Copy-Item $target $backup -Force
            Write-UpdateLog "Backup: $backup"
        }

        Copy-Item $source $target -Force
        Write-UpdateLog "Updated: $target"
    }

    # Japanese narrow strings passed to the foobar2000 SDK are UTF-8.
    $projectBackup = "$projectPath.before_v1.5.2"

    if (-not (Test-Path $projectBackup)) {
        Copy-Item $projectPath $projectBackup -Force
        Write-UpdateLog "Backup: $projectBackup"
    }

    [xml]$projectXml = Get-Content `
        -LiteralPath $projectPath `
        -Raw `
        -Encoding UTF8

    $namespaceUri = $projectXml.Project.NamespaceURI
    $namespaceManager = $null

    if ([string]::IsNullOrWhiteSpace($namespaceUri)) {
        $compileNodes = $projectXml.SelectNodes(
            "//ItemDefinitionGroup/ClCompile"
        )
    }
    else {
        $namespaceManager = New-Object `
            System.Xml.XmlNamespaceManager($projectXml.NameTable)
        $namespaceManager.AddNamespace("msb", $namespaceUri)

        $compileNodes = $projectXml.SelectNodes(
            "//msb:ItemDefinitionGroup/msb:ClCompile",
            $namespaceManager
        )
    }

    if (
        $null -eq $compileNodes -or
        $compileNodes.Count -eq 0
    ) {
        throw "vcxproj内のClCompile設定を確認できませんでした。"
    }

    $projectChanged = $false

    foreach ($compileNode in $compileNodes) {
        if ($null -eq $namespaceManager) {
            $optionNode = $compileNode.SelectSingleNode(
                "AdditionalOptions"
            )
        }
        else {
            $optionNode = $compileNode.SelectSingleNode(
                "msb:AdditionalOptions",
                $namespaceManager
            )
        }

        if ($null -eq $optionNode) {
            if ([string]::IsNullOrWhiteSpace($namespaceUri)) {
                $optionNode = $projectXml.CreateElement(
                    "AdditionalOptions"
                )
            }
            else {
                $optionNode = $projectXml.CreateElement(
                    "AdditionalOptions",
                    $namespaceUri
                )
            }

            [void]$compileNode.AppendChild($optionNode)
        }

        $currentOptions = [string]$optionNode.InnerText

        if ($currentOptions -notmatch '(^|\s)/utf-8(\s|$)') {
            $optionNode.InnerText = (
                "/utf-8 " + $currentOptions
            ).Trim()

            $projectChanged = $true
        }
    }

    if ($projectChanged) {
        $writerSettings = New-Object System.Xml.XmlWriterSettings
        $writerSettings.Encoding = New-Object `
            System.Text.UTF8Encoding($true)
        $writerSettings.Indent = $true
        $writerSettings.NewLineChars = "`r`n"
        $writerSettings.NewLineHandling = "Replace"

        $writer = [System.Xml.XmlWriter]::Create(
            $projectPath,
            $writerSettings
        )

        try {
            $projectXml.Save($writer)
        }
        finally {
            $writer.Dispose()
        }

        Write-UpdateLog "Added compiler option: /utf-8"
    }
    else {
        Write-UpdateLog "Compiler option already present: /utf-8"
    }

    $vswhere = Join-Path ${env:ProgramFiles(x86)} `
        "Microsoft Visual Studio\Installer\vswhere.exe"

    if (-not (Test-Path $vswhere)) {
        throw "vswhere.exeが見つかりません。"
    }

    $msbuild = & $vswhere `
        -latest `
        -products * `
        -requires Microsoft.Component.MSBuild `
        -find "MSBuild\**\Bin\MSBuild.exe" |
        Select-Object -First 1

    if (
        [string]::IsNullOrWhiteSpace($msbuild) -or
        -not (Test-Path $msbuild)
    ) {
        throw "MSBuild.exeが見つかりません。"
    }

    Write-Host "v1.5.2のソース更新が完了しました。" `
        -ForegroundColor Cyan
    Write-Host "Release x64をビルドします。"
    Write-Host ""

    $buildStartUtc = [DateTime]::UtcNow

    $arguments = @(
        $projectPath,
        "/m",
        "/t:Rebuild",
        "/p:Configuration=Release",
        "/p:Platform=x64",
        "/v:normal",
        "/nologo",
        "/bl:$binLog",
        "/fl",
        "/flp:LogFile=$fullLog;Verbosity=diagnostic;Encoding=UTF-8"
    )

    $process = Start-Process `
        -FilePath $msbuild `
        -ArgumentList $arguments `
        -Wait `
        -PassThru `
        -NoNewWindow

    Write-ErrorSummary -Path $fullLog

    if ($process.ExitCode -ne 0) {
        if (-not (Test-Path $errorsLog)) {
            "エラー行を抽出できませんでした。build_full_log.txtを確認してください。" |
                Set-Content $errorsLog -Encoding UTF8
        }

        Write-Host ""
        Write-Host "ビルドに失敗しました。" -ForegroundColor Red
        Write-Host "build_errors.txtを添付してください。" `
            -ForegroundColor Yellow
        exit $process.ExitCode
    }

    $dllPath = Join-Path $projectDir `
        "x64\Release\foo_r128_normalizer.dll"

    if (-not (Test-Path $dllPath)) {
        throw "ビルド後のDLLが見つかりません: $dllPath"
    }

    $dll = Get-Item $dllPath

    if ($dll.Length -lt 4096) {
        throw "生成DLLのサイズが小さすぎます: $($dll.Length) bytes"
    }

    if ($dll.LastWriteTimeUtc -lt $buildStartUtc.AddSeconds(-5)) {
        throw "生成DLLの更新時刻が今回のビルドより古い可能性があります。"
    }

    $machine = Get-PeMachine -Path $dllPath

    if ($machine -ne 0x8664) {
        throw (
            "生成DLLがx64ではありません。PE Machine=0x{0:X4}" -f
            $machine
        )
    }

    if (
        -not (
            Test-BinaryContainsAscii `
                -Path $dllPath `
                -Text "1.5.2"
        )
    ) {
        throw "生成DLL内にバージョン1.3.8を確認できません。"
    }

    $hash = (Get-FileHash $dllPath -Algorithm SHA256).Hash

    @(
        "R128 Normalizer v1.5.2 build validation"
        "DLL: $dllPath"
        "Size: $($dll.Length) bytes"
        "LastWriteTimeUtc: $($dll.LastWriteTimeUtc.ToString('o'))"
        ("PE Machine: 0x{0:X4} (x64)" -f $machine)
        "Embedded version: 1.5.2"
        "SHA256: $hash"
    ) |
        Set-Content $validationLog -Encoding UTF8

    Write-UpdateLog "Build succeeded: $dllPath"
    Write-UpdateLog (
        "Validated x64 / embedded version 1.5.2 / SHA256 $hash"
    )

    Write-Host ""
    Write-Host "v1.5.2のビルドに成功しました。" `
        -ForegroundColor Green
    Write-Host $dllPath -ForegroundColor Green
    exit 0
}
catch {
    $message = $_ | Format-List * -Force | Out-String
    $message | Out-File $fullLog -Append -Encoding UTF8
    $message | Out-File $errorsLog -Append -Encoding UTF8

    Write-Host ""
    Write-Host "エラー: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}
