param([string]$Port = 'COM6', [string]$Drive = 'D')
& "$PSScriptRoot\v2.ps1" deploy midi -Port $Port -Drive $Drive
