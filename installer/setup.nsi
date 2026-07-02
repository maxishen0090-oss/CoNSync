; CoNSync NSIS Installer
!define PRODUCT_NAME "CoNSync"
!define PRODUCT_VERSION "1.74"
!define PRODUCT_PUBLISHER "CoNSync Project"

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "CoNSync-${PRODUCT_VERSION}-Setup.exe"
InstallDir "$PROGRAMFILES64\CoNSync"
RequestExecutionLevel admin

Section "Install"
  SetOutPath "$INSTDIR"
  
  ; CoNSync 主程序
  File "..\build\CoNSync.exe"
  File "..\bonjour_sdk\Lib\x64\dnssd.dll"
  
  ; 所有运行时 DLL
  File "..\build\libgcc_s_seh-1.dll"
  File "..\build\libstdc++-6.dll"
  File "..\build\libwinpthread-1.dll"
  File "..\build\libglib-2.0-0.dll"
  File "..\build\libgobject-2.0-0.dll"
  File "..\build\libcrypto-3-x64.dll"
  File "..\build\libplist-2.0.dll"
  File "..\build\libintl-8.dll"
  File "..\build\libffi-8.dll"
  File "..\build\libgmodule-2.0-0.dll"
  File "..\build\libpcre2-8-0.dll"
  File "..\build\libiconv-2.dll"
  File "..\build\libgstapp-1.0-0.dll"
  File "..\build\libgstbase-1.0-0.dll"
  File "..\build\libgstreamer-1.0-0.dll"
  File "..\build\libgstvideo-1.0-0.dll"
  File "..\build\liborc-0.4-0.dll"
  File "..\build\libgstaudio-1.0-0.dll"
  File "..\build\libgsttag-1.0-0.dll"
  File "..\build\libgstpbutils-1.0-0.dll"
  File "..\build\libgstcodecs-1.0-0.dll"
  File "..\build\libgstcodecparsers-1.0-0.dll"
  File "..\build\libgstd3d11-1.0-0.dll"
  File "..\build\libgstd3dshader-1.0-0.dll"
  File "..\build\libgstdxva-1.0-0.dll"
  File "..\build\libgstallocators-1.0-0.dll"
  File "..\build\libgstfft-1.0-0.dll"
  File "..\build\libgstrtp-1.0-0.dll"
  File "..\build\zlib1.dll"
  
  ; GStreamer 插件
  SetOutPath "$INSTDIR\gstreamer-1.0"
  File "..\build\gstreamer-1.0\*.dll"
  
  SetOutPath "$INSTDIR"
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  
  ; Firewall rule
  ExecWait '"$WINDIR\system32\netsh.exe" advfirewall firewall add rule name="CoNSync AirPlay" dir=in action=allow program="$INSTDIR\CoNSync.exe" enable=yes profile=any'
  
  ; Shortcuts
  CreateDirectory "$SMPROGRAMS\CoNSync"
  CreateShortCut "$SMPROGRAMS\CoNSync\CoNSync.lnk" "$INSTDIR\CoNSync.exe" "" "$INSTDIR\CoNSync.exe" 0
  CreateShortCut "$SMPROGRAMS\CoNSync\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
  CreateShortCut "$DESKTOP\CoNSync.lnk" "$INSTDIR\CoNSync.exe"
  
  ; Registry
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\CoNSync" "DisplayName" "CoNSync - AirPlay Mirroring"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\CoNSync" "UninstallString" "$INSTDIR\Uninstall.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\CoNSync" "DisplayIcon" "$INSTDIR\CoNSync.exe"
SectionEnd

Section "Uninstall"
  ; Remove firewall rule
  ExecWait '"$WINDIR\system32\netsh.exe" advfirewall firewall delete rule name="CoNSync AirPlay"'
  Delete "$INSTDIR\CoNSync.exe"
  Delete "$INSTDIR\*.dll"
  RMDir /r "$INSTDIR\gstreamer-1.0"
  Delete "$INSTDIR\Uninstall.exe"
  RMDir "$INSTDIR"
  Delete "$SMPROGRAMS\CoNSync\CoNSync.lnk"
  Delete "$SMPROGRAMS\CoNSync\Uninstall.lnk"
  RMDir "$SMPROGRAMS\CoNSync"
  Delete "$DESKTOP\CoNSync.lnk"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\CoNSync"
SectionEnd
