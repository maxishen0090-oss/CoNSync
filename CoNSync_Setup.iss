[Setup]
AppId={{F8A9B1C2-D3E4-5F6A-7B8C-9D0E1F2A3B4C}
AppName=CoNSync
AppVersion=2026.8.8
AppPublisher=CoNSync Open Source
UninstallDisplayIcon={app}\CoNSync.ico
DefaultDirName={autopf}\CoNSync
DefaultGroupName=CoNSync
OutputDir=.\Output
OutputBaseFilename=CoNSync_Setup_v2026.8.8
SetupIconFile=CoNSync.ico
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
DisableDirPage=no
DisableProgramGroupPage=no
PrivilegesRequired=admin

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: checkablealone

[Files]
; 主程序与图标
Source: "build\CoNSync.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "CoNSync.ico"; DestDir: "{app}"; Flags: ignoreversion

; 启动脚本
Source: "run_CoNSync.bat"; DestDir: "{app}"; Flags: ignoreversion
Source: "run_CoNSync_fw.bat"; DestDir: "{app}"; Flags: ignoreversion
Source: "CoNSync_Launcher.vbs"; DestDir: "{app}"; Flags: ignoreversion
Source: "debug_mode.bat"; DestDir: "{app}"; Flags: ignoreversion

; MSYS2/UCRT64 核心 DLL
Source: "F:\msys64\ucrt64\bin\*.dll"; DestDir: "{app}"; Flags: ignoreversion

; GStreamer 插件
Source: "F:\msys64\ucrt64\lib\gstreamer-1.0\*.dll"; DestDir: "{app}\lib\gstreamer-1.0"; Flags: ignoreversion
Source: "bonjour.msi"; DestDir: "{tmp}"; Flags: ignoreversion deleteafterinstall

[Icons]
; 开始菜单图标
Name: "{group}\CoNSync"; Filename: "{app}\CoNSync_Launcher.vbs"; IconFilename: "{app}\CoNSync.ico"
Name: "{group}\CoNSync (静默后台模式)"; Filename: "{app}\CoNSync_Launcher.vbs"; IconFilename: "{app}\CoNSync.ico"
Name: "{group}\{cm:UninstallProgram,CoNSync}"; Filename: "{uninstallexe}"

; 桌面图标
Name: "{autodesktop}\CoNSync"; Filename: "{app}\CoNSync_Launcher.vbs"; IconFilename: "{app}\CoNSync.ico"; Tasks: desktopicon

[Run]
Filename: "msiexec.exe"; Parameters: "/i ""{tmp}\bonjour.msi"" /qn"; Description: "正在安装环境依赖 (Apple Bonjour)..."; StatusMsg: "正在配置网络发现服务，请稍候..."; Flags: runhidden waituntilterminated
Filename: "{app}\CoNSync.exe"; Description: "运行 CoNSync 投屏服务"; Flags: nowait postinstall skipifsilent
