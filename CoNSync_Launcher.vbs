' CoNSync_Launcher.vbs ? Smart launcher with network auto-fix
' Silent launch (no console), detects Public network + firewall, auto-elevates

Dim fso, scriptDir, shell, env, wmi, profiles, profile
Dim isPublic, ruleMissing, needsAdmin, coNsyncPath

Set fso = CreateObject("Scripting.FileSystemObject")
scriptDir = fso.GetParentFolderName(WScript.ScriptFullName)
coNsyncPath = scriptDir & "\CoNSync.exe"
Set shell = CreateObject("WScript.Shell")

' ----- Step 1: Check network profile (Public/Private) -----
isPublic = False
On Error Resume Next
Set wmi = GetObject("winmgmts:\.\root\StandardCimv2")
Set profiles = wmi.ExecQuery("SELECT * FROM MSFT_NetConnectionProfile")
For Each profile In profiles
    If profile.NetworkCategory = 0 Then  ' 0 = Public
        isPublic = True
        Exit For
    End If
Next
On Error Goto 0

' ----- Step 2: Check firewall rule -----
ruleMissing = False
Dim fwCheck
fwCheck = shell.Run("netsh advfirewall firewall show rule name=""CoNSync AirPlay""", 0, True)
If fwCheck <> 0 Then ruleMissing = True

' ----- Step 3: Auto-fix via UAC elevation if needed -----
needsAdmin = isPublic Or ruleMissing
If needsAdmin Then
    Dim psCmd, adminTasks
    adminTasks = ""
    If isPublic Then
        adminTasks = adminTasks & "=Get-NetConnectionProfile;if( -and .NetworkCategory -eq 0){Set-NetConnectionProfile -InterfaceIndex .InterfaceIndex -NetworkCategory Private};"
    End If
    If ruleMissing Or isPublic Then
        adminTasks = adminTasks & "netsh advfirewall firewall add rule name=""CoNSync AirPlay"" dir=in action=allow program=""" & coNsyncPath & """ enable=yes protocol=any"
    End If
    psCmd = "-NoProfile -Command ""& {" & adminTasks & "}"""
    
    Dim shellApp
    Set shellApp = CreateObject("Shell.Application")
    shellApp.ShellExecute "powershell.exe", psCmd, "", "runas", 0
    WScript.Sleep 1000
End If

' ----- Step 4: Set GStreamer env and launch CoNSync.exe -----
Set env = shell.Environment("Process")
env("GST_PLUGIN_SYSTEM_PATH") = scriptDir & "\lib\gstreamer-1.0"
env("GST_PLUGIN_PATH") = scriptDir & "\lib\gstreamer-1.0"
shell.Run chr(34) & coNsyncPath & chr(34) & " -vs d3d12videosink -as wasapisink", 0, False
