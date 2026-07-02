@echo off
title CoNSync - AirPlay Mirroring Server
echo ============================================
echo   CoNSync - AirPlay Mirroring Server
echo ============================================
echo.
echo Make sure aTrust VPN is DISCONNECTED first.
echo.
set MSYSTEM=UCRT64
set PATH=C:\msys64\ucrt64\bin;%PATH%
"%~dp0CoNSync.exe"
pause
