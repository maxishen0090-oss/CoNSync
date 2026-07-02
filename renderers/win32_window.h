#ifndef WIN32_WINDOW_H
#define WIN32_WINDOW_H

#ifdef _WIN32

#include <windows.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

HWND win32_window_init(const char *title, int width, int height);
void win32_window_show(void);
void win32_window_set_aspect_ratio(float ratio);
HWND win32_window_get_handle(void);
bool win32_window_is_closed(void);
void win32_window_destroy(void);
void win32_setup_firewall(void);
void win32_window_resize_to_video(int source_w, int source_h);
float win32_window_get_dpi_scale(void);

#ifdef __cplusplus
}
#endif

#endif /* _WIN32 */
#endif /* WIN32_WINDOW_H */
