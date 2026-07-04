# CoNSync AirPlay Receiver

[![Download](https://img.shields.io/github/v/release/maxishen0090-oss/CoNSync?color=blue&label=v2026.7.1&label=Download&style=for-the-badge)](https://github.com/maxishen0090-oss/CoNSync/releases/latest)
[![Windows](https://img.shields.io/badge/Windows-11%20%7C%2010-00A4EF?style=for-the-badge&logo=windows)]()

---

**CoNSync** is a Windows-native AirPlay mirroring receiver. It lets you wirelessly mirror your iPhone, iPad, or Mac screen to any Windows PC ‚Ä?no Apple TV needed.

This project is a fork of [uxplay](https://github.com/antimof/UxPlay), rewritten and heavily optimized for the Windows ecosystem. While uxplay focuses on macOS/Linux, CoNSync is built from the ground up for a native Windows experience.

---

## Download & Install

**[‚¨?Download CoNSync_AirPlay_Receiver_Setup_1.74.exe](https://github.com/maxishen0090-oss/CoNSync/releases/latest)**

> **Size:** 63 MB | **Requires:** Windows 10 64-bit or later

1. Download and run the installer
2. Click through the setup wizard (admin rights required for firewall config)
3. On your iOS device: **Control Center ‚Ü?Screen Mirroring ‚Ü?CoNSync**
4. On your Mac: **Control Center ‚Ü?Screen Mirroring ‚Ü?CoNSync**

> **No extra software needed.** Everything is bundled ‚Ä?GStreamer runtime, codecs, Bonjour service, and all dependencies are included in the installer.

---

## Features

### Streaming
- **AirPlay mirroring** ‚Ä?mirror your iPhone/iPad/Mac screen in real time
- **H.264 / H.265 hardware-accelerated video decoding**
- **AAC / ALAC audio streaming** with WASAPI output
- **Low-latency** streaming with audio-video sync

### Window Management
- **Borderless frameless window** with Windows 11 rounded corners
- **Always-on-top toggle** ‚Ä?press F8
- **Opacity control** ‚Ä?Ctrl + mouse wheel / Ctrl + `+` `-`
- **Move window** ‚Ä?Shift + drag anywhere on the frame
- **Smart resize** ‚Ä?locked aspect ratio; drag edges proportionally
- **Window state persistence** ‚Ä?remembers position, size, opacity, and always-on-top setting across sessions
- **Keyboard shortcut** `Ctrl+Shift+Q` ‚Ä?hide window to system tray

### System Tray
- Minimizes to system tray when window is closed
- Right-click tray menu: **Open Interface** / **Exit Program**
- Custom CoNSync icon in taskbar and tray

### Network & Firewall
- **Automatic firewall rule** configuration on install (all inbound protocols)
- **Smart network detection** at launch ‚Ä?detects Public network profiles
- **One-click UAC elevation** ‚Ä?switches network to Private and adds firewall rule with admin rights
- **Bonjour/mDNS included** ‚Ä?Apple Bonjour service bundled and silently installed
- **Built-in mDNS fallback** ‚Ä?pure C mDNS stack as alternative to Bonjour

### Performance & Compatibility
- **All x86-64 CPUs supported** ‚Ä?compiled with generic `-mtune`, no CPU-specific instructions
- **Full FFmpeg codec support** ‚Ä?H.264, H.265, VP9, AV1, MP3, AAC, FLAC, Opus, and 50+ more codecs
- **Direct3D 12 video rendering** ‚Ä?hardware-accelerated on compatible GPUs
- **WASAPI audio output** ‚Ä?low-latency Windows audio

---

## What Makes CoNSync Different from uxplay

| Area | uxplay (upstream) | CoNSync |
|------|-------------------|---------|
| **Platform focus** | macOS / Linux | **Windows 10/11 native** |
| **Window integration** | Basic X11 window | **Borderless, rounded corners, opacity, always-on-top, persisted state** |
| **System tray** | ‚ù?None | **Full system tray support with right-click menu** |
| **Installer** | ‚ù?Source only | **63 MB self-contained installer with all dependencies** |
| **Bonjour/mDNS** | Requires system Avahi/Bonjour | **Bundled Bonjour install + built-in mDNS fallback** |
| **Firewall** | ‚ù?Manual setup | **Automatic firewall rule at install + runtime network detection** |
| **Codec support** | System codecs only | **Bundled FFmpeg + 50+ codecs** |
| **GStreamer** | Requires manual install | **Bundled GStreamer runtime + all plugins** |
| **Zero-config** | ‚ù?Requires dependency setup | **True zero-config: click install, start mirroring** |
| **Distribution** | Source + manual build | **Greenlight portable folder + Inno Setup installer + GitHub Releases** |

---

## Debug & Troubleshooting

If you encounter issues, use the bundled debug launcher:

```
debug_mode.bat
```

This opens a console window showing GStreamer logs, DLL loading status, and plugin initialization errors.

Common issues:
- **Firewall blocking** ‚Ä?run the installer again, or use `run_CoNSync_fw.bat` in the portable folder
- **Can't find CoNSync on AirPlay** ‚Ä?ensure both devices are on the same network
- **Black screen after connecting** ‚Ä?update your GPU driver, or use `-avdec` flag for software decoding

---

## Building from Source

### Prerequisites
1. [MSYS2](https://www.msys2.org/) with UCRT64 environment
2. Install dependencies in UCRT64 terminal:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-{gcc,cmake,ninja,gstreamer,gst-plugins-{base,good,bad},gst-libav,openssl,libplist,glib2}
```

3. Apple Bonjour SDK (for mDNS): `C:\Program Files\Bonjour SDK\`

### Build

```bash
cd CoNSync-master
mkdir build && cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja -j$(nproc)
```

### Package

After building, generate a portable release folder:

```powershell
powershell -File build_release.ps1
```

Or compile the full installer (requires Inno Setup 6):

```cmd
ISCC.exe CoNSync_AirPlay_Receiver_Setup.iss
```

---

## Repository Structure

```
CoNSync-master/
‚îú‚îÄ‚îÄ CoNSync.cpp                  # Main AirPlay protocol stack (~3300 lines)
‚îú‚îÄ‚îÄ renderers/
‚î?  ‚îú‚îÄ‚îÄ win32_window.c           # Window management (frameless, tray, shortcuts)
‚î?  ‚îú‚îÄ‚îÄ video_renderer.c         # GStreamer video pipeline
‚î?  ‚îî‚îÄ‚îÄ audio_renderer.c         # GStreamer audio pipeline
‚îú‚îÄ‚îÄ lib/                         # AirPlay protocol layer
‚îú‚îÄ‚îÄ build_release.ps1            # One-click portable build script
‚îú‚îÄ‚îÄ CoNSync_Launcher.vbs         # Silent launcher (no CMD window)
‚îú‚îÄ‚îÄ debug_mode.bat               # Debug launcher (shows console logs)
‚îú‚îÄ‚îÄ CoNSync_AirPlay_Receiver_Setup.iss  # Inno Setup installer script
‚îú‚îÄ‚îÄ Bonjour64.msi                # Apple Bonjour service installer
‚îú‚îÄ‚îÄ Release_Installer/           # Final installer (download from Releases)
‚îî‚îÄ‚îÄ huifu/                       # Source backup
```

---

## Credits

- **[uxplay](https://github.com/antimof/UxPlay)** ‚Ä?the original AirPlay receiver project that inspired this Windows-native rewrite
- **[RPiPlay](https://github.com/FD-/RPiPlay)** ‚Ä?codebase this project was originally forked from
- **[GStreamer](https://gstreamer.freedesktop.org/)** ‚Ä?multimedia framework
- **Apple Bonjour SDK** ‚Ä?mDNS service discovery

---

## License

GNU General Public License v3.0. See [LICENSE](LICENSE).

