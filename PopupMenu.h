#include <windows.h>

/*
 * 子控件的弹出菜单窗口相关资源管理
 */
typedef struct popupmenu_ref {
    HRGN hrgn;
    WNDPROC pre_proc;
    WNDPROC proc;
}PopupMenuRef,*pMenuRef;

int PopupMenu_InitialSettings(HWND hwnd);
void PopupMenu_ClearSettings(HWND hwnd);
pMenuRef PopupMenu_GetSettings(HWND hwnd);
int PopupMenu_SetRgn(HWND hwnd,HRGN rgn);

int PopupMenu_DrawMenuItem(HWND hwnd,LPDRAWITEMSTRUCT draw);

LRESULT CALLBACK PopupMenuWndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
