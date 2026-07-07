$ReleaseRoot = "F:\CoNSync\Release_Folder"
$MsvcBinDir  = "F:\msys64\ucrt64\bin"
$PluginDir   = "F:\msys64\ucrt64\lib\gstreamer-1.0"
$ExeSource   = "F:\CoNSync\CoNSync-master\build\CoNSync.exe"
$IconSource  = "F:\CoNSync\CoNSync-master\CoNSync.ico"

if (Test-Path $ReleaseRoot) { Remove-Item "$ReleaseRoot\*" -Recurse -Force }
New-Item -ItemType Directory -Path "$ReleaseRoot\lib\gstreamer-1.0" -Force | Out-Null
Copy-Item $ExeSource "$ReleaseRoot\CoNSync.exe" -Force
Copy-Item $IconSource "$ReleaseRoot\CoNSync.ico" -Force

$coreDlls = "libgcc_s_seh-1.dll","libwinpthread-1.dll","libstdc++-6.dll","libcrypto-3-x64.dll","libglib-2.0-0.dll","libgobject-2.0-0.dll","libgmodule-2.0-0.dll","libgstapp-1.0-0.dll","libgstbase-1.0-0.dll","libgstreamer-1.0-0.dll","libiconv-2.dll","libintl-8.dll","libpcre2-8-0.dll","libplist-2.0.dll","libffi-8.dll"
foreach ($dll in $coreDlls) { if (Test-Path "$MsvcBinDir\$dll") { Copy-Item "$MsvcBinDir\$dll" "$ReleaseRoot\" -Force } }
foreach ($dll in (Get-ChildItem "$MsvcBinDir\libgst*.dll")) { Copy-Item $dll.FullName "$ReleaseRoot\" -Force }
foreach ($plugin in (Get-ChildItem "$PluginDir\*.dll")) { Copy-Item $plugin.FullName "$ReleaseRoot\lib\gstreamer-1.0\" -Force }

# ????
@"
@echo off
title CoNSync AirPlay Server
set "APP_DIR=%~dp0"
set "GST_PLUGIN_SYSTEM_PATH=%APP_DIR%lib\gstreamer-1.0"
set "GST_PLUGIN_PATH=%APP_DIR%lib\gstreamer-1.0"
echo CoNSync AirPlay Mirroring Server 1.74
echo =====================================
"%APP_DIR%CoNSync.exe" -vs d3d12videosink -as wasapisink %*
pause
"@ | Out-File "$ReleaseRoot\run_CoNSync.bat" -Encoding ASCII

# ????
@"
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
"@ | Out-File "$ReleaseRoot\run_CoNSync_fw.bat" -Encoding ASCII

# VBScript ?????? Debug ??????????
Copy-Item "F:\CoNSync\CoNSync_Launcher.vbs" "$ReleaseRoot\CoNSync_Launcher.vbs" -Force
Copy-Item "F:\CoNSync\debug_mode.bat" "$ReleaseRoot\debug_mode.bat" -Force

$totalSize = (Get-ChildItem $ReleaseRoot -Recurse -File | Measure-Object -Property Length -Sum).Sum
$totalFiles = (Get-ChildItem $ReleaseRoot -Recurse -File).Count
Write-Host "=== Release Build Complete ===" -ForegroundColor Cyan
Write-Host "Output: $ReleaseRoot"
Write-Host "Files : $totalFiles ($([math]::Round($totalSize/1MB, 1)) MB)"
Write-Host ""
Write-Host "Daily use : double-click CoNSync_Launcher.vbs (no console)"
Write-Host "Debug     : double-click debug_mode.bat (show console)"
Write-Host "Backup    : double-click run_CoNSync.bat (firewall included)"
