# CoNSync — AirPlay Mirroring & Audio Streaming Receiver for Windows

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](LICENSE)

CoNSync is a Windows port and fork of [UxPlay](https://github.com/antimof/UxPlay), an open-source AirPlay mirroring server.  
It allows you to wirelessly stream video and audio from your iPhone, iPad, or Mac to your Windows PC — just like Apple TV.

> **Note:** This project is a fork of [UxPlay](https://github.com/antimof/UxPlay) (which was a fork of [RPiPlay](https://github.com/FD-/RPiPlay)), with extensive modifications for Windows-specific features, stability, and deployment.

---

## Features

- **AirPlay Screen Mirroring** — H.264/H.265 video + AAC/ALAC audio
- **Borderless Video Window** — Native Windows 11 rounded corners support
- **Window Position & Opacity Persistence** — Automatically remembers per-device window state (MAC-bound)
- **800ms Deferred Window Restoration** — Prevents GStreamer initial frame override
- **Cold-Start WS_EX_LAYERED Fix** — Guarantees opacity restoration on fresh launch
- **Independent System Tray** — Ghost thread with Show Window / Exit menu (no interference with rendering)
- **Smart Firewall Detection** — UAC prompt only on first run; silent skip on subsequent launches
- **Bonjour Silent Installation** — Bonjour service bundled and auto-installed with the setup
- **Multi-Device Concurrent Streaming** — Different devices can stream simultaneously
- **Keyboard Shortcuts**
  - `F8` — Toggle always-on-top
  - `Ctrl + Scroll / Ctrl + +/-` — Adjust opacity
  - `Shift + Drag` — Move window
  - `Ctrl + Shift + Q` — Hide to tray
  - `Tray Double-Click` — Restore window
  - `Tray Right-Click → Show Window` — Restore window
  - `Tray Right-Click → Exit CoNSync` — Quit application

---

## Download

**Latest release:** [CoNSync_Setup_v2026.8.7.exe](https://github.com/maxishen0090-oss/CoNSync/releases/download/v2026.8.7/CoNSync_Setup_v2026.8.7.exe) (112 MB)

The installer includes:
- CoNSync executable
- All required MSYS2 runtime DLLs (GCC, GLib, OpenSSL, etc.)
- GStreamer 1.28.4 with all plugins (D3D12, WASAPI, codecs, etc.)
- Apple Bonjour service installer (silent)
- Launch scripts (normal, debug, VBS silent mode)

See [Releases](https://github.com/maxishen0090-oss/CoNSync/releases) for older versions.

---

## Quick Start

1. Download the latest installer from [Releases](https://github.com/maxishen0090-oss/CoNSync/releases)
2. Run `CoNSync_Setup_v2026.8.7.exe`
3. Launch CoNSync from the Start Menu or desktop shortcut
4. On your iOS device, open **Control Center → Screen Mirroring → CoNSync**

Your device screen will appear on the Windows desktop.  
Use the system tray icon to show/hide the window or exit the application.

---

## Building from Source

### Prerequisites (MSYS2 UCRT64)

```bash
pacman -S --needed \
  mingw-w64-ucrt-x86_64-{gcc,cmake,ninja,pkg-config} \
  mingw-w64-ucrt-x86_64-{gstreamer,gst-plugins-base,gst-plugins-good,gst-plugins-bad,gst-plugins-bad-libs,gst-libav} \
  mingw-w64-ucrt-x86_64-{openssl,libplist,glib2}
```

### Build

```bash
git clone https://github.com/maxishen0090-oss/CoNSync.git
cd CoNSync
mkdir -p build && cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja -j$(nproc)
```

### Package (requires Inno Setup 6)

```bash
iscc CoNSync_Setup.iss
```

Output: `Output\CoNSync_Setup_v2026.8.7.exe`

---

## Files

| Path | Description |
|------|-------------|
| `CoNSync.cpp` | Main entry point (protocol stack, startup logic, ghost tray thread) |
| `renderers/win32_window.c` | Window management (borderless, subclassing, state persistence) |
| `renderers/video_renderer.c` | GStreamer video pipeline |
| `renderers/audio_renderer.c` | GStreamer audio pipeline |
| `lib/` | AirPlay protocol core (RAOP, HTTP, pairing, crypto, DNS-SD) |
| `CoNSync_Setup.iss` | Inno Setup installer script |
| `run_CoNSync.bat` | Standard launch script |
| `run_CoNSync_fw.bat` | Launch script with firewall auto-config |
| `CoNSync_Launcher.vbs` | Silent launch (no console window) |
| `debug_mode.bat` | Debug mode (visible console) |

---

## Requirements

- **OS:** Windows 10 / Windows 11 (x64)
- **Graphics:** Direct3D 12 compatible GPU
- **Network:** Wi-Fi or Ethernet (same subnet as iOS/macOS device)

---

## License

This project is licensed under the **GNU General Public License v3.0**.  
See [LICENSE](LICENSE) for details.

**Acknowledgments:**
- [UxPlay](https://github.com/antimof/UxPlay) — Original fork source
- [RPiPlay](https://github.com/FD-/RPiPlay) — Original project by Florian Draschbacher
- Apple Bonjour SDK — DNS-SD service discovery
