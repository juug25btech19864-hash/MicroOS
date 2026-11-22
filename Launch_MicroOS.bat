@echo off
title Launching MicroOS v2.0
echo ===================================================
echo            LAUNCHING MICROOS v2.0
echo ===================================================
echo Starting MicroOS Virtual Machine in Oracle VirtualBox...
start "" "C:\Program Files\Oracle\VirtualBox\VirtualBoxVM.exe" --startvm "MicroOS"
if %ERRORLEVEL% NEQ 0 (
    "C:\Program Files\Oracle\VirtualBox\VBoxManage.exe" startvm "MicroOS"
)
