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
  
  File "..\build\CoNSync.exe"
  File "..\bonjour_sdk\Lib\x64\dnssd.dll"
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
  
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  
  ; Firewall
  SimpleFC::AddApplication "CoNSync AirPlay" "$INSTDIR\CoNSync.exe" 0 2 "" 1
  Pop $0
  
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
  SimpleFC::RemoveApplicationByName "CoNSync AirPlay"
  Pop $0
  Delete "$INSTDIR\CoNSync.exe"
  Delete "$INSTDIR\*.dll"
  Delete "$INSTDIR\Uninstall.exe"
  RMDir "$INSTDIR"
  Delete "$SMPROGRAMS\CoNSync\CoNSync.lnk"
  Delete "$SMPROGRAMS\CoNSync\Uninstall.lnk"
  RMDir "$SMPROGRAMS\CoNSync"
  Delete "$DESKTOP\CoNSync.lnk"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\CoNSync"
SectionEnd
