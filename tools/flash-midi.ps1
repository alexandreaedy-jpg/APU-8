param([string]$Port = 'COM6')
& "$PSScriptRoot\v2.ps1" flash-pico midi -Port $Port
