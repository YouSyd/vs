#include <windows.h>
#include "workflow.h"

#define CTRL_WORKFLOWVIEW "WorkFlowView"
/*
 * 节点绘制信息
 */
typedef struct node_paints {
    RECT rc;//节点位置
    
}NDPS,*pNDPS;

/*
 * 连通图绘制信息
 */
typedef struct map_paints {
    int cell_cx;
    int cell_cy;
    int hoverid;/* 鼠标滑过的nodeid */
    pNodeMap map;
    pNDPS ndps_list;
}MAPS,*pMAPS;

LRESULT CALLBACK WorkFlowViewProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
int WorkFlowViewPaint(HWND hwnd,WPARAM wParam,LPARAM lParam);
int WorkFlowViewPaint_CalcLines(HDC hdc,pMAPS mps,pNodeConn conn);
void WorkFlowView_DefaultLayout(HWND hwnd);

int WorkFlowView_MouseMove(HWND hwnd,WPARAM wParam,LPARAM lParam);

int cmp_nodeid(const void* a,const void* b);

int WorkFlowView_InitialSettings(HWND hwnd);
pMAPS WorkFlowView_GetSettings(HWND hwnd);
void WorkFlowView_ClearSettings(HWND hwnd);

#define MSG_NODELIGHTING (UINT)(WM_USER+0x0125)
