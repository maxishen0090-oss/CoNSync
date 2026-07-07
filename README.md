
# CoNSync — AirPlay Mirroring & Audio Streaming Receiver for Windows

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](LICENSE)

CoNSync is a Windows port and fork of [UxPlay](https://github.com/antimof/UxPlay) (originally [RPiPlay](https://github.com/FD-/RPiPlay)), an open-source AirPlay mirroring server.  
It allows you to wirelessly mirror your iPhone, iPad, or Mac screen to your Windows PC — no Apple TV required.

---

## Quick Summary

| Capability | Supported |
|-----------|-----------|
| Screen mirroring (H.264) | Yes |
| 4K video (H.265) | Yes |
| Audio streaming (AAC/ALAC) | Yes |
| Multi-device concurrent streaming | Yes |
| Window position & opacity memory | Yes (per-device, MAC-bound) |
| System tray (hide/show/exit) | Yes (independent thread) |
| Bonjour auto-install | Yes (bundled) |
| Firewall auto-config | Yes (UAC only on first run) |
| Windows 11 rounded corners | Yes |

---

## Features — Detailed

### 1. Screen Mirroring
- Stream your iOS/macOS device screen to a Windows PC
- Video: H.264 and H.265 (4K) decoding via GStreamer + D3D12
- Audio: AAC and ALAC via WASAPI
- Automatic resolution matching on first frame

### 2. Window Management
- **Borderless fullscreen-style window** — no title bar, no borders
- **Rounded corners** on Windows 11 (DWM API)
- **Aspect-ratio locked resizing** — drag edges to scale proportionally
- **Shift + drag** — move window by clicking anywhere inside it
- **Always-on-top toggle** — press F8

### 3. Window State Persistence
Each device's window state is saved independently and restored automatically on reconnection:

- Window position (X, Y)
- Window size (width, height)
- Opacity level (5%–100%)
- Always-on-top flag

Saved to: `C:\Users\Public\CoNSync_<DEVICE-MAC>.txt`

### 4. Opacity Control
| Action | Effect |
|--------|--------|
| Ctrl + Mouse Wheel | Increase / decrease opacity |
| Ctrl + Plus (+) | Increase opacity |
| Ctrl + Minus (-) | Decrease opacity |
| Range | 5% (minimum) to 100% (fully opaque) |

### 5. System Tray (Independent Ghost Thread)
The tray runs in its own message loop — completely isolated from the GStreamer rendering pipeline:

| Action | Effect |
|--------|--------|
| Double-click tray icon | Restore / show window |
| Right-click tray icon | Open context menu |
| Menu → Show Window | Restore hidden window |
| Menu → Exit CoNSync | Quit the application entirely |

### 6. Window Close Behavior (Pseudo-Close)
Clicking the window close button (X) does **not** destroy the window or stop the stream.  
It **hides** the window while the GStreamer pipeline continues running.  
To restore: double-click the tray icon or right-click → Show Window.

### 7. Firewall Configuration
- On first run, CoNSync checks if a Windows Firewall rule exists
- If missing, it requests UAC elevation once to add the rule
- On subsequent runs, the check is skipped silently
- No repeated UAC prompts

### 8. Multi-Device Support
- Multiple iOS/macOS devices can connect to the same CoNSync instance
- Each device's window state is saved and restored independently
- Audio streams from different devices are processed concurrently
- No session replacement — all devices stream simultaneously

### 9. Startup Options

| Method | Console Window | Use Case |
|--------|---------------|----------|
| Desktop shortcut (VBS) | Hidden | Daily use (recommended) |
| `CoNSync_Launcher.vbs` | Hidden | Silent launch from scripts |
| `run_CoNSync.bat` | Visible | Debugging with console output |
| `debug_mode.bat` | Visible | Full debug logging |
| `run_CoNSync_fw.bat` | Visible | First-time setup with firewall rule |

### 10. Keyboard Shortcuts (Complete List)

| Shortcut | Action |
|----------|--------|
| `F8` | Toggle always-on-top |
| `Ctrl + Mouse Wheel` | Adjust opacity |
| `Ctrl + Plus (+)` | Increase opacity |
| `Ctrl + Minus (-)` | Decrease opacity |
| `Shift + Drag` | Move window (click anywhere inside) |
| `Ctrl + Shift + Q` | Hide window to tray |
| Tray double-click | Restore window |
| Tray right-click → Show Window | Restore window |
| Tray right-click → Exit CoNSync | Quit |

---

## Installation

1. Download the latest installer from [Releases](https://github.com/maxishen0090-oss/CoNSync/releases)
2. Run `CoNSync_Setup_v2026.8.7.exe`
3. Follow the installer wizard (admin privileges required for Program Files installation and firewall rule)

### What's included in the installer

- `CoNSync.exe` — the main program
- All required MSYS2/UCRT64 runtime DLLs (GCC, GLib, OpenSSL, etc.)
- GStreamer 1.28.4 with all plugins (D3D12, WASAPI, codecs, muxers, etc.)
- Apple Bonjour service installer (installed silently in the background)
- Launch scripts: `run_CoNSync.bat`, `run_CoNSync_fw.bat`, `CoNSync_Launcher.vbs`, `debug_mode.bat`
- CoNSync.ico — application icon
- Uninstaller

Total install size: ~350 MB (compressed installer: 112 MB)

---

## Quick Start

1. After installation, launch CoNSync from the Start Menu or desktop shortcut
2. On your iOS device, open **Control Center → Screen Mirroring**
3. Select **CoNSync** from the device list
4. Your device screen appears on the Windows desktop
5. Use the system tray icon to control the window

### First-time setup
- Windows Firewall may prompt for network access — click **Allow**
- Bonjour service is installed automatically (required for device discovery)

---

## Building from Source

### Prerequisites (MSYS2 UCRT64)

```bash
pacman -S --needed \
  mingw-w64-ucrt-x86_64-{gcc,cmake,ninja,pkg-config} \
  mingw-w64-ucrt-x86_64-{gstreamer,gst-plugins-base,gst-plugins-good,gst-plugins-bad,gst-plugins-bad-libs,gst-libav} \
  mingw-w64-ucrt-x86_64-{openssl,libplist,glib2}
```

Also install [Bonjour SDK for Windows](https://developer.apple.com/download/) (required for `dnssd.lib` at compile time).

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

## Project Structure

| Path | Description |
|------|-------------|
| `CoNSync.cpp` | Main entry: protocol stack, startup logic, ghost tray thread |
| `renderers/win32_window.c` | Window management: borderless, subclassing, state persistence |
| `renderers/video_renderer.c` | GStreamer video pipeline (H.264/H.265 decoding, D3D12 output) |
| `renderers/audio_renderer.c` | GStreamer audio pipeline (AAC/ALAC decoding, WASAPI output) |
| `renderers/mux_renderer.c` | Audio/video muxing for recording |
| `lib/raop.c` | RAOP protocol core (session management, codec negotiation) |
| `lib/raop_rtp.c` | RTP stream receive and parsing |
| `lib/raop_ntp.c` | NTP time synchronization (AV sync) |
| `lib/httpd.c` | Embedded HTTP server (AirPlay signaling) |
| `lib/pairing.c` | AirPlay pairing handshake |
| `lib/crypto.c` | AES decryption (OpenSSL 3.x) |
| `lib/fairplay_playfair.c` | FairPlay content decryption |
| `lib/dnssd.c` | Bonjour/DNS-SD service discovery |
| `lib/playfair/` | Apple FairPlay DRM library |
| `lib/llhttp/` | HTTP protocol parser |
| `lib/mdnsd/` | Embedded mDNS daemon (fallback) |
| `CoNSync_Setup.iss` | Inno Setup installer script |

---

## Requirements

| Component | Requirement |
|-----------|------------|
| OS | Windows 10 / Windows 11 (x64 only) |
| Graphics | Direct3D 12 compatible GPU |
| RAM | 512 MB minimum, 2 GB recommended |
| Network | Wi-Fi or Ethernet, same subnet as iOS/macOS device |
| Disk space | 500 MB for installation |

---

## Acknowledgments

This project is a fork of [UxPlay](https://github.com/antimof/UxPlay), which was itself a fork of [RPiPlay](https://github.com/FD-/RPiPlay) by Florian Draschbacher.

- [RPiPlay](https://github.com/FD-/RPiPlay) — Original AirPlay mirroring server for Raspberry Pi
- [UxPlay](https://github.com/antimof/UxPlay) — macOS/Linux port with extensive enhancements
- Apple Bonjour SDK — DNS-SD service discovery
- GStreamer — Multimedia framework
- OpenSSL — Cryptography

## License

GNU General Public License v3.0. See [LICENSE](LICENSE).
