@echo off
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0push-midi-rom.ps1" %*
