$ErrorActionPreference = 'Stop'

$projectRoot = Split-Path -Parent $PSScriptRoot
$workspaceRoot = Split-Path -Parent $projectRoot

$toolRoot = Join-Path $workspaceRoot 'cc65-2.13.3\cc65'
$binDir = Join-Path $toolRoot 'bin'
$asmIncDir = Join-Path $toolRoot 'asminc'

$cc65 = Join-Path $binDir 'cc65.exe'
$ca65 = Join-Path $binDir 'ca65.exe'
$ld65 = Join-Path $binDir 'ld65.exe'

$crt0 = Join-Path $projectRoot 'crt0.s'
$cfg = Join-Path $projectRoot 'nrom_256_vert.cfg'
$runtime = Join-Path $projectRoot 'runtime.lib'

foreach ($f in @($cc65, $ca65, $ld65, $crt0, $cfg, $runtime)) {
    if (-not (Test-Path -LiteralPath $f)) {
        throw "Missing required file: $f"
    }
}

function Build-One {
    param(
        [string]$SourceName,
        [string]$OutName
    )

    $src = Join-Path $PSScriptRoot $SourceName
    $base = [System.IO.Path]::GetFileNameWithoutExtension($SourceName)
    $asm = Join-Path $PSScriptRoot ($base + '.s')
    $obj = Join-Path $PSScriptRoot ($base + '.o')
    $crt0Obj = Join-Path $PSScriptRoot ($base + '_crt0.o')
    $outRom = Join-Path $PSScriptRoot $OutName

    if (-not (Test-Path -LiteralPath $src)) {
        throw "Missing source: $src"
    }

    Push-Location $PSScriptRoot
    try {
        & $cc65 -Oi $SourceName --add-source
        if ($LASTEXITCODE -ne 0) { throw "cc65 failed for $SourceName" }

        & $ca65 ([System.IO.Path]::GetFileName($asm))
        if ($LASTEXITCODE -ne 0) { throw "ca65 failed for $SourceName" }

        Push-Location $projectRoot
        try {
            & $ca65 "-I$asmIncDir" $crt0 -o $crt0Obj
            if ($LASTEXITCODE -ne 0) { throw "ca65 crt0 failed for $SourceName" }
        } finally {
            Pop-Location
        }

        & $ld65 -C $cfg -o $outRom $crt0Obj $obj $runtime
        if ($LASTEXITCODE -ne 0) { throw "ld65 failed for $SourceName" }
    } finally {
        Pop-Location
    }

    Write-Host "Built: $outRom"
}

Build-One -SourceName 'main_out0.c' -OutName 'V2_OUT0_FIX.nes'
Build-One -SourceName 'main_out1.c' -OutName 'V2_OUT1_FIX.nes'
Build-One -SourceName 'main_out2.c' -OutName 'V2_OUT2_FIX.nes'
