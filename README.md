# CoNSync AirPlay Receiver

[![Download](https://img.shields.io/github/v/release/maxishen0090-oss/CoNSync?color=blue&label=Download&style=for-the-badge)](https://github.com/maxishen0090-oss/CoNSync/releases/latest)
[![Platform](https://img.shields.io/badge/Platform-Windows_11_x64-blue?style=for-the-badge)]()
[![License](https://img.shields.io/badge/License-GPLv3-green?style=for-the-badge)](LICENSE)

---

将你的 Windows 电脑变成一个 **AirPlay 无线投屏接收器**。
iOS / macOS / iPadOS 设备无需安装任何 App，通过系统自带的屏幕镜像功能即可投屏。

## 快速开始

**[⬇ 下载最新安装包](https://github.com/maxishen0090-oss/CoNSync/releases/latest)**

1. 下载 `CoNSync_AirPlay_Receiver_Setup_1.74.exe`
2. 双击运行，一路下一步（会自动提权配置防火墙）
3. 安装完成后，iOS/macOS 设备打开控制中心 → 屏幕镜像 → 选择 **CoNSync**

## 功能特性

| 功能 | 说明 |
|------|------|
| 屏幕镜像 | H.264 / H.265 视频 + AAC / ALAC 音频 |
| 无边框窗口 | Windows 11 圆角支持 |
| 窗口置顶 | F8 快捷键切换 |
| 透明度调节 | Ctrl + 滚轮 / Ctrl + +/- |
| 移动窗口 | Shift + 拖拽画面任意位置 |
| 窗口状态记忆 | 关闭后自动记忆位置、大小、透明度 |
| 系统托盘 | 关闭窗口自动缩到托盘，右键菜单管理 |
| 网络自动修复 | 检测到公用网络或防火墙拦截时自动弹窗提权修复 |
| 零依赖性 | 内置 mDNS 实现 + 捆绑 Bonjour 静默安装 |
| 绿色便携 | 仓库中 `build_release.ps1` 可一键生成免安装版 |

## 仓库内容

```
CoNSync-master/
├── CoNSync.cpp              ← AirPlay 协议栈主程序
├── renderers/
│   ├── win32_window.c       ← 窗口自定义功能（无边框/托盘/快捷键）
│   ├── video_renderer.c     ← GStreamer 视频管道
│   └── audio_renderer.c     ← GStreamer 音频管道
├── lib/                     ← AirPlay 协议层
├── Release_Installer/       ← 最终安装包（通过 Releases 下载）
├── build_release.ps1        ← 一键生成绿色发布包
├── CoNSync_Launcher.vbs     ← 静默启动器（无黑框）
├── debug_mode.bat           ← 调试启动器（显示 GStreamer 日志）
├── CoNSync_AirPlay_Receiver_Setup.iss  ← Inno Setup 安装包脚本
├── Bonjour64.msi            ← Apple Bonjour 服务
└── huifu/                   ← 完整源码备份
```

## 从源码编译

需要 MSYS2 UCRT64 环境：

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-{gcc,cmake,ninja,gstreamer,gst-plugins-{base,good,bad},gst-libav,openssl,libplist,glib2}
cd CoNSync-master && mkdir build && cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja -j$(nproc)
```

## License

GNU General Public License v3.0
