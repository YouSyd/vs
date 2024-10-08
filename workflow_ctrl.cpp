#include "workflow_ctrl.h"
#include "workflow.h"
#include <windowsx.h>
#include <gdiplus.h>

#include "tesseract_test.h"

using namespace Gdiplus;

int WorkFlowView_InitialSettings(HWND hwnd) {
    pMAPS mps=(pMAPS)calloc(sizeof(MAPS),1);
    if(!mps) return -1;
    mps->map=NODEMAP_initial();

    SetWindowLongPtr(hwnd,GWLP_USERDATA,(ULONG_PTR)mps);
    return 0;
}

pMAPS WorkFlowView_GetSettings(HWND hwnd) {
    return (pMAPS)GetWindowLongPtr(hwnd,GWLP_USERDATA);
}

void WorkFlowView_ClearSettings(HWND hwnd) {
    pMAPS mps=WorkFlowView_GetSettings(hwnd);
    
    if(mps) {
        if(mps->ndps_list) {
            free(mps->ndps_list);
            mps->ndps_list=NULL;
        }
        if(mps->map) {
            NODEMAP_release(mps->map);
        }
        mps->map=NULL;
    }
    free(mps);
}

/*
 * WorkFlowView_DefaultLayout
 * 功能：
 * 根据生成连通图，设置节点的默认布局
 */
void WorkFlowView_DefaultLayout(HWND hwnd) {
    pMAPS mps=WorkFlowView_GetSettings(hwnd);
    if(!mps||!mps->map||mps->map->node_count<=0) return;
    pNodeMap map=mps->map;
    
    RECT rc;
    GetClientRect(hwnd,&rc);
    InflateRect(&rc,-rc.left,-rc.top);
    if(map->cols>0) mps->cell_cx=rc.right/map->cols;
    if(map->rows>0) mps->cell_cy=rc.bottom/map->rows;
    
    /*
     * 设置 node的 rect
     */
    pNDPS ndps=(pNDPS)calloc(sizeof(NDPS)*map->node_count,1);
    for(int idx=0;idx<map->node_count;idx++) {
        map->node_list[idx].param=ndps+idx;
#define WORKFLOW_NODE_CELL_PADDING_CX 30
#define WORKFLOW_NODE_CELL_PADDING_CY 25
        int paddingcx=WORKFLOW_NODE_CELL_PADDING_CX,paddingcy=WORKFLOW_NODE_CELL_PADDING_CY;
        SetRect(&(ndps+idx)->rc,0,0,mps->cell_cx,mps->cell_cy);
        InflateRect(&(ndps+idx)->rc,-paddingcx,-paddingcy);
    }
    
    int row_start=0;
    if(map->levels->count<=0) row_start=1;
    for(int idx=0;idx<map->rows+1;idx++) {
        pNMLevel level=map->levels+idx;
        /*
         * 如果当前层没有节点，即level->count<=0 则 row-- 
         */
        for(int i=0;i<level->count;i++) {
            int id=level->pos[i*2];
            int col=level->pos[i*2+1];
            int row=idx-row_start;

            pFlowNode node=(pFlowNode)bsearch(&id,map->node_list,map->node_count,sizeof(FlowNode),cmp_nodeid);
            if(node) {
                pNDPS ndps=(pNDPS)node->param;
                OffsetRect(&ndps->rc,col*mps->cell_cx,row*mps->cell_cy);
            }
        }
    }

    if(mps->ndps_list) {
        free(mps->ndps_list);
    }
    mps->ndps_list=ndps;
    InvalidateRect(hwnd,NULL,TRUE);
}

LRESULT CALLBACK WorkFlowViewProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
    LRESULT ret=0;
    
    //pMAPS mps=WorkFlowView_GetSettings(hwnd);
    //pNodeMap map=mps->map;
    
    switch(msg) {
    case WM_PAINT: {
        return WorkFlowViewPaint(hwnd,wParam,lParam);
    } break;
    case WM_MOUSEMOVE: {
        WorkFlowView_MouseMove(hwnd,wParam,lParam);
    } break;
    case WM_NCDESTROY: {
        WorkFlowView_ClearSettings(hwnd);
    } break;
    case MSG_NODELIGHTING: {
        /* SendMessage(wfpj->workflow,MSG_NODELIGHTING,(WPARAM)page,(LPARAM)0);
         *
         * 页面跳转点亮
         */
        pMAPS mps=WorkFlowView_GetSettings(hwnd);
        if(!mps||!mps->map) return 0;
        PAGECODE page=(PAGECODE)wParam;
        /*
         * id_page_map[page]=nodeid;
         */
        int id_page_map[]={
            -1,/*UNKNOWN_PAGE*/
            2,/*STREAMER_ROOM*/
            3,/*FORTUNEGIFT_DETAIL*/
            3,/*FORTUNEGIFT_DETAIL_2TASK*/
            6,/*FORTUNEGIFT_NOPRIZE*/
            4,/*FORTUNEGIFT_JACKPOT*/
            -1,/*DIAMOND_DETAIL*/
            -1,/*DIAMOND_NOPRICE*/
            -1,/*DIAMOND_JACKPOT*/
            7,/*STREAMER_ROOMCLOSED*/
        };
        int page_nodeid=id_page_map[page];
        mps->checkedid=page_nodeid;

        InvalidateRect(hwnd,NULL,TRUE);
    } break;
    case WM_TIMER: {
        pMAPS mps=WorkFlowView_GetSettings(hwnd);
        if(!mps||!mps->map) return 0;
        
        switch(wParam) {
        case FGCOUNTDOWN_TIMERID: {
            int countdown_seconds=GMajor->fg_countdown+GMajor->timestamp-time(NULL);
            if(countdown_seconds<0) {
                KillTimer(hwnd,FGCOUNTDOWN_TIMERID);
                mps->paint=NULL;
                //RedrawWindow(GetParent(hwnd), NULL, NULL,RDW_INVALIDATE|RDW_UPDATENOW|RDW_FRAME);
            } else {
                mps->paint=NULL;//PaintMajorDetail;
                //InvalidateRect(hwnd,NULL,TRUE);
                RedrawWindow(GetParent(hwnd), NULL, NULL,RDW_INVALIDATE|RDW_UPDATENOW|RDW_FRAME);
            }
        } break;
        }
    } break;
    }
    
    ret=CallWindowProc(DefWindowProc,hwnd,msg,wParam,lParam);
    
    return ret;
}

int WorkFlowView_MouseMove(HWND hwnd,WPARAM wParam,LPARAM lParam) {
    POINT pt={
        GET_X_LPARAM(lParam),
        GET_Y_LPARAM(lParam)
    };
    pMAPS mps=WorkFlowView_GetSettings(hwnd);
    if(!mps) return 0;
    //mps->hoverid=0;
    int hoverid=0;

    pNodeMap map=mps->map;
    if(!map||mps->cell_cx==0||mps->cell_cy==0) return 0;
    int colidx=pt.x/mps->cell_cx;
    int rowidx=pt.y/mps->cell_cy;

    if(rowidx>=map->rows) return 0;

    int nodeid=0;
    int row_start=map->levels->count<=0?1:0;
    pNMLevel level=map->levels+(rowidx+row_start);
    for(int idx=0;idx<level->count;idx++) if(level->pos[idx*2+1]==colidx) nodeid=level->pos[idx*2];
    if(nodeid!=0) {
        pFlowNode node=(pFlowNode)bsearch(&nodeid,map->node_list,map->node_count,sizeof(FlowNode),cmp_nodeid);
        if(node) {
            pNDPS ndps=(pNDPS)node->param;
            if(ndps&&PtInRect(&ndps->rc,pt)) {
                //mps->hoverid=nodeid;
                hoverid=nodeid;
            }
        }
    }
    if(hoverid!=mps->hoverid) {
        mps->hoverid=hoverid;
        InvalidateRect(hwnd,NULL,TRUE);
    }

    return 0;
}

int cmp_nodeid(const void* a,const void* b) { pFlowNode node_a=(pFlowNode)a; pFlowNode node_b=(pFlowNode)b; return node_a->id-node_b->id; }
int WorkFlowViewPaint(HWND hwnd,WPARAM wParam,LPARAM lParam) {
    pMAPS mps=WorkFlowView_GetSettings(hwnd);
    if(!mps) return 0;
    pNodeMap map=mps->map;
    if(!map) return 0;
    
    PAINTSTRUCT ps={0};
    RECT rc,rcMem;
    GetClientRect(hwnd,&rc);
    OffsetRect(&rc,-rc.left,-rc.top);
    CopyRect(&rcMem,&rc);
    int cx=rcMem.right,cy=rcMem.bottom;
    if(!(cx>0&&cy>0)) return 0;

    HDC hdc=BeginPaint(hwnd,&ps);
    HDC memdc=CreateCompatibleDC(hdc);
    HBITMAP bmp=CreateCompatibleBitmap(hdc,cx,cy);
    HBITMAP prebmp=(HBITMAP)SelectObject(memdc,bmp);
    HBRUSH brushbk=CreateSolidBrush(RGB(40,40,40));
    FillRect(memdc,&rcMem,brushbk);
    DeleteObject(brushbk);
    
    //1.绘制节点
    HBRUSH nodebk=CreateSolidBrush(RGB(35,35,35));
    HPEN nodepen=CreatePen(PS_SOLID,1,RGB(95,95,95));
    SetBkMode(memdc,TRANSPARENT);
    LOGFONT lf={0};
    GetObject(GetStockObject(SYSTEM_FONT),sizeof(LOGFONT),&lf);
    lf.lfHeight=18;
    lf.lfWeight=22;
    strcpy(lf.lfFaceName,"微软雅黑");
    HFONT font=CreateFontIndirect(&lf);
    SelectObject(memdc,font);
    SelectObject(memdc,nodebk);
    SelectObject(memdc,nodepen);
    for(int idx=0;idx<map->node_count;idx++) {
        SetTextColor(memdc,RGB(155,155,155));
        pNDPS ndps=(pNDPS)map->node_list[idx].param;
        if(ndps) {
            if(mps->hoverid==map->node_list[idx].id) {
                HBRUSH node_hover=CreateSolidBrush(RGB(0,100,200));
                HBRUSH pre_brush=(HBRUSH)SelectObject(memdc,node_hover);
                //Rectangle(memdc,ndps->rc.left,ndps->rc.top,ndps->rc.right,ndps->rc.bottom);
                {
                    Gdiplus::Graphics graphics(memdc);

                    // 定义渐变矩形区域
                    int cx=ndps->rc.right-ndps->rc.left;
                    int cy=(ndps->rc.bottom-ndps->rc.top);
                    Gdiplus::Rect rect(ndps->rc.left,ndps->rc.top,cx,cy);
                    //尝试绘制 光晕效果的边框
                    // 创建圆角矩形路径
                    int x=rect.GetLeft(),y=rect.GetTop();
                    int width=rect.Width,height=rect.Height;
                    int cornerRadius=10,glowSize=15;
                    
                    // 光晕的中心颜色
                    Gdiplus::Color centerColor(255, 0, 100, 255); // 蓝色带透明
                    // 光晕的外部颜色
                    Gdiplus::Color outerColor(10, 30, 30, 30);   // 完全透明的蓝色

                    // 定义渐变区域
                    Gdiplus::RectF rect2((float)x,(float)y-((width-height)>>1),(float)width,(float)width);
                    Gdiplus::GraphicsPath path2;
                    path2.AddEllipse((float)x,(float)y-((width-height)>>1),(float)width,(float)width);
                    Gdiplus::PathGradientBrush brushgardient(&path2);

                    // 设置中心颜色
                    brushgardient.SetCenterColor(centerColor);

                    // 设置边缘颜色
                    Gdiplus::Color surroundColors[] = { outerColor };
                    int surround_count=1;
                    brushgardient.SetSurroundColors(surroundColors,&surround_count);

                    // 使用 SetBlend 控制颜色的过渡
                    REAL blendPositions[] = { 0.0f, 0.5f, 0.9f, 1.0f };  // 控制渐变的位置
                    REAL blendFactors[] = { 0.0f, 0.2f, 0.4f,1.0f };    // 控制中心颜色扩散范围
                    brushgardient.SetBlend(blendFactors, blendPositions,4);

                    // 绘制渐变圆
                    graphics.FillEllipse(&brushgardient, rect2);

                    
                    {
                    // 创建线性渐变画刷
                    Gdiplus::LinearGradientBrush brush(
                        rect,                  // 渐变区域
                        Color(255,10,10,10), // 起始颜色 (红色)
                        Color(255,60,60,60), // 结束颜色 (蓝色)
                        LinearGradientModeVertical // 渐变方向 (垂直)
                    );

                    // 绘制填充矩形
                    graphics.FillRectangle(&brush, rect);
                    //graphics.FillRectangle(&brush2, rect2);
                    FrameRect(memdc,&ndps->rc,node_hover);
                    }
                }
                SelectObject(memdc,pre_brush);
                DeleteObject(node_hover);
            } else Rectangle(memdc,ndps->rc.left,ndps->rc.top,ndps->rc.right,ndps->rc.bottom);
            
            if(mps->checkedid==map->node_list[idx].id) {
                HBRUSH node_checked=CreateSolidBrush(RGB(66,66,66));
                HBRUSH pre_brush=(HBRUSH)SelectObject(memdc,node_checked);
                //Rectangle(memdc,ndps->rc.left,ndps->rc.top,ndps->rc.right,ndps->rc.bottom);
                {
                    Gdiplus::Graphics graphics(memdc);

                    // 定义渐变矩形区域
                    int cx=ndps->rc.right-ndps->rc.left;
                    int cy=(ndps->rc.bottom-ndps->rc.top);
                    Gdiplus::Rect rect(ndps->rc.left,ndps->rc.top,cx,cy);
                    //Gdiplus::Rect rect2(ndps->rc.left,ndps->rc.top+cy,cx,15);

                    // 创建线性渐变画刷
                    Gdiplus::LinearGradientBrush brush(
                        rect,                  // 渐变区域
                        Color(255,220,220,0), // 起始颜色 (红色)
                        Color(255,200,200,0), // 结束颜色 (蓝色)
                        LinearGradientModeVertical // 渐变方向 (垂直)
                    );

                    // 绘制填充矩形
                    graphics.FillRectangle(&brush, rect);
                    //graphics.FillRectangle(&brush2, rect2);
                    FrameRect(memdc,&ndps->rc,node_checked);
                }
                SelectObject(memdc,pre_brush);
                DeleteObject(node_checked);
                SetTextColor(memdc,RGB(0,125,255));
            }
            
            //绘制文本
            char text_buffer[256]="";
            sprintf(text_buffer,"%s(%d)",map->node_list[idx].text,map->node_list[idx].id);
            DrawText(memdc,text_buffer,-1,&ndps->rc,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
        }
    }
    
    /* 
     * 2.绘制连线
     * 确定两个节点所处的行列，计算折线的位置
     * 前驱节点行与后继点行如果紧邻：
     *   如果同列，直线连接
     *   不同列，需要折两次，计算两次折三段线的坐标
     * 两点跨多行
     *   检查所跨中间行的 最左/最右侧 列，检测计算是需要 一段、三段、五段
     *   初步思路如下：
     *   0.前驱列 col_pre, 后继 col_next;
     *   1.确定折线是左折(col_pre>col_next)还是右折(col_pre<col_next), 不折(col_pre==col_next)
     *   2.确定 中间所有行 的最左列(col_min)、最右列(col_max)
     *     if(col_min<=col_pre&&col_max>=col_pre&&col_pre==col_next) {
     *         不折 需要修正
     *         if(col_pre-col_min<col_max-col_pre) 修正为 左折
     *         else 修正为 右折
     *     }
     *   3.如果左折，if(col_min>col_next) 三段
     *               else  五段
     *     如果右折，if(col_max<col_next) 三段
     *               else  五段
     */
    /*
     * 对 map->node_list 按照 nodeid 进行排序,
     * 该操作建议提至paint函数中
     */
    qsort(map->node_list,map->node_count,sizeof(FlowNode),cmp_nodeid);
    
    for(int idx=0;idx<map->conn_count;idx++) {
        WorkFlowViewPaint_CalcLines(memdc,mps,map->conn_list+idx);
    }

    /*
     * 附加绘图
     */
    if(mps->paint) mps->paint(memdc,&rcMem,mps);

    BitBlt(hdc,0,0,cx,cy,memdc,0,0,SRCCOPY);
    SelectObject(memdc,prebmp);
    DeleteObject(bmp);
    DeleteObject(font);
    DeleteObject(nodepen);
    DeleteObject(nodebk);
    DeleteDC(memdc);
    
    EndPaint(hwnd,&ps);
    return 0;
}

int PaintMajorDetail(HDC hdc,LPRECT prc,void* mapview) {
    if(!prc||!mapview) return -1;
    pMAPS mps=(pMAPS)mapview;

    /*
     * 获取 GMajor
     */
    int countdown_seconds=GMajor->fg_countdown+GMajor->timestamp-time(NULL);
    if(countdown_seconds<=0) return -1;
    int minutes=countdown_seconds/60;
    int seconds=countdown_seconds%60;
    char countdown_str[256]={0};
    sprintf(countdown_str,"%02d:%02d",minutes,seconds);

    int cxMajor=200,cyMajor=80;
    RECT rcMajor={0,0,cxMajor,cyMajor};
    int offsetx=(prc->right-prc->left-cxMajor)/2;
    int offsety=(prc->bottom-prc->top-cyMajor)/2;
    OffsetRect(&rcMajor,offsetx,offsety);
    RECT rcCountDown;
    CopyRect(&rcCountDown,&rcMajor);

    SetBkMode(hdc,TRANSPARENT);
    SetStretchBltMode(hdc,HALFTONE);
    int prevMode=SetGraphicsMode(hdc,GM_ADVANCED);
    HBRUSH brushMajor=CreateSolidBrush(RGB(15,15,15));
    HBRUSH preBrush=(HBRUSH)SelectObject(hdc,brushMajor);
    RoundRect(hdc,rcMajor.left,rcMajor.top,rcMajor.right,rcMajor.bottom,16,16);
    SelectObject(hdc,preBrush);
    DeleteObject(brushMajor);
    SetGraphicsMode(hdc,prevMode);

    LOGFONT lf={0};
    GetObject(GetStockObject(SYSTEM_FONT),sizeof(LOGFONT),&lf);
    lf.lfHeight=35;
    lf.lfWeight=38;
    lf.lfWidth=38;
    strcpy(lf.lfFaceName,"Consolas");
    HFONT font=CreateFontIndirect(&lf);
    HFONT prefont=(HFONT)SelectObject(hdc,font);
    SetTextColor(hdc,RGB(200,0,0));
    DrawText(hdc,countdown_str,-1,&rcCountDown,DT_CENTER|DT_VCENTER|DT_SINGLELINE);

    SelectObject(hdc,prefont);
    DeleteObject(font);

    return 0;
}

int WorkFlowViewPaint_CalcLines(HDC hdc,pMAPS mps,pNodeConn conn) {
    if(!mps) return -1;
    pNodeMap map=mps->map;
    if(!map||!conn||!map->levels) return -1;
        
    //获取前驱、后继的col、row
    /*
     * 从 levels 中 找到 节点
     */
    int prenode_col,prenode_row,nextnode_col,nextnode_row;
    int prenode_flag=0,nextnode_flag=0;
    int step_col_min,step_col_max;
    for(int idx=0;idx<map->rows+1;idx++) {
        if(prenode_flag==1&&nextnode_flag==1) break;
        pNMLevel pL=map->levels+idx;
        for(int i=0;i<pL->count;i++) {
            if(prenode_flag==1&&nextnode_flag==1) break;
            
            if(conn->preid==pL->pos[i*2]) {
                prenode_col=pL->pos[i*2+1]; 
                prenode_row=pL->level_idx;
                prenode_flag=1;
                
                step_col_min=step_col_max=prenode_col;
            }
            if(conn->nextid==pL->pos[i*2]) {
                nextnode_col=pL->pos[i*2+1]; 
                nextnode_row=pL->level_idx;
                nextnode_flag=1;
                break;
            }
            
            if(prenode_flag==1&&pL->level_idx>prenode_row) {
                if(pL->pos[i*2+1]<step_col_min) step_col_min=pL->pos[i*2+1];
                if(pL->pos[i*2+1]>step_col_max) step_col_max=pL->pos[i*2+1];
            }
            /*
             * 刷新 prenode_row 后续行的 step_col_min,step_col_max
             */
        }
    }
    
    /*
     * 根据id找到prenode 和 nextnode
     */
    pFlowNode pre=NULL,next=NULL;
    pre=(pFlowNode)bsearch(&conn->preid,map->node_list,map->node_count,sizeof(FlowNode),cmp_nodeid);
    next=(pFlowNode)bsearch(&conn->nextid,map->node_list,map->node_count,sizeof(FlowNode),cmp_nodeid);
    if(!pre||!next||!pre->param||!next->param) return -1;
    RECT rcPre,rcNext;
    CopyRect(&rcPre,&((pNDPS)pre->param)->rc);
    CopyRect(&rcNext,&((pNDPS)next->param)->rc);
    //pMAPS mps=(pMAPS)map->param;
    //if(!mps) return -1;
    int nodepaddingcy=(mps->cell_cy-(rcPre.bottom-rcPre.top))>>1;
    int nodepaddingcx=(mps->cell_cx-(rcPre.right-rcPre.left))>>1;
    
    /*
     * 判断折线路径
     */
    POINT pt_lines[10];
    int pt_count;
    if(prenode_row+1==nextnode_row) {
        /*
         * 相邻两行
         */
        if(prenode_col==nextnode_col) {
            /*
             * 直连，一段线
             */
            pt_count=2;
        } else if(prenode_col<nextnode_col) {
            /*
             * 右折，三段线
             */
            pt_count=4;
        } else {
            /*
             * 左折，三段线
             */
            pt_count=4;
        }
    } else {
        /*
         * 跨行,
         * 区分三段线和五段线
         * 大致算法(五段线)
         *  1.左折 但nextnode_col <= step_col_min
         *                              +----------+
         *                              | pre_node |
         *                              +----------+
         *    +---------------------------------------+
         *    |                                       |
         *    | step_col_min .........   step_col_max |
         *    |                                       |
         *    +---------------------------------------+
         *         +-----------+
         *         | next_node |
         *         +-----------+
         *
         *
         *  2.右折 但nextnode_col <= step_col_max
         *        +----------+
         *        | pre_node |
         *        +----------+
         *    +---------------------------------------+
         *    |                                       |
         *    | step_col_min .........   step_col_max |
         *    |                                       |
         *    +---------------------------------------+
         *                             +-----------+
         *                             | next_node |
         *                             +-----------+
         *
         *  3.对齐直连， 但 step_col_min <= prenode_col <= step_col_max
         *
         *                  +----------+
         *                  | pre_node |
         *                  +----------+
         *    +---------------------------------------+
         *    |                                       |
         *    | step_col_min .........   step_col_max |
         *    |                                       |
         *    +---------------------------------------+
         *                  +-----------+
         *                  | next_node |
         *                  +-----------+
         *
         */
        if(prenode_col==nextnode_col) {
            if(step_col_min<=prenode_col&&prenode_col<=step_col_max) {
                /*
                 * 直连，五段线
                 */
                pt_count=6;
                /*
                 * 具体是左折还是右折，选取折的少的一端
                 */
                if(prenode_col-step_col_min<step_col_max-prenode_col) {
                    /*
                     * 左折
                     */
                    pt_lines[2].x=pt_lines[1].x-(prenode_col-step_col_min+1)*mps->cell_cx;
                } else {
                    /*
                     * 右折
                     */
                    pt_lines[2].x=pt_lines[1].x+(step_col_min-prenode_col+1)*mps->cell_cx;
                }
            } else {
                /*
                 * 直连 一段线
                 */
                pt_count=2;
            }
        } else if(prenode_col<nextnode_col) {
            if(step_col_max>=nextnode_col) {
                /*
                 * 右折，五段线
                 */
                pt_count=6;
                pt_lines[2].x=pt_lines[1].x+(step_col_min-prenode_col+1)*mps->cell_cx;
            } else {
                /*
                 * 右折，三段线
                 */
                pt_count=4;
            }
        } else {
            if(step_col_min<=nextnode_col) {
                /*
                 * 左折，五段线
                 */
                pt_count=6;
                
                pt_lines[2].x=pt_lines[1].x-(prenode_col-step_col_min+1)*mps->cell_cx;
            } else {
                /*
                 * 左折，三段线
                 */
                pt_count=4;
            } 
        }
    }
    
    switch(pt_count) {
    case 2: {
        /*
         * 直连
         */
        /*
         * 根据 nodeid找到FlowNode,找到 其附加参数 rc,缺少该函数
         * 只能手写 qsort
         */
        
        pt_lines[0].x=(rcPre.left+rcPre.right)>>1;
        pt_lines[0].y=rcPre.bottom;
        
        pt_lines[1].x=(rcNext.left+rcNext.right)>>1;
        pt_lines[1].y=rcNext.top;
    } break;
    case 6: {
        /*
         * 五段线
         */
        pt_lines[0].x=(rcPre.left+rcPre.right)>>1;
        pt_lines[0].y=rcPre.bottom;
        
        pt_lines[5].x=(rcNext.left+rcNext.right)>>1;
        pt_lines[5].y=rcNext.top;
        
        
        pt_lines[1].x=pt_lines[0].x;
        pt_lines[1].y=pt_lines[0].y+nodepaddingcy;
        
        //pt_lines[2].x=?;//依据step_col_max、step_col_min、prenode_col判断
        pt_lines[2].y=pt_lines[1].y;
        
        pt_lines[3].x=pt_lines[2].x;
        pt_lines[3].y=pt_lines[5].y-nodepaddingcy;
        
        pt_lines[4].x=pt_lines[5].x;
        pt_lines[4].y=pt_lines[3].y;
    
    } break;
    case 4: {
        /*
         * 三段线
         */
        pt_lines[0].x=(rcPre.left+rcPre.right)>>1;
        pt_lines[0].y=rcPre.bottom;
        
        pt_lines[3].x=(rcNext.left+rcNext.right)>>1;
        pt_lines[3].y=rcNext.top;
        
        pt_lines[1].x=pt_lines[0].x;
        pt_lines[1].y=pt_lines[0].y+nodepaddingcy;
        
        pt_lines[2].x=pt_lines[3].x;
        pt_lines[2].y=pt_lines[1].y;
        
    } break;
    default: return -1;
    }
    
    /*
     * 画线
     */
    for(int idx=0;idx<pt_count-1;idx++) {
        MoveToEx(hdc,pt_lines[idx].x,pt_lines[idx].y,NULL);
        LineTo(hdc,pt_lines[idx+1].x,pt_lines[idx+1].y);
    }
    
    return 0;
}
