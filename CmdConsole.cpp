/*
 * 控制台程序
 * 控制台管道，查看读入的控制台命令、执行的进程、命令的输出结果
 * 
 * +------------------------------------------------------------------------------+
 * |   CMD(ICON)   命令行                                          时间            |
 * +------------------------------------------------------------------------------+
 * |   (OK/FAILED icons)>>.....(支持折叠吗)                                        |
 * |                                                                              |
 * |                                                                              |
 * +------------------------------------------------------------------------------+
 * 
 * 每次命令执行完毕，需要写入日志，并从日中读取解析
 * 技术点：
 * 从文件加载最近的100条，如果滚动到顶，会添加，默认只取100条
 * 文件中的每条记录需要有个ID,界面也会有ID，当前向滚动的时候，会到文件中找到对应ID，然后往前倒...
 * 在控件的内存中，显示的记录数是恒定的，往前倒的过程中，会扩大，然后新记录进来的时候，会再次循环占用
 * 控件一方面接受pipe_exec 发送的即时记录，一边会从文件中找 ， 当然 pipe_exec 也会写入 文件
 * 
 */

/*
 * 命令的最大长度 256
 * 返回的最大长度 256
 */

#include "ydark.h"
#include "cmdconsole.h"

pCmdView CmdView_GetSettings(HWND hwnd) {
    return (pCmdView)GetWindowLongPtr(hwnd,GWLP_USERDATA);
}

int CmdView_InitialSettings(HWND hwnd) {
    pCmdView cdv=(pCmdView)calloc(sizeof(CmdView),1);

    HMODULE instance=LoadLibrary("CmdView.dll");
    cdv->list=ImageList_Create(24,24,ILC_MASK|ILC_COLOR32,1,1);
    ImageList_AddIcon(cdv->list,LoadIcon(instance,MAKEINTRESOURCE(0x0004)));
    ImageList_AddIcon(cdv->list,LoadIcon(instance,MAKEINTRESOURCE(0x0005)));
    ImageList_AddIcon(cdv->list,LoadIcon(instance,MAKEINTRESOURCE(0x0006)));
    ImageList_AddIcon(cdv->list,LoadIcon(instance,MAKEINTRESOURCE(0x0007)));
    ImageList_AddIcon(cdv->list,LoadIcon(instance,MAKEINTRESOURCE(0x0008)));
    FreeLibrary(instance);
    cdv->capacity=100;
    cdv->occupied=0;
    cdv->head=0;
    cdv->tail=0;
    cdv->cel=(pCEL)calloc(sizeof(CEL)*cdv->capacity,1);
    
    cdv->item_height=180;
    cdv->item_width=600;

    SetWindowLongPtr(hwnd,GWLP_USERDATA,(ULONG_PTR)cdv);
    return 0;
}

void CmdView_ClearSettings(HWND hwnd) {
    pCmdView cdv=CmdView_GetSettings(hwnd);
    if(cdv) {
        if(cdv->list) {
            ImageList_Destroy(cdv->list);
            cdv->list=NULL;
        }

        free(cdv);
    }

}

int CmdView_Realloc(HWND hwnd,int appendsize) {
    if(appendsize<=0) return -1;
    pCmdView cdv=CmdView_GetSettings(hwnd);
    if(!cdv||!cdv->cel) return -1;

    int capacity_curr=cdv->capacity+appendsize;
    pCEL cel=(pCEL)calloc(sizeof(CEL)*capacity_curr,1);
    if(!cel) return -1;

    /*
     * 将cdv->cel 拷贝至 cel，并更新 head,tail 的 index
     * 
     * 注意
     * tail 表示最新最近的 CEL, head 为最旧， head --- tail 之间为占用
     * occupied的含义：
     *  0 : head=tail=0;
     *  n (n<=capacity-1):  head --> tail 为 n;
     *  当全部占满，n=capacity-1, 此时 如果继续加入节点，会从head开始覆盖
     * 
     */
    for(int idx=0;idx<cdv->occupied;idx++) {
        memcpy(cel+idx,cdv->cel+((cdv->head+idx)%cdv->capacity),sizeof(CEL));
    }
    free(cdv->cel);
    cdv->cel=cel;
    cdv->capacity=capacity_curr;
    cdv->head=0;
    cdv->tail=cdv->head+cdv->occupied;

    return 0;
}

int CmdView_AttachCmdList(HWND hwnd,pCEL cel,int cel_count) {
    for(int idx=0;idx<cel_count;idx++) {
        int ret=CmdView_AttachCmdNode(hwnd,cel+idx);
        if(ret!=0) return ret;
    }

    return 0;
}

int CmdView_AttachCmdNode(HWND hwnd,pCEL cel) {
    pCmdView cdv=CmdView_GetSettings(hwnd);
    if(!cdv||!cdv->cel) return -1;

    /*
     * 是否占满
     */
    int isfull=((cdv->tail+2)%cdv->capacity==cdv->head?1:0);

    if(isfull==1) {
        /*
         * head/tail 均需移动
         */
        cdv->head=(cdv->head+1)%cdv->capacity;
        cdv->tail=(cdv->tail+1)%cdv->capacity;
        memcpy(cdv->cel+cdv->tail,cel,sizeof(CEL));
    } else {
        /*
         * 仅需要调整 tail
         */
        cdv->tail=(cdv->tail+1)%cdv->capacity;
        memcpy(cdv->cel+cdv->tail,cel,sizeof(CEL));
        cdv->occupied++;
    }
    InvalidateRect(hwnd,NULL,TRUE);

    return 0;

}

LRESULT CALLBACK CmdViewProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
    LRESULT ret=0;
    
    //pMAPS mps=WorkFlowView_GetSettings(hwnd);
    //pNodeMap map=mps->map;
    
    switch(msg) {
    case WM_PAINT: {
        return CmdViewPaint(hwnd,wParam,lParam);
    } break;
    case WM_MOUSEWHEEL: {
        int wheel_delta=GET_WHEEL_DELTA_WPARAM(wParam);
        int scroll_pixls=((double)wheel_delta/60)*-1;
        pCmdView cdv=CmdView_GetSettings(hwnd);
        if(!cdv) return TRUE;

        RECT rcScroll;
        BOOL hasHorzScrollBar=(CmdView_GetZoneRect(hwnd,ZHSCROLL,&rcScroll)==0);
        BOOL hasVertScrollBar=(CmdView_GetZoneRect(hwnd,ZVSCROLL,&rcScroll)==0);

        if(hasHorzScrollBar||hasVertScrollBar) {
            if(hasHorzScrollBar&&!hasVertScrollBar) {
                cdv->offset.x+=scroll_pixls;
                CmdView_GetZoneRect(hwnd,ZHSTHUMB,&rcScroll);
            } else {
                cdv->offset.y+=scroll_pixls;
                CmdView_GetZoneRect(hwnd,ZVSTHUMB,&rcScroll);
            }
            InvalidateRect(hwnd,NULL,TRUE);
        }
        return TRUE;
    } break;
    case WM_LBUTTONDOWN: {
        POINT pt={
            GET_X_LPARAM(lParam),
            GET_Y_LPARAM(lParam)
        };

        //if(wParam==VK_ESCAPE) {
            pCmdView cdv=CmdView_GetSettings(hwnd);
            if(!cdv) break;
            
            if(CmdView_HitTest(hwnd,pt)==ZHSTHUMB) {
                cdv->prept.x=pt.x;
                cdv->prept.y=-1;
                SetCapture(hwnd);
                break;
            }
            if(CmdView_HitTest(hwnd,pt)==ZVSTHUMB) {
                cdv->prept.y=pt.y;
                cdv->prept.x=-1;
                SetCapture(hwnd);
                break;
            }

            if(cdv->viewcheck_mode!=1) {
                cdv->viewcheck_mode=1;
                InvalidateRect(hwnd,NULL,TRUE);
            }
        //}
    } break;
    case WM_MOUSEMOVE: {
        POINT pt={
            GET_X_LPARAM(lParam),
            GET_Y_LPARAM(lParam)
        };
        pCmdView cdv=CmdView_GetSettings(hwnd);
        if(!cdv) break;
        
        if(GetCapture()==hwnd) {
            if(cdv->prept.x!=-1) {
                RECT rcHorz,rcHorzThumb;
                CmdView_GetZoneRect(hwnd,ZHSCROLL,&rcHorz);
                CmdView_GetZoneRect(hwnd,ZHSTHUMB,&rcHorzThumb);

                cdv->offset.x+=(double)(pt.x-cdv->prept.x)/((rcHorz.right-rcHorz.left)-(rcHorzThumb.right-rcHorzThumb.left))*(cdv->item_width-(rcHorz.right-rcHorz.left));
                cdv->prept.x=pt.x;
                CmdView_GetZoneRect(hwnd,ZHSTHUMB,&rcHorzThumb);
            } else {
                RECT rcVertThumb,rcVert;
                CmdView_GetZoneRect(hwnd,ZVSCROLL,&rcVert);
                CmdView_GetZoneRect(hwnd,ZVSTHUMB,&rcVertThumb);
                int ypage=(rcVert.bottom-rcVert.top)/cdv->item_height;
                if(ypage<1) ypage=1;
                int offsety=(double)(pt.y-cdv->prept.y)/((rcVert.bottom-rcVert.top)-(rcVertThumb.bottom-rcVertThumb.top))*(cdv->occupied-ypage);
                if(abs(offsety)<1) break;
                cdv->offset.y+=offsety;
                cdv->prept.y=pt.y;
                CmdView_GetZoneRect(hwnd,ZVSTHUMB,&rcVertThumb);
            }
            InvalidateRect(hwnd,NULL,TRUE);
        }

    } break;
    case WM_LBUTTONUP: {
        //if(wParam==VK_ESCAPE) {
            pCmdView cdv=CmdView_GetSettings(hwnd);
            if(!cdv) break;
            
            if(GetCapture()==hwnd) ReleaseCapture();

            if(cdv->viewcheck_mode!=0) {
                cdv->viewcheck_mode=0;
                InvalidateRect(hwnd,NULL,TRUE);
            }
        //}
    } break;
    case WM_SIZE: {
        pCmdView cdv=CmdView_GetSettings(hwnd);
        if(cdv) {
            cdv->offset={0,0};
            cdv->prept={0,0};
        }
    } break;
    case WM_NCDESTROY: {
        CmdView_ClearSettings(hwnd);
    } break;
    case WM_ERASEBKGND: {
        return 1;
    } break;
    }
    ret=CallWindowProc(DefWindowProc,hwnd,msg,wParam,lParam);
    return ret;
}

int CmdView_GetContentHeight(HWND hwnd) {
    pCmdView cdv=CmdView_GetSettings(hwnd);
    if(!cdv) return 0;

    return cdv->occupied;
}

int CmdView_GetContentWidth(HWND hwnd) {
    
    return -1;
}

int CmdView_HitTest(HWND hwnd,POINT pt) {
    pCmdView cdv=CmdView_GetSettings(hwnd);
    if(!cdv) return -1;

    RECT rcHorzThumb,rcVertThumb;
    if(CmdView_GetZoneRect(hwnd,ZHSTHUMB,&rcHorzThumb)==0) {
        if(PtInRect(&rcHorzThumb,pt)) return ZHSTHUMB;
    }
    if(CmdView_GetZoneRect(hwnd,ZVSTHUMB,&rcVertThumb)==0) {
        if(PtInRect(&rcVertThumb,pt)) return ZVSTHUMB;
    }

    return 0;
}

int CmdView_GetZoneRect(HWND hwnd,int code,LPRECT prc) {
    RECT rc;
    GetClientRect(hwnd,&rc);
    int cx=rc.right-rc.left,cy=rc.bottom-rc.top;
    pCmdView cdv=CmdView_GetSettings(hwnd);
    if(!cdv) return -1;

    int view_count=cy/cdv->item_height;
    BOOL hasvscrollbar=view_count<cdv->occupied;
    BOOL hashscrollbar=cx-(hasvscrollbar?SCROLLBAR_PIXLS:0)<cdv->item_width;

    switch(code) {
    case ZHSTHUMB: {
        /*
         * HSCROLL: 水平滚动条
         */
        if(!hashscrollbar) return -1;
        RECT rcHorz;
        if(CmdView_GetZoneRect(hwnd,ZHSCROLL,&rcHorz)!=0) return -1;

        int contentcx=rcHorz.right-rcHorz.left;
        int xpage=contentcx;
        int xrange1=0;
        int xrange2=cdv->item_width-xpage;

        if(cdv->offset.x<xrange1) cdv->offset.x=xrange1;
        if(cdv->offset.x>xrange2) cdv->offset.x=xrange2;

        /*
         * vertcx/contentcx=contentcx/cdv->item_width
         */
        int horzcx=(double)contentcx/cdv->item_width*contentcx;
        if(horzcx<THUMB_MIN_PIXLS) horzcx=THUMB_MIN_PIXLS;
        /*
         * horz_offset/ 
         */
        int horz_offset=(double)(contentcx-horzcx)*cdv->offset.x/(cdv->item_width-contentcx);
        SetRect(prc,rc.left+horz_offset,
            rc.bottom-SCROLLBAR_PIXLS,
            rc.left+horz_offset+horzcx,
            rc.bottom-1);
        return 0;
    } break;
    case ZHSCROLL: {
        if(!hashscrollbar) return -1;
        SetRect(prc,rc.left,rc.bottom-SCROLLBAR_PIXLS,rc.right-(hasvscrollbar?SCROLLBAR_PIXLS:0),rc.bottom-1);
        return 0;
    } break;
    case ZVSTHUMB: {
        /*
         * VSCROLL: 垂直滚动条
         */
        if(!hasvscrollbar) return -1;
        int contentcy=cy-(hashscrollbar?SCROLLBAR_PIXLS:0);
        int ypage=contentcy/cdv->item_height;
        if(ypage<=0) ypage=1;
        int yrange1=0;
        int yrange2=cdv->occupied-ypage;

        if(cdv->offset.y<yrange1) cdv->offset.y=yrange1;
        if(cdv->offset.y>yrange2) cdv->offset.y=yrange2;

        /*
         * vertcy： 垂直滚动条的高度
         * vertcy / cy = cdv->ypage / cdv->occupied
         * 
         * cdv->offset.y/(cdv->capacity-ypage)=vert_offset/(contentcy-vertcy)
         */
        int vertcy=((double)ypage)/cdv->occupied*contentcy;
        vertcy=((vertcy<THUMB_MIN_PIXLS)?THUMB_MIN_PIXLS:vertcy);
        int vert_offset=((double)(cdv->offset.y)/(cdv->occupied-ypage))*(contentcy-vertcy);
        SetRect(prc,rc.right-SCROLLBAR_PIXLS,
            rc.top+vert_offset,
            rc.right-1,
            rc.top+vert_offset+vertcy);
        return 0;
    } break;
    case ZVSCROLL: {
        /*
         * 垂直滚动条 
         */
        if(hasvscrollbar) {
            SetRect(prc,rc.right-SCROLLBAR_PIXLS,rc.top,rc.right-1,rc.bottom-(hashscrollbar?SCROLLBAR_PIXLS:0));
            return 0;
        } else return -1;
        
    } break;
    }

    return 0;
}

int CmdViewPaint(HWND hwnd,WPARAM wParam,LPARAM lParam) {
    pCmdView cdv=CmdView_GetSettings(hwnd);
    if(!cdv) return 0;

    PAINTSTRUCT ps={0};
    RECT rc,rcMem;
    GetClientRect(hwnd,&rc);
    OffsetRect(&rc,-rc.left,-rc.top);
    CopyRect(&rcMem,&rc);
    int cx=rcMem.right,cy=rcMem.bottom;
    if(!(cx>0&&cy>0)) return 0;

    HDC hdc=BeginPaint(hwnd,&ps);
    HDC memdc=CreateCompatibleDC(hdc);
    HBITMAP bmp=CreateCompatibleBitmap(hdc,cdv->item_width+SCROLLBAR_PIXLS,cy);
    HBITMAP prebmp=(HBITMAP)SelectObject(memdc,bmp);
    HBRUSH brushbk=CreateSolidBrush(RGB(25,25,25));//CreateSolidBrush(RGB(20,20,20));
    RECT rcBack={0,0,cdv->item_width,cy};
    FillRect(memdc,&rcBack,brushbk);
    DeleteObject(brushbk);    

    int itemcount=cy/cdv->item_height+((cy%cdv->item_height==0)?0:1);

    RECT rcVert,rcHorz;
    int hasVert=0;
    int hasHorz=0;
    if(CmdView_GetZoneRect(hwnd,ZVSCROLL,&rcVert)==0) hasVert=1;
    if(CmdView_GetZoneRect(hwnd,ZHSCROLL,&rcHorz)==0) hasHorz=1;

    //cdv->item_width=cx-((hasVert==1)?SCROLLBAR_PIXLS:0);
    RECT rcCell={0,0,cdv->item_width,cdv->item_height};
    if(cdv->offset.x>0) ExcludeClipRect(memdc,0,0,cdv->offset.x,cy);
    if(itemcount>cdv->occupied-cdv->offset.y) itemcount=cdv->occupied-cdv->offset.y;
    /*
     * 显示方式
     * 将最新的显示在最上面 cdv->cel+tail ---> cdv->cel+tail-1 --> .... --> cdv->cel+(cdv->head+1)
     * 在视图中，显示的是 cdv->cel+(tail+offset.y)
     * int item_idx=(cdv->offset.y+cdv->tail+idx)%cdv->capacity;
     * pCEL cel=cdv->cel+item_idx;
     */
    HICON iconFail=ImageList_GetIcon(cdv->list,0,ILD_NORMAL);
    HICON iconSuccess=ImageList_GetIcon(cdv->list,1,ILD_NORMAL);
    HICON iconTime=ImageList_GetIcon(cdv->list,2,ILD_NORMAL);
    HICON iconClock=ImageList_GetIcon(cdv->list,3,ILD_NORMAL);
    HICON iconCmd=ImageList_GetIcon(cdv->list,4,ILD_NORMAL);
    LOGFONT lf={0};
    GetObject(GetStockObject(SYSTEM_FONT),sizeof(LOGFONT),&lf);
    lf.lfHeight=18;
    lf.lfWeight=22;
    strcpy(lf.lfFaceName,"微软雅黑");
    HFONT font=CreateFontIndirect(&lf);
    HFONT prefont=(HFONT)SelectObject(memdc,font);
    SetTextColor(memdc,RGB(55,55,55));
    SetBkMode(memdc,TRANSPARENT);

    for(int idx=0;idx<itemcount;idx++) {
        RECT rcItem;
        CopyRect(&rcItem,&rcCell);
        OffsetRect(&rcItem,0,idx*cdv->item_height);

        RECT rcBeginIco,rcEndIco;
        RECT rcBegin,rcEnd;
        RECT rcCmdIco;
        RECT rcRetIco;
        RECT rcInput;
        RECT rcOutput;

        int iconcx=20,iconcy=20;
        int paddingcx=5,paddingcy=5;
        int linecy=30;

        int item_idx=(cdv->tail-cdv->offset.y-idx+cdv->capacity)%cdv->capacity;
        pCEL cel=cdv->cel+item_idx;
        if(!cel) break;

        SetTextColor(memdc,RGB(55,55,55));
        /*开始图标 */
        SetRect(&rcBeginIco,rcItem.left+paddingcx,rcItem.top+paddingcy,rcItem.left+paddingcx+linecy,rcItem.top+paddingcy+linecy);
        DrawIconEx(memdc,rcBeginIco.left,rcBeginIco.top,iconTime,iconcx,iconcy,0,0,DI_NORMAL);
        /*开始文本*/
        SetRect(&rcBegin,rcBeginIco.right+paddingcx,rcItem.top+paddingcy,rcItem.left+paddingcx+(cx>>1),rcItem.top+paddingcy+linecy);
        DrawText(memdc,cel->header.begin,-1,&rcBegin,DT_VCENTER|DT_LEFT);

        CopyRect(&rcEnd,&rcBegin);
        OffsetRect(&rcEnd,220,0);
        DrawText(memdc,cel->header.end,-1,&rcEnd,DT_VCENTER|DT_LEFT);

        CopyRect(&rcEndIco,&rcBeginIco);
        OffsetRect(&rcEndIco,220,0);
        DrawIconEx(memdc,rcEndIco.left,rcEndIco.top,iconTime,iconcx,iconcy,0,0,DI_NORMAL);

        SetTextColor(memdc,RGB(55,55,55));
        SetRect(&rcInput,rcBegin.left,rcBegin.bottom+paddingcy,rcItem.right-paddingcx,rcBegin.bottom+paddingcy+linecy*1);
        DrawText(memdc,cel->input,-1,&rcInput,DT_LEFT|DT_WORDBREAK|DT_END_ELLIPSIS);
        SetRect(&rcCmdIco,rcItem.left+paddingcx,rcInput.top,rcItem.left+paddingcx+iconcx,rcInput.top+iconcy);
        DrawIconEx(memdc,rcCmdIco.left,rcCmdIco.top,iconCmd,iconcx,iconcy,0,0,DI_NORMAL);

        SetTextColor(memdc,(cel->header.ret==0?RGB(0,100,200):RGB(200,0,0)));
        SetRect(&rcOutput,rcInput.left,rcInput.bottom+paddingcy,rcInput.right,rcItem.bottom-paddingcy);
        DrawText(memdc,cel->output,-1,&rcOutput,DT_LEFT|DT_WORDBREAK|DT_END_ELLIPSIS|DT_EXPANDTABS);

        SetRect(&rcRetIco,rcCmdIco.left,rcOutput.top,rcCmdIco.left+iconcx,rcOutput.top+iconcy);
        DrawIconEx(memdc,rcRetIco.left,rcRetIco.top,(cel->header.ret==0?iconSuccess:iconFail),iconcx,iconcy,0,0,DI_NORMAL);

        if(cdv->viewcheck_mode==1) {
            RECT rcGuid;
            SetRect(&rcGuid,rcBegin.left,rcBegin.bottom-10,rcBegin.left+300,rcBegin.bottom-5+30);
            SetTextColor(memdc,RGB(200,200,200));
            DrawText(memdc,cel->header.guid,-1,&rcGuid,DT_LEFT|DT_VCENTER);
        }
    }

    /*
     * 垂直滚动条
     */
    Gdiplus::Graphics graphics(memdc);
    if(hasVert==1) {
        RECT rcVertThumb;
        CmdView_GetZoneRect(hwnd,ZVSTHUMB,&rcVertThumb);
        OffsetRect(&rcVert,cdv->offset.x,0);
        OffsetRect(&rcVertThumb,cdv->offset.x,0);

        COLORREF colorBar=RGB(5,5,5);
        HBRUSH brushBar=CreateSolidBrush(colorBar);
        FillRect(memdc,&rcVert,brushBar);
        DeleteObject(brushBar);

        Gdiplus::GraphicsPath path;
        Gdiplus::LinearGradientBrush pbrush(Rect(rcVertThumb.left,rcVertThumb.top,rcVertThumb.right-rcVertThumb.left,rcVertThumb.bottom-rcVertThumb.top),
            Gdiplus::Color(255,53,53,53),
            Gdiplus::Color(155,0,0,0),
            LinearGradientModeHorizontal
        );
        graphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
        path.AddArc(rcVertThumb.left,rcVertThumb.top,SCROLLBAR_PIXLS,SCROLLBAR_PIXLS,180,180);
        path.AddArc(rcVertThumb.left,rcVertThumb.bottom-SCROLLBAR_PIXLS,SCROLLBAR_PIXLS,SCROLLBAR_PIXLS,0,180);
        graphics.FillPath(&pbrush,&path);
    }
    
    /*
     * 水平滚动条
     */
    if(hasHorz==1) {
        RECT rcHorzThumb;
        CmdView_GetZoneRect(hwnd,ZHSTHUMB,&rcHorzThumb);
        OffsetRect(&rcHorz,cdv->offset.x,0);
        OffsetRect(&rcHorzThumb,cdv->offset.x,0);

        COLORREF colorBar=RGB(5,5,5);
        HBRUSH brushBar=CreateSolidBrush(colorBar);
        FillRect(memdc,&rcHorz,brushBar);
        DeleteObject(brushBar);

        Gdiplus::GraphicsPath path;
        Gdiplus::LinearGradientBrush pbrush(Rect(rcHorzThumb.left,rcHorzThumb.top,rcHorzThumb.right-rcHorzThumb.left,rcHorzThumb.bottom-rcHorzThumb.top),
            Gdiplus::Color(255,53,53,53),
            Gdiplus::Color(155,0,0,0),
            LinearGradientModeVertical
        );
        graphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
        path.AddArc(rcHorzThumb.left,rcHorzThumb.top,SCROLLBAR_PIXLS,SCROLLBAR_PIXLS,90,180);
        path.AddArc(rcHorzThumb.right-SCROLLBAR_PIXLS,rcHorzThumb.top,SCROLLBAR_PIXLS,SCROLLBAR_PIXLS,-90,180);
        graphics.FillPath(&pbrush,&path);
    }

    DestroyIcon(iconClock);
    DestroyIcon(iconFail);
    DestroyIcon(iconSuccess);
    DestroyIcon(iconTime);
    DestroyIcon(iconCmd);
    DeleteObject(font);

    BitBlt(hdc,0,0,cx,cy,memdc,cdv->offset.x,0,SRCCOPY);
    SelectObject(memdc,prebmp);
    DeleteObject(bmp);
    DeleteDC(memdc);
    
    EndPaint(hwnd,&ps);
    return 0;
}
