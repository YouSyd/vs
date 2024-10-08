#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include "exec_cmd.h"

#define SCROLLBAR_PIXLS 13
#define THUMB_MIN_PIXLS 50
#define CTRL_CMDCONSOLE "CmdConsole"

typedef struct CMDView {
    HIMAGELIST list;

    /*
     * 初始时候 head=tail=0
     * 新增节点， head++
     */
    pCEL cel;
    int capacity;/* 容量*/
    int occupied; /* 使用量 */
    int head;
    int tail;
    
    char* cmd_plt;
    char* output_plt;
    int* ret_plt;

    POINT offset;
    int item_height;
    int item_width;

    int viewcheck_mode;

    /*
     * 滚动条 拖拽
     */
    POINT prept;

    /*
     * slience_mode=1 激活子进程静默模式
     */
    int slience_mode;
}CmdView,*pCmdView;

int CmdView_InitialSettings(HWND hwnd);
pCmdView CmdView_GetSettings(HWND hwnd);
void CmdView_ClearSettings(HWND hwnd);

/*
 * 向控件发送新来的CEL
 * 控件分配的CEL 内存是有限的，当所有分配都被占据时，新来的CEL 会循环覆盖前面的CEL
 * count 为总尺寸，head / tail 为循环链
 * 在扩大count的时候，程序会很复杂，需要拷贝，重新计算head / tail 
 */
int CmdView_AttachCmdList(HWND hwnd,pCEL cel,int cel_count);
int CmdView_AttachCmdNode(HWND hwnd,pCEL cel);

LRESULT CALLBACK CmdViewProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
int CmdView_GetContentHeight(HWND hwnd);
int CmdView_GetContentWidth(HWND hwnd);
int CmdView_GetZoneRect(HWND hwnd,int code,LPRECT prc);
int CmdView_HitTest(HWND hwnd,POINT pt);
int CmdViewPaint(HWND hwnd,WPARAM wParam,LPARAM lParam);

