@echo off
title CoNSync AirPlay Server
set "APP_DIR=%~dp0"
set "GST_PLUGIN_SYSTEM_PATH=%APP_DIR%lib\gstreamer-1.0"
set "GST_PLUGIN_PATH=%APP_DIR%lib\gstreamer-1.0"
echo CoNSync AirPlay Mirroring Server 1.74
echo =====================================
"%APP_DIR%CoNSync.exe" -vs d3d12videosink -as wasapisink %*
pause
