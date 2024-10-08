#include <windows.h>
#include "workflow.h"

#define CTRL_WORKFLOWVIEW "WorkFlowView"

typedef int (*cb_nodepaint)(HDC,void* node,void* maps);
/*
 * 节点绘制信息
 */
typedef struct node_paints {
    RECT rc;//节点位置
    cb_nodepaint paint;
}NDPS,*pNDPS;


typedef int (*cb_mappaint)(HDC,LPRECT,void* maps);
/*
 * 连通图绘制信息
 */
typedef struct map_paints {
    int cell_cx;
    int cell_cy;
    int hoverid;/* 鼠标滑过的nodeid */
    
    /*
     * 被选中的node 会独立组织一些有特色的绘制交互工作
     */
    int checkedid;/* 被选中的 nodeid */

    pNodeMap map;
    pNDPS ndps_list;

    cb_mappaint paint;
}MAPS,*pMAPS;

/*
 * 这是一个2b的设计
 */
#define FGCOUNTDOWN_TIMERID 0x002b

LRESULT CALLBACK WorkFlowViewProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
int WorkFlowViewPaint(HWND hwnd,WPARAM wParam,LPARAM lParam);
int WorkFlowViewPaint_CalcLines(HDC hdc,pMAPS mps,pNodeConn conn);
void WorkFlowView_DefaultLayout(HWND hwnd);

int WorkFlowView_MouseMove(HWND hwnd,WPARAM wParam,LPARAM lParam);

int cmp_nodeid(const void* a,const void* b);

int WorkFlowView_InitialSettings(HWND hwnd);
pMAPS WorkFlowView_GetSettings(HWND hwnd);
void WorkFlowView_ClearSettings(HWND hwnd);

int PaintMajorDetail(HDC hdc,LPRECT prc,void* mapview);

#define MSG_NODELIGHTING (UINT)(WM_USER+0x0125)
