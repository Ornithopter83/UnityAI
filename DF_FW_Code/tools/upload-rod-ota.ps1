param(
    [string]$Port,
    [Parameter(Mandatory = $true)][string]$Firmware,
    [Parameter(Mandatory = $true)][string]$Version,
    [switch]$DryRun
)

$ErrorActionPreference = 'Stop'
$MaxPayload = 96
$MaxImage = 1250000

function Set-U32([byte[]]$Buffer, [int]$Offset, [uint32]$Value) {
    $Buffer[$Offset] = [byte](($Value -shr 24) -band 0xff)
    $Buffer[$Offset + 1] = [byte](($Value -shr 16) -band 0xff)
    $Buffer[$Offset + 2] = [byte](($Value -shr 8) -band 0xff)
    $Buffer[$Offset + 3] = [byte]($Value -band 0xff)
}

function Get-U32([byte[]]$Buffer, [int]$Offset) {
    return [uint32](([uint32]$Buffer[$Offset] -shl 24) -bor ([uint32]$Buffer[$Offset + 1] -shl 16) -bor ([uint32]$Buffer[$Offset + 2] -shl 8) -bor [uint32]$Buffer[$Offset + 3])
}

function Get-Crc32([byte[]]$Data, [int]$Length) {
    [uint32]$crc = [uint32]::MaxValue
    for ($index = 0; $index -lt $Length; $index++) {
        $crc = $crc -bxor [uint32]$Data[$index]
        for ($bit = 0; $bit -lt 8; $bit++) {
            if ($crc -band 1) { $crc = [uint32](($crc -shr 1) -bxor [uint32]3988292384) }
            else { $crc = [uint32]($crc -shr 1) }
        }
    }
    return [uint32]($crc -bxor [uint32]::MaxValue)
}

function New-Frame([byte]$Type, [uint32]$Session, [uint32]$Sequence, [byte[]]$Payload) {
    $length = 15 + $Payload.Length + 4
    $frame = New-Object byte[] $length
    $frame[0] = 68; $frame[1] = 70; $frame[2] = 82; $frame[3] = 79
    $frame[4] = 1; $frame[5] = $Type
    Set-U32 $frame 6 $Session
    Set-U32 $frame 10 $Sequence
    $frame[14] = [byte]$Payload.Length
    if ($Payload.Length) { [Array]::Copy($Payload, 0, $frame, 15, $Payload.Length) }
    Set-U32 $frame (15 + $Payload.Length) (Get-Crc32 $frame (15 + $Payload.Length))
    return $frame
}

function Convert-HexToBytes([string]$Text) {
    if ($Text.Length % 2) { throw 'Odd hexadecimal response length.' }
    $result = New-Object byte[] ($Text.Length / 2)
    for ($index = 0; $index -lt $result.Length; $index++) { $result[$index] = [Convert]::ToByte($Text.Substring($index * 2, 2), 16) }
    return $result
}

function Convert-BytesToHex([byte[]]$Data) {
    return ([BitConverter]::ToString($Data)).Replace('-', '')
}

function Test-Ack([byte[]]$Ack, [byte]$RequestType, [uint32]$Session, [uint32]$Sequence, [uint32]$ExpectedNext, [uint32]$ExpectedWritten) {
    if ($Ack.Length -lt 29) { return $false }
    if (($Ack[0] -ne 68) -or ($Ack[1] -ne 70) -or ($Ack[2] -ne 82) -or ($Ack[3] -ne 79) -or ($Ack[4] -ne 1) -or ($Ack[5] -ne 5) -or ($Ack[14] -ne 10)) { return $false }
    if ((Get-Crc32 $Ack ($Ack.Length - 4)) -ne (Get-U32 $Ack ($Ack.Length - 4))) { return $false }
    if ((Get-U32 $Ack 6) -ne $Session -or (Get-U32 $Ack 10) -ne $Sequence) { return $false }
    if ($Ack[15] -ne $RequestType) { return $false }
    if ($Ack[16] -ne 0) { throw "Rod OTA error status $($Ack[16]) on frame type $RequestType, sequence $Sequence." }
    return ((Get-U32 $Ack 17) -eq $ExpectedNext) -and ((Get-U32 $Ack 21) -eq $ExpectedWritten)
}

function Send-Frame([System.IO.Ports.SerialPort]$Serial, [byte[]]$Frame, [byte]$Type, [uint32]$Session, [uint32]$Sequence, [uint32]$ExpectedNext, [uint32]$ExpectedWritten) {
    $hex = Convert-BytesToHex $Frame
    for ($attempt = 1; $attempt -le 5; $attempt++) {
        $Serial.Write("`$OR$hex%")
        $deadline = [DateTime]::UtcNow.AddMilliseconds(1500)
        $received = ''
        while ([DateTime]::UtcNow -lt $deadline) {
            $received += $Serial.ReadExisting()
            $matches = [regex]::Matches($received, '\$OR([0-9A-Fa-f]+)%')
            foreach ($match in $matches) {
                $ack = Convert-HexToBytes $match.Groups[1].Value
                if (Test-Ack $ack $Type $Session $Sequence $ExpectedNext $ExpectedWritten) { return }
            }
            Start-Sleep -Milliseconds 5
        }
    }
    throw "No valid Rod ACK for frame type $Type, sequence $Sequence."
}

$firmwarePath = (Resolve-Path -LiteralPath $Firmware).Path
[byte[]]$image = [IO.File]::ReadAllBytes($firmwarePath)
if (($image.Length -eq 0) -or ($image.Length -gt $MaxImage)) { throw "Firmware size must be 1..$MaxImage bytes." }
$versionBytes = [Text.Encoding]::ASCII.GetBytes($Version)
if (($versionBytes.Length -eq 0) -or ($versionBytes.Length -gt 24)) { throw 'Version must contain 1..24 ASCII bytes.' }
$sha = [Security.Cryptography.SHA256]::Create()
[byte[]]$digest = $sha.ComputeHash($image)
$sha.Dispose()
[uint32]$session = [uint32](([DateTimeOffset]::UtcNow.ToUnixTimeSeconds() -band 0x7fffffff) + 1)

$startPayload = New-Object byte[] (38 + $versionBytes.Length)
$startPayload[0] = 1
Set-U32 $startPayload 1 ([uint32]$image.Length)
[Array]::Copy($digest, 0, $startPayload, 5, 32)
$startPayload[37] = [byte]$versionBytes.Length
[Array]::Copy($versionBytes, 0, $startPayload, 38, $versionBytes.Length)
$startFrame = New-Frame 1 $session 0 $startPayload

[uint32]$sequence = 0
[uint32]$written = 0
$dataFrames = New-Object System.Collections.Generic.List[byte[]]
while ($written -lt $image.Length) {
    $count = [Math]::Min($MaxPayload, $image.Length - $written)
    $payload = New-Object byte[] $count
    [Array]::Copy($image, $written, $payload, 0, $count)
    $dataFrames.Add((New-Frame 2 $session $sequence $payload))
    $written += [uint32]$count
    $sequence++
}
$finishFrame = New-Frame 3 $session $sequence ([byte[]]@())
$abortFrame = New-Frame 4 $session $sequence ([byte[]]@())

if ($DryRun) {
    Write-Host "[DRY RUN] $($image.Length) bytes, $($dataFrames.Count) data frames, SHA256 $(Convert-BytesToHex $digest)"
    exit 0
}
if ([string]::IsNullOrWhiteSpace($Port)) { throw '-Port is required unless -DryRun is used.' }

$serial = [System.IO.Ports.SerialPort]::new($Port, 115200, 'None', 8, 'One')
$serial.DtrEnable = $false
$serial.RtsEnable = $false
$serial.ReadTimeout = 50
$serial.WriteTimeout = 1000
$serial.Open()
$started = $false
try {
    $serial.DiscardInBuffer()
    $started = $true
    Send-Frame $serial $startFrame 1 $session 0 0 0
    $written = 0
    for ($index = 0; $index -lt $dataFrames.Count; $index++) {
        $written += [uint32]($dataFrames[$index].Length - 19)
        Send-Frame $serial $dataFrames[$index] 2 $session ([uint32]$index) ([uint32]($index + 1)) $written
        if ((($index + 1) % 256) -eq 0) { Write-Host "[OTA] $written / $($image.Length) bytes" }
    }
    Send-Frame $serial $finishFrame 3 $session $sequence $sequence ([uint32]$image.Length)
    $started = $false
    Write-Host "[OTA] Completed: $Version, $($image.Length) bytes"
}
catch {
    if ($started -and $serial.IsOpen) {
        $abortHex = Convert-BytesToHex $abortFrame
        $serial.Write("`$OR$abortHex%")
        Start-Sleep -Milliseconds 250
    }
    throw
}
finally {
    if ($serial.IsOpen) { $serial.Close() }
    $serial.Dispose()
}
