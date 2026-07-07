@echo off
title CoNSync AirPlay Server
netsh advfirewall firewall show rule name="CoNSync" >nul 2>&1
if %errorlevel% neq 0 (
    netsh advfirewall firewall add rule name="CoNSync" dir=in action=allow program="%~dp0CoNSync.exe" enable=yes >nul 2>&1
    if %errorlevel% neq 0 (
        echo [CoNSync] Requesting admin rights for firewall...
        powershell -Command "Start-Process netsh -ArgumentList 'advfirewall firewall add rule name=""CoNSync"" dir=in action=allow program=""%~dp0CoNSync.exe"" enable=yes' -Verb RunAs -WindowStyle Hidden -Wait" >nul 2>&1
    )
)
set "APP_DIR=%~dp0"
set "GST_PLUGIN_SYSTEM_PATH=%APP_DIR%lib\gstreamer-1.0"
set "GST_PLUGIN_PATH=%APP_DIR%lib\gstreamer-1.0"
echo CoNSync AirPlay Mirroring Server 1.74
echo =====================================
"%APP_DIR%CoNSync.exe" -vs d3d12videosink -as wasapisink %*
pause
