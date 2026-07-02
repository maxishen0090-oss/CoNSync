# Building CoNSync

## Prerequisites

### Windows
1. **MSYS2** (https://www.msys2.org/)
   - Install to `F:\msys64` or adjust paths in CMakeLists.txt
2. **GStreamer** (via MSYS2):
   ```bash
   pacman -S mingw-w64-ucrt-x86_64-gstreamer mingw-w64-ucrt-x86_64-gst-plugins-base mingw-w64-ucrt-x86_64-gst-plugins-good mingw-w64-ucrt-x86_64-gst-plugins-bad mingw-w64-ucrt-x86_64-gst-plugins-ugly mingw-w64-ucrt-x86_64-gst-libav mingw-w64-ucrt-x86_64-gst-rtsp-server
   ```
3. **Bonjour SDK** (for AirPlay discovery):
   - Apple Bonjour is installed with iTunes/QuickTime
   - Or install from: https://developer.apple.com/download/all/?q=Bonjour
   - Set `BONJOUR_SDK_HOME` environment variable

### macOS
1. **Xcode** or **Command Line Tools**
2. **GStreamer**:
   ```bash
   brew install gstreamer gst-plugins-base gst-plugins-good gst-plugins-bad gst-plugins-ugly gst-libav
   ```
3. **Bonjour** is built-in on macOS (no SDK needed)

## Build Steps

### Windows (MSYS2 UCRT64)
```bash
# Open MSYS2 UCRT64 terminal
cd /path/to/CoNSync

# Configure
export BONJOUR_SDK_HOME="/path/to/bonjour_sdk"
mkdir -p build && cd build
cmake .. -G Ninja -DUSE_DNS_SD=1

# Build
ninja

# Run
./CoNSync.exe
```

### macOS
```bash
cd /path/to/CoNSync
mkdir -p build && cd build
cmake .. -G Ninja
ninja
./CoNSync
```

## Creating Installer (Windows)

1. Install **NSIS** (https://nsis.sourceforge.io/)
2. Build CoNSync first
3. Run:
   ```bash
   makensis installer/setup.nsi
   ```
