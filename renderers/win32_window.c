#include "win32_window.h"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _WIN32_IE 0x0600
#include <windows.h>
#include <dwmapi.h>
#include <shellapi.h>
#include <stdio.h>
#include <string.h>


extern char g_client_id[];

/* ---------- globals ---------- */
static int g_src_w = 0, g_src_h = 0;
static bool g_setup_done = false;
static WNDPROC g_orig_proc = NULL;
HWND g_hwnd = NULL;

/* Window state */
static bool g_topmost = false;
static int g_opacity = 255;
static bool g_icon_added = false;
static NOTIFYICONDATAA g_nid;
static char g_state_path[MAX_PATH] = {0};
static char g_exe_path[MAX_PATH] = {0};
static char g_exe_dir[MAX_PATH] = {0};

#define WM_TRAYICON (WM_APP + 1)
#define MIN_OPACITY 13   /* 5% */
#define MAX_OPACITY 255

/* ---------- state file I/O ---------- */
static void get_state_path(void) {
    if (g_state_path[0]) return;
    GetModuleFileNameA(NULL, g_exe_path, sizeof(g_exe_path));
    /* g_exe_dir: directory containing the .exe */
    strncpy(g_exe_dir, g_exe_path, sizeof(g_exe_dir));
    char *d = strrchr(g_exe_dir, '\\'); if (d) *d = 0;
    /* Go up two dirs: ...\build\CoNSync.exe -> ...\CoNSync-master -> CoNSync */
    char *p = strrchr(g_exe_path, '\\'); if (p) *p = 0;
    p = strrchr(g_exe_path, '\\'); if (p) *p = 0;
    snprintf(g_state_path, sizeof(g_state_path), "%s\\.CoNSync_state", g_exe_path);
}

static void load_state(void) {
    get_state_path();
    FILE *f = fopen(g_state_path, "r");
    if (!f) return;
    int x = 0, y = 0, w = 1280, h = 720, op = 255, top = 0;
    char line[128];
    while (fgets(line, sizeof(line), f)) {
        if (sscanf(line, "opacity=%d", &op) == 1) g_opacity = op;
        else if (sscanf(line, "topmost=%d", &top) == 1) g_topmost = !!top;
    }
    fclose(f);
    if (g_opacity < MIN_OPACITY) g_opacity = MIN_OPACITY;
    if (g_opacity > MAX_OPACITY) g_opacity = MAX_OPACITY;
}

static void save_state(HWND hwnd) {
    get_state_path();
    FILE *f = fopen(g_state_path, "w");
    if (!f) return;
    RECT rc; GetWindowRect(hwnd, &rc);
    fprintf(f, "x=%d\ny=%d\nw=%d\nh=%d\nopacity=%d\ntopmost=%d\n",
            rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
            g_opacity, g_topmost ? 1 : 0);
    fclose(f);
}

/* ---------- client-ID-based state I/O ---------- */
static void save_window_state(int x, int y, int w, int h) {
    char path[256];
    snprintf(path, sizeof(path), "C:\\Users\\Public\\CoNSync_%s.txt", g_client_id);
    FILE* f = fopen(path, "w");
    if (f) {
        fprintf(f, "%d %d %d %d %d", x, y, w, h, g_opacity);
        fclose(f);
        printf("[UI Radar] MAC SAVED: %s\n", path); fflush(stdout);
    }
}

static int load_window_state(HWND hwnd, int* x, int* y, int* w, int* h) {
    char path[256];
    snprintf(path, sizeof(path), "C:\\Users\\Public\\CoNSync_%s.txt", g_client_id);
    FILE* f = fopen(path, "r");
    if (f) {
        int temp_op = 255;
        int read_count = fscanf(f, "%d %d %d %d %d", x, y, w, h, &temp_op);
        if (read_count >= 4) {
            if (read_count == 5) {
                g_opacity = temp_op;
                if (g_opacity < MIN_OPACITY) g_opacity = MIN_OPACITY;
                if (g_opacity > MAX_OPACITY) g_opacity = MAX_OPACITY;
                LONG ex_style = GetWindowLong(hwnd, GWL_EXSTYLE);
                SetWindowLong(hwnd, GWL_EXSTYLE, ex_style | WS_EX_LAYERED);
                SetLayeredWindowAttributes(hwnd, 0, g_opacity, LWA_ALPHA);
            }
            fclose(f);
            printf("[UI Radar] MAC LOADED WITH OPACITY: %d\\n", temp_op); fflush(stdout);
            return 1;
        }
        fclose(f);
    }
    return 0;
}

/* ---------- opacity ---------- */
static void apply_opacity(HWND hwnd) {
    LONG ex = GetWindowLong(hwnd, GWL_EXSTYLE);
    if (g_opacity < MAX_OPACITY) {
        if (!(ex & WS_EX_LAYERED))
            SetWindowLong(hwnd, GWL_EXSTYLE, ex | WS_EX_LAYERED);
        SetLayeredWindowAttributes(hwnd, 0, (BYTE)g_opacity, LWA_ALPHA);
    } else {
        if (ex & WS_EX_LAYERED)
            SetWindowLong(hwnd, GWL_EXSTYLE, ex & ~WS_EX_LAYERED);
    }
}

/* ---------- tray icon ---------- */
static void add_tray_icon(HWND hwnd) {
    if (g_icon_added) return;
    HICON hIcon = NULL;
        char icon_path[MAX_PATH]; snprintf(icon_path, sizeof(icon_path), "%s\\CoNSync.ico", g_exe_dir);

    hIcon = (HICON)LoadImageA(NULL, icon_path, IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
    if (!hIcon && g_exe_path[0]) ExtractIconExA(g_exe_path, 0, &hIcon, NULL, 1);
    if (!hIcon) hIcon = LoadIcon(NULL, IDI_APPLICATION);
    memset(&g_nid, 0, sizeof(g_nid));
    g_nid.cbSize = sizeof(g_nid);
    g_nid.hWnd = hwnd;
    g_nid.uID = 1;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_TRAYICON;
    strcpy(g_nid.szTip, "CoNSync AirPlay");
    g_nid.hIcon = hIcon;
    // Shell_NotifyIconA(NIM_ADD, &g_nid);  // Disabled: ghost tray in CoNSync.cpp
    g_icon_added = true;
}

static void remove_tray_icon(void) {
    if (g_icon_added) {
        // Shell_NotifyIconA(NIM_DELETE, &g_nid);  // Disabled: ghost tray in CoNSync.cpp
        g_icon_added = false;
    }
}
/* ---------- set taskbar window icon ---------- */
static void set_window_icon(HWND hwnd) {
    char icon_path[MAX_PATH]; snprintf(icon_path, sizeof(icon_path), "%s\\CoNSync.ico", g_exe_dir);

    HICON hIcon = (HICON)LoadImageA(NULL, icon_path, IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
    if (!hIcon && g_exe_path[0]) ExtractIconExA(g_exe_path, 0, &hIcon, NULL, 1);
    if (!hIcon) hIcon = LoadIcon(NULL, IDI_APPLICATION);
    if (hIcon) {
        SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
    }
}

static void show_tray_menu(HWND hwnd) {
    HMENU menu = CreatePopupMenu();
    AppendMenuA(menu, MF_STRING, 1, "Open Interface");
    AppendMenuA(menu, MF_SEPARATOR, 0, NULL);
    AppendMenuA(menu, MF_STRING, 2, "Exit Program");
    POINT pt; GetCursorPos(&pt);
    SetForegroundWindow(hwnd);
    int cmd = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_NONOTIFY, pt.x, pt.y, 0, hwnd, NULL);
    DestroyMenu(menu);
    switch (cmd) {
    case 1:
        ShowWindow(hwnd, SW_SHOW);
        SetForegroundWindow(hwnd);
        break;
    case 2: {
        remove_tray_icon();
        HWND console = GetConsoleWindow();
        if (console) PostMessage(console, WM_CLOSE, 0, 0);
        break;
    }}
}

/* ---------- window proc ---------- */
static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {

    case WM_TRAYICON:
        if (lp == WM_LBUTTONUP) {
            if (IsWindowVisible(hwnd)) { ShowWindow(hwnd, SW_HIDE); }
            else { ShowWindow(hwnd, SW_SHOW); SetForegroundWindow(hwnd); }
        } else if (lp == WM_RBUTTONUP) {
            show_tray_menu(hwnd);
        }
        return 0;

    /* ---- opacity: Ctrl+Wheel ---- */
    case WM_MOUSEWHEEL: {
        if (GetAsyncKeyState(VK_CONTROL) < 0) {
            int d = GET_WHEEL_DELTA_WPARAM(wp);
            g_opacity += (d > 0) ? 15 : -15;
            if (g_opacity < MIN_OPACITY) g_opacity = MIN_OPACITY;
            if (g_opacity > MAX_OPACITY) g_opacity = MAX_OPACITY;
            apply_opacity(hwnd);
            {
                RECT rc; GetWindowRect(hwnd, &rc);
                save_window_state(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
            }
            return 0;
        }
        break;
    }

    /* ---- keyboard ---- */
    case WM_KEYDOWN: {
        bool ctrl = (GetAsyncKeyState(VK_CONTROL) < 0);
        bool shift = (GetAsyncKeyState(VK_SHIFT) < 0);

        /* F8: topmost */
        if (wp == VK_F8 && !ctrl && !shift) {
            g_topmost = !g_topmost;
            SetWindowPos(hwnd, g_topmost ? HWND_TOPMOST : HWND_NOTOPMOST,
                         0,0,0,0, SWP_NOMOVE|SWP_NOSIZE);
            save_state(hwnd);
            return 0;
        }
        /* Ctrl+ +/- : opacity */
        if (ctrl && !shift && (wp == VK_ADD || wp == VK_OEM_PLUS || wp == '=')) {
            g_opacity += 15; if (g_opacity > MAX_OPACITY) g_opacity = MAX_OPACITY;
            apply_opacity(hwnd); save_state(hwnd);
            {
                RECT rc; GetWindowRect(hwnd, &rc);
                save_window_state(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
            }
            return 0;
        }
        if (ctrl && !shift && (wp == VK_SUBTRACT || wp == VK_OEM_MINUS || wp == '-')) {
            g_opacity -= 15; if (g_opacity < MIN_OPACITY) g_opacity = MIN_OPACITY;
            apply_opacity(hwnd); save_state(hwnd);
            {
                RECT rc; GetWindowRect(hwnd, &rc);
                save_window_state(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
            }
            return 0;
        }
                /* Ctrl+Shift+Q: hide window to tray */
        if (ctrl && shift && (wp == 'Q' || wp == 'q')) {
            add_tray_icon(hwnd);
            ShowWindow(hwnd, SW_HIDE);
            return 0;
        }
        break;
    }

    /* ---- resize lock ---- */
    case WM_SIZING: {
        RECT *r = (RECT *)lp;
        int w = r->right - r->left;
        if (g_src_w > 0 && g_src_h > 0) {
            int nh = (int)((float)w * (float)g_src_h / (float)g_src_w + 0.5f);
            if (nh < 50) nh = 50;
            r->bottom = r->top + nh;
        }
        return TRUE;
    }
    case WM_EXITSIZEMOVE: {
        RECT rect;
        GetWindowRect(hwnd, &rect);
        save_window_state(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
        break;
    }

    /* ---- hit test: edge resize + Shift+drag ---- */
    case WM_NCHITTEST: {
        POINT pt = { LOWORD(lp), HIWORD(lp) };
        ScreenToClient(hwnd, &pt);
        RECT rc; GetClientRect(hwnd, &rc);
        int e = 8;
        if (pt.y < e) {
            if (pt.x < e) return HTTOPLEFT;
            if (pt.x >= rc.right - e) return HTTOPRIGHT;
            return HTTOP;
        }
        if (pt.y >= rc.bottom - e) {
            if (pt.x < e) return HTBOTTOMLEFT;
            if (pt.x >= rc.right - e) return HTBOTTOMRIGHT;
            return HTBOTTOM;
        }
        if (pt.x < e) return HTLEFT;
        if (pt.x >= rc.right - e) return HTRIGHT;
        return HTCAPTION;
        //return HTCLIENT;  // unreachable after HTCAPTION
    }

    case WM_CLOSE: {
        RECT rect;
        GetWindowRect(hwnd, &rect);
        save_window_state(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
        ShowWindow(hwnd, SW_HIDE);
        return 0;
    }


    /* ---- middle-double-click: hide to tray ---- */
    case WM_MBUTTONDBLCLK:
        add_tray_icon(hwnd);
        ShowWindow(hwnd, SW_HIDE);
    case WM_TIMER:
        if (wp == 9999) {
            KillTimer(hwnd, 9999);
            int tx, ty, tw, th;
            if (load_window_state(hwnd, &tx, &ty, &tw, &th)) {
                SetWindowPos(hwnd, HWND_TOP, tx, ty, tw, th, SWP_NOZORDER | SWP_FRAMECHANGED);
                SendMessage(hwnd, WM_SIZE, 0, 0);
                printf("[UI Radar] Timer Override Executed!\n");
                fflush(stdout);
            ShowWindow(hwnd, SW_SHOWNORMAL);
            }
        }
        break;
    case WM_DESTROY:
        remove_tray_icon();
        g_setup_done = false;
        g_icon_added = false;
        g_orig_proc = NULL;
        break;
    }
    return CallWindowProc(g_orig_proc, hwnd, msg, wp, lp);
}

/* ---------- debug enum ---------- */
static BOOL CALLBACK enum_debug(HWND hwnd, LPARAM lp) {
    DWORD pid;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid != GetCurrentProcessId()) return TRUE;
    char cls[128], title[256];
    GetClassNameA(hwnd, cls, sizeof(cls));
    GetWindowTextA(hwnd, title, sizeof(title));
        char log_path[MAX_PATH]; snprintf(log_path, sizeof(log_path), "%s\\window_debug.txt", g_exe_dir);

    FILE *f = fopen(log_path, "a");
    if (f) { fprintf(f,"pid=%lu class=%s title='%s' vis=%d\n",(unsigned long)pid,cls,title,IsWindowVisible(hwnd)); fclose(f); }
    HWND *r = (HWND *)lp;
    if (!*r && !strstr(cls,"Console")) *r = hwnd;
    return TRUE;
}

/* ---------- apply borderless + load state + tray ---------- */
static void apply_borderless(HWND hwnd) {
    LONG style = GetWindowLong(hwnd, GWL_STYLE);
    style &= ~(WS_CAPTION|WS_THICKFRAME|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_SYSMENU);
    style |= WS_POPUP;
    SetWindowLong(hwnd, GWL_STYLE, style);
    LONG ex = GetWindowLong(hwnd, GWL_EXSTYLE);
    ex &= ~(WS_EX_DLGMODALFRAME|WS_EX_CLIENTEDGE|WS_EX_STATICEDGE);
    SetWindowLong(hwnd, GWL_EXSTYLE, ex);
    SendMessage(hwnd, WM_SIZE, SIZE_RESTORED, 0);
    InvalidateRect(hwnd, NULL, TRUE);

    HMODULE dm = LoadLibraryA("dwmapi.dll");
    if (dm) {
        typedef HRESULT(WINAPI *F)(HWND,DWORD,LPCVOID,DWORD);
        F func = (F)GetProcAddress(dm,"DwmSetWindowAttribute");
        if (func) { DWORD c=2; func(hwnd,33,&c,sizeof(c)); }
        FreeLibrary(dm);
    }

    g_orig_proc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)wnd_proc);
    SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)GetStockObject(BLACK_BRUSH));
    SetWindowPos(hwnd,NULL,0,0,0,0,SWP_FRAMECHANGED|SWP_DRAWFRAME|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER);

    /* Load saved state */
    load_state();
    apply_opacity(hwnd);
    if (g_topmost)
        SetWindowPos(hwnd, HWND_TOPMOST, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE);

    /* Apply saved position/size */
    FILE *f = fopen(g_state_path, "r");
    if (f) {
        int x=0,y=0,w=1280,h=720; char line[128];
        while (fgets(line,sizeof(line),f)) {
            sscanf(line,"x=%d",&x); sscanf(line,"y=%d",&y);
            sscanf(line,"w=%d",&w); sscanf(line,"h=%d",&h);
        }
        fclose(f);
        SetWindowPos(hwnd, NULL, x, y, w, h, SWP_NOZORDER);
    }

    /* Taskbar icon */
    set_window_icon(hwnd);

    /* Tray icon */
    add_tray_icon(hwnd);

    g_hwnd = hwnd;
    int tx, ty, tw, th;
    if (load_window_state(hwnd, &tx, &ty, &tw, &th)) {
        SetWindowPos(hwnd, HWND_TOP, tx, ty, tw, th, SWP_NOZORDER | SWP_FRAMECHANGED);
        SendMessage(hwnd, WM_SIZE, 0, 0);
    }
    SetTimer(hwnd, 9999, 800, NULL);
    printf("[UI Radar] Init Load & Timer Started in apply_borderless\\n"); fflush(stdout);
    g_setup_done = true;
}

/* ---------- public API ---------- */
void win32_setup_video_window(void) {
    if (g_setup_done) {
        /* Verify our subclass is still active 锟斤拷 GStreamer may have overwritten it */
        if (g_nid.hWnd && IsWindow(g_nid.hWnd)) {
            WNDPROC cur = (WNDPROC)GetWindowLongPtr(g_nid.hWnd, GWLP_WNDPROC);
            if (cur == wnd_proc) {
                return;  /* Still valid */
            }
        }
        /* Our subclass was overwritten or the window was destroyed 锟斤拷 re-apply */
        g_icon_added = false;
        g_setup_done = false;
        g_orig_proc = NULL;
    }
        char log_path[MAX_PATH]; snprintf(log_path, sizeof(log_path), "%s\\window_debug.txt", g_exe_dir);

    FILE *f = fopen(log_path,"w");
    if (f) { fprintf(f,"CoNSync PID=%lu\n",(unsigned long)GetCurrentProcessId()); fclose(f); }

    const char *cls[] = {"GstD3D12Window","GstD3D11Window","GstGLWindowD3D11","GstGLWindowEGL","GstWindow","GstSink",NULL};
    for (int i=0; cls[i]; i++) {
        HWND h=FindWindowA(cls[i],NULL);
        if (h) {
            DWORD pid;
            GetWindowThreadProcessId(h, &pid);
            if (pid == GetCurrentProcessId()) { apply_borderless(h); return; }
        }
    }

    HWND h2=NULL; EnumWindows(enum_debug,(LPARAM)&h2);
    if (h2) { apply_borderless(h2); return; }

        char log_path2[MAX_PATH]; snprintf(log_path2, sizeof(log_path2), "%s\\window_debug.txt", g_exe_dir);

    f = fopen(log_path2,"a");
    if (f) { fprintf(f,"NO WINDOW FOUND\n"); fclose(f); }
}

void win32_set_aspect_ratio(int src_w, int src_h) {
    g_src_w=src_w; g_src_h=src_h;
}

#endif








