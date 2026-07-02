; CoNSync Installer - NSIS Modern UI

;--------------------------------
; Includes
!include "MUI2.nsh"
!include "FileFunc.nsh"
!include "WinVer.nsh"

;--------------------------------
; General
!define PRODUCT_NAME "CoNSync"
!define PRODUCT_VERSION "1.74"
!define PRODUCT_PUBLISHER "CoNSync Project"
!define PRODUCT_WEB_SITE "https://github.com/maxishen0090-oss/CoNSync"

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "CoNSync-${PRODUCT_VERSION}-Setup.exe"
InstallDir "$PROGRAMFILES64\${PRODUCT_NAME}"
InstallDirRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "InstallLocation"
RequestExecutionLevel admin

BrandingText "${PRODUCT_NAME} ${PRODUCT_VERSION}"

; Installer icon
Icon "CoNSync.ico"
; Uninstaller icon
UninstallIcon "CoNSync.ico"

; Show details
ShowInstDetails "nevershow"
ShowUnInstDetails "nevershow"

; Version info
VIProductVersion "1.7.4.0"
VIAddVersionKey "ProductName" "${PRODUCT_NAME}"
VIAddVersionKey "FileVersion" "${PRODUCT_VERSION}"
VIAddVersionKey "ProductVersion" "${PRODUCT_VERSION}"
VIAddVersionKey "CompanyName" "${PRODUCT_PUBLISHER}"
VIAddVersionKey "LegalCopyright" ""
VIAddVersionKey "FileDescription" "${PRODUCT_NAME} - AirPlay Mirroring Server"

;--------------------------------
; Modern UI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "CoNSync.ico"
!define MUI_UNICON "CoNSync.ico"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "installer-header.bmp"
!define MUI_HEADERIMAGE_RIGHT
!define MUI_WELCOMEFINISHPAGE_BITMAP "installer-sidebar.bmp"
!define MUI_WELCOMEPAGE_TEXT "This wizard will guide you through the installation of ${PRODUCT_NAME} ${PRODUCT_VERSION}.$\r$\n$\r$\n${PRODUCT_NAME} is an AirPlay mirroring server that lets you mirror your iOS/iPadOS/macOS device screen to your Windows computer.$\r$\n$\r$\n$_CLICK"
!define MUI_FINISHPAGE_RUN "$INSTDIR\${PRODUCT_NAME}.exe"
!define MUI_FINISHPAGE_RUN_TEXT "Run ${PRODUCT_NAME}"
!define MUI_FINISHPAGE_LINK "Visit ${PRODUCT_NAME} on GitHub"
!define MUI_FINISHPAGE_LINK_LOCATION "${PRODUCT_WEB_SITE}"
!define MUI_FINISHPAGE_NOREBOOTSUPPORT

;--------------------------------
; Pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "LICENSE.txt"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
; Language
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "SimpChinese"

;--------------------------------
; Install Section
Section "Install" SecInstall
  SetOutPath "$INSTDIR"
  
  ; Main program
  File "..\build\CoNSync.exe"
  File "..\bonjour_sdk\Lib\x64\dnssd.dll"
  
  ; Runtime DLLs
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
  
  ; GStreamer plugins
  SetOutPath "$INSTDIR\gstreamer-1.0"
  File "..\build\gstreamer-1.0\*.dll"
  
  SetOutPath "$INSTDIR"
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  
  ; Firewall rule
  ExecWait '"$WINDIR\system32\netsh.exe" advfirewall firewall add rule name="CoNSync AirPlay" dir=in action=allow program="$INSTDIR\CoNSync.exe" enable=yes profile=any'
  
  ; Start Menu shortcuts
  CreateDirectory "$SMPROGRAMS\${PRODUCT_NAME}"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\${PRODUCT_NAME}.lnk" "$INSTDIR\CoNSync.exe" "" "$INSTDIR\CoNSync.exe" 0
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\Uninstall ${PRODUCT_NAME}.lnk" "$INSTDIR\Uninstall.exe"
  
  ; Desktop shortcut (optional)
  CreateShortCut "$DESKTOP\${PRODUCT_NAME}.lnk" "$INSTDIR\CoNSync.exe" "" "$INSTDIR\CoNSync.exe" 0
  
  ; Registry for Add/Remove Programs
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "DisplayName" "${PRODUCT_NAME} - AirPlay Mirroring Server"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "UninstallString" "$INSTDIR\Uninstall.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "QuietUninstallString" '"$INSTDIR\Uninstall.exe" /S'
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "InstallLocation" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "DisplayIcon" "$INSTDIR\CoNSync.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "Publisher" "${PRODUCT_PUBLISHER}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "NoRepair" 1
  
  ; Estimate size
  ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
  IntFmt $0 "0x%08X" $0
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "EstimatedSize" "$0"

SectionEnd

;--------------------------------
; Uninstall Section
Section "Uninstall"
  ; Remove firewall rule
  ExecWait '"$WINDIR\system32\netsh.exe" advfirewall firewall delete rule name="CoNSync AirPlay"'
  
  ; Remove files
  Delete "$INSTDIR\CoNSync.exe"
  Delete "$INSTDIR\*.dll"
  RMDir /r "$INSTDIR\gstreamer-1.0"
  Delete "$INSTDIR\Uninstall.exe"
  RMDir "$INSTDIR"
  
  ; Remove shortcuts
  Delete "$SMPROGRAMS\${PRODUCT_NAME}\${PRODUCT_NAME}.lnk"
  Delete "$SMPROGRAMS\${PRODUCT_NAME}\Uninstall ${PRODUCT_NAME}.lnk"
  RMDir "$SMPROGRAMS\${PRODUCT_NAME}"
  Delete "$DESKTOP\${PRODUCT_NAME}.lnk"
  
  ; Remove registry
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
  
  ; Auto-remove ProgramData state file if exists
  Delete "$APPDATA\CoNSync\window_state.ini"
SectionEnd
