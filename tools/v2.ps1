param(
  [Parameter(Position = 0)]
  [ValidateSet('build-pico', 'flash-pico', 'build-rom', 'push-rom', 'deploy', 'status')]
  [string]$Command = 'status',

  [Parameter(Position = 1)]
  [ValidateSet('midi', 'midi-v2b1', 'midi-v2b2', 'midi-v2b3', 'midi-v2b4', 'midi-v2b5', 'midi-v2b6', 'midi-v2b7', 'midi-v2b8', 'midi-v2b9', 'midi-v2b10', 'midi-v2b11', 'midi-v2b12', 'midi-v2b13', 'midi-v2b14', 'midi-v2b15', 'midi-v2b16', 'midi-v2b17', 'midi-v2b18', 'midi-v2b19', 'midi-v2b20', 'midi-v2b21', 'midi-v2b22')]
  [string]$Target = 'midi',

  [string]$Port = 'COM6',
  [string]$Drive = 'D'
)

$ErrorActionPreference = 'Stop'

$Root = Split-Path -Parent $PSScriptRoot
$ArduinoCli = Join-Path $env:LOCALAPPDATA 'Programs\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe'
$Fqbn = 'rp2040:rp2040:rpipico'

$Targets = @{
  midi = @{
    Sketch = Join-Path $Root 'arduino\PicoNesV2A_MidiInP1P2'
    SketchName = 'PicoNesV2A_MidiInP1P2'
    RomDir = Join-Path $Root 'Project-V2-A\v2a_midi_in_p1p2'
    RomBuild = Join-Path $Root 'Project-V2-A\v2a_midi_in_p1p2\build.ps1'
    RomFile = Join-Path $Root 'Project-V2-A\v2a_midi_in_p1p2\V2_MIDI_IN_P1P2.nes'
    BuildRoot = Join-Path $Root 'tmp-pico-build\PicoNesV2A_MidiInP1P2'
  }
  'midi-v2b1' = @{
    Sketch = Join-Path $Root 'arduino\PicoNesV2B_Step1P1P2EdgeMidi'
    SketchName = 'PicoNesV2B_Step1P1P2EdgeMidi'
    RomDir = Join-Path $Root 'Project-V2-A\v2b_step1_p1p2_edge_midi'
    RomBuild = Join-Path $Root 'Project-V2-A\v2b_step1_p1p2_edge_midi\build.ps1'
    RomFile = Join-Path $Root 'Project-V2-A\v2b_step1_p1p2_edge_midi\V2_MIDI_IN_P1P2.nes'
    BuildRoot = Join-Path $Root 'tmp-pico-build\PicoNesV2B_Step1P1P2EdgeMidi'
  }
  'midi-v2b2' = @{
    Sketch = Join-Path $Root 'arduino\PicoNesV2B_Step2P1P2TriEdgeMidi'
    SketchName = 'PicoNesV2B_Step2P1P2TriEdgeMidi'
    RomDir = Join-Path $Root 'Project-V2-A\v2b_step2_p1p2tri_edge_midi'
    RomBuild = Join-Path $Root 'Project-V2-A\v2b_step2_p1p2tri_edge_midi\build.ps1'
    RomFile = Join-Path $Root 'Project-V2-A\v2b_step2_p1p2tri_edge_midi\V2_MIDI_IN_P1P2.nes'
    BuildRoot = Join-Path $Root 'tmp-pico-build\PicoNesV2B_Step2P1P2TriEdgeMidi'
  }
  'midi-v2b3' = @{
    Sketch = Join-Path $Root 'arduino\PicoNesV2B_Step3P1P2TriNoiseEdgeMidi'
    SketchName = 'PicoNesV2B_Step3P1P2TriNoiseEdgeMidi'
    RomDir = Join-Path $Root 'Project-V2-A\v2b_step3_p1p2trinoise_edge_midi'
    RomBuild = Join-Path $Root 'Project-V2-A\v2b_step3_p1p2trinoise_edge_midi\build.ps1'
    RomFile = Join-Path $Root 'Project-V2-A\v2b_step3_p1p2trinoise_edge_midi\V2_MIDI_IN_P1P2.nes'
    BuildRoot = Join-Path $Root 'tmp-pico-build\PicoNesV2B_Step3P1P2TriNoiseEdgeMidi'
  }
  'midi-v2b4' = @{
    Sketch = Join-Path $Root 'arduino\PicoNesV2B_Step4P1P2TriNoiseDmcProbeMidi'
    SketchName = 'PicoNesV2B_Step4P1P2TriNoiseDmcProbeMidi'
    RomDir = Join-Path $Root 'Project-V2-A\v2b_step4_p1p2trinoisedmc_probe_midi'
    RomBuild = Join-Path $Root 'Project-V2-A\v2b_step4_p1p2trinoisedmc_probe_midi\build.ps1'
    RomFile = Join-Path $Root 'Project-V2-A\v2b_step4_p1p2trinoisedmc_probe_midi\V2_MIDI_IN_P1P2.nes'
    BuildRoot = Join-Path $Root 'tmp-pico-build\PicoNesV2B_Step4P1P2TriNoiseDmcProbeMidi'
  }
  'midi-v2b5' = @{
    Sketch = Join-Path $Root 'arduino\PicoNesV2B_Step5P1P2TriNoiseDmcCtrlMidi'
    SketchName = 'PicoNesV2B_Step5P1P2TriNoiseDmcCtrlMidi'
    RomDir = Join-Path $Root 'Project-V2-A\v2b_step5_p1p2trinoisedmc_ctrl_midi'
    RomBuild = Join-Path $Root 'Project-V2-A\v2b_step5_p1p2trinoisedmc_ctrl_midi\build.ps1'
    RomFile = Join-Path $Root 'Project-V2-A\v2b_step5_p1p2trinoisedmc_ctrl_midi\V2_MIDI_IN_P1P2.nes'
    BuildRoot = Join-Path $Root 'tmp-pico-build\PicoNesV2B_Step5P1P2TriNoiseDmcCtrlMidi'
  }
  'midi-v2b6' = @{
    Sketch = Join-Path $Root 'arduino\PicoNesV2B_Step5ABGrooveAuxArpMidi'
    SketchName = 'PicoNesV2B_Step5ABGrooveAuxArpMidi'
    RomDir = Join-Path $Root 'Project-V2-A\v2b_step5ab_groove_arp_aux_midi'
    RomBuild = Join-Path $Root 'Project-V2-A\v2b_step5ab_groove_arp_aux_midi\build.ps1'
    RomFile = Join-Path $Root 'Project-V2-A\v2b_step5ab_groove_arp_aux_midi\V2_MIDI_IN_P1P2.nes'
    BuildRoot = Join-Path $Root 'tmp-pico-build\PicoNesV2B_Step5ABGrooveAuxArpMidi'
  }
  'midi-v2b7' = @{
    Sketch = Join-Path $Root 'arduino\PicoNesV2B_Step5ACArpPressureMidi'
    SketchName = 'PicoNesV2B_Step5ACArpPressureMidi'
    RomDir = Join-Path $Root 'Project-V2-A\v2b_step5ac_arp_pressure_midi'
    RomBuild = Join-Path $Root 'Project-V2-A\v2b_step5ac_arp_pressure_midi\build.ps1'
    RomFile = Join-Path $Root 'Project-V2-A\v2b_step5ac_arp_pressure_midi\V2_MIDI_IN_P1P2.nes'
    BuildRoot = Join-Path $Root 'tmp-pico-build\PicoNesV2B_Step5ACArpPressureMidi'
  }
  'midi-v2b8' = @{
    Sketch = Join-Path $Root 'arduino\PicoNesV2B_Step5ADExactMidiPanelLock'
    SketchName = 'PicoNesV2B_Step5ADExactMidiPanelLock'
    RomDir = Join-Path $Root 'Project-V2-A\v2b_step5ad_exact_midi_panel_lock'
    RomBuild = Join-Path $Root 'Project-V2-A\v2b_step5ad_exact_midi_panel_lock\build.ps1'
    RomFile = Join-Path $Root 'Project-V2-A\v2b_step5ad_exact_midi_panel_lock\V2_MIDI_IN_P1P2.nes'
    BuildRoot = Join-Path $Root 'tmp-pico-build\PicoNesV2B_Step5ADExactMidiPanelLock'
  }
  'midi-v2b9' = @{
    Sketch = Join-Path $Root 'arduino\PicoNesV2B_Step5AEHardRealtimeNotes'
    SketchName = 'PicoNesV2B_Step5AEHardRealtimeNotes'
    RomDir = Join-Path $Root 'Project-V2-A\v2b_step5ae_hard_realtime_notes'
    RomBuild = Join-Path $Root 'Project-V2-A\v2b_step5ae_hard_realtime_notes\build.ps1'
    RomFile = Join-Path $Root 'Project-V2-A\v2b_step5ae_hard_realtime_notes\V2_MIDI_IN_P1P2.nes'
    BuildRoot = Join-Path $Root 'tmp-pico-build\PicoNesV2B_Step5AEHardRealtimeNotes'
  }
  'midi-v2b10' = @{
    Sketch = Join-Path $Root 'arduino\PicoNesV2B_Step5AFNoiseBoundP2Glide'
    SketchName = 'PicoNesV2B_Step5AFNoiseBoundP2Glide'
    RomDir = Join-Path $Root 'Project-V2-A\v2b_step5af_noise_bound_p2_glide'
    RomBuild = Join-Path $Root 'Project-V2-A\v2b_step5af_noise_bound_p2_glide\build.ps1'
    RomFile = Join-Path $Root 'Project-V2-A\v2b_step5af_noise_bound_p2_glide\V2_MIDI_IN_P1P2.nes'
    BuildRoot = Join-Path $Root 'tmp-pico-build\PicoNesV2B_Step5AFNoiseBoundP2Glide'
  }
  'midi-v2b11' = @{
    Sketch = Join-Path $Root 'arduino\PicoNesV2B_Step5AGStream5LaneEvent'
    SketchName = 'PicoNesV2B_Step5AGStream5LaneEvent'
    RomDir = Join-Path $Root 'Project-V2-A\v2b_step5ag_stream_5lane_event'
    RomBuild = Join-Path $Root 'Project-V2-A\v2b_step5ag_stream_5lane_event\build.ps1'
    RomFile = Join-Path $Root 'Project-V2-A\v2b_step5ag_stream_5lane_event\V2_MIDI_IN_P1P2.nes'
    BuildRoot = Join-Path $Root 'tmp-pico-build\PicoNesV2B_Step5AGStream5LaneEvent'
  }
  'midi-v2b12' = @{
    Sketch = Join-Path $Root 'arduino\PicoNesV2B_Step5AHStreamFastAudioGuard'
    SketchName = 'PicoNesV2B_Step5AHStreamFastAudioGuard'
    RomDir = Join-Path $Root 'Project-V2-A\v2b_step5ah_stream_fast_audio_guard'
    RomBuild = Join-Path $Root 'Project-V2-A\v2b_step5ah_stream_fast_audio_guard\build.ps1'
    RomFile = Join-Path $Root 'Project-V2-A\v2b_step5ah_stream_fast_audio_guard\V2_MIDI_IN_P1P2.nes'
    BuildRoot = Join-Path $Root 'tmp-pico-build\PicoNesV2B_Step5AHStreamFastAudioGuard'
  }
  'midi-v2b13' = @{
    Sketch = Join-Path $Root 'arduino\PicoNesV2B_Step5AIExpressiveFxTriGlide'
    SketchName = 'PicoNesV2B_Step5AIExpressiveFxTriGlide'
    RomDir = Join-Path $Root 'Project-V2-A\v2b_step5ai_expressive_fx_tri_glide'
    RomBuild = Join-Path $Root 'Project-V2-A\v2b_step5ai_expressive_fx_tri_glide\build.ps1'
    RomFile = Join-Path $Root 'Project-V2-A\v2b_step5ai_expressive_fx_tri_glide\V2_MIDI_IN_P1P2.nes'
    BuildRoot = Join-Path $Root 'tmp-pico-build\PicoNesV2B_Step5AIExpressiveFxTriGlide'
  }
  'midi-v2b14' = @{
    Sketch = Join-Path $Root 'arduino\PicoNesV2B_Step5AJPolyDelayReleaseAntiPop'
    SketchName = 'PicoNesV2B_Step5AJPolyDelayReleaseAntiPop'
    RomDir = Join-Path $Root 'Project-V2-A\v2b_step5aj_poly_delay_release_antipop'
    RomBuild = Join-Path $Root 'Project-V2-A\v2b_step5aj_poly_delay_release_antipop\build.ps1'
    RomFile = Join-Path $Root 'Project-V2-A\v2b_step5aj_poly_delay_release_antipop\V2_MIDI_IN_P1P2.nes'
    BuildRoot = Join-Path $Root 'tmp-pico-build\PicoNesV2B_Step5AJPolyDelayReleaseAntiPop'
  }
  'midi-v2b15' = @{
    Sketch = Join-Path $Root 'arduino\PicoNesV2B_Step5AKVolumeCurve'
    SketchName = 'PicoNesV2B_Step5AKVolumeCurve'
    RomDir = Join-Path $Root 'Project-V2-A\v2b_step5ak_volume_curve'
    RomBuild = Join-Path $Root 'Project-V2-A\v2b_step5ak_volume_curve\build.ps1'
    RomFile = Join-Path $Root 'Project-V2-A\v2b_step5ak_volume_curve\V2_MIDI_IN_P1P2.nes'
    BuildRoot = Join-Path $Root 'tmp-pico-build\PicoNesV2B_Step5AKVolumeCurve'
  }
  'midi-v2b16' = @{
    Sketch = Join-Path $Root 'arduino\PicoNesV2B_Step5ALLinearVolumeWaveRelease'
    SketchName = 'PicoNesV2B_Step5ALLinearVolumeWaveRelease'
    RomDir = Join-Path $Root 'Project-V2-A\v2b_step5al_linear_volume_wave_release'
    RomBuild = Join-Path $Root 'Project-V2-A\v2b_step5al_linear_volume_wave_release\build.ps1'
    RomFile = Join-Path $Root 'Project-V2-A\v2b_step5al_linear_volume_wave_release\V2_MIDI_IN_P1P2.nes'
    BuildRoot = Join-Path $Root 'tmp-pico-build\PicoNesV2B_Step5ALLinearVolumeWaveRelease'
  }
  'midi-v2b17' = @{
    Sketch = Join-Path $Root 'arduino\PicoNesV2B_Step5AMSliderDiagWaveAntiPop'
    SketchName = 'PicoNesV2B_Step5AMSliderDiagWaveAntiPop'
    RomDir = Join-Path $Root 'Project-V2-A\v2b_step5am_slider_diag_wave_antipop'
    RomBuild = Join-Path $Root 'Project-V2-A\v2b_step5am_slider_diag_wave_antipop\build.ps1'
    RomFile = Join-Path $Root 'Project-V2-A\v2b_step5am_slider_diag_wave_antipop\V2_MIDI_IN_P1P2.nes'
    BuildRoot = Join-Path $Root 'tmp-pico-build\PicoNesV2B_Step5AMSliderDiagWaveAntiPop'
  }
  'midi-v2b18' = @{
    Sketch = Join-Path $Root 'arduino\PicoNesV2B_Step5ANLogSliderComp'
    SketchName = 'PicoNesV2B_Step5ANLogSliderComp'
    RomDir = Join-Path $Root 'Project-V2-A\v2b_step5an_log_slider_comp'
    RomBuild = Join-Path $Root 'Project-V2-A\v2b_step5an_log_slider_comp\build.ps1'
    RomFile = Join-Path $Root 'Project-V2-A\v2b_step5an_log_slider_comp\V2_MIDI_IN_P1P2.nes'
    BuildRoot = Join-Path $Root 'tmp-pico-build\PicoNesV2B_Step5ANLogSliderComp'
  }
  'midi-v2b19' = @{
    Sketch = Join-Path $Root 'arduino\PicoNesV2B_Step5AOSliderTrimLongEnv'
    SketchName = 'PicoNesV2B_Step5AOSliderTrimLongEnv'
    RomDir = Join-Path $Root 'Project-V2-A\v2b_step5ao_slider_trim_long_env'
    RomBuild = Join-Path $Root 'Project-V2-A\v2b_step5ao_slider_trim_long_env\build.ps1'
    RomFile = Join-Path $Root 'Project-V2-A\v2b_step5ao_slider_trim_long_env\V2_MIDI_IN_P1P2.nes'
    BuildRoot = Join-Path $Root 'tmp-pico-build\PicoNesV2B_Step5AOSliderTrimLongEnv'
  }
  'midi-v2b20' = @{
    Sketch = Join-Path $Root 'arduino\PicoNesV2B_Step5APHiZoneSlider'
    SketchName = 'PicoNesV2B_Step5APHiZoneSlider'
    RomDir = Join-Path $Root 'Project-V2-A\v2b_step5ap_hizone_slider'
    RomBuild = Join-Path $Root 'Project-V2-A\v2b_step5ap_hizone_slider\build.ps1'
    RomFile = Join-Path $Root 'Project-V2-A\v2b_step5ap_hizone_slider\V2_MIDI_IN_P1P2.nes'
    BuildRoot = Join-Path $Root 'tmp-pico-build\PicoNesV2B_Step5APHiZoneSlider'
  }
  'midi-v2b21' = @{
    Sketch = Join-Path $Root 'arduino\PicoNesV2B_Step5AQLongerEnv'
    SketchName = 'PicoNesV2B_Step5AQLongerEnv'
    RomDir = Join-Path $Root 'Project-V2-A\v2b_step5aq_longer_env'
    RomBuild = Join-Path $Root 'Project-V2-A\v2b_step5aq_longer_env\build.ps1'
    RomFile = Join-Path $Root 'Project-V2-A\v2b_step5aq_longer_env\V2_MIDI_IN_P1P2.nes'
    BuildRoot = Join-Path $Root 'tmp-pico-build\PicoNesV2B_Step5AQLongerEnv'
  }
  'midi-v2b22' = @{
    Sketch = Join-Path $Root 'arduino\PicoNesV2B_Step5ARFlatEnvPitch'
    SketchName = 'PicoNesV2B_Step5ARFlatEnvPitch'
    RomDir = Join-Path $Root 'Project-V2-A\v2b_step5ar_flat_env_pitch'
    RomBuild = Join-Path $Root 'Project-V2-A\v2b_step5ar_flat_env_pitch\build.ps1'
    RomFile = Join-Path $Root 'Project-V2-A\v2b_step5ar_flat_env_pitch\V2_MIDI_IN_P1P2.nes'
    BuildRoot = Join-Path $Root 'tmp-pico-build\PicoNesV2B_Step5ARFlatEnvPitch'
  }
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
