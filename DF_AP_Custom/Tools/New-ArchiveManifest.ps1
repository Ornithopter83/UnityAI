[CmdletBinding()]
param([string]$Workspace = '')

$ErrorActionPreference = 'Stop'
if ([string]::IsNullOrWhiteSpace($Workspace)) {
    $Workspace = if ($PSScriptRoot) { Split-Path -Parent $PSScriptRoot } else { (Get-Location).Path }
}
$workspace = [System.IO.Path]::GetFullPath($Workspace)
$outputPath = Join-Path $workspace 'Docs\ORIGINAL_ARCHIVE_MANIFEST.csv'
$summaryPath = Join-Path $workspace 'Docs\ORIGINAL_ARCHIVE_MANIFEST_SUMMARY.md'
$archiveNames = @('ExportedProject', 'AuxiliaryFiles')
$utf8 = New-Object System.Text.UTF8Encoding($false)

function ConvertTo-CsvField([object]$Value) {
    $text = if ($null -eq $Value) { '' } else { [string]$Value }
    return '"' + $text.Replace('"', '""') + '"'
}

$archives = foreach ($archiveName in $archiveNames) {
    $archivePath = Join-Path $workspace $archiveName
    if (-not (Test-Path -LiteralPath $archivePath -PathType Container)) {
        throw "Archive directory was not found: $archivePath"
    }

    $files = @(Get-ChildItem -LiteralPath $archivePath -Recurse -Force -File | Sort-Object FullName)
    [pscustomobject]@{
        Name = $archiveName
        Path = $archivePath
        Files = $files
        Count = $files.Count
        Bytes = [long](($files | Measure-Object Length -Sum).Sum)
        LatestWriteUtc = ($files | Sort-Object LastWriteTimeUtc -Descending | Select-Object -First 1).LastWriteTimeUtc
    }
}

$writer = New-Object System.IO.StreamWriter($outputPath, $false, $utf8)
try {
    $writer.WriteLine('archive,relative_path,bytes,last_write_utc,sha256')
    foreach ($archive in $archives) {
        foreach ($file in $archive.Files) {
            $relativePath = $file.FullName.Substring($archive.Path.Length + 1).Replace('\', '/')
            $hash = (Get-FileHash -LiteralPath $file.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
            $fields = @(
                (ConvertTo-CsvField $archive.Name),
                (ConvertTo-CsvField $relativePath),
                (ConvertTo-CsvField $file.Length),
                (ConvertTo-CsvField $file.LastWriteTimeUtc.ToString('o')),
                (ConvertTo-CsvField $hash)
            )
            $writer.WriteLine($fields -join ',')
        }
    }
}
finally {
    $writer.Dispose()
}

foreach ($archive in $archives) {
    $after = @(Get-ChildItem -LiteralPath $archive.Path -Recurse -Force -File)
    $afterBytes = [long](($after | Measure-Object Length -Sum).Sum)
    $afterLatest = ($after | Sort-Object LastWriteTimeUtc -Descending | Select-Object -First 1).LastWriteTimeUtc
    if ($after.Count -ne $archive.Count -or $afterBytes -ne $archive.Bytes -or $afterLatest -ne $archive.LatestWriteUtc) {
        throw "Archive changed during hashing: $($archive.Name)"
    }
}

$manifestHash = (Get-FileHash -LiteralPath $outputPath -Algorithm SHA256).Hash.ToLowerInvariant()
$totalFiles = ($archives | Measure-Object Count -Sum).Sum
$totalBytes = [long](($archives | Measure-Object Bytes -Sum).Sum)
$requiredGiB = [math]::Round($totalBytes / 1GB, 2)
$summaryLines = @(
    '# Original Archive Manifest Summary',
    '',
    "생성일: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss K')",
    '',
    '| 원본 영역 | 파일 수 | 바이트 |',
    '|---|---:|---:|'
)
foreach ($archive in $archives) {
    $summaryLines += "| ``$($archive.Name)/`` | $($archive.Count) | $($archive.Bytes) |"
}
$summaryLines += @(
    "| 합계 | $totalFiles | $totalBytes |",
    '',
    "- Manifest: ``Docs/ORIGINAL_ARCHIVE_MANIFEST.csv``",
    "- Manifest SHA-256: ``$manifestHash``",
    '- 각 행은 원본 영역, 상대 경로, 바이트, UTC 수정 시각, SHA-256을 기록한다.',
    '- 생성 전후 파일 수, 총 크기, 최신 수정 시각이 동일함을 확인했다.',
    "- 별도 물리 백업에는 최소 $requiredGiB GiB와 파일시스템 여유 공간이 필요하다.",
    '- 별도 백업 위치는 사용자가 저장 장치를 지정할 때 확정하며, 현재 워크스페이스에는 중복 복사하지 않는다.'
)
[System.IO.File]::WriteAllLines($summaryPath, $summaryLines, $utf8)

Write-Output "Manifest rows: $totalFiles"
Write-Output "Manifest SHA-256: $manifestHash"
