#include "frame.h"

typedef struct __rs_workflow_test__ {
    int cx;
    int cy;
    char title[256];
    HINSTANCE inst;
    HICON icon_app;
    WNDPROC wrapped_proc;
    
    impl_frameinst impl;//界面初始化函数
    
    void* extra;    
}wf_test,*pwf_test;

typedef struct proj_param {
    HWND frame;
    HWND workflow;
    HWND imgsratch;
    HWND cmdview;

    HANDLE ocrthread;
    int ocrthreadsuspend_ref;/* holy fuck，居然要写这么丑陋的代码 */
    
}wfpj_param,*pwfpj_param;

extern pwf_test f;

pwf_test initframemodule(HINSTANCE inst);
pwf_test getframemodule();
void clearframemodule(pwf_test p);

int WorkFlow_Test(HWND hwnd);

LRESULT CALLBACK TestProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);

int NodeMap_Test(HWND hwnd);
int ProjCfgPaint(HWND hwnd,HDC hdc,CONST LPRECT prc,POINT cur);
int ProjCfgZone(HWND hwnd,EFNCZone zone,LPRECT prc,BOOL allign_flag);
int ProjCfgClick(HWND hwnd,EFNCZone zone);

int ProjTimerPaint(HWND hwnd,HDC hdc,CONST LPRECT prc,POINT cur);
int ProjTimerZone(HWND hwnd,EFNCZone zone,LPRECT prc,BOOL allign_flag);
int ProjTimerClick(HWND hwnd,EFNCZone zone);

