$ErrorActionPreference = 'Stop'

$toolRoot = Join-Path $PSScriptRoot '..\cc65-2.13.3\cc65'
$binDir = Join-Path $toolRoot 'bin'
$asmIncDir = Join-Path $toolRoot 'asminc'

$cc65 = Join-Path $binDir 'cc65.exe'
$ca65 = Join-Path $binDir 'ca65.exe'
$ld65 = Join-Path $binDir 'ld65.exe'

$src = Join-Path $PSScriptRoot 'main.c'
$asm = Join-Path $PSScriptRoot 'main.s'
$obj = Join-Path $PSScriptRoot 'main.o'
$dmcAsm = Join-Path $PSScriptRoot 'dmc_samples.s'
$dmcObj = Join-Path $PSScriptRoot 'dmc_samples.o'
$crt0 = Join-Path $PSScriptRoot 'crt0.s'
$crt0Obj = Join-Path $PSScriptRoot 'crt0.o'
$cfg = Join-Path $PSScriptRoot 'nrom_256_vert.cfg'
$runtime = Join-Path $PSScriptRoot 'runtime.lib'
$outRom = Join-Path $PSScriptRoot 'game.nes'

foreach ($f in @($cc65, $ca65, $ld65, $src, $dmcAsm, $crt0, $cfg, $runtime)) {
    if (-not (Test-Path -LiteralPath $f)) {
        throw "Missing required file: $f"
    }
}

Push-Location $PSScriptRoot
try {
    # --- Configuration ROM V16-DN3 ---
    # MASK=1 : D0 = bit 0 du port $4017 (NES pin 2)
    # D3_MASK=8 : D3 = bit 3 du port $4017 (NES pin 6, port 2 uniquement)
    # INVERT=1 : le hardware NES inverse les signaux (LOW = bit 1 lu)
    # P2_ONLY=0 : toutes les voix actives (P1+P2+TRI+NOISE)
    # FAST_P2=1 : accepte magic byte 0x5A (format 12 bytes)
    # COMPACT=1 : format compact 12 bytes
    & $cc65 -Oi -DCONTROLLER_RX_ENABLED=1 -DCONTROLLER_RX_MASK=1 -DCONTROLLER_RX_D3_MASK=8 -DCONTROLLER_RX_INVERT=1 -DCONTROLLER_RX_D3_INVERT=1 -DCONTROLLER_RX_COMPACT=1 -DCONTROLLER_RX_P2_ONLY=0 -DCONTROLLER_RX_FAST_P2=1 -DCONTROLLER_RX_FAST_IDLE_DELAY=0 -DCONTROLLER_RX_POLL_DIV=1 main.c --add-source
    if ($LASTEXITCODE -ne 0) { throw "cc65 failed" }

    & $ca65 main.s
    if ($LASTEXITCODE -ne 0) { throw "ca65 main failed" }

    & $ca65 dmc_samples.s -o $dmcObj
    if ($LASTEXITCODE -ne 0) { throw "ca65 dmc samples failed" }

    & $ca65 "-I$asmIncDir" crt0.s -o $crt0Obj
    if ($LASTEXITCODE -ne 0) { throw "ca65 crt0 failed" }

    & $ld65 -C $cfg -o $outRom $crt0Obj $obj $dmcObj $runtime
    if ($LASTEXITCODE -ne 0) { throw "ld65 failed" }
} finally {
    Pop-Location
}

Write-Host "Separate-channels ROM built: $outRom"
