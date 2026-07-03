#ifndef WIN32_WINDOW_H
#define WIN32_WINDOW_H
#ifdef _WIN32
void win32_setup_video_window(void);
void win32_set_aspect_ratio(int src_w, int src_h);
#endif
#endif
