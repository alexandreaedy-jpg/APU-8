@echo off
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0build-midi.ps1" %*
