#include "img_scratch.h"
#include "ydark.h"
#include "trzcfg_ctrl.h"
#include "trzcfg.h"
#include <gdiplus.h>

#include "workflow_proj.h"

LRESULT CALLBACK ScratchViewProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
    LRESULT ret=0;

    switch(msg) {
    case WM_PAINT: {
        return ScratchView_Paint(hwnd,wParam,lParam);
    } break;
    case WM_LBUTTONDOWN: {
        pScratchView psv=ScratchView_GetSettings(hwnd);
        if(!psv) return 1;

        POINT pt={
            GET_X_LPARAM(lParam),
            GET_Y_LPARAM(lParam)
        };
        if(psv->mode==1) {
            RECT rcSave,rcCfg;
            if(/* ScratchView_GetZoneRect(hwnd,CTRL_CONFIG,&rcCfg)==0|| */
               ScratchView_GetZoneRect(hwnd,CTRL_SAVE,&rcSave)==0) {
                if(PtInRect(&rcSave,pt)/* ||PtInRect(&rcCfg,pt) */) {
                    //MessageBox(hwnd,"You have clicked the save button.","TEST",MB_OK);
                    memset(&trzcfg,0x00,sizeof(TRZ));
                    trzcfg.width=abs(psv->cx);trzcfg.height=abs(psv->cy);
                    trzcfg.x=(psv->cx>0)?psv->pos.x:(psv->pos.x+psv->cx);
                    trzcfg.y=(psv->cy>0)?psv->pos.y:(psv->pos.y+psv->cy);

                    HINSTANCE inst=(HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE);
                    INT_PTR result=DialogBox(inst,MAKEINTRESOURCE(IDD_TRZCFG),hwnd,TRZCfgHandle);
                    HWND frame=GetParent(hwnd);
                    UINT style=(UINT)GetWindowLongPtr(frame,GWL_STYLE);
                    SetWindowLongPtr(frame,GWL_STYLE,(LONG_PTR)(style&~WS_DISABLED));
                    return 0;
                }
            }
            RECT rcCancel;
            if(ScratchView_GetZoneRect(hwnd,CTRL_CANCEL,&rcCancel)==0) {
                if(PtInRect(&rcCancel,pt)) {
                    psv->pos={0,0};
                    psv->cx=0;psv->cy=0;
                    InvalidateRect(hwnd,NULL,TRUE);
                    return 0;
                }
            }
            RECT rcOCR;
            if(ScratchView_GetZoneRect(hwnd,CTRL_OCR,&rcOCR)==0) {
                if(PtInRect(&rcOCR,pt)) {
                    //进行OCR识别
                    //...
                    InvalidateRect(hwnd,NULL,TRUE);
                    return 0;
                }
            }

            /*
             * 水平滚动条
             */
            if(ScratchView_HitTest(hwnd,pt)==(int)ZHSTHUMB) {
                SetCapture(hwnd);
                psv->prept.x=pt.x;
                psv->prept.y=-1;
                return 0;
            }
            if(ScratchView_HitTest(hwnd,pt)==(int)ZVSTHUMB) {
                SetCapture(hwnd);
                psv->prept.y=pt.y;
                psv->prept.x=-1;
                return 0;
            }

            RECT rcImg;
            if(ScratchView_GetZoneRect(hwnd,IMG_VIEW,&rcImg)==0) {
                if(PtInRect(&rcImg,pt)) {
                    /*
                     * psv->pos 记录的是 相对于 图像的位置坐标
                     * 不是相对于屏幕窗口的偏移
                     */
                    psv->pos.x=pt.x-rcImg.left+psv->offset.x;
                    psv->pos.y=pt.y-rcImg.top+psv->offset.y;
                    psv->cx=psv->cy=0;
                }
            }

        }

        RECT rcFile;
        if(ScratchView_GetZoneRect(hwnd,CTRL_FILE,&rcFile)==0&&PtInRect(&rcFile,pt)) {
            /*
                * 打开新文件
                */
            char filename[256]={0};
            OPENFILENAME opfn;
            ZeroMemory(&opfn,sizeof(OPENFILENAME));
            opfn.lStructSize=sizeof(OPENFILENAME);
            opfn.lpstrFilter="所有文件\0*.*\0";
            opfn.nFilterIndex=1;
            opfn.lpstrFile=filename;
            opfn.lpstrFile[0]='\0';
            opfn.nMaxFile=sizeof(filename);
            opfn.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;

            if(GetOpenFileName(&opfn)) {
                ScratchView_SetImage(hwnd,filename);
                return 0;
            }
        }
        RECT rcExpend;
        if(ScratchView_GetZoneRect(hwnd,CTRL_MODE_ORI,&rcExpend)==0&&PtInRect(&rcExpend,pt)) {
            if(psv->mode!=1) {
                ScratchView_ChangeMode(hwnd,1);
                return 0;
            }
        }
        RECT rcCollapse;
        if(ScratchView_GetZoneRect(hwnd,CTRL_MODE_SCALE,&rcCollapse)==0&&PtInRect(&rcCollapse,pt)) {
            if(psv->mode!=0) {
                ScratchView_ChangeMode(hwnd,0);
                return 0;
            }
        }
        RECT rcLaunch;
        if(ScratchView_GetZoneRect(hwnd,CTRL_LAUNCHWORK,&rcLaunch)==0&&PtInRect(&rcLaunch,pt)) {
            pwfpj_param wfpj=(pwfpj_param)(f->extra);
            if(!wfpj) return 0;

            if(psv->mode==0) {
                //ResumeThread(wfpj->ocrthread);
                return 0;
            }
            
            /*
             * 判断线程是否有效  
             */
            DWORD exitcode;
            BOOL result=GetExitCodeThread(wfpj->ocrthread,&exitcode);
            if(!result) {
                /*
                 * 线程句柄无效
                 * 创建线程，即时启动
                 */
                MessageBox(hwnd,"线程无效，将启动新的线程...","提示",MB_OK);
                if(wfpj->ocrthread!=NULL) CloseHandle(wfpj->ocrthread);
                DWORD threadid;
                wfpj->ocrthread=CreateThread(NULL,0,DouYinOCRJOB,NULL,0,&threadid);
                wfpj->ocrthreadsuspend_ref=0;
            } else {
                if(exitcode!=STILL_ACTIVE) {
                    /*
                     * 线程已经结束
                     */
                    MessageBox(hwnd,"线程已退出，将启动新的线程...","提示",MB_OK);
                    if(wfpj->ocrthread!=NULL) CloseHandle(wfpj->ocrthread);
                    DWORD threadid;
                    wfpj->ocrthread=CreateThread(NULL,0,DouYinOCRJOB,NULL,0,&threadid);
                    wfpj->ocrthreadsuspend_ref=0;
                } else {
                    /*
                     * 线程尚在运行
                     * 判断线程是否挂起
                     */
                    //由于使用非公开windwos sdk接口，该函数在较新版本中可能存在兼容性问题
                    //经过简单测试，该函数不被兼容，baga
                    //if(IsThreadSuspended(wfpj->ocrthread)) {
                    if(wfpj->ocrthreadsuspend_ref>0) {
                        ResumeThread(wfpj->ocrthread);
                        wfpj->ocrthreadsuspend_ref=0;
                    }

                    MessageBox(hwnd,"线程运行中...","提示",MB_OK);
                    return 0;
                }
            }
        }
        RECT rcSuspend;
        if(ScratchView_GetZoneRect(hwnd,CTRL_SUSPENDWORK,&rcSuspend)==0&&PtInRect(&rcSuspend,pt)) {
            if(psv->mode!=0) {
                
                return 0;
            }
        }

    } break;
    case WM_MOUSEMOVE: {
        pScratchView psv=ScratchView_GetSettings(hwnd);
        if(!psv) return 1;

        POINT pt={
            GET_X_LPARAM(lParam),
            GET_Y_LPARAM(lParam)
        };
        if(psv->mode==1) {
            /*
             * 拖拽滚动条
             */
            if(GetCapture()==hwnd) {
                /*
                 * 设置新的offset
                 */
                if(psv->prept.x!=-1) {
                    /*
                     * 水平滚动
                     * 根据鼠标移动的像素 pt.x-psv->prept.x 
                     * 计算 offset 移动的值
                     * pt.x-psv->prept.x  / cxScrollbar-cxScrollThumb  = offsetx / psv->pixels_x - viewcx
                     */
                    RECT rcHorzThumb,rcHorz;
                    ScratchView_GetZoneRect(hwnd,ZHSCROLL,&rcHorz);
                    ScratchView_GetZoneRect(hwnd,ZHSTHUMB,&rcHorzThumb);
                    
                    psv->offset.x+=(double)(pt.x-psv->prept.x)/((rcHorz.right-rcHorz.left)-(rcHorzThumb.right-rcHorzThumb.left))*(psv->pixels_x-(rcHorz.right-rcHorz.left));
                    psv->prept.x=pt.x;
                    ScratchView_GetZoneRect(hwnd,ZHSTHUMB,&rcHorzThumb);
                } else {
                    /*
                     * 垂直滚动
                     */
                    RECT rcVertThumb,rcVert;
                    ScratchView_GetZoneRect(hwnd,ZVSCROLL,&rcVert);
                    ScratchView_GetZoneRect(hwnd,ZVSTHUMB,&rcVertThumb);
                    psv->offset.y+=(double)(pt.y-psv->prept.y)/((rcVert.bottom-rcVert.top)-(rcVertThumb.bottom-rcVertThumb.top))*(psv->pixels_y-(rcVert.bottom-rcVert.top));
                    psv->prept.y=pt.y;
                    ScratchView_GetZoneRect(hwnd,ZVSTHUMB,&rcVertThumb);
                }

                InvalidateRect(hwnd,NULL,TRUE);
                return 0;
            }

            RECT rcImg;
            /*
             * 左键按下
             */
            if(ScratchView_GetZoneRect(hwnd,IMG_VIEW,&rcImg)==0&&(wParam&MK_LBUTTON==MK_LBUTTON)) {
                if(pt.x<rcImg.left) pt.x=rcImg.left;
                if(pt.x>rcImg.right) pt.x=rcImg.right;

                if(pt.y<rcImg.top) pt.y=rcImg.top;
                if(pt.y>rcImg.bottom) pt.y=rcImg.bottom;

                psv->cx=pt.x-rcImg.left+psv->offset.x-psv->pos.x;
                psv->cy=pt.y-rcImg.top+psv->offset.y-psv->pos.y;
            }
        }
        
        psv->pt={pt.x,pt.y};
        InvalidateRect(hwnd,NULL,TRUE);

    } break;
    case WM_LBUTTONUP: {
        if(GetCapture()==hwnd) ReleaseCapture();
    } break;
    case WM_MOUSEWHEEL: {
        int wheel_delta=GET_WHEEL_DELTA_WPARAM(wParam);
        int scroll_pixls=((double)wheel_delta/10)*2*-1;
        pScratchView psv=ScratchView_GetSettings(hwnd);
        if(!psv) return TRUE;

        RECT rcScroll;
        BOOL hasHorzScrollBar=(ScratchView_GetZoneRect(hwnd,ZHSCROLL,&rcScroll)==0);
        BOOL hasVertScrollBar=(ScratchView_GetZoneRect(hwnd,ZVSCROLL,&rcScroll)==0);

        if(hasHorzScrollBar||hasVertScrollBar) {
            if(hasHorzScrollBar&&!hasVertScrollBar) {
                psv->offset.x+=scroll_pixls;
                ScratchView_GetZoneRect(hwnd,ZHSCROLL,&rcScroll);
            }
            else {
                psv->offset.y+=scroll_pixls;
                ScratchView_GetZoneRect(hwnd,ZVSCROLL,&rcScroll);
            }
            InvalidateRect(hwnd,NULL,TRUE);
        }
        return TRUE;
    } break;
    case WM_ERASEBKGND: return 1;
    case WM_NCDESTROY: {
        ScratchView_ClearSettings(hwnd);
    } break;
    case WM_SIZE: {
        /*
         * 更新 offset
         */
        RECT rcScroll;
        BOOL hasHorzScrollBar=(ScratchView_GetZoneRect(hwnd,ZHSCROLL,&rcScroll)==0);
        BOOL hasVertScrollBar=(ScratchView_GetZoneRect(hwnd,ZVSCROLL,&rcScroll)==0);
        pScratchView psv=ScratchView_GetSettings(hwnd);
        if(psv) {
            psv->offset={0,0};
            psv->prept={0,0};
        }
        InvalidateRect(hwnd,NULL,TRUE);
    } break;
    case MSG_SECTIONSETTINGS: {
        pTRZ trz=(pTRZ)wParam;
        TRZCODE trzcode=(TRZCODE)lParam;

        trzcfg.code=trzcode;
        /*
         * 打开对话框
         */
        pScratchView psv=ScratchView_GetSettings(hwnd);
        if(!psv) return 1;

        HINSTANCE inst=(HINSTANCE)GetWindowLongPtr(hwnd,GWLP_HINSTANCE);
        INT_PTR result=DialogBox(inst,MAKEINTRESOURCE(IDD_TRZCFG),hwnd,TRZCfgHandle);
        HWND frame=GetParent(hwnd);
        UINT style=(UINT)GetWindowLongPtr(frame,GWL_STYLE);
        SetWindowLongPtr(frame,GWL_STYLE,(LONG_PTR)(style&~WS_DISABLED));        

    } break;
    }

    ret=CallWindowProc(DefWindowProc,hwnd,msg,wParam,lParam);
    
    return ret;
}

int ScratchView_InitialSettings(HWND hwnd) {
    pScratchView psv=(pScratchView)calloc(sizeof(ScratchView),1);
    if(!psv) return -1;
    psv->mode=1;
    psv->offset={0,0};

    psv->smps_count=9;
    psv->smps=(pTRZSmps)calloc(sizeof(TRZSmps),psv->smps_count);
    if(psv->smps) {
        for(int idx=0;idx<psv->smps_count;idx++) {
            pTRZSmps smps=psv->smps+idx;
            smps->code=(PAGECODE)idx;
        }
    }

    HMODULE instance=LoadLibrary("scratchview.dll");
    psv->list=ImageList_Create(24,24,ILC_MASK|ILC_COLOR32,1,1);
    ImageList_AddIcon(psv->list,LoadIcon(instance,MAKEINTRESOURCE(0x0001)));
    ImageList_AddIcon(psv->list,LoadIcon(instance,MAKEINTRESOURCE(0x0002)));
    ImageList_AddIcon(psv->list,LoadIcon(instance,MAKEINTRESOURCE(0x0003)));
    ImageList_AddIcon(psv->list,LoadIcon(instance,MAKEINTRESOURCE(0x0004)));
    ImageList_AddIcon(psv->list,LoadIcon(instance,MAKEINTRESOURCE(0x0005)));
    ImageList_AddIcon(psv->list,LoadIcon(instance,MAKEINTRESOURCE(0x0006)));
    ImageList_AddIcon(psv->list,LoadIcon(instance,MAKEINTRESOURCE(0x0007)));
    ImageList_AddIcon(psv->list,LoadIcon(instance,MAKEINTRESOURCE(0x0008)));
    ImageList_AddIcon(psv->list,LoadIcon(instance,MAKEINTRESOURCE(0x0009)));
    FreeLibrary(instance);

    SetWindowLongPtr(hwnd,GWLP_USERDATA,(ULONG_PTR)psv);
    return 0;
}

int ScratchView_SetImage(HWND hwnd,char* img) {
    pScratchView psv=ScratchView_GetSettings(hwnd);
    if(!psv) return -1;

    memset(psv->img_path,0x00,sizeof(psv->img_path));
    strcpy(psv->img_path,img);
    ScratchView_ChangeMode(hwnd,0);

    //wchar_t img_wchar[256]={0};
    //int wchar_len=MultiByteToWideChar(CP_ACP,0,psv->img_path,-1,NULL,0);
    //MultiByteToWideChar(CP_ACP,0,psv->img_path,-1,img_wchar,wchar_len);
    if(psv->img) {
        delete psv->img;
        psv->img=NULL;
    }

    /*
     * 读取文件内存，这里没有处理异常，后续更正
     */
    char* img_buffer=NULL;
    int img_size;
    HANDLE img_file;
    DWORD dwRead;

    img_file=CreateFile(psv->img_path,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    if(img_file==INVALID_HANDLE_VALUE) {
        MessageBox(hwnd,"打开文件失败","异常",MB_OK|MB_ICONERROR);
        return -1;
    }
    img_size=GetFileSize(img_file,NULL);
    img_buffer=(char*)calloc(sizeof(char)*img_size,1);
    ReadFile(img_file,img_buffer,img_size,&dwRead,NULL);
    if(dwRead<img_size) {
        char read_result_string[256]={0};
        sprintf(read_result_string,"文件尺寸 %d (bytes) , 读取尺寸 %d (bytes)",img_size,dwRead);
        MessageBox(hwnd,read_result_string,"读取文件异常",MB_OK|MB_ICONWARNING);
    }
    CloseHandle(img_file);

    /*
     * 使用流，避免文件占用
     */
    HGLOBAL hGlobal=GlobalAlloc(GMEM_MOVEABLE,img_size);
    void* pData=GlobalLock(hGlobal);
    memcpy(pData,img_buffer,img_size);
    GlobalUnlock(hGlobal);
    IStream* stream=NULL;
    HRESULT hr=CreateStreamOnHGlobal(hGlobal,TRUE,&stream);
    if(img_buffer) free(img_buffer);

    psv->cx=0;
    psv->cy=0;
    psv->pos={0,0};
    psv->img=new Gdiplus::Bitmap(stream);
    stream->Release();
    if(psv->img->GetLastStatus()!=Ok) {
        /*
         * 加载图像出现异常
         */
        MessageBox(hwnd,img,"读取文件内存失败",MB_OK);
        delete psv->img;
        psv->img=NULL;
        psv->pixels_x=psv->pixels_y=0;
        InvalidateRect(hwnd,NULL,TRUE);
        return -1;
    } else {
        psv->pixels_x=psv->img->GetWidth();
        psv->pixels_y=psv->img->GetHeight();
        InvalidateRect(hwnd,NULL,TRUE);
        return 0;
    }
}

int ScratchView_ChangeMode(HWND hwnd,int mode) {
    pScratchView psv=ScratchView_GetSettings(hwnd);
    if(!psv) return -1;

    if(psv->mode!=mode) {
        psv->mode=mode;
        psv->cx=psv->cy=0;
        psv->pos={0,0};
        InvalidateRect(hwnd,NULL,TRUE);
    }
    return 0;
}

pScratchView ScratchView_GetSettings(HWND hwnd) {
    return (pScratchView)GetWindowLongPtr(hwnd,GWLP_USERDATA);
}

void ScratchView_ClearSettings(HWND hwnd) {
    pScratchView psv=ScratchView_GetSettings(hwnd);
    if(!psv) {

        if(psv->smps) {
            free(psv->smps);
            psv->smps=NULL;
        }

        if(psv->list) {
            ImageList_Destroy(psv->list);
            psv->list=NULL;
        }

        if(psv->img) {
            delete psv->img;
            psv->img=NULL;
        }
        free(psv);
    }
}

int ScratchView_HitTest(HWND hwnd,POINT pt) {
    pScratchView psv=ScratchView_GetSettings(hwnd);
    if(!psv) return -1;

    RECT rcView,rcHSThumb,rcVSThumb;

    if(psv->mode==1) {
        if(ScratchView_GetZoneRect(hwnd,ZHSTHUMB,&rcHSThumb)==0) {
            if(PtInRect(&rcHSThumb,pt)) {
                return ZHSTHUMB;
            }
        }
        if(ScratchView_GetZoneRect(hwnd,ZVSTHUMB,&rcVSThumb)==0) {
            if(PtInRect(&rcVSThumb,pt)) {
                return ZVSTHUMB;
            }
        }
    
    }
    return 0;
}

int ScratchView_GetZoneRect(HWND hwnd,int poscode,LPRECT prc) {
    pScratchView psv=ScratchView_GetSettings(hwnd);
    if(!psv) return -1;

    RECT rc;
    GetClientRect(hwnd,&rc);
    int viewcx=rc.right-rc.left;
    int viewcy=rc.bottom-rc.top;
    BOOL hasvertbar=FALSE;
    BOOL hashorzbar=FALSE;
    /* 识别模式 */
    if(psv->mode==1) {
        hasvertbar=viewcy<psv->pixels_y;//垂直滚动条
        hashorzbar=viewcx<psv->pixels_x;//水平滚动条
    } 

/*
 * 图像视图区 
 */
    int panel_cy=35;
    
    switch(poscode) {
    case ZHSTHUMB: {
        if(!hashorzbar) return -1;
        int contentcx=viewcx-(hasvertbar?SCROLLBAR_PIXLS:0);
        int xpage=contentcx;
        int xrange1=0;
        int xrange2=psv->pixels_x-xpage;
        /*
         * contentcx 展示视图区
         * xrange : offsetx 值域范围
         * xpage : 页宽
         * psv->pixels_x 内容宽度
         * 等式：
         * 
         */
        if(psv->offset.x<xrange1) psv->offset.x=xrange1;
        if(psv->offset.x>xrange2) psv->offset.x=xrange2;

        int horzcx=((double)contentcx)*contentcx/psv->pixels_x;
        horzcx=((horzcx<THUMB_MIN_PIXLS)?THUMB_MIN_PIXLS:horzcx);
        int horz_offset=((double)(contentcx-horzcx)*psv->offset.x)/(psv->pixels_x-contentcx);
        SetRect(prc,rc.left+horz_offset,
            rc.bottom-SCROLLBAR_PIXLS,
            rc.left+horz_offset+horzcx,
            rc.bottom-1);
    } break;
    case ZHSCROLL: {
        if(!hashorzbar) return -1;
        int contentcx=viewcx-(hasvertbar?SCROLLBAR_PIXLS:0);
        int xpage=contentcx;
        int xrange1=0;
        int xrange2=psv->pixels_x-xpage;
        /*
         * contentcx 展示视图区
         * xrange : offsetx 值域范围
         * xpage : 页宽
         * psv->pixels_x 内容宽度
         * 等式：
         * 
         */
        if(psv->offset.x<xrange1) psv->offset.x=xrange1;
        if(psv->offset.x>xrange2) psv->offset.x=xrange2;

        SetRect(prc,rc.left,
            rc.bottom-SCROLLBAR_PIXLS,
            rc.right-(hasvertbar?SCROLLBAR_PIXLS:0),
            rc.bottom-1);
    } break;
    case ZVSTHUMB: {
        if(!hasvertbar) return -1;
        int contentcy=viewcy-(hashorzbar?SCROLLBAR_PIXLS:0);
        int ypage=contentcy;
        int yrange1=0;
        int yrange2=psv->pixels_y-ypage;
        /*
         * contentcx 展示视图区
         * xrange : offsetx 值域范围
         * xpage : 页宽
         * psv->pixels_x 内容宽度
         * 等式：
         * 
         */
        if(psv->offset.y<yrange1) psv->offset.y=yrange1;
        if(psv->offset.y>yrange2) psv->offset.y=yrange2;

        int vertcy=((double)contentcy)*contentcy/psv->pixels_y;
        vertcy=((vertcy<THUMB_MIN_PIXLS)?THUMB_MIN_PIXLS:vertcy);
        int vert_offset=((double)(contentcy-vertcy)*psv->offset.y)/(psv->pixels_y-contentcy);
        SetRect(prc,rc.right-SCROLLBAR_PIXLS,
            rc.top+vert_offset,
            rc.right-1,
            rc.top+vert_offset+vertcy);        
    } break;
    case ZVSCROLL: {
        if(!hasvertbar) return -1;
        int contentcy=viewcy-(hashorzbar?SCROLLBAR_PIXLS:0);
        int ypage=contentcy;
        int yrange1=0;
        int yrange2=psv->pixels_y-ypage;
        /*
         * contentcx 展示视图区
         * xrange : offsetx 值域范围
         * xpage : 页宽
         * psv->pixels_x 内容宽度
         * 等式：
         * 
         */
        if(psv->offset.y<yrange1) psv->offset.y=yrange1;
        if(psv->offset.y>yrange2) psv->offset.y=yrange2;

        // int vertcy=((double)contentcy)*contentcy/psv->pixels_y;
        // int vert_offset=((double)(contentcy-vertcy)*psv->offset.y)/(psv->pixels_y-contentcy);
        SetRect(prc,rc.right-SCROLLBAR_PIXLS,
            rc.top,
            rc.right-1,
            rc.bottom-1);   
    } break;
    case IMG_VIEW: {
        int viewcx,viewcy;
        POINT ptbase={rc.right,rc.bottom};
        SetRect(prc,rc.left,rc.top,rc.right,rc.bottom);
        if(psv->mode==0) {
            /*
             * 图像纵横比
             */
            double ratio=(double)psv->pixels_x/psv->pixels_y;
            viewcx=rc.right-rc.left;
            viewcy=rc.bottom-rc.top;
            double view_ratio=(double)viewcx/viewcy;

            if(ratio>view_ratio) {
                /*
                 * 以 viewcx 为基准进行比例适配
                 */
                if(viewcx<psv->pixels_x) {
                    /*
                     * 图像会被压缩
                     */
                    int imgviewcx=viewcx;
                    int imgviewcy=imgviewcx/((double)psv->pixels_x/psv->pixels_y);
                    int offsety=(viewcy-imgviewcy)>>1;
                    SetRect(
                        prc,
                        rc.left,
                        rc.top+offsety,
                        rc.right,
                        rc.top+offsety+imgviewcy
                    );
                } else {
                    /*
                     * 原图尺寸
                     */
                    int imgviewcx=psv->pixels_x;
                    int imgviewcy=psv->pixels_y;
                    int offsetx=(viewcx-imgviewcx)>>1;
                    int offsety=(viewcy-imgviewcy)>>1;
                    SetRect(prc,rc.left+offsetx,rc.top+offsety,
                        rc.left+offsetx+imgviewcx,rc.top+offsety+imgviewcy
                    );
                }
            } else {
                /*
                 * 以 viewcy 为基准进行比例适配
                 */
                if(viewcy<psv->pixels_y) {
                    int imgviewcy=viewcy;
                    int imgviewcx=imgviewcy*((double)psv->pixels_x/psv->pixels_y);
                    int offsetx=(viewcx-imgviewcx)>>1;
                    SetRect(prc,
                        rc.left+offsetx,rc.top,rc.left+offsetx+imgviewcx,rc.bottom
                    );
                } else {
                    int imgviewcy=psv->pixels_y;
                    int imgviewcx=psv->pixels_x;

                    int offsetx=(viewcx-imgviewcx)>>1;
                    int offsety=(viewcy-imgviewcy)>>1;
                    SetRect(prc,rc.left+offsetx,rc.top+offsety,
                        rc.left+offsetx+imgviewcx,rc.top+offsety+imgviewcy
                    );
                }
            }
            break;
        }
        if(psv->mode==1) {

            /*
             * 当前具有垂直滚动条
             * 说明 右侧有滚动条占据
             * 视图右侧区域需要调整
             */
            if(hasvertbar) {
                ptbase.x-=SCROLLBAR_PIXLS;//垂直滚动条
                prc->right=ptbase.x;
            }
            if(hashorzbar) {
                ptbase.y-=SCROLLBAR_PIXLS;//水平滚动条
                prc->bottom=ptbase.y;
            }
        } 

        /*
         * 调整prc居中位置
         * 以长度 psv->pixels_x/psv->pixels_y 为准
         */
        viewcx=rc.right-prc->left;
        viewcy=prc->bottom-prc->top;
        if(psv->pixels_x<viewcx) {
            prc->left+=(viewcx-psv->pixels_x)>>1;
        }
        if(psv->pixels_y<viewcy) {
            prc->top+=(viewcy-psv->pixels_y)>>1;
        }

        if(prc->left+psv->pixels_x<prc->right) prc->right=prc->left+psv->pixels_x;
        if(prc->top+psv->pixels_y<prc->bottom) prc->bottom=prc->top+psv->pixels_y;
    } break;
    case CTRL_PANEL: {
        //if(psv->mode!=1) return -1;
        int panel_cx= panel_cy*8;
        int offsetx=(rc.right-rc.top-panel_cx)>>1;
        SetRect(prc,rc.left+offsetx,rc.top,rc.left+offsetx+panel_cx,rc.top+panel_cy);
    } break;
    case CTRL_MODE_SCALE: {
        if(ScratchView_GetZoneRect(hwnd,CTRL_PANEL,prc)!=0) return -1;
        prc->right=prc->left+panel_cy;

        OffsetRect(prc,panel_cy*1,0);
    } break;
    case CTRL_MODE_ORI: {
        if(ScratchView_GetZoneRect(hwnd,CTRL_PANEL,prc)!=0) return -1;
        prc->right=prc->left+panel_cy;

        OffsetRect(prc,panel_cy*2,0);
    } break;
    case CTRL_FILE: {
        if(ScratchView_GetZoneRect(hwnd,CTRL_PANEL,prc)!=0) return -1;
        prc->right=prc->left+panel_cy;

        OffsetRect(prc,panel_cy*3,0);
    } break;
    case CTRL_CONFIG: {
        if(ScratchView_GetZoneRect(hwnd,CTRL_PANEL,prc)!=0) return -1;
        prc->right=prc->left+panel_cy;

        OffsetRect(prc,panel_cy*4,0);
    } break;
    case CTRL_LAUNCHWORK: {
        if(ScratchView_GetZoneRect(hwnd,CTRL_PANEL,prc)!=0) return -1;
        prc->right=prc->left+panel_cy;

        OffsetRect(prc,panel_cy*5,0);

    } break;
    case CTRL_SUSPENDWORK: {
        if(ScratchView_GetZoneRect(hwnd,CTRL_PANEL,prc)!=0) return -1;
        prc->right=prc->left+panel_cy;

        OffsetRect(prc,panel_cy*6,0);

    } break;
    case CTRL_SELRECT: {
        /*
         * mode==1时，选中的区域
         */
        RECT rcImg;
        if(ScratchView_GetZoneRect(hwnd,IMG_VIEW,&rcImg)!=0) return -1;
        if(psv->mode!=1||(psv->cx==0&&psv->cy==0)) return -1;

        RECT rcScreenImg;
        CopyRect(&rcScreenImg,&rcImg);
        OffsetRect(&rcScreenImg,-psv->offset.x,-psv->offset.y);

        RECT rcTag={
            (psv->cx>0?psv->pos.x:psv->pos.x+psv->cx),
            (psv->cy>0?psv->pos.y:psv->pos.y+psv->cy),
            (psv->cx>0?psv->pos.x:psv->pos.x+psv->cx)+abs(psv->cx),
            (psv->cy>0?psv->pos.y:psv->pos.y+psv->cy)+abs(psv->cy)
        };
        OffsetRect(&rcTag,rcScreenImg.left,rcScreenImg.top);
        CopyRect(prc,&rcTag);

    } break;
    case CTRL_SAVE: {
        RECT rcSel;
        if(ScratchView_GetZoneRect(hwnd,CTRL_SELRECT,&rcSel)!=0) return -1;
        SetRect(prc,rcSel.left,rcSel.top-26,rcSel.left+26,rcSel.top);
    } break;
    case CTRL_CANCEL: {
        if(ScratchView_GetZoneRect(hwnd,CTRL_SAVE,prc)!=0) return -1;
        OffsetRect(prc,26,0);
    } break;
    case CTRL_OCR: {
        if(ScratchView_GetZoneRect(hwnd,CTRL_CANCEL,prc)!=0) return -1;
        OffsetRect(prc,26,0);
    } break;
    case SAMPLES_VIEW: {
        if(psv->mode!=2) return -1;

        /* 
         * 默认返回第一行第一列的数据
         */
        int cell_x=viewcx/3;
        int cell_y=viewcy/3;
        int paddingcx=25,paddingcy=15;
        SetRect(prc,rc.left,rc.top,rc.left+cell_x,rc.top+cell_y);
    } break;
    }

    return 0;
}

int ScratchView_Paint(HWND hwnd,WPARAM wParam,LPARAM lParam) {
    pScratchView psv=ScratchView_GetSettings(hwnd);
    if(!psv) return 0;

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
    HBRUSH brushbk=CreateSolidBrush(RGB(20,20,20));
    FillRect(memdc,&rcMem,brushbk);
    DeleteObject(brushbk);

    
    Gdiplus::Graphics graphics(memdc);
    //wchar_t img_wchar[256]={0};
    //int wchar_len=MultiByteToWideChar(CP_ACP,0,psv->img_path,-1,NULL,0);
    //MultiByteToWideChar(CP_ACP,0,psv->img_path,-1,img_wchar,wchar_len);
    Gdiplus::Bitmap* img=psv->img;
    if(!img) {
        psv->pixels_x=psv->pixels_y=0;
    } else {//return 0;

        psv->pixels_x=img->GetWidth();
        psv->pixels_y=img->GetHeight();
    }
    if(psv->pixels_x<=0||psv->pixels_y<=0) {
        HICON iconWarning=LoadIcon(NULL,IDI_WARNING);
        ICONINFO iconInfo={0};
        BITMAP iconbmp;
        GetIconInfo(iconWarning,&iconInfo);
        GetObject(iconInfo.hbmMask,sizeof(BITMAP),&iconbmp);
        if(iconInfo.hbmMask) {

            DeleteObject(iconInfo.hbmColor);
        }
        DeleteObject(iconInfo.hbmMask);
        int iconHeight=iconbmp.bmHeight;
        int iconWidth=iconbmp.bmWidth;
        
        char error_msg[256]={0};
        sprintf(error_msg,"文件[%s]打开失败(%d)",psv->img_path,GetLastError());
        RECT rcErr,rcText,rcIcon;
        int errcx=450,errcy=80;
        POINT ptErr={(cx-errcx)>>1,(cy-errcy)>>1};
        SetRect(&rcErr,ptErr.x,ptErr.y,ptErr.x+errcx,ptErr.y+errcy);
        SetRect(&rcIcon,rcErr.left+15,((rcErr.bottom-rcErr.top-iconHeight)>>1)+rcErr.top,rcErr.left+15+iconWidth,((rcErr.bottom-rcErr.top-iconHeight)>>1)+rcErr.top+iconHeight);
        SetRect(&rcText,rcIcon.right+15,rcErr.top+25,rcErr.right-15,rcErr.bottom-15);

        HPEN penWarning=CreatePen(PS_SOLID,1,RGB(0,100,200));
        HBRUSH brushWarning=CreateSolidBrush(RGB(45,45,45));
        SetGraphicsMode(memdc,GM_ADVANCED);
        SetBkMode(memdc,TRANSPARENT);
        SelectObject(memdc,penWarning);
        SelectObject(memdc,brushWarning);
        Rectangle(memdc,rcErr.left,rcErr.top,rcErr.right,rcErr.bottom);
        DeleteObject(penWarning);
        DeleteObject(brushWarning);
        
        // HPEN penIcon=CreatePen(PS_SOLID,1,RGB(55,55,55));
        // HPEN preIconPen=(HPEN)SelectObject(memdc,penIcon);
        // HBRUSH brushIcon=CreateSolidBrush(RGB(55,55,55));
        // HBRUSH preIconBrush=(HBRUSH)SelectObject(memdc,brushIcon);
        // Ellipse(memdc,rcIcon.left,rcIcon.top,rcIcon.right,rcIcon.bottom);
        // SelectObject(memdc,preIconBrush);
        // DeleteObject(brushIcon);
        // SelectObject(memdc,preIconPen);
        // DeleteObject(penIcon);
        // Gdiplus::Graphics grahpics(memdc);
        // grahpics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
        // Gdiplus::SolidBrush brushIcon(Gdiplus::Color(255,255,0,0));
        // graphics.FillEllipse(&brushIcon,rcIcon.left,rcIcon.top,iconWidth,iconHeight);

        DrawIconEx(memdc,rcIcon.left,rcIcon.top,iconWarning,iconWidth,iconHeight,0,0,DI_NORMAL);
        SetTextColor(memdc,RGB(255,242,0));
        SetBkMode(memdc,TRANSPARENT);
        LOGFONT lf={0};
        GetObject(GetStockObject(SYSTEM_FONT),sizeof(LOGFONT),&lf);
        lf.lfHeight=18;
        lf.lfWeight=22;
        strcpy(lf.lfFaceName,"微软雅黑");
        HFONT font=CreateFontIndirect(&lf);
        HFONT prefont=(HFONT)SelectObject(memdc,font);
        DrawText(memdc,error_msg,-1,&rcText,DT_VCENTER|DT_LEFT|DT_WORDBREAK);
        SelectObject(memdc,prefont);
        DeleteObject(font);
        goto SCRATCHVIEWPAINT_EXIT;
    }

    RECT rcImg;
    ScratchView_GetZoneRect(hwnd,IMG_VIEW,&rcImg);
    /*
     * mode==1 原始尺寸
     */
    if(psv->mode==1) {
        /*
         * 1.根据图像的像素，当前视窗的大小 判断有无滚动条
         *   如果有，取offset信息 ， 计算图像视窗、滚动条视窗RECT
         * 2.绘制
         * 
         * 该控件需要一个 hittest函数, 对点击位置进行判断
         * 一个getzonerect 函数 ， 对模式位置进行定义
         */
        
        /*
         * 绘制图像
         */
        POINT pt={rcImg.left,rcImg.top};
        
        graphics.DrawImage(img,
            Gdiplus::Rect(rcImg.left,rcImg.top,rcImg.right-rcImg.left,rcImg.bottom-rcImg.top),
            psv->offset.x,psv->offset.y,
            rcImg.right-rcImg.left,rcImg.bottom-rcImg.top,
            Gdiplus::UnitPixel
        );

        if(psv->cx!=0&&psv->cy!=0) {
            /*
             * 坐标转换逻辑
             * 1. rcImg的坐标为屏幕坐标
             * 2. rcImg进行offset偏移，得到图像的屏幕坐标
             * 3. psv->pos 是 图像坐标
             * 处理思路如下
             * 1. OffsetRect(&rcImg,-psv->pos.x,-psv->pox.y); 
             *    得到图像的0坐标相对于屏幕的位置
             * 2. rcTag 记录的是相对于图像的位置，需要转换出其屏幕坐标
             *    第1步得到的 {rcImg.left,rcImg.top} 对应图像的 {0,0}
             *    故需要对rcTag 进行偏移
             *    OffsetRect(&rcTag,-rcImg.left,-rcImg.top);
             */
            // RECT rcScreenImg;
            // CopyRect(&rcScreenImg,&rcImg);
            // OffsetRect(&rcScreenImg,-psv->offset.x,-psv->offset.y);

            // RECT rcTag={
            //     (psv->cx>0?psv->pos.x:psv->pos.x+psv->cx),
            //     (psv->cy>0?psv->pos.y:psv->pos.y+psv->cy),
            //     (psv->cx>0?psv->pos.x:psv->pos.x+psv->cx)+abs(psv->cx),
            //     (psv->cy>0?psv->pos.y:psv->pos.y+psv->cy)+abs(psv->cy)
            // };
            // OffsetRect(&rcTag,rcScreenImg.left,rcScreenImg.top);
            RECT rcTag;
            if(ScratchView_GetZoneRect(hwnd,CTRL_SELRECT,&rcTag)==0) {
                HBRUSH brushtag=CreateSolidBrush(RGB(35,35,35));
                FrameRect(memdc,&rcTag,brushtag);
                RECT rcOps={rcTag.left,rcTag.top-26,rcTag.left+26*3,rcTag.top};
                FillRect(memdc,&rcOps,brushtag);
                DeleteObject(brushtag);
            }
            
            LOGFONT lf={0};
            GetObject(GetStockObject(SYSTEM_FONT),sizeof(LOGFONT),&lf);
            lf.lfHeight=18;
            lf.lfWeight=22;
            strcpy(lf.lfFaceName,"微软雅黑");
            HFONT font=CreateFontIndirect(&lf);
            HFONT prefont=(HFONT)SelectObject(memdc,font);
            SetTextColor(memdc,RGB(0,50,100));
            SetBkMode(memdc,TRANSPARENT);
            RECT rcTagText{rcImg.left,rcTag.bottom,rcTag.right,rcTag.bottom+25};
            char tagtext[256]={0};
            POINT pos={
                (psv->cx>0)?psv->pos.x:(psv->pos.x+psv->cx),
                (psv->cy>0)?psv->pos.y:(psv->pos.y+psv->cy)
            };
            sprintf(tagtext,"位置：(%d,%d) %d×%d",pos.x,pos.y,abs(psv->cx),abs(psv->cy));
            DrawText(memdc,tagtext,-1,&rcTagText,DT_VCENTER|DT_RIGHT);
            SelectObject(memdc,prefont);
            DeleteObject(font);

            HBRUSH brushCtrls=CreateSolidBrush(RGB(200,200,200));
            RECT rcSave,rcCancel,rcOCR;
            if(ScratchView_GetZoneRect(hwnd,CTRL_SAVE,&rcSave)==0) {
                if(PtInRect(&rcSave,psv->pt)) {
                    FillRect(memdc,&rcSave,brushCtrls);
                }

                InflateRect(&rcSave,-5,-5);
                HICON iSave=ImageList_GetIcon(psv->list,0,ILD_NORMAL);
                DrawIconEx(memdc,rcSave.left,rcSave.top,iSave,16,16,0,0,DI_NORMAL);
                DestroyIcon(iSave);
            }
            if(ScratchView_GetZoneRect(hwnd,CTRL_CANCEL,&rcCancel)==0) {
                if(PtInRect(&rcCancel,psv->pt)) {
                    FillRect(memdc,&rcCancel,brushCtrls);
                }
                InflateRect(&rcCancel,-5,-5);
                HICON iCancel=ImageList_GetIcon(psv->list,1,ILD_NORMAL);
                DrawIconEx(memdc,rcCancel.left,rcCancel.top,iCancel,16,16,0,0,DI_NORMAL);
                DestroyIcon(iCancel);
            }
            if(ScratchView_GetZoneRect(hwnd,CTRL_OCR,&rcOCR)==0) {
                if(PtInRect(&rcOCR,psv->pt)) {
                    FillRect(memdc,&rcOCR,brushCtrls);
                }
                InflateRect(&rcOCR,-5,-5);
                HICON iOCR=ImageList_GetIcon(psv->list,8,ILD_NORMAL);
                DrawIconEx(memdc,rcOCR.left,rcOCR.top,iOCR,16,16,0,0,DI_NORMAL);
                DestroyIcon(iOCR);
            }

            DeleteObject(brushCtrls);
        }
        
        /*
         * 绘制滚动条
         */
        //水平滚动条
        RECT rcHorz,rcHorzThumb;
        if(ScratchView_GetZoneRect(hwnd,ZHSCROLL,&rcHorz)==0) {
            ScratchView_GetZoneRect(hwnd,ZHSTHUMB,&rcHorzThumb);

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

        //垂直滚动条
        RECT rcVert,rcVertThumb;
        if(ScratchView_GetZoneRect(hwnd,ZVSCROLL,&rcVert)==0) {
            ScratchView_GetZoneRect(hwnd,ZVSTHUMB,&rcVertThumb);
            
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
    } else if(psv->mode==0) {
        /*
         * 绘制图像 ，图像可能会被压缩
         */
        graphics.DrawImage(img,Gdiplus::Rect(rcImg.left,rcImg.top,rcImg.right-rcImg.left,rcImg.bottom-rcImg.top));
    } else if(psv->mode==2) {
        /*
         * 图例模式
         * 在该案中，默认有9个图例
         * 默认将视图区域分割为 3×3的区域
         */
        int multi_count=0;
        for(int idx=0;idx<20&&page_name[idx][0]!='\0';idx++) multi_count++;
        
        RECT cell;
        ScratchView_GetZoneRect(hwnd,SAMPLES_VIEW,&cell);


        int cell_padding_cx=25,cell_padding_cy=15;
        for(int row=0;row<3;row++) {
            for(int col=0;col<3;col++) {
                int offsetx=col*(cell.right-cell.left);
                int offsety=row*(cell.bottom-cell.top);

                RECT cell_cur;
                CopyRect(&cell_cur,&cell);
                OffsetRect(&cell_cur,offsetx,offsety);
                InflateRect(&cell_cur,-cell_padding_cx,-cell_padding_cy);

                RECT rcCellImg,rcCellText;
                CopyRect(&rcCellImg,&cell_cur);
                rcCellImg.bottom-=25;
                CopyRect(&rcCellText,&cell_cur);
                rcCellText.top=rcCellImg.bottom;
                pTRZSmps smp=psv->smps+(row*3+col);
                if(smp->img_path[0]=='\0') {
                    
                } else {
                    

                }
            }
        }
    }

SCRATCHVIEWPAINT_EXIT:
    /*
     * 绘制 panel
     */
    RECT rcPanel;
    if(ScratchView_GetZoneRect(hwnd,CTRL_PANEL,&rcPanel)==0) {
        if(PtInRect(&rcPanel,psv->pt)) {
            Gdiplus::GraphicsPath path;
            int slashoffset=rcPanel.bottom-rcPanel.top;
            path.AddLine((int)rcPanel.left, (int)rcPanel.top,(int)rcPanel.left+slashoffset,(int)rcPanel.bottom);
            path.AddLine((int)rcPanel.left+slashoffset,(int)rcPanel.bottom,(int)rcPanel.right-slashoffset,(int)rcPanel.bottom);
            path.AddLine((int)rcPanel.right-slashoffset,(int)rcPanel.bottom,(int)rcPanel.right,(int)rcPanel.top);
            path.AddLine((int)rcPanel.right,(int)rcPanel.top,(int)rcPanel.left,(int)rcPanel.top);

            Gdiplus::LinearGradientBrush panelBrush(Gdiplus::Rect(rcPanel.left,rcPanel.top,rcPanel.right-rcPanel.left,rcPanel.bottom-rcPanel.top),
                Gdiplus::Color(255,55,55,55),
                Gdiplus::Color(255,35,35,35),
                LinearGradientModeVertical
            );
            graphics.FillPath(&panelBrush,&path);

            HBRUSH brushicon=CreateSolidBrush(RGB(10,10,10));
            RECT rcFile,rcExpend,rcCollapse,rcMultiMode,rcLaunch,rcSuspend;
            if(ScratchView_GetZoneRect(hwnd,CTRL_FILE,&rcFile)==0) {
                if(PtInRect(&rcFile,psv->pt)) {
                    FillRect(memdc,&rcFile,brushicon);
                }                    
                HICON icon=ImageList_GetIcon(psv->list,2,ILD_NORMAL);
                InflateRect(&rcFile,-5,-5);
                DrawIconEx(memdc,rcFile.left,rcFile.top,icon,24,24,0,0,DI_NORMAL);
                //DrawIconEx(memdc,rcFile.left,rcFile.top,icon);
                DestroyIcon(icon);
            }
            if(ScratchView_GetZoneRect(hwnd,CTRL_MODE_ORI,&rcExpend)==0) {
                if(PtInRect(&rcExpend,psv->pt)) {
                    FillRect(memdc,&rcExpend,brushicon);
                }
                HICON icon=ImageList_GetIcon(psv->list,3,ILD_NORMAL);
                InflateRect(&rcExpend,-5,-5);
                DrawIconEx(memdc,rcExpend.left,rcExpend.top,icon,24,24,0,0,DI_NORMAL);
                DestroyIcon(icon);
            }
            if(ScratchView_GetZoneRect(hwnd,CTRL_MODE_SCALE,&rcCollapse)==0) {
                if(PtInRect(&rcCollapse,psv->pt)) {
                    FillRect(memdc,&rcCollapse,brushicon);
                }
                HICON icon=ImageList_GetIcon(psv->list,4,ILD_NORMAL);
                InflateRect(&rcCollapse,-5,-5);
                DrawIconEx(memdc,rcCollapse.left,rcCollapse.top,icon,24,24,0,0,DI_NORMAL);
                DestroyIcon(icon);
            }
            if(ScratchView_GetZoneRect(hwnd,CTRL_CONFIG,&rcMultiMode)==0) {
                if(PtInRect(&rcMultiMode,psv->pt)) {
                    FillRect(memdc,&rcMultiMode,brushicon);
                }                    
                HICON icon=ImageList_GetIcon(psv->list,5,ILD_NORMAL);
                InflateRect(&rcMultiMode,-5,-5);
                DrawIconEx(memdc,rcMultiMode.left,rcMultiMode.top,icon,24,24,0,0,DI_NORMAL);
                DestroyIcon(icon);
            }
            if(ScratchView_GetZoneRect(hwnd,CTRL_LAUNCHWORK,&rcLaunch)==0) {
                if(PtInRect(&rcLaunch,psv->pt)) {
                    FillRect(memdc,&rcLaunch,brushicon);
                }                    
                HICON icon=ImageList_GetIcon(psv->list,6,ILD_NORMAL);
                InflateRect(&rcLaunch,-5,-5);
                DrawIconEx(memdc,rcLaunch.left,rcLaunch.top,icon,24,24,0,0,DI_NORMAL);
                DestroyIcon(icon);
            }
            if(ScratchView_GetZoneRect(hwnd,CTRL_SUSPENDWORK,&rcSuspend)==0) {
                if(PtInRect(&rcSuspend,psv->pt)) {
                    FillRect(memdc,&rcSuspend,brushicon);
                }                    
                HICON icon=ImageList_GetIcon(psv->list,7,ILD_NORMAL);
                InflateRect(&rcSuspend,-5,-5);
                DrawIconEx(memdc,rcSuspend.left,rcSuspend.top,icon,24,24,0,0,DI_NORMAL);
                DestroyIcon(icon);
            }

            DeleteObject(brushicon);
        }
    }

    BitBlt(hdc,0,0,cx,cy,memdc,0,0,SRCCOPY);
    SelectObject(memdc,prebmp);
    DeleteObject(bmp);
    DeleteDC(memdc);
    
    EndPaint(hwnd,&ps);
    return 0;
}