@echo off
title MicroOS GUI Launcher
echo ============================================================
echo      MICROOS GUI EDITION - Graphical Desktop Launcher
echo ============================================================
echo.

REM 1. Clean up any stuck background VBoxSVC or orphaned VM processes
echo [*] Clearing any stuck background VirtualBox services...
taskkill /F /IM VirtualBoxVM.exe >nul 2>&1
taskkill /F /IM VBoxSVC.exe >nul 2>&1
timeout /t 1 /nobreak >nul

REM 2. Start VirtualBox VM directly in user's desktop session
set VBOX="C:\Program Files\Oracle\VirtualBox\VirtualBoxVM.exe"
set VMNAME=MicroOS-GUI

echo [*] Starting MicroOS Graphical Desktop window...
if exist %VBOX% (
    start "" %VBOX% --startvm "%VMNAME%"
) else (
    start "" "C:\Program Files\Oracle\VirtualBox\VBoxManage.exe" startvm "%VMNAME%" --type gui
)

echo [+] MicroOS GUI window launched!
echo     (You can now minimize or close this launcher window.)
timeout /t 3 >nul
