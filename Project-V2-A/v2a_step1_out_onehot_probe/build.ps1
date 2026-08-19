$ErrorActionPreference = 'Stop'

$projectRoot = Split-Path -Parent $PSScriptRoot
$workspaceRoot = Split-Path -Parent $projectRoot

$toolRoot = Join-Path $workspaceRoot 'cc65-2.13.3\cc65'
$binDir = Join-Path $toolRoot 'bin'
$asmIncDir = Join-Path $toolRoot 'asminc'

$cc65 = Join-Path $binDir 'cc65.exe'
$ca65 = Join-Path $binDir 'ca65.exe'
$ld65 = Join-Path $binDir 'ld65.exe'

$src = Join-Path $PSScriptRoot 'main.c'
$asm = Join-Path $PSScriptRoot 'main.s'
$obj = Join-Path $PSScriptRoot 'main.o'
$crt0 = Join-Path $projectRoot 'crt0.s'
$crt0Obj = Join-Path $PSScriptRoot 'crt0.o'
$cfg = Join-Path $projectRoot 'nrom_256_vert.cfg'
$runtime = Join-Path $projectRoot 'runtime.lib'
$outRom = Join-Path $PSScriptRoot 'game_v2a_step1_out_onehot_probe.nes'

foreach ($f in @($cc65, $ca65, $ld65, $src, $crt0, $cfg, $runtime)) {
    if (-not (Test-Path -LiteralPath $f)) {
        throw "Missing required file: $f"
    }
}

Push-Location $PSScriptRoot
try {
    & $cc65 -Oi main.c --add-source
    if ($LASTEXITCODE -ne 0) { throw "cc65 failed" }

    & $ca65 main.s
    if ($LASTEXITCODE -ne 0) { throw "ca65 main failed" }

    Push-Location $projectRoot
    try {
        & $ca65 "-I$asmIncDir" $crt0 -o $crt0Obj
        if ($LASTEXITCODE -ne 0) { throw "ca65 crt0 failed" }
    } finally {
        Pop-Location
    }

    & $ld65 -C $cfg -o $outRom $crt0Obj $obj $runtime
    if ($LASTEXITCODE -ne 0) { throw "ld65 failed" }
} finally {
    Pop-Location
}

Write-Host "V2A step1 OUT one-hot probe ROM built: $outRom"
