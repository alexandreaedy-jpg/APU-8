$ErrorActionPreference = 'Stop'

$toolRoot = Join-Path $PSScriptRoot '..\cc65-2.13.3\cc65'
$binDir = Join-Path $toolRoot 'bin'
$asmIncDir = Join-Path $toolRoot 'asminc'

$cc65 = Join-Path $binDir 'cc65.exe'
$ca65 = Join-Path $binDir 'ca65.exe'
$ld65 = Join-Path $binDir 'ld65.exe'

$src = Join-Path $PSScriptRoot 'main.c'
$dmcAsm = Join-Path $PSScriptRoot 'dmc_samples.s'
$crt0 = Join-Path $PSScriptRoot 'crt0.s'
$cfg = Join-Path $PSScriptRoot 'nrom_256_vert.cfg'
$runtime = Join-Path $PSScriptRoot 'runtime.lib'
$outRom = Join-Path $PSScriptRoot 'game_opto_rx.nes'

foreach ($f in @($cc65, $ca65, $ld65, $src, $dmcAsm, $crt0, $cfg, $runtime)) {
    if (-not (Test-Path -LiteralPath $f)) {
        throw "Missing required file: $f"
    }
}

Push-Location $PSScriptRoot
try {
    & $cc65 -Oi -DV18_1_SHIFT_D0_ONLY=1 -DCONTROLLER_RX_ENABLED=1 -DCONTROLLER_RX_MASK=1 -DCONTROLLER_RX_D3_MASK=8 -DCONTROLLER_RX_D4_MASK=16 -DCONTROLLER_RX_INVERT=1 -DCONTROLLER_RX_D3_INVERT=0 -DCONTROLLER_RX_D4_INVERT=1 -DCONTROLLER_RX_D3_REQUIRE_STABLE=0 -DCONTROLLER_RX_LATCH_HIGH_DELAY=16 -DCONTROLLER_RX_LATCH_LOW_DELAY=32 -DCONTROLLER_RX_ADVANCE_HIGH_DELAY=24 -DCONTROLLER_RX_ADVANCE_LOW_DELAY=24 -DCONTROLLER_RX_BIT_SETTLE_DELAY=12 -DCONTROLLER_RX_P2_ONLY=0 -DCONTROLLER_RX_FAST_P2=1 -DCONTROLLER_RX_FAST_IDLE_DELAY=0 -DCONTROLLER_RX_POLL_DIV=1 -DV2_EXPANSION_PROTO1_ENABLED=0 main.c --add-source
    if ($LASTEXITCODE -ne 0) { throw "cc65 failed" }

    & $ca65 main.s
    if ($LASTEXITCODE -ne 0) { throw "ca65 main failed" }

    & $ca65 dmc_samples.s -o dmc_samples.o
    if ($LASTEXITCODE -ne 0) { throw "ca65 dmc samples failed" }

    & $ca65 "-I$asmIncDir" crt0.s -o crt0.o
    if ($LASTEXITCODE -ne 0) { throw "ca65 crt0 failed" }

    & $ld65 -C $cfg -o $outRom crt0.o main.o dmc_samples.o $runtime
    if ($LASTEXITCODE -ne 0) { throw "ld65 failed" }
} finally {
    Pop-Location
}

Write-Host "Opto RX ROM built: $outRom"
