/*
 * 该程序用于自动刷取抖音app的福袋与斗钻
 * 福袋与斗钻公用一个预览界面，即直播间初始界面，获取当前直播间人数、有无斗钻或福袋标志，剩余时间
 * 对于福袋直播间，点击福袋，进入福袋详细信息界面，并操作参与抽福袋
 * 程序等待剩余时间，进入福袋抽奖结束界面，确定是否中奖，分为 中奖 和 未中奖 两个界面
 *     未中奖，点击×，回到预览界面，或者 直播结束 界面
 *     中奖，点击领奖等，切换直播间，回到预览界面。
 * 对于斗钻直播间，点击斗钻，进入斗钻详细信息界面，并操作参与抽斗钻
 * 
 * 截图类型：
 * 1、公共预览界面
 * 2、福袋详细信息界面
 * 3、福袋结算界面-未中奖
 * 4、福袋结算界面-中奖
 * 5、斗钻详细信息界面
 * 6、斗钻结算界面-未中
 * 7、斗钻结算界面-中
 * 8、直播结束界面
 *
 * 状态机
 * ------------------------------------------------------                 
 *
 *           +------+--<<---[3]----<<---+
 *           |      |                   |                          
 *           ↓      ↓                   |                      
 * ---->> --[8]--- [1]>+----[2]---->>-->+-----[]
 *           ↑      ↑                   |                          
 *           |      |                   |     
 *           +------+--<<---[4]----<<---+
 *
 *
 *           +------+--<<---[6]----<<---+
 *           |      |                   |                          
 *           ↓      ↓                   |                      
 * ---->> --[8]--- [1]>+----[5]---->>-->+-----[]
 *           ↑      ↑                   |                          
 *           |      |                   |     
 *           +------+--<<---[7]----<<---+
 *
 * Actions:
 * ------------------------------------------------------         
 * 1--->2 : 点击福袋，等待5s，设置定时器，到时候自动截屏分析，确定是由状态2切换到何种状态
 * 2--->3 : 自动弹出
 * 2--->4 : 自动弹出
 * 2--->8 : 自动弹出
 * 3--->1 : 点击屏幕非弹框区
 * 4--->1 : 点击领奖，切换到交易界面，点击回到直播间（这里漏了一个交易界面状态）
 * 8--->1 : 进入其它直播间
 *
 */

#ifndef TESSERACT_TEST_HEADER
#define TESSERACT_TEST_HEADER

#include <stdio.h>
#include <stdlib.h>

#include <opencv2/opencv.hpp>

#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

#include <windows.h>

#include "exec_cmd.h"

#pragma comment(lib,"opencv_calib3d4.lib")
#pragma comment(lib,"opencv_core4.lib")
#pragma comment(lib,"opencv_dnn4.lib")
#pragma comment(lib,"opencv_features2d4.lib")
#pragma comment(lib,"opencv_flann4.lib")
#pragma comment(lib,"opencv_highgui4.lib")
#pragma comment(lib,"opencv_imgcodecs4.lib")
#pragma comment(lib,"opencv_imgproc4.lib")
#pragma comment(lib,"opencv_ml4.lib")
#pragma comment(lib,"opencv_objdetect4.lib")
#pragma comment(lib,"opencv_photo4.lib")
#pragma comment(lib,"opencv_stitching4.lib")
#pragma comment(lib,"opencv_video4.lib")
#pragma comment(lib,"opencv_videoio4.lib")

#pragma comment(lib,"tesseract54.lib")
#pragma comment(lib,"leptonica-1.84.1.lib")

#define ENUM_NAMES(name,desc) enum_names(name,desc,#name)
#define ENUMNAMES_SECTION ENUM_NAMES(STREAMER_ROOM_IMG,"直播间标志搜索区域") \
ENUM_NAMES(STREAMER,"主播") \
ENUM_NAMES(THUMBUP_OR_STREAMER_DESC,"点赞描述") \
ENUM_NAMES(ALIVECOUNT,"在线人数") \
ENUM_NAMES(FORTUNEGIFT_OR_DIAMOND_RANGE,"福袋的搜索范围") \
ENUM_NAMES(DOU_DIAMOND_DESC,"斗钻参与人数、价值") \
ENUM_NAMES(DOU_DIAMOND_COUNTDOWN,"斗钻倒计时")\
ENUM_NAMES(DOU_FORTUNEGIFT_COUNTDOWN,"福袋倒计时")\
ENUM_NAMES(FORTUNEGIFT_CLICKRECT,"福袋区域")\
ENUM_NAMES(DIAMOND_CLICKRECT,"抖钻区域")\
\
ENUM_NAMES(DOU_FORTUNEGIFT_DETAIL_IMG,"福袋详情页面识别标志")\
ENUM_NAMES(DOU_FORTUNEGIFT_COUNTDOWN_DETAIL,"详情页面倒计时(分)")\
ENUM_NAMES(DOU_FORTUNEGIFT_COUNTDOWN_DETAIL2,"详情页面倒计时(秒)")\
ENUM_NAMES(DOU_FORTUNEGIFT_DETAIL_TEXT,"详情描述")\
ENUM_NAMES(DOU_FORTUNEGIFT_DETAIL_TASK1,"任务描述")\
ENUM_NAMES(DOU_FORTUNEGIFT_DETAIL_TASK2,"任务描述2")\
ENUM_NAMES(DOU_FORTUNEGIFT_DETAIL_EXECTASK,"任务执行按钮")\
\
ENUM_NAMES(DOU_DIAMOND_DETAIL_IMG,"斗钻详情页面识别标志")\
\
ENUM_NAMES(DOU_FORTUNEGIFT_DRAW_NOPRIZE_IMG,"福袋未中奖页面识别标志")\
ENUM_NAMES(DOU_FORTUNEGIFT_DRAW_NOPRIZE_EXIT,"福袋未中奖页面退出按钮")\
\
ENUM_NAMES(DOU_FORTUNEGIFT_DRAW_JACKPOT_IMG,"福袋中奖页面标志")\
ENUM_NAMES(DOU_FORTUNEGIFT_DRAW_JACKPOT_PRIZEDESC,"福袋中奖页面-中奖内容描述")\
ENUM_NAMES(DOU_FORTUNEGIFT_DRAW_JACKPOT_CFMAWARD,"福袋中奖页面-领奖按钮")\
ENUM_NAMES(DOU_FORTUNEGIFT_DRAW_JACKPOT_AFMPROTO,"福袋中奖页面-check协议")\
\
ENUM_NAMES(STREAMER_ROOMCLOSED_IMG,"直播间关闭标志搜索区域")

typedef enum {
    // STREAMER_ROOM_IMG,//直播间标志搜索区域
    // STREAMER,//主播
    // THUMBUP_OR_STREAMER_DESC,//点赞描述
    // ALIVECOUNT,//在线人数
    // FORTUNEGIFT_OR_DIAMOND_RANGE,//福袋的搜索范围
    // DOU_DIAMOND_DESC,//斗钻参与人数、价值
    // DOU_DIAMOND_COUNTDOWN,//斗钻倒计时
    // DOU_FORTUNEGIFT_COUNTDOWN,//福袋倒计时
    // FORTUNEGIFT_CLICKRECT,
    // DIAMOND_CLICKRECT,
    
    // DOU_FORTUNEGIFT_DETAIL_IMG,//详情页面识别标志
    // DOU_FORTUNEGIFT_COUNTDOWN_DETAIL,//详情页面倒计时
    // DOU_FORTUNEGIFT_COUNTDOWN_DETAIL2,//详情页面倒计时
    // DOU_FORTUNEGIFT_DETAIL_TEXT,//详情描述
    // DOU_FORTUNEGIFT_DETAIL_TASK1, //福袋任务1
    // DOU_FORTUNEGIFT_DETAIL_TASK2,//福袋任务2
    // DOU_FORTUNEGIFT_DETAIL_EXECTASK, //执行福袋任务1
    
    // DOU_DIAMOND_DETAIL_IMG,//斗钻详情页面识别标志
    
    // DOU_FORTUNEGIFT_DRAW_NOPRIZE_IMG,//福袋未中奖页面识别标志
    // DOU_FORTUNEGIFT_DRAW_NOPRIZE_EXIT,//福袋未中奖页面退出按钮
    
    // DOU_FORTUNEGIFT_DRAW_JACKPOT_IMG,//福袋中奖页面标志
    // DOU_FORTUNEGIFT_DRAW_JACKPOT_PRIZEDESC,//福袋中奖页面-中奖内容描述
    // DOU_FORTUNEGIFT_DRAW_JACKPOT_CFMAWARD,//福袋中奖页面-领奖按钮
    // DOU_FORTUNEGIFT_DRAW_JACKPOT_AFMPROTO,//福袋中奖页面-check协议
    
    // STREAMER_ROOMCLOSED_IMG,//直播间关闭标志搜索区域
#define enum_names(enum_code,enum_desc,enum_name) enum_code,
    ENUMNAMES_SECTION
#undef enum_names
    
}TRZCODE;
extern char section_name[100][256];

#define ENUMNAMES_PAGE ENUM_NAMES(UNKNOWN_PAGE,"未知页面") \
ENUM_NAMES(STREAMER_ROOM,"直播间主界面") \
ENUM_NAMES(FORTUNEGIFT_DETAIL,"福袋详情页面") \
ENUM_NAMES(FORTUNEGIFT_DETAIL_2TASK,"福袋详情页面(两步任务)") \
ENUM_NAMES(FORTUNEGIFT_NOPRIZE,"福袋未中奖界面") \
ENUM_NAMES(FORTUNEGIFT_JACKPOT,"福袋中奖界面") \
ENUM_NAMES(DIAMOND_DETAIL,"斗钻详情页面") \
ENUM_NAMES(DIAMOND_NOPRICE,"斗钻未中奖界面") \
ENUM_NAMES(DIAMOND_JACKPOT,"斗钻中奖界面") \
ENUM_NAMES(STREAMER_ROOMCLOSED,"直播间关闭界面")

typedef enum {
    // UNKNOWN_PAGE=-1,//未知页面
    // STREAMER_ROOM,//直播间主界面
    // FORTUNEGIFT_DETAIL,//福袋信息
    // FORTUNEGIFT_DETAIL_2TASK,//福袋信息，两步任务
    // FORTUNEGIFT_NOPRIZE,//福袋未中奖界面
    // FORTUNEGIFT_JACKPOT,//福袋中奖界面
    // DIAMOND_DETAIL,//斗钻信息
    // DIAMOND_NOPRICE,//斗钻未中奖界面
    // DIAMOND_JACKPOT,//斗钻中奖界面
    // STREAMER_ROOMCLOSED,//直播间关闭界面
#define enum_names(enum_code,enum_desc,enum_name) enum_code,
    ENUMNAMES_PAGE
#undef enum_names

}PAGECODE;
extern char page_name[20][256];

typedef struct text_rect_zone {
    int x;
    int y;
    int width;
    int height;
    TRZCODE code;
    
    char* pText;
    
    PAGECODE page;
    char traindata[256];//traindata字库，默认为 chi_sim
    char whitelist[256];// tessedit_char_whitelist 参数
    
} TRZ,*pTRZ;

typedef struct major {
    pTRZ trz;//list
    
    int hasfg;//福袋标记
    int alivecount;//在线人数
    int timestamp;//获取当前倒计时的时间戳
    int fg_countdown;//福袋倒计时剩余秒数
    char* fg_desc;//福袋描述
    PAGECODE currpage;//当前页面
    
    int taskcount;
    int taskstatus[3];
    
}Major,*pMajor;

extern pMajor GMajor;
extern TRZ sections[];

int TextRectOCR(tesseract::TessBaseAPI* api,int x,int y,int width,int height,char** pText);
int ClipTRZ(Pix* pix,pTRZ trz);
int ClipTRZ(Pix* pix,pTRZ trz,char* clip_file);
int TextTRZ(tesseract::TessBaseAPI* api,Pix* pix,pTRZ trz,int clip_flag);
int TextTRZ(tesseract::TessBaseAPI* api,Pix* pix,pTRZ trz);
int InitialSections(tesseract::TessBaseAPI* api,Pix* pix,pTRZ trz);
void ReleaseSections(pTRZ trz);

PAGECODE GetPageCode(tesseract::TessBaseAPI* api,const char* source_img,pTRZ list);
PAGECODE GetPageCode(const char* source_img,pTRZ list);

int searchtemplate(const char* source_img,TRZCODE code,pTRZ list);
pTRZ gettrzbycode(pTRZ list,TRZCODE code);

int getehimagename(const char* img_name,char* eh_name);

int ImgEnhanceBeforeOCR(const char* img_file,char* eh_img_file);
int TextTRZEh(tesseract::TessBaseAPI* api,const char* img_file,pTRZ trz);

int streamer_room_ocr(tesseract::TessBaseAPI* api,pTRZ trz,char* img_file);
int fg_detail_ocr(tesseract::TessBaseAPI* api,pTRZ trz,char* img_file);
    
/*********************************************************/
// test code.
/*
 * PageAnalysis_Test:测试页面识别
 */    
void PageAnalysis_Test();
//...

/*********************************************************/

int NextPageStrategies_TRACEONE(pTRZ trz,PAGECODE currpage);
PAGECODE PageAnalysis(tesseract::TessBaseAPI* api,pTRZ trz);
void Flows();

int GetModulePath(char* path);

DWORD WINAPI DouYinOCRJOB(LPVOID lParam);

#endif
