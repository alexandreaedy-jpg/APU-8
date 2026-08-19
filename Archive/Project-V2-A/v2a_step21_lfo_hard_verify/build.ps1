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
$samplesSrc = Join-Path $projectRoot 'dmc_samples.s'

foreach ($f in @($cc65, $ca65, $ld65, $crt0, $cfg, $runtime, $samplesSrc)) {
    if (-not (Test-Path -LiteralPath $f)) {
        throw "Missing required file: $f"
    }
}

$obj = Join-Path $PSScriptRoot 'main.o'
$crt0Obj = Join-Path $PSScriptRoot 'main_crt0.o'
$samplesObj = Join-Path $PSScriptRoot 'dmc_samples.o'
$outRom = Join-Path $PSScriptRoot 'V2_LFO_HARD_VERIFY.nes'

Push-Location $PSScriptRoot
try {
    & $cc65 -Oi 'main.c' --add-source
    if ($LASTEXITCODE -ne 0) { throw 'cc65 failed for main.c' }

    & $ca65 'main.s'
    if ($LASTEXITCODE -ne 0) { throw 'ca65 failed for main.s' }

    Push-Location $projectRoot
    try {
        & $ca65 "-I$asmIncDir" $crt0 -o $crt0Obj
        if ($LASTEXITCODE -ne 0) { throw 'ca65 crt0 failed' }
    } finally {
        Pop-Location
    }

    & $ca65 $samplesSrc -o $samplesObj
    if ($LASTEXITCODE -ne 0) { throw 'ca65 failed for dmc_samples.s' }

    & $ld65 -C $cfg -o $outRom $crt0Obj $obj $samplesObj $runtime
    if ($LASTEXITCODE -ne 0) { throw 'ld65 failed' }
} finally {
    Pop-Location
}

Write-Host "Built: $outRom"
