[CmdletBinding()]
param([string]$Workspace = '')

$ErrorActionPreference = 'Stop'
if ([string]::IsNullOrWhiteSpace($Workspace)) {
    $Workspace = if ($PSScriptRoot) { Split-Path -Parent $PSScriptRoot } else { (Get-Location).Path }
}
$workspace = [System.IO.Path]::GetFullPath($Workspace)
$assetRoot = Join-Path $workspace 'ExportedProject\Assets'
$inventoryPath = Join-Path $workspace 'Docs\ASSET_GUID_INVENTORY.csv'
$scriptRefsPath = Join-Path $workspace 'Docs\SCENE_PREFAB_SCRIPT_GUIDS.csv'
$unresolvedRefsPath = Join-Path $workspace 'Docs\UNRESOLVED_ASSET_REFERENCES.csv'
$auditPath = Join-Path $workspace 'Docs\ASSET_REFERENCE_AUDIT.md'
$utf8 = New-Object System.Text.UTF8Encoding($false)

function ConvertTo-CsvField([object]$Value) {
    $text = if ($null -eq $Value) { '' } else { [string]$Value }
    return '"' + $text.Replace('"', '""') + '"'
}

function Get-RelativePath([string]$Path) {
    return $Path.Substring($assetRoot.Length + 1).Replace('\', '/')
}

function Get-MetaGuid([string]$MetaPath) {
    if (-not (Test-Path -LiteralPath $MetaPath -PathType Leaf)) { return '' }
    $match = Select-String -LiteralPath $MetaPath -Pattern '^guid:\s*([0-9a-fA-F]{32})\s*$' | Select-Object -First 1
    if ($null -eq $match) { return '' }
    return $match.Matches[0].Groups[1].Value.ToLowerInvariant()
}

$allFiles = @(Get-ChildItem -LiteralPath $assetRoot -Recurse -Force -File | Sort-Object FullName)
$metaFiles = @($allFiles | Where-Object Extension -EQ '.meta')
$assetFiles = @($allFiles | Where-Object Extension -NE '.meta')
$guidToPaths = @{}

foreach ($meta in $metaFiles) {
    $guid = Get-MetaGuid $meta.FullName
    if ([string]::IsNullOrEmpty($guid)) { continue }
    if (-not $guidToPaths.ContainsKey($guid)) {
        $guidToPaths[$guid] = New-Object System.Collections.Generic.List[string]
    }
    $guidToPaths[$guid].Add((Get-RelativePath $meta.FullName.Substring(0, $meta.FullName.Length - 5)))
}

$inventoryWriter = New-Object System.IO.StreamWriter($inventoryPath, $false, $utf8)
try {
    $inventoryWriter.WriteLine('relative_path,extension,bytes,meta_status,guid')
    foreach ($asset in $assetFiles) {
        $metaPath = $asset.FullName + '.meta'
        $guid = Get-MetaGuid $metaPath
        $metaStatus = if (Test-Path -LiteralPath $metaPath -PathType Leaf) { 'Present' } else { 'Missing' }
        $fields = @(
            (ConvertTo-CsvField (Get-RelativePath $asset.FullName)),
            (ConvertTo-CsvField $asset.Extension.ToLowerInvariant()),
            (ConvertTo-CsvField $asset.Length),
            (ConvertTo-CsvField $metaStatus),
            (ConvertTo-CsvField $guid)
        )
        $inventoryWriter.WriteLine($fields -join ',')
    }
}
finally {
    $inventoryWriter.Dispose()
}

$yamlExtensions = @('.unity', '.prefab', '.asset', '.mat', '.controller', '.anim', '.overridecontroller', '.playable')
$scenePrefabExtensions = @('.unity', '.prefab')
$guidRegex = [regex]'guid:\s*([0-9a-fA-F]{32})'
$scriptRegex = [regex]'m_Script:\s*\{[^\r\n}]*guid:\s*([0-9a-fA-F]{32})'
$referenceCount = 0
$unresolved = New-Object System.Collections.Generic.List[object]
$scriptReferences = New-Object System.Collections.Generic.List[object]

foreach ($source in $assetFiles | Where-Object { $yamlExtensions -contains $_.Extension.ToLowerInvariant() }) {
    try { $text = [System.IO.File]::ReadAllText($source.FullName) } catch { continue }
    $relative = Get-RelativePath $source.FullName
    foreach ($line in [System.IO.File]::ReadLines($source.FullName)) {
        $propertyMatch = [regex]::Match($line, '^\s*([A-Za-z0-9_]+):')
        $property = if ($propertyMatch.Success) { $propertyMatch.Groups[1].Value } else { '' }
        foreach ($match in $guidRegex.Matches($line)) {
            $guid = $match.Groups[1].Value.ToLowerInvariant()
            if ($guid -match '^0{16}[ef]0{15}$' -or $guid -eq ('0' * 32)) { continue }
            $referenceCount++
            if (-not $guidToPaths.ContainsKey($guid)) {
                $unresolved.Add([pscustomobject]@{
                    Source = $relative
                    Extension = $source.Extension.ToLowerInvariant()
                    Property = $property
                    Guid = $guid
                })
            }
        }
    }

    if ($scenePrefabExtensions -contains $source.Extension.ToLowerInvariant()) {
        foreach ($match in $scriptRegex.Matches($text)) {
            $guid = $match.Groups[1].Value.ToLowerInvariant()
            $resolved = if ($guidToPaths.ContainsKey($guid)) { $guidToPaths[$guid][0] } else { '' }
            $status = if ($resolved) { 'Resolved' } else { 'Unresolved' }
            $scriptReferences.Add([pscustomobject]@{
                Source = $relative
                SourceType = $source.Extension.ToLowerInvariant()
                Guid = $guid
                ScriptPath = $resolved
                Status = $status
            })
        }
    }
}

$scriptWriter = New-Object System.IO.StreamWriter($scriptRefsPath, $false, $utf8)
try {
    $scriptWriter.WriteLine('source_path,source_type,script_guid,script_path,status')
    foreach ($item in $scriptReferences | Sort-Object Source, Guid -Unique) {
        $fields = @(
            (ConvertTo-CsvField $item.Source),
            (ConvertTo-CsvField $item.SourceType),
            (ConvertTo-CsvField $item.Guid),
            (ConvertTo-CsvField $item.ScriptPath),
            (ConvertTo-CsvField $item.Status)
        )
        $scriptWriter.WriteLine($fields -join ',')
    }
}
finally {
    $scriptWriter.Dispose()
}

$duplicates = @($guidToPaths.GetEnumerator() | Where-Object { $_.Value.Count -gt 1 } | Sort-Object Name)
$missingMeta = @($assetFiles | Where-Object { -not (Test-Path -LiteralPath ($_.FullName + '.meta') -PathType Leaf) })
$unresolvedUnique = @($unresolved | Sort-Object Source, Property, Guid -Unique)
$unresolvedScripts = @($scriptReferences | Where-Object Status -EQ 'Unresolved' | Sort-Object Source, Guid -Unique)
$unresolvedWriter = New-Object System.IO.StreamWriter($unresolvedRefsPath, $false, $utf8)
try {
    $unresolvedWriter.WriteLine('source_path,source_type,property,guid')
    foreach ($item in $unresolvedUnique) {
        $fields = @(
            (ConvertTo-CsvField $item.Source),
            (ConvertTo-CsvField $item.Extension),
            (ConvertTo-CsvField $item.Property),
            (ConvertTo-CsvField $item.Guid)
        )
        $unresolvedWriter.WriteLine($fields -join ',')
    }
}
finally {
    $unresolvedWriter.Dispose()
}

$shaderCandidates = @($unresolvedUnique | Where-Object Property -Match 'Shader')
$animatorCandidates = @($unresolvedUnique | Where-Object Property -Match 'Controller|Avatar|Motion|Animation')
$audioCandidates = @($unresolvedUnique | Where-Object Property -Match 'Audio|Clip')
$tmpCandidates = @($unresolvedUnique | Where-Object Property -Match 'fontAsset|FontAsset|Font')
$auditLines = @(
    '# Asset Reference Audit',
    '',
    "생성일: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss K')",
    '',
    '## 전체 결과',
    '',
    "- 비-meta 파일: $($assetFiles.Count)",
    "- meta 파일: $($metaFiles.Count)",
    "- meta 누락 파일: $($missingMeta.Count)",
    "- 고유 GUID: $($guidToPaths.Count)",
    "- 중복 GUID: $($duplicates.Count)",
    "- 조사한 YAML GUID 참조: $referenceCount",
    "- 해소되지 않은 고유 파일/GUID 조합: $($unresolvedUnique.Count)",
    "- Scene/Prefab script 참조: $($scriptReferences.Count)",
    "- 해소되지 않은 Scene/Prefab script 참조: $($unresolvedScripts.Count)",
    "- 누락 Shader 후보: $($shaderCandidates.Count)",
    "- 누락 Animator/Animation 후보: $($animatorCandidates.Count)",
    "- 누락 Audio 후보: $($audioCandidates.Count)",
    "- 누락 TMP/Font 후보: $($tmpCandidates.Count)",
    '',
    '전체 파일/GUID 상태는 `ASSET_GUID_INVENTORY.csv`, Scene/Prefab의 script 참조는 `SCENE_PREFAB_SCRIPT_GUIDS.csv`, 해소되지 않은 상세 참조는 `UNRESOLVED_ASSET_REFERENCES.csv`에 기록했다.',
    '',
    '## 중복 GUID'
)
if ($duplicates.Count -eq 0) {
    $auditLines += ''
    $auditLines += '- 없음'
} else {
    foreach ($duplicate in $duplicates | Select-Object -First 100) {
        $auditLines += "- ``$($duplicate.Name)``: $($duplicate.Value -join ', ')"
    }
}
$auditLines += @('', '## 해소되지 않은 Scene/Prefab script 참조')
if ($unresolvedScripts.Count -eq 0) {
    $auditLines += ''
    $auditLines += '- 없음'
} else {
    foreach ($item in $unresolvedScripts | Select-Object -First 200) {
        $auditLines += "- ``$($item.Source)`` → ``$($item.Guid)``"
    }
}
$auditLines += @('', '## 해소되지 않은 YAML 참조 분포', '', '| 참조 파일 확장자 | 고유 파일/GUID 조합 |', '|---|---:|')
foreach ($group in $unresolvedUnique | Group-Object Extension | Sort-Object Count -Descending) {
    $auditLines += "| ``$($group.Name)`` | $($group.Count) |"
}
$auditLines += @(
    '',
    '해소되지 않은 참조에는 Unity built-in 이외의 package/Asset Store 파일, export 누락 파일, 실제 Missing Reference가 함께 포함될 수 있다. 자동 생성하거나 삭제하지 않는다.',
    '',
    '## P0 수직 조각 선정',
    '',
    '- Scene: 신규 `Bootstrap`, 안전한 `LoadingScene`, 첫 Gameplay 기준인 Busan',
    '- Core: `StateManager`, `IStateinterface`, `FishGameManager`, `StageManager`, `DataManager`',
    '- Gameplay: `MainObjectCont`, `RodCont`, `TensionCont`',
    '- 데이터: `StageData_stage_busan` 및 직접 참조되는 map/fish/quest 설정',
    '- 입력: 실제 Serial/Card 대신 keyboard/mouse simulator와 adapter',
    '',
    '## Placeholder 후보',
    '',
    '- 라이선스와 버전이 확정되지 않은 Spine 로딩 애니메이션',
    '- 실제 Serial, IMU, Reel Motor, Card 장치 연결',
    '- 결제와 외부 네트워크 서비스',
    '- 원본 패키지를 확인하기 전의 누락 Shader/Material 효과',
    '- 첫 Gameplay 수직 조각에서 아직 닫히지 않은 환경·연출 자산'
)
[System.IO.File]::WriteAllLines($auditPath, $auditLines, $utf8)

Write-Output "Inventory rows: $($assetFiles.Count)"
Write-Output "Duplicate GUIDs: $($duplicates.Count)"
Write-Output "Unresolved script references: $($unresolvedScripts.Count)"
