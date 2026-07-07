# CoNSync — AirPlay Mirroring Server for Windows

> 版本 **1.74 (2026.7)** | 代码封印完成 | 发布就绪

CoNSync 是一个 Windows 平台上的 AirPlay 镜像接收服务器。iOS / macOS / iPadOS 设备可以通过系统自带的 AirPlay 屏幕镜像功能，将画面无线投屏到这台 Windows 电脑上。

## 功能特性

- ✅ **AirPlay 屏幕镜像**（H.264 / H.265 视频 + AAC / ALAC 音频）
- ✅ **完全无边框视频窗口**（Windows 11 圆角支持）
- ✅ **窗口置顶切换**（F8 快捷键）
- ✅ **透明度调节**（Ctrl + 滚轮 / Ctrl + +/-，最低 5%）
- ✅ **Shift + 拖拽移动窗口**
- ✅ **等比例缩放窗口**（保持原始宽高比）
- ✅ **窗口位置 + 透明度持久化记忆**（基于设备 MAC 地址绑定）
- ✅ **800ms 延迟反杀 GStreamer 覆盖**
- ✅ **冷启动 WS_EX_LAYERED 强制修复**
- ✅ **系统托盘独立线程运行**（隐藏窗口消息循环，退出菜单）
- ✅ **防火墙规则智能检测**（仅首次提权）
- ✅ **Bonjour 自动静默安装**
- ✅ **多设备独立连接**

## 快速开始

1. 下载安装包：[Releases](https://github.com/maxishen0090-oss/CoNSync/releases)
2. 运行 CoNSync_Setup_v1.74.exe
3. iOS 设备打开控制中心 → 屏幕镜像 → 选择 "CoNSync"

## 快捷键

| 快捷键 | 功能 |
|--------|------|
| F8 | 切换窗口置顶 |
| Ctrl + 滚轮 | 调节透明度 |
| Ctrl + + / Ctrl + - | 调节透明度 |
| Shift + 拖拽 | 移动窗口 |
| Ctrl + Shift + Q | 隐藏窗口到托盘 |
| 托盘双击 | 唤醒窗口 |
| 托盘右键 → Show Window | 唤醒窗口 |
| 托盘右键 → Exit CoNSync | 退出程序 |

## 系统要求

- Windows 10 / Windows 11 (x64)
- 支持 Direct3D 12 的显卡
- 同一局域网

## 构建

`ash
# MSYS2 UCRT64 环境
pacman -S --needed mingw-w64-ucrt-x86_64-{gcc,cmake,ninja,pkg-config,gstreamer,gst-plugins-base,gst-plugins-good,gst-plugins-bad,gst-plugins-bad-libs,gst-libav,openssl,libplist,glib2}

cd CoNSync-master
mkdir -p build && cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja -j\
proc\
`

## 许可证

GPL v3 — 详见 [LICENSE](LICENSE)

本项目基于 [RPiPlay](https://github.com/FD-/RPiPlay) 修改，原始版权归 Florian Draschbacher 所有。
