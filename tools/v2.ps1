param(
  [Parameter(Position = 0)]
  [ValidateSet('build-pico', 'flash-pico', 'build-rom', 'push-rom', 'deploy', 'status')]
  [string]$Command = 'status',

  [Parameter(Position = 1)]
  [ValidateSet('midi', 'midi-v2a', 'midi-v2b1', 'midi-v2b2', 'midi-v2b3', 'midi-v2b4', 'midi-v2b5', 'midi-v2b6', 'midi-v2b7', 'midi-v2b8', 'midi-v2b9', 'midi-v2b10', 'midi-v2b11', 'midi-v2b12', 'midi-v2b13', 'midi-v2b14', 'midi-v2b15', 'midi-v2b16', 'midi-v2b17', 'midi-v2b18', 'midi-v2b19', 'midi-v2b20', 'midi-v2b21', 'midi-v2b22')]
  [string]$Target = 'midi',

  [string]$Port = 'COM6',
  [string]$Drive = 'D'
)

$ErrorActionPreference = 'Stop'

$Root = Split-Path -Parent $PSScriptRoot
$ArduinoCli = Join-Path $env:LOCALAPPDATA 'Programs\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe'
$Fqbn = 'rp2040:rp2040:rpipico'
$ProjectCurrentRoot = Join-Path $Root 'Project-V2-A'
$ProjectArchiveRoot = Join-Path $Root 'Archive\Project-V2-A'
$ArduinoCurrentRoot = Join-Path $Root 'arduino'
$ArduinoArchiveRoot = Join-Path $Root 'Archive\arduino'

function New-TargetConfig {
  param(
    [string]$SketchBase,
    [string]$SketchName,
    [string]$RomBase,
    [string]$RomFolder
  )

  return @{
    Sketch = Join-Path $SketchBase $SketchName
    SketchName = $SketchName
    RomDir = Join-Path $RomBase $RomFolder
    RomBuild = Join-Path $RomBase "$RomFolder\build.ps1"
    RomFile = Join-Path $RomBase "$RomFolder\V2_MIDI_IN_P1P2.nes"
    BuildRoot = Join-Path $Root "tmp-pico-build\$SketchName"
  }
}

$Targets = @{
  midi = New-TargetConfig $ArduinoCurrentRoot 'PicoNesV2B_Step5ARFlatEnvPitch' $ProjectCurrentRoot 'v2b_step5ar_flat_env_pitch'
  'midi-v2a' = New-TargetConfig $ArduinoArchiveRoot 'PicoNesV2A_MidiInP1P2' $ProjectArchiveRoot 'v2a_midi_in_p1p2'
  'midi-v2b1' = New-TargetConfig $ArduinoArchiveRoot 'PicoNesV2B_Step1P1P2EdgeMidi' $ProjectArchiveRoot 'v2b_step1_p1p2_edge_midi'
  'midi-v2b2' = New-TargetConfig $ArduinoArchiveRoot 'PicoNesV2B_Step2P1P2TriEdgeMidi' $ProjectArchiveRoot 'v2b_step2_p1p2tri_edge_midi'
  'midi-v2b3' = New-TargetConfig $ArduinoArchiveRoot 'PicoNesV2B_Step3P1P2TriNoiseEdgeMidi' $ProjectArchiveRoot 'v2b_step3_p1p2trinoise_edge_midi'
  'midi-v2b4' = New-TargetConfig $ArduinoArchiveRoot 'PicoNesV2B_Step4P1P2TriNoiseDmcProbeMidi' $ProjectArchiveRoot 'v2b_step4_p1p2trinoisedmc_probe_midi'
  'midi-v2b5' = New-TargetConfig $ArduinoArchiveRoot 'PicoNesV2B_Step5P1P2TriNoiseDmcCtrlMidi' $ProjectArchiveRoot 'v2b_step5_p1p2trinoisedmc_ctrl_midi'
  'midi-v2b6' = New-TargetConfig $ArduinoArchiveRoot 'PicoNesV2B_Step5ABGrooveAuxArpMidi' $ProjectArchiveRoot 'v2b_step5ab_groove_arp_aux_midi'
  'midi-v2b7' = New-TargetConfig $ArduinoArchiveRoot 'PicoNesV2B_Step5ACArpPressureMidi' $ProjectArchiveRoot 'v2b_step5ac_arp_pressure_midi'
  'midi-v2b8' = New-TargetConfig $ArduinoArchiveRoot 'PicoNesV2B_Step5ADExactMidiPanelLock' $ProjectArchiveRoot 'v2b_step5ad_exact_midi_panel_lock'
  'midi-v2b9' = New-TargetConfig $ArduinoArchiveRoot 'PicoNesV2B_Step5AEHardRealtimeNotes' $ProjectArchiveRoot 'v2b_step5ae_hard_realtime_notes'
  'midi-v2b10' = New-TargetConfig $ArduinoArchiveRoot 'PicoNesV2B_Step5AFNoiseBoundP2Glide' $ProjectArchiveRoot 'v2b_step5af_noise_bound_p2_glide'
  'midi-v2b11' = New-TargetConfig $ArduinoArchiveRoot 'PicoNesV2B_Step5AGStream5LaneEvent' $ProjectArchiveRoot 'v2b_step5ag_stream_5lane_event'
  'midi-v2b12' = New-TargetConfig $ArduinoArchiveRoot 'PicoNesV2B_Step5AHStreamFastAudioGuard' $ProjectArchiveRoot 'v2b_step5ah_stream_fast_audio_guard'
  'midi-v2b13' = New-TargetConfig $ArduinoArchiveRoot 'PicoNesV2B_Step5AIExpressiveFxTriGlide' $ProjectArchiveRoot 'v2b_step5ai_expressive_fx_tri_glide'
  'midi-v2b14' = New-TargetConfig $ArduinoArchiveRoot 'PicoNesV2B_Step5AJPolyDelayReleaseAntiPop' $ProjectArchiveRoot 'v2b_step5aj_poly_delay_release_antipop'
  'midi-v2b15' = New-TargetConfig $ArduinoArchiveRoot 'PicoNesV2B_Step5AKVolumeCurve' $ProjectArchiveRoot 'v2b_step5ak_volume_curve'
  'midi-v2b16' = New-TargetConfig $ArduinoArchiveRoot 'PicoNesV2B_Step5ALLinearVolumeWaveRelease' $ProjectArchiveRoot 'v2b_step5al_linear_volume_wave_release'
  'midi-v2b17' = New-TargetConfig $ArduinoArchiveRoot 'PicoNesV2B_Step5AMSliderDiagWaveAntiPop' $ProjectArchiveRoot 'v2b_step5am_slider_diag_wave_antipop'
  'midi-v2b18' = New-TargetConfig $ArduinoArchiveRoot 'PicoNesV2B_Step5ANLogSliderComp' $ProjectArchiveRoot 'v2b_step5an_log_slider_comp'
  'midi-v2b19' = New-TargetConfig $ArduinoArchiveRoot 'PicoNesV2B_Step5AOSliderTrimLongEnv' $ProjectArchiveRoot 'v2b_step5ao_slider_trim_long_env'
  'midi-v2b20' = New-TargetConfig $ArduinoArchiveRoot 'PicoNesV2B_Step5APHiZoneSlider' $ProjectArchiveRoot 'v2b_step5ap_hizone_slider'
  'midi-v2b21' = New-TargetConfig $ArduinoArchiveRoot 'PicoNesV2B_Step5AQLongerEnv' $ProjectArchiveRoot 'v2b_step5aq_longer_env'
  'midi-v2b22' = New-TargetConfig $ArduinoCurrentRoot 'PicoNesV2B_Step5ARFlatEnvPitch' $ProjectCurrentRoot 'v2b_step5ar_flat_env_pitch'
}

function Get-TargetConfig {
  param([string]$Name)
  if (-not $Targets.ContainsKey($Name)) {
    throw "Unknown target: $Name"
  }
  return $Targets[$Name]
}

function Invoke-BuildPico {
  param($Cfg)
  $buildPath = Join-Path $Cfg.BuildRoot 'build'
  $outPath = Join-Path $Cfg.BuildRoot 'out'
  New-Item -ItemType Directory -Force -Path $buildPath, $outPath | Out-Null
  & $ArduinoCli compile --fqbn $Fqbn --build-path $buildPath --output-dir $outPath $Cfg.Sketch
  if ($LASTEXITCODE -ne 0) { throw 'Pico compile failed' }
}

function Invoke-FlashPico {
  param($Cfg, [string]$UploadPort)
  $outPath = Join-Path $Cfg.BuildRoot 'out'
  & $ArduinoCli upload -p $UploadPort --fqbn $Fqbn --input-dir $outPath $Cfg.Sketch
  if ($LASTEXITCODE -ne 0) { throw 'Pico upload failed' }
}

function Invoke-BuildRom {
  param($Cfg)
  & powershell -NoProfile -ExecutionPolicy Bypass -File $Cfg.RomBuild
  if ($LASTEXITCODE -ne 0) { throw 'ROM build failed' }
}

function Invoke-PushRom {
  param($Cfg, [string]$TargetDrive)
  $dest = "$TargetDrive`:\game.nes"
  Copy-Item -LiteralPath $Cfg.RomFile -Destination $dest -Force
  Get-Item -LiteralPath $dest | Select-Object FullName, Length, LastWriteTime
}

$cfg = Get-TargetConfig $Target

switch ($Command) {
  'build-pico' { Invoke-BuildPico $cfg }
  'flash-pico' { Invoke-FlashPico $cfg $Port }
  'build-rom' { Invoke-BuildRom $cfg }
  'push-rom' { Invoke-PushRom $cfg $Drive }
  'deploy' {
    Invoke-BuildRom $cfg
    Invoke-PushRom $cfg $Drive
    Invoke-BuildPico $cfg
    Invoke-FlashPico $cfg $Port
  }
  'status' {
    [pscustomobject]@{
      Target = $Target
      Sketch = $cfg.Sketch
      Rom = $cfg.RomFile
      Out = (Join-Path $cfg.BuildRoot 'out')
      Port = $Port
      Drive = "$Drive`:\game.nes"
    }
  }
}
