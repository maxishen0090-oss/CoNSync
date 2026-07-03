@echo off
title CoNSync AirPlay Server

REM === ???????????? UAC? ===
netsh advfirewall firewall show rule name="CoNSync" >nul 2>&1
if %errorlevel% neq 0 (
    netsh advfirewall firewall add rule name="CoNSync" dir=in action=allow program="F:\CoNSync\CoNSync-master\build\CoNSync.exe" enable=yes >nul 2>&1
    if %errorlevel% neq 0 (
        echo [CoNSync] ??????????????...
        powershell -Command "Start-Process netsh -ArgumentList 'advfirewall firewall add rule name=""CoNSync"" dir=in action=allow program=""F:\CoNSync\CoNSync-master\build\CoNSync.exe"" enable=yes' -Verb RunAs -WindowStyle Hidden -Wait" >nul 2>&1
    )
)

set MSYSTEM=UCRT64
set MSYS2_PATH_TYPE=inherit
set GST_PLUGIN_SYSTEM_PATH=F:\msys64\ucrt64\lib\gstreamer-1.0
set GST_PLUGIN_PATH=F:\msys64\ucrt64\lib\gstreamer-1.0
echo CoNSync AirPlay Mirroring Server 1.74
echo =====================================
"F:\msys64\usr\bin\bash.exe" -l -c "/f/CoNSync/CoNSync-master/build/CoNSync -vs d3d12videosink -as wasapisink %*"
pause
