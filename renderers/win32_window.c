#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <windowsx.h>
#include <dwmapi.h>
#include <shellapi.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "win32_window.h"

#define WM_TRAYICON (WM_APP + 3)
static HWND     g_hwnd = NULL;
static bool     g_closed = false;
static HANDLE   g_window_ready = NULL;
static HANDLE   g_window_thread = NULL;
static DWORD    g_window_thread_id = 0;
static float    g_aspect = 16.0f / 9.0f;
static int      g_def_width = 1280, g_def_height = 720;
static char     g_title[256] = "CoNSync";
static float    g_dpi_scale = 1.0f;
static bool     g_always_on_top = false;
static int      g_opacity = 255;
static bool     g_saved_size_loaded = false;
static char     g_state_path[MAX_PATH] = {0};

static void     save_window_state(HWND hwnd);
static void     load_window_state(HWND hwnd);
static void     build_state_path(void);

static void set_rounded_corners(HWND hwnd) {
    HMODULE dwm = LoadLibraryA("dwmapi.dll");
    if (dwm) {
        typedef HRESULT (WINAPI *Dwma)(HWND, DWORD, LPCVOID, DWORD);
        Dwma p = (Dwma)GetProcAddress(dwm, "DwmSetWindowAttribute");
        if (p) { DWORD c = 2; p(hwnd, 33, &c, sizeof(c)); }
        FreeLibrary(dwm);
    }
}

static void apply_aspect_ratio(RECT *r, int edge) {
    float ar = g_aspect; if (ar <= 0.01f) return;
    int w = r->right - r->left, h = r->bottom - r->top;
    if (w < 160) w = 160; if (h < 90) h = 90;
    switch (edge) {
    case WMSZ_LEFT: case WMSZ_RIGHT:      w = (int)(h * ar + 0.5f); break;
    case WMSZ_TOP: case WMSZ_BOTTOM:      h = (int)(w / ar + 0.5f); break;
    default: if (w / ar > h) h = (int)(w / ar + 0.5f); else w = (int)(h * ar + 0.5f); break;
    }
    switch (edge) {
    case WMSZ_LEFT:        r->left  = r->right - w; break;
    case WMSZ_RIGHT:       r->right = r->left + w;  break;
    case WMSZ_TOP:         r->top    = r->bottom - h; break;
    case WMSZ_BOTTOM:      r->bottom = r->top + h;    break;
    case WMSZ_TOPLEFT:     r->left = r->right - w; r->top = r->bottom - h; break;
    case WMSZ_TOPRIGHT:    r->right = r->left + w; r->top = r->bottom - h; break;
    case WMSZ_BOTTOMLEFT:  r->left = r->right - w; r->bottom = r->top + h; break;
    case WMSZ_BOTTOMRIGHT: r->right = r->left + w; r->bottom = r->top + h; break;
    }
}

/* ========= main video window procedure ========= */
static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_NCCALCSIZE:
        return 0;

    case WM_NCHITTEST: {
        POINT pt = { GET_X_LPARAM(lp), GET_Y_LPARAM(lp) };
        ScreenToClient(hwnd, &pt);
        RECT rc; GetClientRect(hwnd, &rc);
        int b = 4, x = pt.x, y = pt.y, w = rc.right, h = rc.bottom;
        if      (y < b && x < b)       return HTTOPLEFT;
        else if (y < b && x >= w-b)    return HTTOPRIGHT;
        else if (y >= h-b && x < b)    return HTBOTTOMLEFT;
        else if (y >= h-b && x >= w-b) return HTBOTTOMRIGHT;
        else if (x < b)                return HTLEFT;
        else if (x >= w-b)             return HTRIGHT;
        else if (y < b)                return HTTOP;
        else if (y >= h-b)             return HTBOTTOM;
        if (GetAsyncKeyState(VK_SHIFT) & 0x8000) return HTCAPTION;
        return HTCLIENT;
    }

    case WM_LBUTTONDOWN: {
        POINT pt = { GET_X_LPARAM(lp), GET_Y_LPARAM(lp) };
        RECT rc; GetClientRect(hwnd, &rc);
        int b = 4, x = pt.x, y = pt.y, w = rc.right, h = rc.bottom;
        UINT e = 0;
        if (y < b)        e = (x < b) ? WMSZ_TOPLEFT : (x >= w-b) ? WMSZ_TOPRIGHT : WMSZ_TOP;
        else if (y >= h-b) e = (x < b) ? WMSZ_BOTTOMLEFT : (x >= w-b) ? WMSZ_BOTTOMRIGHT : WMSZ_BOTTOM;
        else if (x < b)   e = WMSZ_LEFT;
        else if (x >= w-b) e = WMSZ_RIGHT;
        if (e) { ReleaseCapture(); SendMessageW(hwnd, WM_SYSCOMMAND, SC_SIZE | e, 0); return 0; }
        return DefWindowProcW(hwnd, msg, wp, lp);
    }

    case WM_SIZING:
        apply_aspect_ratio((RECT *)lp, (int)wp);
        return TRUE;

    case WM_GETMINMAXINFO: {
        MINMAXINFO *m = (MINMAXINFO *)lp;
        m->ptMinTrackSize.x = 320; m->ptMinTrackSize.y = 180;
        return 0;
    }

    case WM_SETCURSOR:
        switch (LOWORD(lp)) {
        case HTLEFT: case HTRIGHT: SetCursor(LoadCursorA(NULL,(LPCSTR)IDC_SIZEWE)); return TRUE;
        case HTTOP: case HTBOTTOM: SetCursor(LoadCursorA(NULL,(LPCSTR)IDC_SIZENS)); return TRUE;
        case HTTOPLEFT: case HTBOTTOMRIGHT: SetCursor(LoadCursorA(NULL,(LPCSTR)IDC_SIZENWSE)); return TRUE;
        case HTTOPRIGHT: case HTBOTTOMLEFT: SetCursor(LoadCursorA(NULL,(LPCSTR)IDC_SIZENESW)); return TRUE;
        default: return DefWindowProcW(hwnd, msg, wp, lp);
        }

    case WM_MOUSEWHEEL:
        if (GetKeyState(VK_CONTROL) & 0x8000) {
            int d = (short)HIWORD(wp) / WHEEL_DELTA * 25;
            g_opacity = max(13, min(255, g_opacity + d));
            SetLayeredWindowAttributes(hwnd, 0, (BYTE)g_opacity, LWA_ALPHA);
            return 0;
        }
        return DefWindowProcW(hwnd, msg, wp, lp);

    case WM_KEYDOWN:
        if (wp == 'Q' && (GetKeyState(VK_CONTROL)&0x8000) && (GetKeyState(VK_SHIFT)&0x8000))
            { ShowWindow(hwnd, SW_HIDE); return 0; }
        if ((wp == VK_OEM_PLUS || wp == VK_ADD) && (GetKeyState(VK_CONTROL)&0x8000))
            { g_opacity = min(255, g_opacity + 25); SetLayeredWindowAttributes(hwnd,0,(BYTE)g_opacity,LWA_ALPHA); return 0; }
        if ((wp == VK_OEM_MINUS || wp == VK_SUBTRACT) && (GetKeyState(VK_CONTROL)&0x8000))
            { g_opacity = max(13, g_opacity - 25); SetLayeredWindowAttributes(hwnd,0,(BYTE)g_opacity,LWA_ALPHA); return 0; }
        if (wp == VK_F8) {
            g_always_on_top = !g_always_on_top;
            SetWindowPos(hwnd, g_always_on_top ? HWND_TOPMOST : HWND_NOTOPMOST, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
            return 0;
        }
        return 0;

    case WM_NCPAINT:
        return 0;

    case WM_TRAYICON: {
        if (LOWORD(lp) == WM_RBUTTONDOWN) {
            HMENU hMenu = CreatePopupMenu();
            AppendMenuW(hMenu, MF_STRING, 1001, L"\u6253\u5f00\u7a97\u53e3");
            AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
            AppendMenuW(hMenu, MF_STRING, 1002, L"\u9000\u51fa");
            POINT pt; GetCursorPos(&pt);
            SetForegroundWindow(hwnd);
            TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
            DestroyMenu(hMenu);
        } else if (LOWORD(lp) == WM_LBUTTONDOWN) {
            ShowWindow(hwnd, SW_SHOW);
            SetForegroundWindow(hwnd);
        }
        return 0;
    }

    case WM_COMMAND:
        if (LOWORD(wp) == 1001) {
            ShowWindow(hwnd, SW_SHOW);
            SetForegroundWindow(hwnd);
        } else if (LOWORD(wp) == 1002) {
            DestroyWindow(hwnd);
        }
        return 0;

    case WM_CLOSE:
        /* Hide window instead of closing */
        ShowWindow(hwnd, SW_HIDE);
        return 0;

    case WM_DESTROY: {
        NOTIFYICONDATAW nd = {sizeof(nd)};
        nd.hWnd = hwnd; nd.uID = 1;
        Shell_NotifyIconW(NIM_DELETE, &nd);
        save_window_state(hwnd);
        g_closed = true;
        PostQuitMessage(0);
        return 0;
    }

    default:
        return DefWindowProcW(hwnd, msg, wp, lp);
    }
}

/* ========= state persistence ========= */
static void build_state_path(void) {
    if (g_state_path[0]) return;
    GetEnvironmentVariableA("USERPROFILE", g_state_path, MAX_PATH);
    strncat(g_state_path, "\\.consync_window", MAX_PATH - strlen(g_state_path) - 1);
}

static void save_window_state(HWND hwnd) {
    build_state_path();
    RECT rc; GetWindowRect(hwnd, &rc);
    FILE *f = fopen(g_state_path, "w");
    if (!f) return;
    fprintf(f, "pos_x=%d\npos_y=%d\nwidth=%d\nheight=%d\ntopmost=%d\nopacity=%d\n",
            rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top,
            g_always_on_top ? 1 : 0, g_opacity);
    fclose(f);
}

static void load_window_state(HWND hwnd) {
    build_state_path();
    FILE *f = fopen(g_state_path, "r");
    if (!f) return;
    int x=-1,y=-1,w=-1,h=-1,topmost=0,opacity=255;
    char line[64];
    while (fgets(line,sizeof(line),f)) {
        int v=0;
        if (sscanf(line,"pos_x=%d",&v)==1) x=v;
        else if (sscanf(line,"pos_y=%d",&v)==1) y=v;
        else if (sscanf(line,"width=%d",&v)==1) w=v;
        else if (sscanf(line,"height=%d",&v)==1) h=v;
        else if (sscanf(line,"topmost=%d",&v)==1) topmost=v;
        else if (sscanf(line,"opacity=%d",&v)==1) opacity=v;
    }
    fclose(f);
    if (w>0 && h>0) {
        if (x<0) x=(GetSystemMetrics(SM_CXSCREEN)-w)/2;
        if (y<0) y=(GetSystemMetrics(SM_CYSCREEN)-h)/2;
        SetWindowPos(hwnd,NULL,x,y,w,h,SWP_NOZORDER|SWP_NOACTIVATE);
        g_saved_size_loaded = true;
    }
    if (topmost) { g_always_on_top=true; SetWindowPos(hwnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE); }
    if (opacity>=0 && opacity<=255) { g_opacity=opacity; SetLayeredWindowAttributes(hwnd,0,(BYTE)opacity,LWA_ALPHA); }
}

/* ========= window thread ========= */
static DWORD WINAPI window_thread(LPVOID param) {
    (void)param;
    HINSTANCE hi = GetModuleHandleW(NULL);
    WNDCLASSEXW wc = {sizeof(wc)};
    wc.style = CS_HREDRAW|CS_VREDRAW;
    wc.lpfnWndProc = wnd_proc;
    wc.hInstance = hi;
    wc.hCursor = LoadCursorA(NULL,(LPCSTR)IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszClassName = L"CoNSyncVideoWnd";
    RegisterClassExW(&wc);

    DWORD style = WS_POPUP;
    RECT wr = {0,0,g_def_width,g_def_height};
    AdjustWindowRectEx(&wr,style,FALSE,0);
    int win_w=wr.right-wr.left, win_h=wr.bottom-wr.top;
    int sw=GetSystemMetrics(SM_CXSCREEN), sh=GetSystemMetrics(SM_CYSCREEN);
    int x=(sw-win_w)/2, y=(sh-win_h)/2;

    wchar_t wtitle[256];
    MultiByteToWideChar(CP_UTF8,0,g_title,-1,wtitle,256);

    g_hwnd = CreateWindowExW(WS_EX_LAYERED, L"CoNSyncVideoWnd", wtitle, style,
                             x,y,win_w,win_h, NULL,NULL,hi,NULL);
    if (!g_hwnd) { SetEvent(g_window_ready); return 1; }

    set_rounded_corners(g_hwnd);
    SetLayeredWindowAttributes(g_hwnd,0,255,LWA_ALPHA);
    g_dpi_scale = GetDpiForWindow(g_hwnd)/96.0f;
    load_window_state(g_hwnd);
    SetEvent(g_window_ready);

    /* Add system tray icon */
    NOTIFYICONDATAW nid = {sizeof(nid)};
    nid.hWnd = g_hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIconW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(1));
    wcscpy(nid.szTip, L"CoNSync - AirPlay Mirroring");
    Shell_NotifyIconW(NIM_ADD, &nid);

    MSG msg;
    while (GetMessageW(&msg,NULL,0,0)) { TranslateMessage(&msg); DispatchMessageW(&msg); }
    g_hwnd = NULL;
    return 0;
}

/* ========= public API ========= */
HWND win32_window_init(const char *title, int width, int height) {
    if (g_window_ready) return NULL;
    if (title) { strncpy(g_title,title,sizeof(g_title)-1); g_title[sizeof(g_title)-1]=0; }
    if (width>0) g_def_width=width;
    if (height>0) g_def_height=height;
    g_closed = false;
    g_window_ready = CreateEventW(NULL,TRUE,FALSE,NULL);
    if (!g_window_ready) return NULL;
    g_window_thread = CreateThread(NULL,0,window_thread,NULL,0,&g_window_thread_id);
    if (!g_window_thread) { CloseHandle(g_window_ready); g_window_ready=NULL; return NULL; }
    WaitForSingleObject(g_window_ready,5000);
    return g_hwnd;
}

void win32_window_show(void) {
    if (g_hwnd) { ShowWindow(g_hwnd,SW_SHOW); SetForegroundWindow(g_hwnd); }
}

void win32_window_set_aspect_ratio(float ratio) {
    if (ratio>0.01f && ratio<100.0f) g_aspect = ratio;
}

HWND win32_window_get_handle(void) { return g_hwnd; }
bool win32_window_is_closed(void) { return g_closed; }

void win32_window_destroy(void) {
    if (g_hwnd) PostMessageW(g_hwnd,WM_CLOSE,0,0);
    if (g_window_thread) { WaitForSingleObject(g_window_thread,3000); CloseHandle(g_window_thread); g_window_thread=NULL; }
    if (g_window_ready) { CloseHandle(g_window_ready); g_window_ready=NULL; }
    g_hwnd = NULL;
}

void win32_window_resize_to_video(int source_w, int source_h) {
    if (!g_hwnd) return;
    if (g_saved_size_loaded) {
        g_saved_size_loaded = false;
        win32_window_set_aspect_ratio((float)source_w/(float)source_h);
        win32_window_show();
        return;
    }
    int sw=GetSystemMetrics(SM_CXSCREEN), sh=GetSystemMetrics(SM_CYSCREEN);
    int mw=(int)(sw*0.9f), mh=(int)(sh*0.9f);
    float r = (float)source_w/(float)source_h;
    int w=source_w, h=source_h;
    if (w>mw) { w=mw; h=(int)(w/r); }
    if (h>mh) { h=mh; w=(int)(h*r); }
    SetWindowPos(g_hwnd,NULL,(sw-w)/2,(sh-h)/2,w,h, SWP_NOZORDER|SWP_NOACTIVATE);
    win32_window_set_aspect_ratio(r);
    win32_window_show();
}

float win32_window_get_dpi_scale(void) { return g_dpi_scale; }

void win32_setup_firewall(void) {
    char exe[MAX_PATH];
    DWORD len = GetModuleFileNameA(NULL,exe,MAX_PATH);
    if (len==0||len>=MAX_PATH) return;

    char tmp[MAX_PATH];
    GetTempPathA(MAX_PATH,tmp);
    strncat(tmp,"\\consync_fw.txt",MAX_PATH-strlen(tmp)-1);

    char cmd[MAX_PATH*2];
    _snprintf(cmd,sizeof(cmd),
        "netsh advfirewall firewall show rule name='CoNSync' > \"%s\" 2>nul",tmp);
    system(cmd);

    FILE *f = fopen(tmp,"r");
    if (f) {
        char line[256];
        while (fgets(line,sizeof(line),f)) { if (strstr(line,"CoNSync")) { fclose(f); remove(tmp); return; } }
        fclose(f);
    }
    remove(tmp);

    fprintf(stderr,"[CoNSync] Adding firewall rule (UAC prompt)...\n");
    _snprintf(cmd,sizeof(cmd),
        "netsh advfirewall firewall add rule name='CoNSync' dir=in action=allow program=\"%s\" enable=yes profile=any",exe);
    ShellExecuteA(NULL,"runas","netsh",cmd,NULL,SW_HIDE);
    fprintf(stderr,"[CoNSync] Firewall rule added.\n");
}
