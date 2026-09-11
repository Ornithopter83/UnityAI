[CmdletBinding()]
param(
    [string]$RepositoryRoot
)

$ErrorActionPreference = 'Stop'

function Get-DFSourceVersion {
    param([string]$SourcePath, [string]$Symbol, [string]$Prefix)

    $source = [IO.File]::ReadAllText($SourcePath)
    # Ignore comments, while preserving string literals and line boundaries.
    $tokens = '"(?:\\.|[^"\\])*"|/\*[\s\S]*?\*/|//[^\r\n]*'
    $source = [regex]::Replace($source, $tokens, [System.Text.RegularExpressions.MatchEvaluator]{
        param($item)
        if ($item.Value.StartsWith('//') -or $item.Value.StartsWith('/*')) {
            return [regex]::Replace($item.Value, '[^\r\n]', ' ')
        }
        return $item.Value
    })
    if ($source -match '(?m)^\s*#\s*(if|ifdef|ifndef|elif|else|endif)\b') {
        throw "Conditional version definitions are not supported: $SourcePath"
    }

    $definition = '(?m)^\s*String\s+' + [regex]::Escape($Symbol) + '\s*=\s*"(?<version>[^"\r\n]*)"\s*;[ \t]*\r?$'
    $definitions = [regex]::Matches($source, $definition)
    if ($definitions.Count -ne 1) {
        throw "Expected exactly one String $Symbol = `"...`"; definition in $SourcePath"
    }
    $version = $definitions[0].Groups['version'].Value
    $format = '^' + [regex]::Escape($Prefix) + '[0-9]+(?:\.[0-9]+){3}$'
    if ($version -cnotmatch $format -or $version.Length -gt 24) {
        throw "Invalid $Symbol in ${SourcePath}: use ${Prefix}1.0.0.0 format, up to 24 characters."
    }
    return $version
}

try {
    if (-not $RepositoryRoot) { $RepositoryRoot = Split-Path -Parent $PSScriptRoot }
    $mainVersion = Get-DFSourceVersion (Join-Path $RepositoryRoot 'firmware\DF_Main\src\Version.cpp') 'mainVer' 'Vm'
    $rodVersion = Get-DFSourceVersion (Join-Path $RepositoryRoot 'firmware\DF_Rod\src\Version.cpp') 'rodVer' 'Vr'
    # Publish both only after validation; callers must never retain stale versions.
    Write-Output "DF_MAIN_VERSION=$mainVersion"
    Write-Output "DF_ROD_VERSION=$rodVersion"
    exit 0
}
catch {
    [Console]::Error.WriteLine('[ERROR] Firmware version: ' + $_.Exception.Message)
    exit 1
}