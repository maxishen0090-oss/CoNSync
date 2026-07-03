# CoNSync AirPlay Mirroring Server — 部署技术文档

> 版本：1.74 | 最后更新：2026-07-03  
> 源码位置：`F:\CoNSync\huifu\CoNSync-master\`  
> 编译产物：`F:\CoNSync\huifu\CoNSync-master\build\CoNSync.exe`

---

## 一、项目简介

CoNSync 是一个 **Windows 平台上的 AirPlay 镜像接收服务器**。iOS / macOS / iPadOS 设备可以通过系统自带的 AirPlay 屏幕镜像功能，将画面无线投屏到这台 Windows 电脑上。

### 功能特性

- AirPlay 屏幕镜像（H.264 / H.265 视频 + AAC / ALAC 音频）
- 完全无边框视频窗口（Windows 11 圆角支持）
- 窗口置顶切换（F8 快捷键）
- 透明度调节（Ctrl + 滚轮 / Ctrl + +/-，最低 5%）
- Shift + 拖拽移动窗口
- 等比例缩放窗口（保持原始宽高比）
- 窗口状态自动记忆（位置、大小、透明度、置顶状态）
- 系统托盘后台运行（关闭窗口自动缩到托盘）
- 任务栏自定义图标
- 托盘菜单：Open Interface / Exit Program（英文）
- 退出快捷键：Ctrl + Shift + Q
- 防火墙自动放行
- 蓝牙 LE 信标发现（可选）

---

## 二、程序工作原理

```
┌─────────────────────────────────────────────────────────────────┐
│                       启动流程                                   │
│                                                                  │
│  run_CoNSync.cmd                                                 │
│       │                                                          │
│       ① 设置环境变量                                              │
│          MSYSTEM=UCRT64                                          │
│          GST_PLUGIN_SYSTEM_PATH=指向 GStreamer 插件目录            │
│          GST_PLUGIN_PATH=同上                                     │
│       │                                                          │
│       ② 通过 bash.exe 启动 CoNSync.exe                            │
│          bash -l -c "/path/to/CoNSync -vs d3d12videosink ..."     │
│       │                                                          │
│       ③ CoNSync.exe 启动                                         │
│          ├── 初始化 GStreamer 视频/音频管道                         │
│          ├── 启动 DNSSD 服务广播（Bonjour）                        │
│          ├── 启动 HTTP 服务器（AirPlay 信令）                       │
│          └── 等待客户端连接                                        │
│                                                                  │
│  iOS / macOS 设备发现并连接                                        │
│       │                                                          │
│       ④ AirPlay 配对 → 加密握手                                   │
│       ⑤ 视频流 → GStreamer 解码 → d3d12videosink 渲染             │
│       ⑥ 音频流 → wasapisink 播放                                  │
│       │                                                          │
│       ⑦ 视频窗口出现                                              │
│          → win32_setup_video_window() 检测到窗口                   │
│          → apply_borderless() 应用全部自定义功能                    │
│             ├── 无边框 + 圆角                                      │
│             ├── 窗口过程子类化（接管所有消息）                       │
│             ├── 设置任务栏图标（CoNSync.ico）                       │
│             ├── 添加系统托盘图标                                    │
│             ├── 加载记忆的状态（位置/大小/透明度/置顶）              │
│             └── 所有快捷键生效                                     │
└─────────────────────────────────────────────────────────────────┘
```

### 窗口自定义功能的实现机制

所有窗口功能（无边框、托盘、图标、快捷键、透明度等）通过 **Windows 窗口子类化（Subclassing）** 实现：

1. `SetWindowLongPtr(hwnd, GWLP_WNDPROC, wnd_proc)` 接管 GStreamer 视频窗口的窗口过程
2. 在 `wnd_proc` 中处理 `WM_CLOSE` → 隐藏到托盘、`WM_NCHITTEST` → 边缘缩放、键盘事件 → 快捷键等
3. 原始窗口过程保存在 `g_orig_proc`，未被处理的消息通过 `CallWindowProc` 回传

核心源文件：`renderers/win32_window.c`

---

## 三、项目文件结构

```
F:\CoNSync\
├── huifu\                              ← 完整可恢复备份
│   └── CoNSync-master\                 ← 所有源码 + 编译产物
│       ├── CoNSync.cpp                 ← 主程序入口 (AirPlay 协议实现)
│       ├── CoNSync.rc                  ← Windows 资源文件（绑定图标）
│       ├── CoNSync.ico                 ← 程序图标文件
│       ├── CMakeLists.txt              ← 顶层构建配置
│       │
│       ├── lib\                        ← AirPlay 协议层（编译为 libairplay.a）
│       │   ├── raop.c / raop_rtp.c    ← 实时音视频传输协议
│       │   ├── httpd.c                ← 嵌入式 HTTP 服务器
│       │   ├── http_request.c         ← HTTP 请求解析
│       │   ├── http_response.c        ← HTTP 响应构建
│       │   ├── pairing.c              ← AirPlay 配对握手
│       │   ├── srp.c                  ← Secure Remote Password 协议
│       │   ├── crypto.c               ← AES 解密（基于 OpenSSL）
│       │   ├── dnssd.c                ← Bonjour 网络服务发现
│       │   ├── fairplay_playfair.c    ← FairPlay 解密
│       │   ├── mirror_buffer.c        ← 镜像数据缓冲
│       │   ├── logger.c               ← 日志系统
│       │   └── ...（约 30+ 个 .c 文件）
│       │
│       ├── lib\playfair\              ← Apple FairPlay DRM 解密
│       ├── lib\llhttp\                ← HTTP 协议解析引擎
│       ├── lib\dns_sd\                ← DNS-SD (mDNS) 实现
│       ├── lib\mdnsd\                 ← 备选 mDNS 实现
│       │
│       ├── renderers\                 ← 渲染层（编译为 librenderers.a）
│       │   ├── video_renderer.c       ← GStreamer 视频管道构建与管理
│       │   ├── audio_renderer.c       ← GStreamer 音频管道构建与管理
│       │   ├── mux_renderer.c         ← 音视频混合录制
│       │   ├── win32_window.c         ← ⭐ 窗口自定义功能核心（约 340 行）
│       │   │     - 无边框窗口 / 圆角
│       │   │     - 系统托盘 / 任务栏图标
│       │   │     - 快捷键 (F8/Ctrl+滚轮/Ctrl+Shift+Q)
│       │   │     - Shift+拖拽移动
│       │   │     - 等比例缩放 / 窗口记忆
│       │   │     - 定时验证子类化安全（防 GStreamer 覆盖）
│       │   └── win32_window.h         ← 接口声明
│       │
│       ├── build\                     ← 编译输出
│       │   ├── CoNSync.exe            ← 已编译好的可执行文件
│       │   ├── build.ninja            ← Ninja 构建规则
│       │   └── CMakeCache.txt         ← CMake 配置缓存
│       │
│       └── run_CoNSync.cmd            ← 启动脚本模板
│
├── run_CoNSync.cmd                    ← 本机启动脚本
└── scan_airplay.py                    ← AirPlay 设备扫描辅助脚本
```

---

## 四、依赖清单

### 4.1 编译依赖（仅在需要重新编译时用到）

| 组件 | 版本 | MSYS2 包名 | 用途 |
|------|------|-----------|------|
| GCC | 16.1.0 | `mingw-w64-ucrt-x86_64-gcc` | C/C++ 编译器 |
| CMake | 4.3.4 | `mingw-w64-ucrt-x86_64-cmake` | 构建系统 |
| Ninja | 1.13.2 | `mingw-w64-ucrt-x86_64-ninja` | 构建执行器 |
| pkg-config | — | `mingw-w64-ucrt-x86_64-pkg-config` | 依赖查找 |
| GStreamer | 1.28.4 | `mingw-w64-ucrt-x86_64-gstreamer` | 音视频解码渲染框架 |
| GStreamer Base | 1.28.4 | `...-gst-plugins-base` | 基础插件集 |
| GStreamer Good | 1.28.4 | `...-gst-plugins-good` | 标准插件集 |
| GStreamer Bad | 1.28.4 | `...-gst-plugins-bad` | 扩展插件集（含 d3d12） |
| GStreamer libav | 1.28.4 | `...-gst-libav` | FFmpeg 编解码后端 |
| GLib | 2.88.2 | `mingw-w64-ucrt-x86_64-glib2` | GStreamer 底层基础库 |
| OpenSSL | 3.6.3 | `mingw-w64-ucrt-x86_64-openssl` | 加密、AirPlay 配对 |
| libplist | 2.7.0 | `mingw-w64-ucrt-x86_64-libplist` | Apple plist 格式解析 |
| Bonjour SDK | — | 外部安装（Apple 官方） | mDNS 服务发现 |

### 4.2 运行时依赖（部署时必须包含）

`CoNSync.exe` 编译为动态链接，需要以下文件才能运行：

```
msys64\ucrt64\bin\                     ← 约 28 个关键 DLL
├── libgcc_s_seh-1.dll                 [GCC 异常处理运行时]
├── libwinpthread-1.dll                [POSIX 线程库]
├── libstdc++-6.dll                    [C++ 标准库]
├── libcrypto-3-x64.dll                [OpenSSL 加密库]
├── libglib-2.0-0.dll                  [GLib 核心]
├── libgobject-2.0-0.dll               [GObject 类型系统]
├── libgmodule-2.0-0.dll               [GLib 模块加载]
├── libgthread-2.0-0.dll               [GLib 线程支持]
├── libgstreamer-1.0-0.dll             [GStreamer 核心]
├── libgstapp-1.0-0.dll                [GStreamer App 辅助]
├── libgstbase-1.0-0.dll               [GStreamer 基础]
├── libgstvideo-1.0-0.dll              [GStreamer 视频]
├── libgstsdp-1.0-0.dll                [GStreamer SDP]
├── libplist-2.0.dll                   [Apple plist 解析]
├── libintl-9.dll / libintl-8.dll      [国际化]
├── libiconv-2.dll                     [字符编码转换]
└── libpcre2-8-0.dll                   [正则表达式]

msys64\ucrt64\lib\gstreamer-1.0\     ← 242 个 GStreamer 插件 DLL
├── libgstd3d12.dll                   [Direct3D 12 视频渲染器]
├── libgstwasapi.dll                  [Windows Audio API 音频渲染器]
├── libgstdecodebin2.dll              [自动解码器]
├── libgsth264parse.dll               [H.264 码流解析]
├── libgsth265parse.dll               [H.265 码流解析]
├── libgstvideoconvert.dll            [视频格式转换]
├── libgstvideoscale.dll              [视频缩放]
├── libgstvideorate.dll               [视频帧率调整]
├── libgstcapssetter.dll              [Caps 过滤器]
├── libgstqueue2.dll                  [队列/缓冲]
├── libgstapp.dll                     [AppSrc/AppSink]
├── libgstcoreelements.dll            [核心元素]
├── libgsttypefindfunctions.dll       [类型检测]
├── libgstautodetect.dll              [自动检测]
└── ...（其余 228 个插件，大多数为可选加载）

msys64\usr\bin\
├── bash.exe                          [MSYS2 shell，启动入口]
├── msys-2.0.dll                      [Cygwin 兼容层]
└── cygreadline7.dll                  [命令行编辑]
```

### 4.3 配置依赖

| 文件 | 用途 |
|------|------|
| `Bonjour SDK\Lib\x64\dnssd.lib` | mDNS 服务发现静态库（编译时链接到 .exe 中） |
| `CoNSync.ico` | 程序图标（编译时绑定到 .rc 资源中） |

---

## 五、部署方案对比

### 方案 A：完整 MSYS2 环境拷贝（推荐内部分发）

```
操作方式：将 F:\msys64\ 和 F:\CoNSync\ 两个文件夹完整复制到目标机
交付大小：约 5 GB
目标机路径：建议保持与源机一致（如 F:\msys64\），否则需要改脚本路径
```

**优点：**
- 最快：拷贝即可用，无需任何安装步骤
- 最完整：包括编译工具链，可以在目标机继续开发
- 路径一致时零配置

**缺点：**
- 体积大（5 GB），包含大量开发用文件
- 包含 MSYS2 系统文件，不适合对外分发

---

### 方案 B：精简运行时包（适合外部分发）

```
操作方式：用脚本提取仅运行必需的 DLL 和插件，与 CoNSync.exe 打包
交付大小：约 300 MB（可压缩至 100-150 MB）
交付物：一个自包含文件夹，解压即用
```

**优点：**
- 干净：不含开发文件，只有运行所需
- 绿色：无需安装，解压后双击 .cmd 即可运行
- 无路径依赖：可以放在任何目录

**缺点：**
- 需要提前执行一次提取脚本
- 不支持在目标机重新编译

---

### 方案 C：InnoSetup 安装包（适合正式发布）

```
操作方式：编写 InnoSetup 脚本，将运行时 + .exe 打包为 Windows 安装程序
交付大小：约 150 MB（压缩后）
交付物：Setup_CoNSync_x.x.x.exe，双击安装
```

**优点：**
- 专业体验：有安装向导、开始菜单快捷方式、卸载程序
- 自动处理防火墙规则（UAC 提权）
- 可以添加自动启动、文件关联等

**缺点：**
- 需要编写 InnoSetup 脚本（一次性工作）
- 更新时需要重新打包

---

## 六、部署后使用说明

### 启动

```bash
# 双击 run_CoNSync.cmd
# 或在命令行中运行
cd /d F:\CoNSync
run_CoNSync.cmd
```

### 首次运行

1. 首次运行会弹出防火墙提示，需要允许入站连接
2. 确保电脑和 iOS/macOS 设备在**同一局域网**内
3. 在 iOS/macOS 设备上打开控制中心 → 屏幕镜像 → 选择 "CoNSync"

### 快捷键

| 快捷键 | 功能 |
|--------|------|
| F8 | 切换窗口置顶 |
| Ctrl + 滚轮 | 调节透明度 |
| Ctrl + + / Ctrl + - | 调节透明度 |
| Shift + 拖拽 | 移动窗口（点击画面任意位置拖拽） |
| Ctrl + Shift + Q | 隐藏窗口到系统托盘 |
| 托盘菜单 "Open Interface" | 显示窗口 |
| 托盘菜单 "Exit Program" | 完全退出程序 |

---

## 七、重新编译说明（按需）

如果需要在目标机上修改源码并重新编译：

```bash
# 1. 打开 MSYS2 UCRT64 终端
# 2. 安装编译依赖（如尚未安装）
pacman -S --needed \
  mingw-w64-ucrt-x86_64-gcc \
  mingw-w64-ucrt-x86_64-cmake \
  mingw-w64-ucrt-x86_64-ninja \
  mingw-w64-ucrt-x86_64-pkg-config \
  mingw-w64-ucrt-x86_64-gstreamer \
  mingw-w64-ucrt-x86_64-gst-plugins-base \
  mingw-w64-ucrt-x86_64-gst-plugins-good \
  mingw-w64-ucrt-x86_64-gst-plugins-bad \
  mingw-w64-ucrt-x86_64-gst-plugins-bad-libs \
  mingw-w64-ucrt-x86_64-gst-libav \
  mingw-w64-ucrt-x86_64-openssl \
  mingw-w64-ucrt-x86_64-libplist \
  mingw-w64-ucrt-x86_64-glib2

# 3. 编译
cd /path/to/CoNSync-master
mkdir -p build && cd build
cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=Release
ninja -j$(nproc)
```

### 增量编译（修改后快速重编）

```bash
cd /path/to/CoNSync-master/build
ninja
```

Ninja 会自动检测哪些文件变更，仅重新编译修改的部分，通常只需 1-3 秒。

---

## 八、关键源码快速索引

| 文件 | 行数 | 说明 |
|------|------|------|
| `renderers/win32_window.c` | ~340 | ⭐ 所有窗口自定义功能入口 |
| `renderers/win32_window.h` | ~10 | 窗口功能接口声明 |
| `renderers/video_renderer.c` | ~1200 | GStreamer 视频管道（含窗口初始化调用） |
| `renderers/audio_renderer.c` | ~400 | GStreamer 音频管道 |
| `CoNSync.cpp` | ~3300 | 主程序（AirPlay 协议栈、启动逻辑） |
| `CMakeLists.txt` | ~100 | 构建配置 |

---

## 九、恢复备份

所有源码和编译产物的完整备份位于：

```
F:\CoNSync\huifu\
├── CoNSync-master\      ← 完整源码 + 构建配置 + CoNSync.exe
├── run_CoNSync.cmd      ← 启动脚本
└── run_CoNSync_d3d.cmd  ← D3D 启动脚本
```

如需恢复到当前状态，将 `huifu\` 下全部内容覆盖回 `F:\CoNSync\` 即可。

---

*文档版本：v1.0 | 适用版本：CoNSync 1.74*
