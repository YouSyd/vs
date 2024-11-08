#include "PopupMenu.h"
#include <stdio.h>
#include "workflow_proj.h"

int PopupMenu_InitialSettings(HWND hwnd) {
    pMenuRef mr=(pMenuRef)calloc(sizeof(PopupMenuRef),1);
    if(!mr) return -1;

    mr->proc=(WNDPROC)PopupMenuWndProc;
    mr->pre_proc=(WNDPROC)SetWindowLongPtr(hwnd,GWLP_WNDPROC,(ULONG_PTR)mr->proc);
    mr->hrgn=NULL;

    SetWindowLongPtr(hwnd,GWLP_USERDATA,(ULONG_PTR)mr);
    //SetWindowLongPtr(hwnd,GWL_STYLE,(LONG_PTR)((UINT)(GetWindowLongPtr(hwnd,GWL_STYLE))&(~WS_BORDER)));
    SetWindowLong(hwnd,GWL_STYLE,((UINT)(GetWindowLong(hwnd,GWL_STYLE)))&(~WS_BORDER));
    return 0;
}

void PopupMenu_ClearSettings(HWND hwnd) {
    pMenuRef mr=PopupMenu_GetSettings(hwnd);

    if(mr) {
        SetWindowLongPtr(hwnd,GWLP_WNDPROC,(ULONG_PTR)mr->pre_proc);

        if(mr->hrgn) {
            DeleteObject(mr->hrgn);
            mr->hrgn=NULL;
        }

        free(mr);

        SetWindowLongPtr(hwnd,GWLP_USERDATA,NULL);
    }
}

pMenuRef PopupMenu_GetSettings(HWND hwnd) {
    return (pMenuRef)GetWindowLongPtr(hwnd,GWLP_USERDATA);
}

int PopupMenu_SetRgn(HWND hwnd,HRGN rgn) {
    pMenuRef mr=PopupMenu_GetSettings(hwnd);
    if(!mr) return -1;

    if(mr->hrgn) {
        DeleteObject(mr->hrgn);
        mr->hrgn=NULL;
    }
    mr->hrgn=rgn;
    SetWindowRgn(hwnd,mr->hrgn,TRUE);
    SetWindowLong(hwnd,GWL_STYLE,((UINT)(GetWindowLong(hwnd,GWL_STYLE)))&(~WS_BORDER));
    
    // {
    //     /*
    //      * 在尝试中发现， Popup Menu的窗口似乎会阻挡其owner window的消息传递，当Menu 窗口关闭后
    //      * 其Owner 窗口的消息才会正常传递？
    //      * 尽管如此， 其 Owner的 WM_DRAWITEM 等消息依然能够正常传递
    //      */
    //     char text[256]={0};
    //     UINT style=GetWindowLongPtr(hwnd,GWL_STYLE);
    //     sprintf(text,"0x%08x,WS_BORDER=%s",style,(((style&WS_BORDER)==WS_BORDER)?"YES":"NO"));
    //     SetWindowText(((pwfpj_param)(f->extra))->frame,text);
    //     //UpdateWindow(((pwfpj_param)(f->extra))->frame);
    //     SetWindowPos(((pwfpj_param)(f->extra))->frame, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    // }

    return 0;
}

/*
 * 尝试实验后，发现PopupMenu的窗口过程 WM_PAINT 、 WM_NCPAINT 并不会生效
 * 对于PopupMenu , 操作系统似乎是采取某种特殊的方式完成绘制和消息传递的
 * 无论在其Owner窗体中如何设置菜单属性（比如设置 MFT_OWNERDRAW 或不设置） PopupMenu 的窗口WM_PAINT等消息均不会响应
 * 但对于更改HRGN的WM_SIZE 消息，其窗口过程是能够正常向响应的。
 * 对于更改非客户区的WM_NCCALCSIZE ， 也能正常响应。
 */
LRESULT CALLBACK PopupMenuWndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
    pMenuRef mr=PopupMenu_GetSettings(hwnd);
    if(!mr) return 0;

    switch(msg) {
    // case WM_NCCALCSIZE: { 
    //     if(wParam==TRUE) {
    //         // LPNCCALCSIZE_PARAMS calc_param = (LPNCCALCSIZE_PARAMS)lParam;
    //         // RECT rect_new;
    //         // RECT rect_old;
    //         // RECT client_rect_new;
    //         // RECT client_rect_old;
    //         // pComboBoxStyle cs=ComboBox_GetSettings(hwnd);   

    //         // CopyRect(&rect_new,&(calc->rgrc[0]));
    //         // CopyRect(&rect_old,&(calc->rgrc[1]));
    //         // CopyRect(&client_rect_old,&(calc->rgrc[2]));
            
    //         // HDC hdc=GetWindowDC(hwnd);
    //         // RECT wnd_rc={0};
            
    //         // int cy=0,top_margin=0;;
    //         // TEXTMETRIC tm={0};
    //         // GetTextMetrics(hdc,&tm);
    //         // cy=tm.tmHeight+tm.tmExternalLeading;
    //         // top_margin=((rect_new.bottom-rect_new.top)-cs->font_cy)>>1;
    //         // if(top_margin<=0) top_margin=0;
            
    //         // cs->font_cy=cy;
            
    //         // client_rect_new = {rect_new.left+cs->margin_left,
    //         //                 rect_new.top,
    //         //                 rect_new.right,
    //         //                 rect_new.bottom};
    //         // CopyRect(&(calc->rgrc[0]),&client_rect_new);
    //         // CopyRect(&(calc->rgrc[1]),&rect_new);
    //         // CopyRect(&(calc->rgrc[2]),&rect_old);
            
    //         // return WVR_VALIDRECTS;
    //         NCCALCSIZE_PARAMS* pParams = (NCCALCSIZE_PARAMS*)lParam;
            
    //         //CopyRect(&pParams->rgrc[2],&pParams->rgrc[1]);
    //         //CopyRect(&pParams->rgrc[1],&pParams->rgrc[0]);

    //         // 修改 pParams->rgrc[0]，使客户区占据整个窗口
    //         pParams->rgrc[0].left -= GetSystemMetrics(SM_CXFRAME);  // 设置左边距
    //         pParams->rgrc[0].right += GetSystemMetrics(SM_CXFRAME); // 设置右边距
    //         pParams->rgrc[0].top -= GetSystemMetrics(SM_CYFRAME);   // 设置上边距
    //         pParams->rgrc[0].bottom += GetSystemMetrics(SM_CYFRAME); // 设置下边距


    //         //return WVR_VALIDRECTS;  // 处理完毕，返回 0 表示不需要系统处理
    //         return 0;
    //     }
    // } break;
    // case WM_NCACTIVATE:
    // case WM_NCPAINT: {
    //     /*
    //      * 处理边框
    //      */
    //     RECT rc,rc_mem;
    //     GetWindowRect(hwnd,&rc);
    //     CopyRect(&rc_mem,&rc);
    //     OffsetRect(&rc_mem,-rc_mem.left,-rc_mem.top);

    //     HDC hdc=GetWindowDC(hwnd);
    //     HDC memdc=CreateCompatibleDC(hdc);
    //     HBITMAP bmp=CreateCompatibleBitmap(hdc,rc_mem.right,rc_mem.bottom);
    //     HBITMAP pre_bmp=(HBITMAP)SelectObject(memdc,bmp);
    //     HBRUSH brushNC=CreateSolidBrush(RGB(0,50,100));
    //     FillRect(memdc,&rc_mem,brushNC);

    //     BitBlt(hdc,rc.left,rc.top,rc_mem.right,rc_mem.bottom,memdc,rc_mem.right,rc_mem.bottom,SRCCOPY);

    //     DeleteObject(brushNC);
    //     DeleteObject(bmp);
    //     DeleteDC(memdc);
    //     ReleaseDC(hwnd,hdc);
    //     return 0;
    // } break;
    // case WM_PAINT: {
    //     //HRESULT ret=CallWindowProc(mr->pre_proc,hwnd,msg,wParam,lParam);
        
    //     RECT rc,rc_mem;
    //     GetClientRect(hwnd,&rc);
    //     CopyRect(&rc_mem,&rc);
    //     OffsetRect(&rc_mem,-rc_mem.left,-rc_mem.top);

    //     PAINTSTRUCT ps={0};
    //     HDC hdc=BeginPaint(hwnd,&ps);
    //     //ExcludeClipRect(hdc,rc.left+3,rc.top+3,rc.right-3,rc.bottom-3);
    //     HDC memdc=CreateCompatibleDC(hdc);
    //     HBITMAP bmp=CreateCompatibleBitmap(hdc,rc_mem.right,rc_mem.bottom);
    //     HBITMAP pre_bmp=(HBITMAP)SelectObject(memdc,bmp);
    //     HBRUSH brushNC=CreateSolidBrush(RGB(0,50,100));
    //     FillRect(memdc,&rc_mem,brushNC);

    //     BitBlt(hdc,rc.left,rc.top,rc_mem.right,rc_mem.bottom,memdc,rc_mem.right,rc_mem.bottom,SRCCOPY);

    //     DeleteObject(brushNC);
    //     DeleteObject(bmp);
    //     DeleteDC(memdc);
    //     EndPaint(hwnd,&ps);
    //     return 0;
    // } break;
    case WM_ERASEBKGND: {
        //return 1;
    } break;
    case WM_NCDESTROY: {
        WNDPROC pre_proc=mr->pre_proc;
        PopupMenu_ClearSettings(hwnd);
        return CallWindowProc(pre_proc,hwnd,msg,wParam,lParam);
    } break;
    case WM_SIZE: {
        if(wParam==SIZE_RESTORED) {
            RECT rc;
            GetWindowRect(hwnd,&rc);
            OffsetRect(&rc,-rc.left,-rc.top);
            int width=rc.right;
            int height=rc.bottom;

            //HRGN rgn=CreateRoundRectRgn(1,1,width,height,30,30);
            
            HRGN rgn=CreateRectRgn(1,1,width-1,height-1);

            // 放弃该方式
            // HRGN rgn=CreateRectRgn(1,1+15,width-1,height-1);
            // POINT pt[3]={
            //     {5,1+15},
            //     {5+15,1},
            //     {5+15*2,1+15}
            // };
            // HRGN rgn2=CreatePolygonRgn(pt,3,WINDING);
            // CombineRgn(rgn,rgn,rgn2,RGN_OR);
            // DeleteObject(rgn2);
            PopupMenu_SetRgn(hwnd,rgn);
        }
    } break;
    }
    return CallWindowProc(mr->pre_proc,hwnd,msg,wParam,lParam);
}

/*
 * 由于只能无法通过菜单窗体的WM_PAINT/WM_NCPAINT 控制菜单整体外观的绘制
 * 只能对菜单项进行局部外观绘制的拼凑
 * 这里需要具备识别菜单项首尾的能力，以完成外观细节补充。（包括边框、圆角顶点等）
 */
int PopupMenu_DrawMenuItem(HWND hwnd,LPDRAWITEMSTRUCT draw)
{
    UINT menu_item_id=draw->itemID;
    char text[256]="";
    RECT rect;
    HDC hdc;
    HMENU menu;
    HWND menu_hwnd;
    MENUITEMINFO mi_if={0}/*,itemtype={0}*/;
    COLORREF rgb_bk=RGB(43,43,43);//RGB(70,70,70);
    COLORREF rgb_disabled=RGB(150,150,150);
    COLORREF rgb_text_disabled=RGB(100,100,100);
    COLORREF rgb_check=RGB(65,65,65);
    COLORREF rgb_text_check=RGB(0,128,255);
    COLORREF rgb_text=RGB(255,255,255);
    HBRUSH brush,pre_brush;
    HPEN pen,pre_pen;
    
    hdc=draw->hDC;
    RECT rcItem;
    CopyRect(&rcItem,&(draw->rcItem));
    InflateRect(&(draw->rcItem),1,1);
    CopyRect(&rect,&(draw->rcItem));
    menu=(HMENU)draw->hwndItem;
    menu_hwnd=draw->hwndItem;
    
    mi_if.cbSize=sizeof(mi_if);
    mi_if.fMask=MIIM_STRING|MIIM_STATE|MIIM_ID;
    mi_if.dwTypeData=text;
    mi_if.cch=sizeof(text);
    GetMenuItemInfo(menu,menu_item_id,FALSE,&mi_if);

    /*
     * 识别菜单项是第一项还是最后一项
     */
    int itemcount=GetMenuItemCount(menu);
    int is_first_item=(GetMenuItemID(menu,0)==menu_item_id)?1:0;
    int is_last_item=(GetMenuItemID(menu,itemcount-1)==menu_item_id)?1:0;

    // 放弃该方式
    // if(is_first_item) rect.top+=15+1;

    HBRUSH brushbk=CreateSolidBrush(rgb_bk);
    if(draw->itemState&ODS_CHECKED==ODS_CHECKED) {
        if(mi_if.fState&MFS_DISABLED==MFS_DISABLED) {
            brush=CreateSolidBrush(rgb_bk);
            pre_brush=(HBRUSH)SelectObject(hdc,brush);
            SetTextColor(hdc,rgb_text_disabled);
            pen=CreatePen(PS_SOLID,1,rgb_text_disabled);
            pre_pen=(HPEN)SelectObject(hdc,pen);
        }
        else {
            brush=CreateSolidBrush(rgb_check);
            pre_brush=(HBRUSH)SelectObject(hdc,brush);
            SetTextColor(hdc,rgb_text_check);
            pen=CreatePen(PS_SOLID,1,rgb_check);
            pre_pen=(HPEN)SelectObject(hdc,pen);
        }
    }
    else {
        brush=CreateSolidBrush(rgb_bk);
        pre_brush=(HBRUSH)SelectObject(hdc,brush);
        if(mi_if.fState&MFS_DISABLED==MFS_DISABLED) {
            SetTextColor(hdc,rgb_text_disabled);
            pen=CreatePen(PS_SOLID,1,rgb_text_disabled);
            pre_pen=(HPEN)SelectObject(hdc,pen);
        }
        else {
            SetTextColor(hdc,rgb_text);
            pen=CreatePen(PS_SOLID,1,rgb_bk);
            pre_pen=(HPEN)SelectObject(hdc,pen);
        }
    }

    SetBkMode(hdc,TRANSPARENT);
    RECT rcBack;
    CopyRect(&rcBack,&rect);
    FillRect(hdc,&rect,brushbk);
    InflateRect(&rcBack,-3,-2);
    FillRect(hdc,&rcBack,brush);


    if(strlen(text)>0) {
        RECT check_rect;
        CopyRect(&check_rect,&rect);
        check_rect.right=rect.left+(rect.bottom-rect.top);

        if((UINT)(mi_if.fState&MFS_CHECKED)==(UINT)MFS_CHECKED) {
            DrawText(hdc,"√",-1,&check_rect,DT_SINGLELINE|DT_VCENTER|DT_CENTER);
        }
        rect.left = rect.left+50;//(rect.bottom-rect.top);
        DrawText(hdc,text,-1,&rect,DT_SINGLELINE|DT_VCENTER|DT_EXPANDTABS);//DT_EXPANDTABS???\t
    }
    else {
        //绘制分割条
        POINT pt[5]={{rect.left+5,(rect.top+rect.bottom)/2},
                     {rect.right-5,(rect.top+rect.bottom)/2},
                     {rect.left+5,(rect.top+rect.bottom)/2+1},
                     {rect.right-5,(rect.top+rect.bottom)/2+1}};
        
        COLORREF color_1=RGB(19,19,19),color_2=RGB(68,68,68);
        HPEN sp_line1=CreatePen(PS_SOLID,1,color_1);
        HPEN sp_line2=CreatePen(PS_SOLID,1,color_2);
        HPEN pre_sp;
        
        pre_sp=(HPEN)SelectObject(hdc,sp_line1);
        MoveToEx(hdc,pt[0].x,pt[0].y,&(pt[4]));
        LineTo(hdc,pt[1].x,pt[1].y);
        
        SelectObject(hdc,sp_line2);
        MoveToEx(hdc,pt[2].x,pt[2].y,&(pt[4]));
        LineTo(hdc,pt[3].x,pt[3].y);
        SelectObject(hdc,pre_sp);
        DeleteObject(sp_line1);
        DeleteObject(sp_line2);
    }

    Gdiplus::Graphics graphics(hdc);
    graphics.SetSmoothingMode(SmoothingModeHighQuality);
    Gdiplus::Pen penEdge(Gdiplus::Color(255,43,43,43),3.0f);
    InflateRect(&rcItem,2,2);
    int x=15,y=15;//半径
    if(is_first_item==1) {
        /*
         * 绘制圆弧
         */
        // Gdiplus::Rect rcLeft(rcItem.left,rcItem.top,x*2,y*2);
        // Gdiplus::Rect rcRight(rcItem.right-x*2,rcItem.top,x*2,y*2);
        // graphics.DrawLine(&penEdge,rcItem.left,rcItem.bottom,rcLeft.GetLeft(),rcLeft.GetTop()+y);
        // graphics.DrawArc(&penEdge,rcLeft,-180,90);
        // graphics.DrawLine(&penEdge,rcLeft.GetLeft()+x,rcLeft.GetTop(),rcRight.GetRight()-x,rcRight.GetTop());
        // graphics.DrawArc(&penEdge,rcRight,-90,90);
        // graphics.DrawLine(&penEdge,rcRight.GetRight(),rcRight.GetTop()+y,rcItem.right,rcItem.bottom);
    }
    if(is_last_item==1) {


    }
    if(is_first_item+is_last_item==0) {

    }
    
    DeleteObject(brushbk);
    DeleteObject(SelectObject(hdc,pre_brush));
    DeleteObject(SelectObject(hdc,pre_pen));
    return 0;
}
