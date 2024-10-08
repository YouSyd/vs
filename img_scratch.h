#ifndef IMG_SCRATCH_HEADER
#define IMG_SCRATCH_HEADER

#include <stdio.h>
#include <stdlib.h>

#include <opencv2/opencv.hpp>

#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <gdiplus.h>

#include "tesseract_test.h"

#define CTRL_IMGSCRATCH "ImgScratch"
#define SCROLLBAR_PIXLS 13
#define THUMB_MIN_PIXLS 50
/*
 * 手机截图，用于定义选区 
 * 支持缩放，原大小滚动，文件可以设置默认打开文件，也可以手动指定打开问题
 * 显示模式: 全图、原尺寸，支持窗口内滚动
 * 支持点击选择区域，调整选区等
 */
typedef struct trz_proj {
    char projname[256];
    int width;
    int height;

    pTRZ trz_list;
    int trz_count;
} TRZProj,*pTRZProj;

typedef struct trz_samples{
    PAGECODE code;
    char img_path[256];
    char pagedesc[256];

    pTRZ trz_list;
    int trz_count;
}TRZSmps,*pTRZSmps;

typedef struct scratchview {
    HIMAGELIST list;
    /*
     * 滚动条 位移
     */
    POINT offset;

    /*
     * 当前鼠标位置
     */
    POINT pt;

    /* 
     * 选区起点，选区高宽 
     */
    POINT pos;
    int cx;
    int cy;

    /*
     * 图像
     * 是该记录图像的内存对象 gdiplus::Image 还是记录 文件名呢？
     */
    char img_path[256];
    Gdiplus::Bitmap* img;
    int pixels_x; /* 图像分辨率 */
    int pixels_y;
    
    /*
     * 显示模式 0-自适应窗口
     *         1-原始尺寸
     *         2-samples 选图界面
     */
    int mode;

    /*
     * 处理滚动条滚动,左键位置
     */
    POINT prept;

    /* 图例*/
    pTRZSmps smps;
    int smps_count;

    /* 输出或接口输入的依据*/
    pTRZProj proj;
} ScratchView,*pScratchView;

int ScratchView_InitialSettings(HWND hwnd);
pScratchView ScratchView_GetSettings(HWND hwnd);
int ScratchView_SetImage(HWND hwnd,char* img);
int ScratchView_ChangeMode(HWND hwnd,int mode);
void ScratchView_ClearSettings(HWND hwnd);

#define IMG_VIEW        20
#define CTRL_PANEL      21
#define CTRL_MODE_SCALE 22 /* 自适应缩略图 */
#define CTRL_MODE_ORI   23 /* 原始尺寸 */
#define CTRL_SELRECT    24 /* 设置选区 */
#define CTRL_FILE       25 /* 打开新的图片 */
#define CTRL_CONFIG     26 /* 打开选区配置 与选区保存按钮同操作*/
#define SAMPLES_VIEW    27 /* 多模板页面视图 mode==2*/
#define CTRL_SAVE       28 /* 选区的保留按钮*/
#define CTRL_CANCEL     29 /* 选区的撤销按钮*/
#define CTRL_OCR        32 /* 选区的OCR识别 */
#define CTRL_LAUNCHWORK 30 /* 启动工作流程*/
#define CTRL_SUSPENDWORK    31 /* 暂停工作流程*/
int ScratchView_GetZoneRect(HWND hwnd,int poscode,LPRECT prc);
int ScratchView_HitTest(HWND hwnd,POINT pt);

LRESULT CALLBACK ScratchViewProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
int ScratchView_Paint(HWND hwnd,WPARAM wParam,LPARAM lParam);

/*
 * 设定TRZ的消息
 * WPARAM pTRZ :指向需要修正坐标的TRZ
 * LPARAM TRZCODE :选区代码 
 */
#define MSG_SECTIONSETTINGS (UINT)(WM_USER+0x0010)


#endif
