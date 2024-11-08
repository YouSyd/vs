#include "tesseract_test.h"
#include "workflow_proj.h"
#include "workflow_ctrl.h"
#include "img_scratch.h"

char datapath[256]="E:\\SRC_REF\\tesseract\\self-bulid\\test\\tessdata";

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
char section_name[100][256]={
#define enum_names(enum_code,enum_desc,enum_name) \
enum_desc "(" enum_name ")" ,
//TOSTRING(enum_code) "：" enum_desc "(" enum_name ")" ,
    ENUMNAMES_SECTION
#undef enum_names
};

char page_name[20][256]={
#define enum_names(enum_code,enum_desc,enum_name) \
enum_desc "(" enum_name ")" ,
//TOSTRING(enum_code) "：" enum_desc "(" enum_name ")" ,
    ENUMNAMES_PAGE
#undef enum_names
};

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
TRZ sections[]={
    //直播间主页面标志图（识别页面）
    {
        38,1432,695-38,1500-1432,STREAMER_ROOM_IMG,NULL,STREAMER_ROOM,
    },
    {
        97,72,136,33,STREAMER,NULL,STREAMER_ROOM,
        //97,72,60,33,STREAMER,NULL
    },
    {
        97,107,136,18,THUMBUP_OR_STREAMER_DESC,NULL,STREAMER_ROOM,
    },
    {
        573,83,50,30,ALIVECOUNT,NULL,STREAMER_ROOM,"eng","0123456789",
    },
    {
        20,210,300,70,FORTUNEGIFT_OR_DIAMOND_RANGE,NULL,STREAMER_ROOM,
    },
    //以下均需要在运行中计算
    //---------------------------------------
    //福袋点击区域
    {
        0,0,0,0,FORTUNEGIFT_CLICKRECT,NULL,STREAMER_ROOM,
    },
    //斗钻点击区域
    {
        0,0,0,0,DIAMOND_CLICKRECT,NULL,STREAMER_ROOM,
    },
    //斗钻价值描述区域 这个不一定有，可能程序中会被取消
    {
        0,0,0,0,DOU_DIAMOND_DESC,NULL,STREAMER_ROOM,
    },
    //斗钻倒计时
    {
        18,47,45,18,DOU_DIAMOND_COUNTDOWN,NULL,STREAMER_ROOM,"eng","0123456789:",
    },
    //福袋倒计时
    {
        18,47,45,18,DOU_FORTUNEGIFT_COUNTDOWN,NULL,STREAMER_ROOM,"eng","0123456789:",
    },
    //-------------------------------------------
    //福袋详情页面-详情页标志页图（识别页面）
    {
        230,700,510-230,120,DOU_FORTUNEGIFT_DETAIL_IMG,NULL,FORTUNEGIFT_DETAIL,
    }, 
    //福袋详情页面-倒计时
    {
        277,785,310-277,815-785,DOU_FORTUNEGIFT_COUNTDOWN_DETAIL,NULL,FORTUNEGIFT_DETAIL,"eng","0123456789",
    }, 
    //福袋详情页面-倒计时2
    {
        333,785,366-333,815-785,DOU_FORTUNEGIFT_COUNTDOWN_DETAIL2,NULL,FORTUNEGIFT_DETAIL,"eng","0123456789",
    },
    //福袋详情页面-福袋描述
    {
        260,865,670-260,1020-865,DOU_FORTUNEGIFT_DETAIL_TEXT,NULL,FORTUNEGIFT_DETAIL,
    },
    {
        31,1117,657,194,DOU_FORTUNEGIFT_TASKRECT,NULL,FORTUNEGIFT_DETAIL,
    },
    //福袋详情页面-任务描述 1
    {
        50,1205,670-50,1245-1205,DOU_FORTUNEGIFT_DETAIL_TASK1,NULL,FORTUNEGIFT_DETAIL,
    },
    //福袋详情页面-任务描述 2
    {
        50,1250,670-50,1280-1250,DOU_FORTUNEGIFT_DETAIL_TASK2,NULL,FORTUNEGIFT_DETAIL,
    },
    //福袋详情页面- 参与任务
    {
        50,1370,610,70,DOU_FORTUNEGIFT_DETAIL_EXECTASK,NULL,FORTUNEGIFT_DETAIL,
    },
    
    //----------------------------------------------------
    //斗钻详情页面
    { 
        30,800,660,100,DOU_DIAMOND_DETAIL_IMG,NULL,DIAMOND_DETAIL,
    },
    
        
    //----------------------------------------------------
    //福袋抽奖结果界面-未中奖
    {
        115,430,500,80,DOU_FORTUNEGIFT_DRAW_NOPRIZE_IMG,NULL,FORTUNEGIFT_NOPRIZE,
    },
    {
        135,815,440,60,DOU_FORTUNEGIFT_DRAW_NOPRIZE_EXIT,NULL,FORTUNEGIFT_NOPRIZE,
    },
    
    
    //----------------------------------------------------
    //福袋抽奖结果界面-中奖 
    // 福袋中奖页面 只有 DOU_FORTUNEGIFT_DRAW_JACKPOT_POPUP 、DOU_FORTUNEGIFT_DRAW_JACKPOT_CLOSEBTN 是可用坐标
    // 其余坐标需要推算
    {
        115,430,500,80,DOU_FORTUNEGIFT_DRAW_JACKPOT_IMG,NULL,FORTUNEGIFT_JACKPOT,
    },
    {
        110,497,505,103,DOU_FORTUNEGIFT_DRAW_JACKPOT_PRIZEDESC,NULL,FORTUNEGIFT_JACKPOT,
    },
    {
        110,820,500,100,DOU_FORTUNEGIFT_DRAW_JACKPOT_CFMAWARD,NULL,FORTUNEGIFT_JACKPOT,
    },
    {
        139,942,50,44,DOU_FORTUNEGIFT_DRAW_JACKPOT_AFMPROTO,NULL,FORTUNEGIFT_JACKPOT,
    },
    {
        139,942,50,44,DOU_FORTUNEGIFT_DRAW_JACKPOT_AFMPROTO_CHECKED,NULL,FORTUNEGIFT_JACKPOT,
    },
    {
        338,1069,44,42,DOU_FORTUNEGIFT_DRAW_JACKPOT_CLOSEBTN,NULL,FORTUNEGIFT_JACKPOT,
    },
    {
        74,395,573,637,DOU_FORTUNEGIFT_DRAW_JACKPOT_POPUP,NULL,FORTUNEGIFT_JACKPOT,
    },

    //----------------------------------------------------
    //直播间 已经关闭
    {
        254,120,213,49,STREAMER_ROOMCLOSED_IMG,NULL,STREAMER_ROOMCLOSED,
    },

    //----------------------------------------------------
    //福袋中奖后领奖界面
    {
        2,48,718,109,DOU_FORTUNEGIFT_AWARDFORTUNEGIFT_IMG,NULL,FORTUNEGIFT_AFMAWARD,
    },
    {
        452,1410,237,83,DOU_FORTUNEGIFT_AWARDFORTUNEGIFT_AFMBTN,NULL,FORTUNEGIFT_AFMAWARD,
    },
    //----------------------------------------------------
    // fixed new trz sections.
    {
        996,145,35,35,STREAMERROOM_QUIT,NULL,STREAMER_ROOM,
    },
};

const int sections_counts=sizeof(sections)/sizeof(TRZ);

pMajor GMajor;
    
/*
 * streamer_room_ocr: 
 * 用途：识别直播间画面要素
 */
int streamer_room_ocr(tesseract::TessBaseAPI* api,pTRZ trz,char* img_file) {
//int test(char* img_file) {
    char* outputText=NULL;
    
    Pix* img=pixRead(img_file);
    InitialSections(api,img,trz);

    char fortunegift_clip_img[256]={0};
    char eh_img_name[256]={0};
    char path[256]={0};
    GetModulePath(path);
    sprintf(fortunegift_clip_img,"%spics\\%d.png",path,(int)FORTUNEGIFT_OR_DIAMOND_RANGE);
    ImgEhanceBeforeOCR(fortunegift_clip_img,eh_img_name);
    
    int has_diamond=(searchtemplate(fortunegift_clip_img,DIAMOND_CLICKRECT,trz)==0?1:0);
    int has_fortunegift=(searchtemplate(fortunegift_clip_img,FORTUNEGIFT_CLICKRECT,trz)==0?1:0);
    GMajor->hasfg=has_fortunegift;
    
    //计算DOU_FORTUNEGIFT_COUNTDOWN
    if(has_fortunegift==1) {
        //fortunegift_countdown
        pTRZ trz_fc=gettrzbycode(trz,DOU_FORTUNEGIFT_COUNTDOWN);
        if(trz_fc==NULL) {
            printf("Something was wrong (DOU_FORTUNEGIFT_COUNTDOWN)...\n");
        }
        
        //FORTUNEGIFT_OR_DIAMOND_RANGE
        pTRZ trz_fodr=gettrzbycode(trz,FORTUNEGIFT_OR_DIAMOND_RANGE);
        if(trz_fodr==NULL) {
            printf("Something was wrong (FORTUNEGIFT_OR_DIAMOND_RANGE)...\n");
        }
        
        //裁剪出 DOU_FORTUNEGIFT_COUNTDOWN 
        trz_fc->x=trz_fodr->x+trz_fc->x;
        trz_fc->y=trz_fodr->y+trz_fc->y;
        TextTRZ(api,img,trz_fc);
        trz_fc->x-=trz_fodr->x;
        trz_fc->y-=trz_fodr->y;
    }
    
    if(has_diamond==1) {
        //DOU_DIAMOND_COUNTDOWN
        pTRZ trz_dc=gettrzbycode(trz,DOU_DIAMOND_COUNTDOWN);
        if(trz_dc==NULL) {
            printf("Something was wrong (DOU_DIAMOND_COUNTDOWN)...\n");
        }
        
        //FORTUNEGIFT_OR_DIAMOND_RANGE
        pTRZ trz_fodr=gettrzbycode(trz,FORTUNEGIFT_OR_DIAMOND_RANGE);
        if(trz_fodr==NULL) {
            printf("Something was wrong (FORTUNEGIFT_OR_DIAMOND_RANGE)...\n");
        }
        
        //裁剪出 DOU_DIAMOND_COUNTDOWN 
        trz_dc->x=trz_fodr->x+trz_dc->x;
        trz_dc->y=trz_fodr->y+trz_dc->y;
        TextTRZ(api,img,trz_dc);
        trz_dc->x-=trz_fodr->x;
        trz_dc->y-=trz_fodr->y;
    }
    
    pTRZ trz_streamer=gettrzbycode(trz,STREAMER);
    pTRZ trz_ac=gettrzbycode(trz,ALIVECOUNT);
    pTRZ trz_fc=gettrzbycode(trz,DOU_FORTUNEGIFT_COUNTDOWN);
    int alivecount=0;
    if(1!=sscanf(trz_ac->pText,"%d",&alivecount)) {
        printf("直播间人数转换出现问题[%s]\n",trz_ac->pText);
    }
    GMajor->alivecount=alivecount;
    printf("【%s】的直播间，目前有 %s 人，该直播间福袋:[%s],抖钻[%s]",trz_streamer->pText,trz_ac->pText,
        (has_fortunegift==1)?"√":"×",
        (has_diamond==1)?"√":"×");
    
    if(has_fortunegift==1) printf("，福袋开启剩余时间【%s】\n",trz_fc->pText);

    pixDestroy(&img);
    
    return 0;
}

/*
 * fg_detail_ocr
 * 用途：识别超级福袋详情页面要素 (一步任务)
 */
int fg_detail_ocr(tesseract::TessBaseAPI* api,pTRZ trz,char* img_file) {
    int result=-1;
    Pix* img=pixRead(img_file);
    
    //1.识别倒计时
    pTRZ trz_cd1=gettrzbycode(trz,DOU_FORTUNEGIFT_COUNTDOWN_DETAIL);
    pTRZ trz_cd2=gettrzbycode(trz,DOU_FORTUNEGIFT_COUNTDOWN_DETAIL2);
    if(!trz_cd1||!trz_cd2) {
        printf("福袋详情-识别福袋详情页面出错：未能取到倒计时\n");
        goto fg_detail_ocr_exit;
    }
    /*
     * 此处有猫腻
     * --------------------------------------
     * 如何编写通用的倒计时识别？
     * 
     * come on
     * 最好能识别出弹框
     * 继而识别出弹框中的三个大矩形
     * 1、福袋物品框
     * 2、任务框
     * 3、点击按钮
     * 
     * --------------------------------------
     * 任务框中的内容很重要，两个未“达成”的任务
     * 
     * 
     * 
     * 
     * 
     * 
     */
    //trz_cd1->y+=48;trz_cd2->y+=48;
    TextTRZ(api,img,trz_cd1);
    TextTRZ(api,img,trz_cd2);
    //trz_cd1->y-=48;trz_cd2->y-=48;
    printf("倒计时[%s:%s]\n",trz_cd1->pText,trz_cd2->pText);
    
    int minutes=0,seconds=0;
    if(1!=sscanf(trz_cd1->pText,"%d",&minutes)) {
        printf("福袋详情-倒计时分钟%s 转换出错\n",trz_cd1->pText);
        goto fg_detail_ocr_exit;
    }
    if(1!=sscanf(trz_cd2->pText,"%d",&seconds)) {
        printf("福袋详情-倒计时秒数%s 转换出错\n",trz_cd2->pText);
        goto fg_detail_ocr_exit;
    }
    
    GMajor->timestamp=time(NULL);
    GMajor->fg_countdown=minutes*60+seconds;
    //识别福袋内容 DOU_FORTUNEGIFT_DETAIL_TEXT
    pTRZ trz_fdt=gettrzbycode(trz,DOU_FORTUNEGIFT_DETAIL_TEXT);
    if(!trz_fdt) {
        printf("福袋详情-识别福袋详情页面出错：未能取到福袋内容文本\n");
        goto fg_detail_ocr_exit;
    }
    //trz_fdt->y+=48;
    TextTRZ(api,img,trz_fdt);
    //trz_fdt->y-=48;
    printf("福袋内容[%s]\n",trz_fdt->pText);
    GMajor->fg_desc=trz_fdt->pText;
    
    //两步任务？一步任务？
    //... 没想好咋做
    //初步思路如下：
    //扫描图片，检测任务类型，一步？两步？执行了几步？当前状态？
    //收集完信息后，外面做决策
    /* 
     * 经过图像对比发现，一个任务和两个任务的图略有区别，(超级福袋)
     * 的起始位置不同
     * 先找到 其位置，以此为基准，判断是一步任务、两步任务
     * 在本机 720 × 1600 ， 两部任务比一步任务 y 小 48
     *
     */

    /*
     * 上述方案无法通用，简单依靠像素差可能存在问题。
     * 现在计划将方案升级 
     */
    
    
    // pTRZ trz_tk1=gettrzbycode(trz,DOU_FORTUNEGIFT_DETAIL_TASK1);
    // if(!trz_tk1) {
    //     printf("福袋详情-识别福袋详情页面出错：未能取到任务1区域\n");
    //     goto fg_detail_ocr_exit;
    // }
    pTRZ trz_tk=gettrzbycode(trz,DOU_FORTUNEGIFT_TASKRECT);
    if(!trz_tk) {
        printf("福袋详情-识别福袋详情页面出错：未能取到任务描述区域\n");
        goto fg_detail_ocr_exit;
    }
    //trz_tk1->y+=48;
    //TextTRZ(api,img,trz_tk1);
    TextTRZEh(api,img_file,trz_tk);//使用处理后的图片提高识别准确率
    //trz_tk1->y-=48;
    if(trz_tk->pText) {
        printf("任务1[%s]\n",trz_tk->pText);
        GMajor->taskstatus[0]=((strstr(trz_tk->pText,"已达成"))?1:0);    
    } else {
        GMajor->taskstatus[0]=0;
    }

    //检查执行任务文本
    pTRZ trz_et=gettrzbycode(trz,DOU_FORTUNEGIFT_DETAIL_EXECTASK);
    if(!trz_et) {
        printf("福袋详情-识别福袋详情页面出错：未能取到执行任务按钮的文本\n");
        goto fg_detail_ocr_exit;
    }
    TextTRZ(api,img,trz_et);
    printf("执行任务按钮的文本[%s]\n",trz_et->pText);
    
    result=0;
fg_detail_ocr_exit:    
    pixDestroy(&img);
    return result;
}

/*
 * fg_detail_2task_ocr
 * 用途：识别超级福袋详情页面要素(两步任务)
 */
int fg_detail_2task_ocr(tesseract::TessBaseAPI* api,pTRZ trz,char* img_file) {
    int result=-1;
    Pix* img=pixRead(img_file);
    
    //1.识别倒计时
    pTRZ trz_cd1=gettrzbycode(trz,DOU_FORTUNEGIFT_COUNTDOWN_DETAIL);
    pTRZ trz_cd2=gettrzbycode(trz,DOU_FORTUNEGIFT_COUNTDOWN_DETAIL2);
    if(!trz_cd1||!trz_cd2) {
        printf("福袋详情-识别福袋详情页面出错：未能取到倒计时\n");
        goto fg_detail_2task_ocr_exit;
    }
    TextTRZ(api,img,trz_cd1);
    TextTRZ(api,img,trz_cd2);
    printf("倒计时[%s:%s]\n",trz_cd1->pText,trz_cd2->pText);
    
    int minutes=0,seconds=0;
    if(1!=sscanf(trz_cd1->pText,"%d",&minutes)) {
        printf("福袋详情-倒计时分钟%s 转换出错\n",trz_cd1->pText);
        goto fg_detail_2task_ocr_exit;
    }
    if(1!=sscanf(trz_cd2->pText,"%d",&seconds)) {
        printf("福袋详情-倒计时秒数%s 转换出错\n",trz_cd2->pText);
        goto fg_detail_2task_ocr_exit;
    }
    GMajor->timestamp=time(NULL);
    GMajor->fg_countdown=minutes*60+seconds;
    
    //识别福袋内容 DOU_FORTUNEGIFT_DETAIL_TEXT
    pTRZ trz_fdt=gettrzbycode(trz,DOU_FORTUNEGIFT_DETAIL_TEXT);
    if(!trz_fdt) {
        printf("福袋详情-识别福袋详情页面出错：未能取到福袋内容文本\n");
        goto fg_detail_2task_ocr_exit;
    }
    TextTRZ(api,img,trz_fdt);
    GMajor->fg_desc=trz_fdt->pText;
    printf("福袋内容[%s]\n",trz_fdt->pText);
    
    //两步任务？一步任务？
    //... 没想好咋做
    //初步思路如下：
    //扫描图片，检测任务类型，一步？两步？执行了几步？当前状态？
    //收集完信息后，外面做决策
    /* 
     * 经过图像对比发现，一个任务和两个任务的图略有区别，(超级福袋)
     * 的起始位置不同
     * 先找到 其位置，以此为基准，判断是一步任务、两步任务
     * 在本机 720 × 1600 ， 两部任务比一步任务 y 小 48
     *
     */
    
    
    // pTRZ trz_tk1=gettrzbycode(trz,DOU_FORTUNEGIFT_DETAIL_TASK1);
    // if(!trz_tk1) {
    //     printf("福袋详情-识别福袋详情页面出错：未能取到任务1区域\n");
    //     goto fg_detail_2task_ocr_exit;
    // }
    // pTRZ trz_tk2=gettrzbycode(trz,DOU_FORTUNEGIFT_DETAIL_TASK2);
    // if(!trz_tk2) {
    //     printf("福袋详情-识别福袋详情页面出错：未能取到任务2区域\n");
    //     goto fg_detail_2task_ocr_exit;
    // }
    // //TextTRZ(api,img,trz_tk1);
    // TextTRZEh(api,img_file,trz_tk1);//使用处理后的图片提高识别准确率
    // if(trz_tk1->pText) printf("任务1[%s]\n",trz_tk1->pText);
    // //TextTRZ(api,img,trz_tk2);
    // TextTRZEh(api,img_file,trz_tk2);//使用处理后的图片提高识别准确率
    // if(trz_tk2->pText) printf("任务2[%s]\n",trz_tk2->pText);
    // GMajor->taskstatus[0]=((strstr(trz_tk1->pText,"已达成"))?1:0);    
    // GMajor->taskstatus[1]=((strstr(trz_tk2->pText,"已达成"))?1:0);
    pTRZ trz_tk=gettrzbycode(trz,DOU_FORTUNEGIFT_TASKRECT);
    if(!trz_tk) {
        printf("福袋详情-识别福袋详情页面出错：未能取到任务描述区域\n");
        goto fg_detail_2task_ocr_exit;
    }
    if(!trz_tk->pText) {
        GMajor->taskstatus[0]=GMajor->taskstatus[1]=0;
    } else {
        char* plt=strstr(trz_tk->pText,"达成");
        char* plt2=strstr(plt+4,"达成");
        if(plt&&plt2) {
            plt=plt-2;
            plt2=plt2-2;
            GMajor->taskstatus[0]=((strstr(plt,"已达成")==plt)?1:0);
            GMajor->taskstatus[1]=((strstr(plt2,"已达成")==plt2)?1:0);
        } else {
            printf("福袋详情-识别福袋详情页面出错：未能取到任务描述区域关于任务状态的描述\n");
            goto fg_detail_2task_ocr_exit;
        }
    }
    
    //检查执行任务文本
    pTRZ trz_et=gettrzbycode(trz,DOU_FORTUNEGIFT_DETAIL_EXECTASK);
    if(!trz_et) {
        printf("福袋详情-识别福袋详情页面出错：未能取到执行任务按钮的文本\n");
        goto fg_detail_2task_ocr_exit;
    }
    TextTRZ(api,img,trz_et);
    printf("执行任务按钮的文本[%s]\n",trz_et->pText);
    
    result=0;
fg_detail_2task_ocr_exit:    
    pixDestroy(&img);
    return result;
}

/*
 * 福袋开奖结果-中奖页面要素识别
 */
int fg_draw_jackpot_ocr(tesseract::TessBaseAPI* api,pTRZ trz,char* img_file) {
    printf("中奖页面.\n");
    
    int ret=-1;
    //将中奖图片存入文件夹 ./pics/award/
    time_t now=time(NULL);
    struct tm* local=localtime(&now);
    char award_name[256]="";
    char path[256]={0};
    GetModulePath(path);
    sprintf(award_name,"%spics\\award\\%04d_%02d_%02d_%02d_%02d_%02d.png",
        path,
        local->tm_year+1900,local->tm_mon+1,local->tm_mday,
        local->tm_hour,local->tm_min,local->tm_sec
    );
    CopyFile(img_file,award_name,TRUE);
    
    /*
     * 由于领奖按钮 和 确认协议的 checkbox 的位置不确定
     * 此处需要查找修正
     * 首先裁剪出popup
     * 继而在裁剪的popup上寻找相关的 afmbtn/proto_checkbox
     */
    pTRZ trz_popup=gettrzbycode(trz,DOU_FORTUNEGIFT_DRAW_JACKPOT_POPUP);
    if(!trz_popup) {
        printf("获取选区DOU_FORTUNEGIFT_DRAW_JACKPOT_POPUP失败。\n");
        goto fg_draw_jackpot_ocr_exit;
    }
    //裁剪
    char clip_file[256]={0};
    //对图像指定区域裁切保存
    Pix* pixSource=pixRead(img_file);
    ClipTRZ(pixSource,trz_popup,clip_file);
    pixDestroy(&pixSource);
    
    pTRZ trz_affimbtn=gettrzbycode(trz,DOU_FORTUNEGIFT_DRAW_JACKPOT_CFMAWARD,0);
    if(!trz_affimbtn) {
        printf("获取选区 DOU_FORTUNEGIFT_DRAW_JACKPOT_CFMAWARD 失败。\n");
        goto fg_draw_jackpot_ocr_exit;
    }
    if(searchtemplate(clip_file,DOU_FORTUNEGIFT_DRAW_JACKPOT_CFMAWARD,trz)!=0) {
        printf("查找匹配的领奖确认按钮失败.\n");
        goto fg_draw_jackpot_ocr_exit;
    } else {
        /*
         * 修正相对坐标的DOU_FORTUNEGIFT_DRAW_JACKPOT_CFMAWARD
         */
        trz_affimbtn->x+=trz_popup->x;trz_affimbtn->y+=trz_popup->y;
    }
    
    pTRZ trz_proto=gettrzbycode(trz,DOU_FORTUNEGIFT_DRAW_JACKPOT_AFMPROTO,0);
    if(!trz_proto) {
        printf("获取选区 DOU_FORTUNEGIFT_DRAW_JACKPOT_AFMPROTO 失败。\n");
        goto fg_draw_jackpot_ocr_exit;
    }

    /*
     * 检查选区内有无圆圈 
     */
    if(searchtemplate(clip_file,DOU_FORTUNEGIFT_DRAW_JACKPOT_AFMPROTO,trz)!=0) {
        printf("获取 DOU_FORTUNEGIFT_DRAW_JACKPOT_AFMPROTO 失败。\n");
        goto fg_draw_jackpot_ocr_exit;
    } else {
        trz_proto->x+=trz_popup->x;trz_proto->y+=trz_popup->y;
    }

    pTRZ trz_proto_checked=gettrzbycode(trz,DOU_FORTUNEGIFT_DRAW_JACKPOT_AFMPROTO_CHECKED,0);
    if(!trz_proto_checked) {
        printf("获取选区 DOU_FORTUNEGIFT_DRAW_JACKPOT_AFMPROTO_CHECKED 失败。\n");
        goto fg_draw_jackpot_ocr_exit;
    }
    /*
     * 将 trz_proto_checked的坐标更新为 trz_proto;
     */
    trz_proto_checked->x=trz_proto->x;trz_proto_checked->y=trz_proto->y;
    trz_proto_checked->width=trz_proto->width;trz_proto_checked->height=trz_proto->height;
    
    /*
    * 未找到 同意协议（未选中）的checkbox
    * 判定为 同意协议（已选中）
    * 此处进一步识别确认
    * 注意 这里调用 searchtemplate 确认协议是否点选的时候， update_trz 标志 必须置为0，即 不得更新已经刷新的 
    *  DOU_FORTUNEGIFT_DRAW_JACKPOT_AFMPROTO / DOU_FORTUNEGIFT_DRAW_JACKPOT_AFMPROTO_CHECKED 
    *  trz 坐标
    */   
    int isproto_checked=-1;
    if(searchtemplate(clip_file,DOU_FORTUNEGIFT_DRAW_JACKPOT_AFMPROTO,trz,0)==0) {
        isproto_checked=0;
    } else {
        if(searchtemplate(clip_file,DOU_FORTUNEGIFT_DRAW_JACKPOT_AFMPROTO_CHECKED,trz,0)==0) {
            isproto_checked=1;
        }
    }
    GMajor->proto_checked=isproto_checked;

    if(isproto_checked==-1) {
        printf("判定是否点选同意协议失败.\n");
        goto fg_draw_jackpot_ocr_exit;
    } else {
        /*
         * 更新 trz_proto、trz_proto_checked
         */
        // if(isproto_checked==0) {
        //     trz_proto->x+=trz_popup->x;trz_proto->y+=trz_popup->y;
        //     trz_proto_checked->x=trz_proto->x;trz_proto_checked->y=trz_proto->y;
        //     trz_proto_checked->width=trz_proto->width;trz_proto_checked->height=trz_proto->height;
        // } else {
        //     trz_proto_checked->x+=trz_popup->x;trz_proto_checked->y+=trz_popup->y;
        //     trz_proto->x=trz_proto_checked->x;trz_proto->y=trz_proto_checked->y;
        //     trz_proto->width=trz_proto_checked->width;trz_proto->height=trz_proto_checked->height;
        // }
    }
    ret=0;
fg_draw_jackpot_ocr_exit:
    return ret;
}

/*
 * 福袋开奖结果 -- 没有中奖 页面要素识别
 */
int fg_draw_noprize_ocr(tesseract::TessBaseAPI* api,pTRZ trz,char* img_file) {
    printf("未中奖页面.\n");
    return 0;
}

int ImgEhanceBeforeOCR(const char* img_file,char* eh_img_file) {
    cv::Mat image=cv::imread(img_file);
    if(image.empty()) {
        printf("无法打开图像[%s]。\n",img_file);
        return -1;
    }
    
    //灰度处理
    cv::Mat gray;
    cv::cvtColor(image,gray,cv::COLOR_BGR2GRAY);
    
    //自适应阈值
    cv::Mat adaptiveThresh;
    cv::adaptiveThreshold(gray,adaptiveThresh,255,cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 15, -2);
    
    //cv::Mat hline=getStructuringElement(cv::MORPH_RECT,cv::Size(image.cols/16,1),cv::Point(-1,-1));
    //cv::Mat vline=getStructuringElement(cv::MORPH_RECT,cv::Size(1,image.rows/16),cv::Point(-1,-1));
    
    cv::Mat kernel=getStructuringElement(cv::MORPH_RECT,cv::Size(1,1),cv::Point(-1,-1));
    
    //膨胀与腐蚀
    cv::Mat dilated,eroded;
    //cv::Mat kernel=cv::getStructuringElement(cv::MORPH_RECT,cv::Size(2,2));
    cv::erode(adaptiveThresh,eroded,kernel);
    cv::dilate(eroded,dilated,kernel);
    
    // 反色处理
    cv::Mat inverted;
    cv::bitwise_not(dilated,inverted);
        
    //边缘处理
    //cv::blur(inverted,inverted,cv::Size(2,2),cv::Point(-1,-1));
        
    //去噪
    cv::Mat denoised;
    cv::fastNlMeansDenoising(inverted,denoised,50,7,21);
    
    if(eh_img_file[0]=='\0') getehimagename(img_file,eh_img_file);
    cv::imwrite(eh_img_file,denoised);
    
    return 0;
}

int ClipTRZ(Pix* pix,pTRZ trz) {
    if(!pix||!trz) {
        printf("Clip TRZ 失败：图像或选区数据无效。\n");
        return -1;
    }
    
    //裁剪图片
    BOX* clipBox=boxCreate(trz->x,trz->y,trz->width,trz->height);
    if(clipBox==NULL) {
        printf("创建裁剪区失败,TRZ CODE:%d.\n",trz->code);
    }
    PIX* clipImg=pixClipRectangle(pix,clipBox,NULL);
    if(clipImg==NULL) {
        printf("图片裁剪失败\n");
    } else {
        //写入文件
        char clip_file[256]={0};
        char path[256]={0};
        GetModulePath(path);
        sprintf(clip_file,"%spics\\%d.png",path,(int)trz->code);
        if(0!=pixWrite(clip_file,clipImg,IFF_PNG)) {
            printf("裁剪图片写入文件%s 失败\n",clip_file);
        } else {
            //printf("裁剪图片写入文件%s\n",clip_file);
        }
    }
    
    //释放资源
    pixDestroy(&clipImg);
    boxDestroy(&clipBox);
    
    return 0;
}

int ClipTRZ(Pix* pix,pTRZ trz,char* clip_file) {
    if(!pix||!trz) {
        printf("Clip TRZ 失败：图像或选区数据无效。\n");
        return -1;
    }
    
    //裁剪图片
    BOX* clipBox=boxCreate(trz->x,trz->y,trz->width,trz->height);
    if(clipBox==NULL) {
        printf("创建裁剪区失败,TRZ CODE:%d.\n",trz->code);
    }
    PIX* clipImg=pixClipRectangle(pix,clipBox,NULL);
    if(clipImg==NULL) {
        printf("图片裁剪失败\n");
    } else {
        char path[256]={0};
        GetModulePath(path);
        //写入文件
        if(clip_file[0]=='\0') sprintf(clip_file,"%spics\\%d.png",path,(int)trz->code);
        if(0!=pixWrite(clip_file,clipImg,IFF_PNG)) {
            printf("裁剪图片写入文件%s 失败\n",clip_file);
        } else {
            //printf("裁剪图片写入文件%s\n",clip_file);
        }
    }
    
    //释放资源
    pixDestroy(&clipImg);
    boxDestroy(&clipBox);
    
    return 0;
}

/*
 * TextTRZ的增强版，需要对图像进行预处理
 * 该版本效率极低，一个图片需要3读2写，待优化，后期考虑是否可以使用内存而非文件过度
 */
int TextTRZEh(tesseract::TessBaseAPI* api,const char* img_file,pTRZ trz) {
    char clip_file[256]={0};
    char eh_img_file[256]={0};
    
    //1.对图像指定区域裁切保存
    Pix* pixSource=pixRead(img_file);
    ClipTRZ(pixSource,trz,clip_file);
    pixDestroy(&pixSource);
    
    //2.对裁切图像进行识别前的处理
    // 对 eh_img_file 赋值
    // 增强图为原图名加上 "_Eh"
    getehimagename(clip_file,eh_img_file);
    ImgEhanceBeforeOCR(clip_file,eh_img_file);
    
    //3.识图
    Pix* pix=pixRead(eh_img_file);
    TextTRZ(api,pix,trz,0);//不需要裁剪
    pixDestroy(&pix);
    return 0;
}

/*
 * 取回的OCR文本附加处理
 */
int TextTRZExtra(pTRZ trz) {
    if(!trz||!trz->pText) return 0;
    
    char* plt=trz->pText;
    size_t len=strlen(plt);
    
    if(plt[len-1]=='\n') plt[len-1]='\0';
    
    //福袋详情内容去回车换行符
    if(trz->code==DOU_FORTUNEGIFT_DETAIL_TEXT&&trz->pText) {
        char* plt_buffer=(char*)calloc(sizeof(char)*(len+1),1);
        if(!plt_buffer) return -1;
        for(int idx=0,dst_idx=0;idx<len;idx++) {
            if(plt[idx]!='\r'&&plt[idx]!='\n') {
                plt_buffer[dst_idx]=plt[idx];
                dst_idx++;
            }
        }
        
        memcpy(trz->pText,plt_buffer,len);
        trz->pText[len]='\0';
        if(plt_buffer) free(plt_buffer);
    }
    return 0;
}

int TextTRZ(tesseract::TessBaseAPI* api,Pix* pix,pTRZ trz,int clip_flag) {
    if(clip_flag!=0) ClipTRZ(pix,trz);  

    //获取图片高宽
    int img_width=pixGetWidth(pix);
    int img_height=pixGetHeight(pix);

    if(img_width<=0||img_height<=0) return -1;

    //检查TRZ设置选区是否合法
    if(clip_flag!=0) {
        if(trz->x<0||trz->x>=img_width||
            trz->y<0||trz->y>=img_height||
            trz->x+trz->width>img_width||
            trz->y+trz->height>img_height
        ) {
            if(f->extra) {
                pwfpj_param wfpj=(pwfpj_param)f->extra;
                SendMessage(wfpj->imgsratch,MSG_SECTIONSETTINGS,(WPARAM)trz,(LPARAM)trz->code);
            } 
        }
    } else {
        if(trz->width<=0||trz->width>img_width||
            trz->height<=0||trz->height>img_height
        ) {
            if(f->extra) {
                pwfpj_param wfpj=(pwfpj_param)f->extra;
                SendMessage(wfpj->imgsratch,MSG_SECTIONSETTINGS,(WPARAM)trz,(LPARAM)trz->code);
            } 
        }
    }

    //设置api的traindata  
    if(trz->traindata[0]=='\0') strcpy(trz->traindata,"chi_sim");
    if(api->Init(datapath,trz->traindata)) {
        printf("Initial Tesseract failed.\n");
        return -1;
    }

    //加载图片
    api->SetImage(pix);
    
    //设置tessedit_char_whitelist参数
    if(trz->whitelist[0]!='\0')  api->SetVariable("tessedit_char_whitelist",trz->whitelist);
    else api->SetVariable("tessedit_char_whitelist", "");

    if(trz->pText) {
        free(trz->pText);
        trz->pText=NULL;
    }

    int result;
    if(clip_flag!=0) result=TextRectOCR(api,trz->x,trz->y,trz->width,trz->height,&(trz->pText));
    else result=TextRectOCR(api,0,0,trz->width,trz->height,&(trz->pText));
    
    if(result==0) result=TextTRZExtra(trz); 
    
    //终止当前的识别绘画
    api->End();
    
    return result;
}

int TextTRZ(tesseract::TessBaseAPI* api,Pix* pix,pTRZ trz) {
    return TextTRZ(api,pix,trz,1);
}

int InitialSections(tesseract::TessBaseAPI* api,Pix* pix,pTRZ trz) {
    size_t trz_size=_msize(trz)/sizeof(TRZ);
    
    for(int idx=0;idx<trz_size;idx++) {
        if((trz+idx)->code<=FORTUNEGIFT_OR_DIAMOND_RANGE&&
           (trz+idx)->code>=STREAMER) {
            if((trz+idx)->code==ALIVECOUNT) {
                api->SetVariable("tessedit_char_whitelist", "0123456789");
            } else if((trz+idx)->code==DOU_DIAMOND_COUNTDOWN||
                (trz+idx)->code==DOU_DIAMOND_COUNTDOWN) {
                api->SetVariable("tessedit_char_whitelist", "0123456789:");
            }
            if((trz+idx)->code==STREAMER) {
                char eh_name[256]="";
                char path[256]={0};
                GetModulePath(path);
                sprintf(eh_name,"%spics\\%d.png",path,(int)STREAMER);
                TRZ rtmp;
                memcpy(&rtmp,trz+idx,sizeof(rtmp));
                rtmp.x=rtmp.y=0;
                TextTRZEh(api,eh_name,&rtmp);
                (trz+idx)->pText=rtmp.pText; 
            } else TextTRZ(api,pix,trz+idx);
        }
    }
    
    return 0;
}

void ReleaseSections(pTRZ trz) {
    if(!trz) return;
    size_t trz_size=_msize(trz)/sizeof(TRZ);
    
    for(int idx=0;idx<trz_size;idx++) {
        if((trz+idx)->pText) delete[] (trz+idx)->pText;
        (trz+idx)->pText=NULL;
    }
    
    delete[] trz;
}

int TextRectOCR(tesseract::TessBaseAPI* api,int x,int y,int width,int height,char** pText) {
    api->SetRectangle(x,y,width,height);
    char* outputTextUTF8=NULL;
    
    outputTextUTF8=api->GetUTF8Text();
    if(outputTextUTF8==NULL) return -1;
    
    //utf8 转 ACII
    wchar_t* text_wchar=NULL;
    int wchar_length=MultiByteToWideChar(CP_UTF8,0,outputTextUTF8,-1,NULL,0);
    if(wchar_length<=0) {
        delete[] outputTextUTF8;
        outputTextUTF8=NULL;
        return -1;
    }
    
    text_wchar=new wchar_t[wchar_length+1];
    MultiByteToWideChar(CP_UTF8,0,outputTextUTF8,-1,text_wchar,wchar_length);
    
    char* outputText=NULL;
    int gbk_length=WideCharToMultiByte(CP_ACP,0,text_wchar,-1,NULL,0,NULL,NULL);
    if(gbk_length<=0) {
        delete[] outputTextUTF8;
        outputTextUTF8=NULL;
        if(text_wchar) delete[] text_wchar;
        text_wchar=NULL;
        return -1;
    }
    
    outputText=new char[gbk_length+1]; 
    WideCharToMultiByte(CP_ACP,0,text_wchar,-1,outputText,gbk_length,NULL,NULL);
    
    if(outputTextUTF8) delete[] outputTextUTF8;
    outputTextUTF8=NULL;
    if(text_wchar) delete[] text_wchar;
    text_wchar=NULL;
    
    //gbk_length=strlen(outputText);
    //if(outputText[gbk_length-1]=='\n') outputText[gbk_length-1]='\0';
    
    *pText=outputText;
    
    return 0;
}

int getehimagename(const char* img_name,char* eh_name) {
#define EHANCE_IMG_EXT  "_Eh"
    for(int idx=strlen(img_name)-1;idx>=0;idx--) 
        if(img_name[idx]=='.') {
            strncpy(eh_name,img_name,idx);
            strcat(eh_name,EHANCE_IMG_EXT);
            strcat(eh_name,&img_name[idx]);
            //printf("source name:%s, ehance name:%s\n",img_name,eh_name);
            return 0;
        }
    return -1;
}

int searchtemplate(const char* source_img,TRZCODE code,pTRZ list) {
    return searchtemplate(source_img,code,list,1);
}

/*
 * update_trz ： 默认值为 1 - 更新匹配的trz
 */
int searchtemplate(const char* source_img,TRZCODE code,pTRZ list,int update_trz) {
    char template_imgs[][256]={
        "pics\\diamond_template.png",
        "pics\\gift_template.png",
        "pics\\fortunegift_award_template.png",
        "pics\\fortunegift_protunchecked_template.png", //协议未点选
        "pics\\fortunegift_protchecked_template.png",
        "pics\\fortunegift_detail_template.png",
    };

    char path[256]={0};
    GetModulePath(path);
    for(int idx=0;idx<sizeof(template_imgs)/sizeof(char[256]);idx++) {
        char tmp[256]={0};
        sprintf(tmp,"%s%s",path,template_imgs[idx]);
        strcpy(template_imgs[idx],tmp);
    }

    char* template_img=NULL;
    
    switch((int)code) {
    case DIAMOND_CLICKRECT: 
        template_img=&template_imgs[0][0];
        break;
    case FORTUNEGIFT_CLICKRECT:
        template_img=&template_imgs[1][0];
        break;
    case DOU_FORTUNEGIFT_DRAW_JACKPOT_CFMAWARD: 
        template_img=&template_imgs[2][0];
        break;
    case DOU_FORTUNEGIFT_DRAW_JACKPOT_AFMPROTO:
        template_img=&template_imgs[3][0];
        break;
    case DOU_FORTUNEGIFT_DRAW_JACKPOT_AFMPROTO_CHECKED:
        template_img=&template_imgs[4][0];
        break;
    case DOU_FORTUNEGIFT_DETAIL_IMG:
        template_img=&template_imgs[5][0];
        break;
    }
    
    //printf("source img:%s,template img:%s\n",source_img,template_img);
    
    cv::Mat source=cv::imread(source_img);
    cv::Mat templt=cv::imread(template_img);
        
    if(source.empty()||templt.empty()) {
        printf("加载图片或者模板图片失败\n");
        return -1;
    }
    
    // 转换为灰度图像
    cv::Mat sourceGray,templateGray;
    cv::cvtColor(source,sourceGray,cv::COLOR_BGR2GRAY);
    cv::cvtColor(templt,templateGray,cv::COLOR_BGR2GRAY);

    cv::Mat result;
    int result_cols=sourceGray.cols-templateGray.cols+1;
    int result_rows=sourceGray.rows-templateGray.rows+1;
    result.create(result_rows,result_cols,CV_32FC1);
    
    //模板匹配
    //TM_SQDIFF_NORMED 使用归一化方差，越小越好
    //TM_CCOEFF_NORMED 相关度，越大越好
    cv::matchTemplate(sourceGray,templateGray,result,cv::TM_CCOEFF_NORMED);
    
    //归一化
    //cv::normalize(result,result,0,1,cv::NORM_MINMAX,-1,cv::Mat());
        
    //找到最佳匹配位置
    double minVal,maxVal;
    cv::Point minLoc,maxLoc,matchLoc;
    cv::minMaxLoc(result,&minVal,&maxVal,&minLoc,&maxLoc);
    printf("匹配最佳位置的方差%lf\n",maxVal);
    
    matchLoc=maxLoc;
    double threshold=0.9;
    if(maxVal>threshold) {
        int x,y,width,height;//注意x,y 为右下角坐标
        x=matchLoc.x;
        y=matchLoc.y;
        width=templt.cols;
        height=templt.rows;

        pTRZ trz=gettrzbycode(list,code,0);
        if(trz&&update_trz==1) {
            trz->x=x;trz->y=y;trz->width=width;trz->height=height;
        }
        //printf("找到匹配位置:(%d,%d) %d×%d\n",x,y,width,height);
        
        //在图像上绘制矩形以显示匹配位置
        cv::rectangle(source,matchLoc,cv::Point(matchLoc.x+templt.cols,
            matchLoc.y+templt.rows),cv::Scalar::all(250),2,8,0);
        
        char path[256]={0};
        GetModulePath(path);
        char searched_file[256]={0};
        sprintf(searched_file,"%spics\\searched.png",path);
        cv::imwrite(searched_file,source);
        
        return 0;
    } else {
        printf("未找到相似度匹配的图像。\n");
        return -1;
    }
}

/*
 * --------------------------------------------------
 * wasted.
 * --------------------------------------------------
 */
int searchtemplate2(const char* source_img,const char* template_img) {
    
    //printf("source img:%s,template img:%s\n",source_img,template_img);
    
    cv::Mat source=cv::imread(source_img);
    cv::Mat templt=cv::imread(template_img,cv::IMREAD_UNCHANGED);
        
    if(source.empty()||templt.empty()) {
        printf("加载图片或者模板图片失败\n");
        return -1;
    }
    
    // 转换为灰度图像
    //cv::Mat sourceGray,templateGray;
    //cv::cvtColor(source,sourceGray,cv::COLOR_BGR2GRAY);
    //cv::cvtColor(templt,templateGray,cv::COLOR_BGR2GRAY);
    
    //template 提取非透明部分
    cv::Mat sourceNonTrans;
    cv::extractChannel(source,sourceNonTrans,3);//取alpha通道
        
    cv::Mat templtNonTrans;
    cv::extractChannel(templt,templtNonTrans,3);
    
    cv::Mat result;
    cv::matchTemplate(sourceNonTrans,templtNonTrans,result,cv::TM_CCORR_NORMED);
    
    //归一化
    //cv::normalize(result,result,0,1,cv::NORM_MINMAX,-1,cv::Mat());
        
    //找到最佳匹配位置
    double minVal,maxVal;
    cv::Point minLoc,maxLoc,matchLoc;
    cv::minMaxLoc(result,&minVal,&maxVal,&minLoc,&maxLoc);
    printf("匹配最佳位置的方差范围:%lf~%lf\n",minVal,maxVal);
    
    matchLoc=maxLoc;
    double threshold=0.9;
    if(maxVal>threshold) {
        int x,y,width,height;
        x=matchLoc.x+templt.cols;
        y=matchLoc.y+templt.rows;
        width=templt.cols;
        height=templt.rows;
        printf("找到匹配位置:(%d,%d) %d×%d\n",x,y,width,height);
        
        //在图像上绘制矩形以显示匹配位置
        cv::rectangle(source,matchLoc,cv::Point(matchLoc.x+templt.cols,
            matchLoc.y+templt.rows),cv::Scalar::all(250),2,8,0);
        
        cv::imwrite("./pics/searched.png",source);
        
        return 0;
    } else {
        printf("未找到相似度匹配的图像。\n");
        return -1;
    }
}
/* -------------------------------------------------- */

pTRZ gettrzbycode(pTRZ list,TRZCODE code,int checktrz) {
    if(list==NULL) return NULL;
     
    int length=_msize(list)/sizeof(TRZ);
    for(int idx=0;idx<length;idx++) {
        if((list+idx)->code==code) {
            pTRZ section=list+idx;
            if(!section||section->height<=0||section->width<=0) {
                /*
                 * 发送消息
                 */
                if(checktrz==1&&f->extra) {
                    pwfpj_param wfpj=(pwfpj_param)f->extra;
                    SendMessage(wfpj->imgsratch,MSG_SECTIONSETTINGS,(WPARAM)section,(LPARAM)code);
                }
            }

            return list+idx;
        }
    }
    return NULL;
}

pTRZ gettrzbycode(pTRZ list,TRZCODE code) {
    return gettrzbycode(list,code,1);
}

PAGECODE GetPageCode(const char* source_img,pTRZ list) {
    return GetPageCode(NULL,source_img,list);
}
/*
 * 识别当前页面代码
 */
PAGECODE GetPageCode(tesseract::TessBaseAPI* api,const char* source_img,pTRZ list) {
    pwfpj_param wfpj=(pwfpj_param)f->extra;
    //页面标志图
    //请勿随意调整以下模板文件、页面代码、搜索区域的顺序，
    //顺序经过设计，即便同步调整也可能会导致错误。
    char template_imgs[][256]={
        "pics\\fortunegift_draw_jackpot_template.png",//中奖模板图
        "pics\\fortunegift_draw_noprize_template.png",//未中奖模板图
        "pics\\streamer_room_template.png",
        "pics\\fortunegift_detail_template.png",
        "pics\\diamond_detail_template.png",
        "pics\\fortunegift_afmaward_template.png",
    };

    char path[256]={0};
    GetModulePath(path);
    for(int idx=0;idx<sizeof(template_imgs)/sizeof(char[256]);idx++) {
        char tmp[256]={0};
        sprintf(tmp,"%s%s",path,template_imgs[idx]);
        strcpy(template_imgs[idx],tmp);
    }

    //页面代码
    PAGECODE pages[]={
        FORTUNEGIFT_JACKPOT,//福袋中奖界面
        FORTUNEGIFT_NOPRIZE,//福袋未中奖界面
        STREAMER_ROOM,
        FORTUNEGIFT_DETAIL,
        DIAMOND_DETAIL,
        FORTUNEGIFT_AFMAWARD,
    };
    //主图待搜索区域
    TRZCODE zones[]={
        DOU_FORTUNEGIFT_DRAW_JACKPOT_IMG,//福袋中奖页面标志
        DOU_FORTUNEGIFT_DRAW_NOPRIZE_IMG,//福袋未中奖页面识别标志
        STREAMER_ROOM_IMG,
        DOU_FORTUNEGIFT_DETAIL_IMG,
        DOU_DIAMOND_DETAIL_IMG,
        DOU_FORTUNEGIFT_AWARDFORTUNEGIFT_IMG,
    };
        
    char* template_img=NULL;
    
    for(int idx=0;idx<sizeof(template_imgs)/sizeof(char[256]);idx++) {
        template_img=&template_imgs[idx][0];
                
        pTRZ trz=gettrzbycode(list,zones[idx]);
        if(trz==NULL) {
            printf("获取TRZ信息失败,trz code:%d\n",zones[idx]);
            continue;
        }
        
        cv::Mat img=cv::imread(source_img);
        if(img.data==NULL) return UNKNOWN_PAGE;

        cv::Mat source(img,cv::Rect(trz->x,trz->y,trz->width,trz->height));
        cv::Mat templt=cv::imread(template_img);
        if(source.empty()||templt.empty()) {
            printf("加载图片[%s]或者模板图片[%s]失败\n",source_img,template_img);
            if(wfpj->workflow) SendMessage(wfpj->workflow,MSG_NODELIGHTING,(WPARAM)UNKNOWN_PAGE,(LPARAM)0);
            return UNKNOWN_PAGE;
        }
        
        // 转换为灰度图像
        cv::Mat sourceGray,templateGray;
        cv::cvtColor(source,sourceGray,cv::COLOR_BGR2GRAY);
        cv::cvtColor(templt,templateGray,cv::COLOR_BGR2GRAY);
    
        cv::Mat result;
        int result_cols=sourceGray.cols-templateGray.cols+1;
        int result_rows=sourceGray.rows-templateGray.rows+1;
        result.create(result_rows,result_cols,CV_32FC1);
        
        //模板匹配
        //TM_SQDIFF_NORMED 使用归一化方差，越小越好
        //TM_CCOEFF_NORMED 相关度，越大越好
        cv::matchTemplate(sourceGray,templateGray,result,cv::TM_CCOEFF_NORMED);
            
        //找到最佳匹配位置
        double minVal,maxVal;
        cv::Point minLoc,maxLoc,matchLoc;
        cv::minMaxLoc(result,&minVal,&maxVal,&minLoc,&maxLoc);
        
        matchLoc=maxLoc;
        double threshold=0.9;
        if(maxVal>threshold) {
            PAGECODE code=pages[idx];
            if(FORTUNEGIFT_DETAIL==pages[idx]) {
                // //trz->x=matchLoc.x;trz->y=matchLoc.y;
                // int y_offset=trz->y+matchLoc.y;
                // if(y_offset>=700&&y_offset<=740) {
                //     //两部任务模式 718
                //     printf("两步任务模式,template Y:%d\n",y_offset);
                    
                //     code=FORTUNEGIFT_DETAIL_2TASK;
                // } else {
                //     //一步任务模式 766, offset = 766-718=48;
                //     printf("一步任务模式,template Y:%d\n",y_offset);
                // }
                /*
                * 尝试用某种方式识别是一步还是两步
                */
                pTRZ trz_taskzone=gettrzbycode(list,DOU_FORTUNEGIFT_TASKRECT);
                if(trz_taskzone) {
                    //Pix* pixImg=pixRead(source_img); 
                    if(!api) {
                        tesseract::TessBaseAPI* api_new=new tesseract::TessBaseAPI();
                        TextTRZEh(api_new,source_img,trz_taskzone);
                        if(api_new) delete api_new;
                    } else {
                        TextTRZEh(api,source_img,trz_taskzone);
                    }
                    //pixDestroy(&pixImg);

                    if(trz_taskzone->pText) {
#define CHK_TASK_KEYWORDS   "达成"                    
                        char* task_plt=strstr(trz_taskzone->pText,CHK_TASK_KEYWORDS);
                        if(!task_plt) {
                            code=UNKNOWN_PAGE;
                        } else {
                            task_plt+=strlen(CHK_TASK_KEYWORDS);
                            if(strstr(task_plt,CHK_TASK_KEYWORDS)) {
                                code=FORTUNEGIFT_DETAIL_2TASK;
                            }
                        }

                    }
                }
            }
            
            //printf("source img:%s,template img:%s,",source_img,template_img);
            //printf("匹配最佳位置的方差:%lf\n",maxVal);
            if(wfpj->workflow) SendMessage(wfpj->workflow,MSG_NODELIGHTING,(WPARAM)code,(LPARAM)0);
            return code;
        }  
    }
    
    //如果找图没找到，开启文字识别
    // 直播间已经关闭 STREAMER_ROOMCLOSED_IMG
    pTRZ trz_rc=gettrzbycode(list,STREAMER_ROOMCLOSED_IMG);
    
    if(!api) {
        tesseract::TessBaseAPI* api_new=new tesseract::TessBaseAPI();
        TextTRZEh(api_new,source_img,trz_rc);
        if(api_new) delete api_new;
    } else {
        TextTRZEh(api,source_img,trz_rc);
    }
        
    if(strstr(trz_rc->pText,"直播已结束")) {
        if(wfpj->workflow) SendMessage(wfpj->workflow,MSG_NODELIGHTING,(WPARAM)STREAMER_ROOMCLOSED,(LPARAM)0);    
        return STREAMER_ROOMCLOSED;
    }

    if(wfpj->workflow) SendMessage(wfpj->workflow,MSG_NODELIGHTING,(WPARAM)UNKNOWN_PAGE,(LPARAM)0);    
    return UNKNOWN_PAGE;
}

#ifdef TESSERACT_UNITTEST
int main() {
    
    //PageAnalysis_Test();
    Flows();
    
    printf("Exit Normally.\n");
    return 0;
}
#endif

DWORD WINAPI DouYinOCRJOB(LPVOID lParam) {
    /*
     * 启动流程
     */

    Flows();

    return 0;
}

BOOL IsThreadSuspended(HANDLE threadHandle) {
    NtQueryInformationThreadFunc NtQueryInformationThread =
        (NtQueryInformationThreadFunc)GetProcAddress(
            GetModuleHandleA("ntdll.dll"),"NtQueryInformationThread");

    if(!NtQueryInformationThread) return FALSE;

    THREAD_BASIC_INFORMATION tbi;
    NTSTATUS status=NtQueryInformationThread(threadHandle,(THREADINFOCLASS)0,&tbi,sizeof(tbi),NULL);

    if(status==0&&tbi.SuspendCount>0) {
        return TRUE; // 线程被挂起
    }
    return FALSE; // 线程未被挂起
}

/*
 * output_buffer format.
 * ---------------------------------------------------------
 * PS E:\SRC_REF\tesseract\self-bulid\test\vs> adb devices
 * List of devices attached
 * 192.168.5.5:5555        device
 * 192.168.5.3:5555        device
 * 
 */
void subproc_analysishost(int exitcode,void* pEP,void* param) {
    pExecPipes pep=(pExecPipes)pEP;
    if(!pep) return;
    char* host_plt=(char*)param;
    int host_length=0;
    for(int idx=0;idx<pep->output_len;idx++) {
        if(pep->output_buffer[idx]=='\n') {
            char host[256]={0};
            int ip1,ip2,ip3,ip4,port;
            if(5==sscanf(&pep->output_buffer[idx+1],"%d.%d.%d.%d:%d",&ip1,&ip2,&ip3,&ip4,&port)) {
                sprintf(host,"%d.%d.%d.%d:%d",ip1,ip2,ip3,ip4,port);
                strcat(host_plt+host_length,host);
                host_length+=strlen(host)+1;
            }
        }
    }
}

void Flows() {
    if(GMajor==NULL) return;
    resetmajor(GMajor);

    time_t tstart;
    time(&tstart);

    char path[256]={0};
    GetModuleFileName(NULL,path,sizeof(path));
    if(strrchr(path,'\\')) {
        char* plt=strrchr(path,'\\');
        *(plt+1)='\0';
    }
    
    tesseract::TessBaseAPI* api=new tesseract::TessBaseAPI();

    //初始化host
    pwfpj_param wfpj=(pwfpj_param)f->extra;
    char host_list[1024]={0};
    char cmd_buffer[256]="adb devices";
    if(0!=subproc_execmd((char*)"cmd",(char*)cmd_buffer,subproc_analysishost,host_list)) {
        printf("获取host失败.\n");
        MessageBox(wfpj->frame,"获取host失败.","异常",MB_OK|MB_ICONINFORMATION);
        goto FLOWS_EXIT;
    } else {
        HMODULE module_host=LoadLibrary("host.dll");
        if(module_host) {
            SHOWDIALOGPROC cfmhost=(SHOWDIALOGPROC)GetProcAddress(module_host,"ConfirmHost");
            if(cfmhost) {
                cfmhost((HINSTANCE)module_host,wfpj->frame,(LPARAM)host_list);
                memcpy(GMajor->host,&host_list[256],sizeof(GMajor->host));
            } else {
                MessageBox(wfpj->frame,"fuck, proc addr not found.","异常",MB_OK|MB_ICONINFORMATION);
            }
            FreeLibrary(module_host);
        } else {
            MessageBox(wfpj->frame,"加载host配置失败","异常",MB_OK|MB_ICONINFORMATION);
        }
    }

    //根据页面类型确定执行页面切换的策略
    int refresh_flag=1;
    PAGECODE prepage=UNKNOWN_PAGE;
    int close_repeat=0;
    while(refresh_flag) {
        //1. 捕捉图像
        int retry_count=0;
        while(retry_count<=5) {
            char img_fullpath[256]={0};
            sprintf(img_fullpath,"%s%s",path,"pics\\x.png");
            //对于指定host，参数 adb -s $host ....
            char cmd3[256]={0};
            sprintf(cmd3,"adb -s %s exec-out screencap -p > %s",GMajor->host,img_fullpath);
            if(0!=subproc_execmd((char*)"cmd",(char*)cmd3)) {retry_count++;printf("Command Execution failed.\n");Sleep(1000);}
            else break;
                
            Sleep(15);
        }
        
        //2.分析图像界面，识别页面，并解析页面内容
        PAGECODE page=PageAnalysis(api,GMajor->trz);

        /*
         * 如果当前页面为 直播间主界面，且执行策略为 2-持续执行2小时后关机
         * 判断是否超过两小时,如果超过
         * 1.关闭直播间
         * 2.关闭电脑
         */
        if(page==STREAMER_ROOM&&GMajor->strategycode==2) {
            time_t tnow;
            time(&tnow);
            if(difftime(tnow,tstart)>=2*60*60) {
                char cmdexit[256]={0};
                pTRZ trz_ext=gettrzbycode(GMajor->trz,STREAMERROOM_QUIT);//很抱歉，目前没有设置直播间退出的选区，如果添加，会导致前面的方案设置失败
                if(trz_ext) {
                    sprintf(cmdexit,"adb -s %s input tap %d %d",trz_ext->x+trz_ext->x+(trz_ext->width>>1),trz_ext->y+trz_ext->y+(trz_ext->height>>1));
                    subproc_execmd(NULL,cmdexit);
                    Sleep(5000);
                }

                char cmdshutdown[256]={0};
                sprintf(cmdshutdown,"shutdown /s /f /t 0");
                subproc_execmd(NULL,cmdshutdown);
                break;
            }
        }

        //3. 确定下一个页面
        if(0!=NextPageStrategies_TRACEONE(GMajor->trz,page)) {
            /*
             * 页面跳转出现了异常
             */
            break;
        }

        if(page==prepage&&page==UNKNOWN_PAGE) {
            close_repeat++;
            Sleep(15*1000);
        }
        if(close_repeat>=3) break;
        prepage=page;
    }

FLOWS_EXIT:
    if(api) delete api; 
    //clearmajor(GMajor);
    //GMajor=NULL;
}

/*
 * 无论当前给定的是什么页面，识别出来是否相关，如果相关，从页面获取所需的所有信息。
 * 面对未知的界面，将反复调用该函数
 */
PAGECODE PageAnalysis(tesseract::TessBaseAPI* api,pTRZ trz) {
    char img_file[][256]={
        //"pics\\x.png"
        "pics\\x.png"
    };

    //检测图像是否有效

    char path[256]={0};
    GetModulePath(path);
    for(int idx=0;idx<sizeof(img_file)/sizeof(char[256]);idx++) {
        char tmp[256]={0};
        sprintf(tmp,"%s%s",path,img_file[idx]);
        strcpy(img_file[idx],tmp);
    }

    int idx=0;
    PAGECODE page=GetPageCode(api,img_file[idx],trz);
    
    switch(page) {
    case FORTUNEGIFT_JACKPOT: 
        fg_draw_jackpot_ocr(api,trz,img_file[idx]);
    break;
    case FORTUNEGIFT_NOPRIZE: 
        fg_draw_noprize_ocr(api,trz,img_file[idx]);
    break;
    case STREAMER_ROOM: 
        streamer_room_ocr(api,trz,img_file[idx]);
    break;
    case STREAMER_ROOMCLOSED: 
        {
            printf("直播已结束.\n");
        }
    break;
    case FORTUNEGIFT_DETAIL: 
        fg_detail_ocr_prepare_dynamic(trz,img_file[idx]);
        GMajor->taskcount=1;
        fg_detail_ocr(api,trz,img_file[idx]);
    break;
    case FORTUNEGIFT_DETAIL_2TASK: 
        fg_detail_ocr_prepare_dynamic(trz,img_file[idx]);
        GMajor->taskcount=2;
        fg_detail_2task_ocr(api,trz,img_file[idx]);
    break;
    case DIAMOND_DETAIL: 
        //diamond_detail_ocr(api,trz,img_file[idx]);
    break;
    case FORTUNEGIFT_AFMAWARD:
        //...
    break;
    default: 
        /*
         * 对于未知页面，其分析与处置尚未被支持，自动化流程无以为继
         * 故一旦遇到位置页面，将进行3次 15S的等待尝试
         * 如仍无法识别，流程不得不终止
         */
        printf("Unknown PAGE,image file [%s]\n",img_file[idx]);
        /*
         * 将图像写入 pics/unknown/
         */
        char unknown_file[256]={0};
        time_t rawtime;
        struct tm* tminfo;
        char tm_buffer[256]={0};
        time(&rawtime);
        tminfo=localtime(&rawtime);
        strftime(tm_buffer,sizeof(tm_buffer),"%Y_%m_%d_%H_%M_%S",tminfo);

        sprintf(unknown_file,"%spics\\unknown\\%s.png",path,tm_buffer);
        CopyFile(img_file[idx],unknown_file,FALSE);
    }    
    return page;
}

int GetModulePath(char* path) {
    char fullpath[MAX_PATH]={0};

    if(GetModuleFileName((HMODULE)f->inst,fullpath,MAX_PATH)!=0) {
        char* plt=strrchr(fullpath,'\\');
        if(plt) {
            strncpy(path,fullpath,plt-&fullpath[0]+1);
            return 0;
        }
    }
    return -1;
}
/*
 * 获取下一个页面的策略
 * 这里可以有很多种策略，比如在一个直播间持续蹲守，
 * 可以切换不同的直播间，寻找人少、或者价值更高的直播间
 * 根据时间、或者货品类型切换等
 * 这将成为一个扩展接口，根据各自的需要自行实现
 * NextPageStrategies_TRACEONE 实现的是对一个特定直播间的蹲守策略
 * 蹲守规则如下：
 * 如果该直播间尚在直播，持续检查福袋，直至中奖或者直播间关闭。
 * 
 * 你可以实现自己的页面切换策略
 */
int NextPageStrategies_TRACEONE(pTRZ trz,PAGECODE currpage) {
    /*
     * 获取当前直播间的page，计算出下一个页面
     * 如果下一个页面需要界面点击或者操作，执行
     * 如果下一个页面需要定时等待，等待
     * 如果下一个页面需要循环尝试，循环
     * 直至切换至下一个页面。
     */
    switch(currpage) {
    case STREAMER_ROOM: {
        //获取详情,是否有福袋？
        if(GMajor->hasfg) {
            pTRZ trz_fodr=gettrzbycode(trz,FORTUNEGIFT_OR_DIAMOND_RANGE);
            if(!trz_fodr) return -1;
            pTRZ trz_fc=gettrzbycode(trz,FORTUNEGIFT_CLICKRECT);
            if(!trz_fc) return -1;
            
            char cmd_tapfg[256]={0};
            sprintf(cmd_tapfg,"input tap %d %d",trz_fodr->x+trz_fc->x+(trz_fc->width>>1),trz_fodr->y+trz_fc->y+(trz_fc->height>>1));
            subproc_execmd(NULL,cmd_tapfg);
            Sleep(2040);
        } else {
            //等30秒
            Sleep(30*1000);
        }
    } break;
    case FORTUNEGIFT_DETAIL: 
    case FORTUNEGIFT_DETAIL_2TASK: {
        //等待
        time_t stamp=GMajor->fg_countdown+GMajor->timestamp;
        struct tm* local_time=localtime(&stamp);
        char time_str[256]="";
        strftime(time_str,sizeof(time_str),"%H:%M:%S",local_time);
        printf("开始休眠，预计福袋在%d秒后[%s]开奖.\n",GMajor->fg_countdown,time_str);
        /*
         * 设置了参与时机
         * 参与时机暂定不得低于开奖前30秒
         */
        if(GMajor->attend_countdown>0&&GMajor->fg_countdown-GMajor->attend_countdown>=30) {
            Sleep((GMajor->fg_countdown-GMajor->attend_countdown)*1000);
        }
        
        int task_count=2;
        if(currpage==FORTUNEGIFT_DETAIL) task_count=1;     
        //检查任务是否已经执行
        for(int idx=0;idx<task_count;idx++) {
            printf("任务%d:%s\n",idx+1,GMajor->taskstatus[idx]==0?"未完成":"已完成");
            if(GMajor->taskstatus[idx]==0) {
                //执行参与
                pTRZ trz_btn=gettrzbycode(trz,DOU_FORTUNEGIFT_DETAIL_EXECTASK);
                if(!trz_btn) return -1;
                
                char cmd_tapfg[256]={0};
                sprintf(cmd_tapfg,"input tap %d %d",trz_btn->x+(trz_btn->width>>1),trz_btn->y+(trz_btn->height>>1));
                subproc_execmd(NULL,cmd_tapfg);
                Sleep(1200);
                
                //这里实际上可能要break，并执行新的页面判断
            }
        }
        /*
         * 如果两个任务都是已经完成的，点击参与不会自动退出当前页面
         * 该界面的下一步界面需要切回 直播间
         */
        
        char path[256]={0};
        GetModulePath(path);
        //截图，检查当前页面
        int retry_count=0;
        while(retry_count<=5) {
            char cmd3[256]={0};
            sprintf(cmd3,"adb -s %s exec-out screencap -p > %spics\\x.png",GMajor->host,path);
            if(0!=subproc_execmd((char*)"cmd",(char*)cmd3)) {retry_count++;printf("Command Execution failed.\n");Sleep(1000);}
            else break;
                
            Sleep(105);
        }

        char imagepath[256]={0};
        sprintf(imagepath,"%spics\\x.png",path);
        PAGECODE page=GetPageCode(imagepath,trz);
        if(page==FORTUNEGIFT_DETAIL_2TASK||page==FORTUNEGIFT_DETAIL) {
            pTRZ trz_fodr=gettrzbycode(trz,FORTUNEGIFT_OR_DIAMOND_RANGE);
            if(!trz_fodr) return -1;
            pTRZ trz_fc=gettrzbycode(trz,FORTUNEGIFT_CLICKRECT);
            if(!trz_fc) return -1;
            
            char cmd_tapfg[256]={0};
            sprintf(cmd_tapfg,"input tap %d %d",trz_fodr->x+trz_fc->x+(trz_fc->width>>1),trz_fodr->y+trz_fc->y+(trz_fc->height>>1));
            subproc_execmd(NULL,cmd_tapfg);
            Sleep(1040);
        }
        
        int countdown_seconds=GMajor->fg_countdown+GMajor->timestamp-time(NULL);
        if(countdown_seconds>=30) {
            /*
             * 设置倒计时
             */
            if(f->extra) {
                pwfpj_param wfpj=(pwfpj_param)(f->extra);
                SetTimer(wfpj->workflow,FGCOUNTDOWN_TIMERID,750,NULL);
            }
            Sleep((countdown_seconds+1)*1000);
        } else if(countdown_seconds>0) {
            //切回主界面等待30秒
            pTRZ trz_fodr=gettrzbycode(trz,FORTUNEGIFT_OR_DIAMOND_RANGE);
            if(!trz_fodr) return -1;
            pTRZ trz_fc=gettrzbycode(trz,FORTUNEGIFT_CLICKRECT);
            if(!trz_fc) return -1;
            
            char cmd_tapfg[256]={0};
            sprintf(cmd_tapfg,"input tap %d %d",trz_fodr->x+trz_fc->x+(trz_fc->width>>1),trz_fodr->y+trz_fc->y+(trz_fc->height>>1));
            subproc_execmd(NULL,cmd_tapfg);
            Sleep((countdown_seconds+1)*1000);          
        }
    } break;
    case FORTUNEGIFT_NOPRIZE: {
        //关闭页面，继续蹲守
        printf("抱歉未中奖。\n");
        pTRZ trz_close=gettrzbycode(trz,DOU_FORTUNEGIFT_DRAW_NOPRIZE_EXIT);
        if(!trz_close) return -1;
                
        char cmd_tapfg[256]={0};
        sprintf(cmd_tapfg,"input tap %d %d",trz_close->x+(trz_close->width>>1),trz_close->y+(trz_close->height>>1));
        subproc_execmd(NULL,cmd_tapfg);
        Sleep(1400);
    } break;
    case FORTUNEGIFT_JACKPOT: {
        if(GMajor->proto_checked==0) {
            pTRZ trz_proto=gettrzbycode(trz,DOU_FORTUNEGIFT_DRAW_JACKPOT_AFMPROTO);
            if(!trz_proto) return -1;

            char cmd_tapcheck[256]={0};
            sprintf(cmd_tapcheck,"input tap %d %d",trz_proto->x+(trz_proto->width>>1),trz_proto->y+(trz_proto->height>>1));
            subproc_execmd(NULL,cmd_tapcheck);
            Sleep(1400);
        }

        pTRZ trz_afm=gettrzbycode(trz,DOU_FORTUNEGIFT_DRAW_JACKPOT_CFMAWARD);
        if(!trz_afm) return -1;
        char cmd_afmaward[256]={0};
        sprintf(cmd_afmaward,"input tap %d %d",trz_afm->x+(trz_afm->width>>1),trz_afm->y+(trz_afm->height>>1));
        subproc_execmd(NULL,cmd_afmaward);
        Sleep(1400);
    } break;
    case FORTUNEGIFT_AFMAWARD: {
        pTRZ trz_awardafm=gettrzbycode(trz,DOU_FORTUNEGIFT_AWARDFORTUNEGIFT_AFMBTN);
        if(!trz_awardafm) return -1;

        char cmd_awardafm[256]={0};
        sprintf(cmd_awardafm,"input tap %d %d",trz_awardafm->x+(trz_awardafm->width>>1),trz_awardafm->y+(trz_awardafm->height));
        subproc_execmd(NULL,cmd_awardafm);
        Sleep(1400);

        //切换回新的直播间...
        return -1;
    } break;
    case STREAMER_ROOMCLOSED: {

        //蹲守模式，如果直播间关闭，就退出了
        return -1;
    } break;
    default: {} break;
    }
    return 0;
}

void PageAnalysis_Test() {
    char img_file[][256]={
        "./pics/x.png",
        "./pics/a20.png",
        "./pics/jackpot.png",
        "./pics/a3.png",
        "./pics/stream_closed.png",  
    };
    
    int sections_size=sizeof(sections)/sizeof(TRZ);
    pTRZ trz=new TRZ[sections_size];
    memcpy(trz,sections,sizeof(sections));
    
    tesseract::TessBaseAPI* api=new tesseract::TessBaseAPI();    
    
    for(int idx=0;idx<sizeof(img_file)/sizeof(char[256]);idx++) {
        PAGECODE page=GetPageCode(api,img_file[idx],trz);

        switch(page) {
        case FORTUNEGIFT_JACKPOT: 
            fg_draw_jackpot_ocr(api,trz,img_file[idx]);
        break;
        case FORTUNEGIFT_NOPRIZE: 
            fg_draw_noprize_ocr(api,trz,img_file[idx]);
        break;
        case STREAMER_ROOM: 
            streamer_room_ocr(api,trz,img_file[idx]);
        break;
        case STREAMER_ROOMCLOSED: 
            {
                printf("直播已结束.\n");
            }
        break;
        case FORTUNEGIFT_DETAIL: 
            fg_detail_ocr(api,trz,img_file[idx]);
        break;
        case FORTUNEGIFT_DETAIL_2TASK: 
            fg_detail_2task_ocr(api,trz,img_file[idx]);
        break;
        case DIAMOND_DETAIL: 
            //diamond_detail_ocr(api,trz,img_file[idx]);
        break;
        default: 
            printf("Unknown PAGE,image file [%s]\n",img_file[idx]);
        }
    }
    
    ReleaseSections(trz);
    if(api) delete api;
    
}

int opencv_recognize_rectangles(char* filename,void* prc_list,int* prc_count) {
    cv::Mat img=cv::imread(filename);
    if(img.empty()) {
        printf("打开文件%s失败！",filename);
        return -1;
    }

    cv::Mat gray;
    cv::cvtColor(img,gray,cv::COLOR_BGR2GRAY);

    //使用Canny 进行边缘检测,通常灰度范围50~150
    //对于低对比度的图像
    //阈值下限在10~30，上限在30~70
    cv::Mat edges;
    cv::Canny(gray,edges,20,60);

    //寻找轮廓
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(edges,contours,cv::RETR_TREE,cv::CHAIN_APPROX_NONE);

    *prc_count=0;
    //筛选合适的矩形
    for(size_t i=0;i<contours.size();i++) {
        if(contours[i].size()>100) {
            cv::RotatedRect rect=cv::minAreaRect(contours[i]);

            cv::Point2f vertices[4];
            rect.points(vertices);

            float rcWidth=rect.size.width;
            float rcAngle=rect.angle;
            if(rcAngle<-45.0f) rcAngle+=90;

            if(!(abs(rcAngle)<1.0f||abs(rcAngle-90)<1.0f)||rcWidth<30.0f) continue;

            int pos[4]={
                (int)vertices[0].x,(int)vertices[0].y,
                //(int)vertices[1].x,(int)vertices[1].y,
                (int)vertices[2].x,(int)vertices[2].y,
                //(int)vertices[3].x,(int)vertices[3].y
            };

            //验证并传输gdi合法的矩形
            if(pos[0]>pos[2]) {
                int tmp=pos[0];
                pos[0]=pos[2];
                pos[2]=tmp;
            }
            if(pos[1]>pos[3]) {
                int tmp=pos[1];
                pos[1]=pos[3];
                pos[3]=tmp;
            }

            assert(*prc_count<1000*4);
            memcpy(((int*)prc_list)+(*prc_count),pos,sizeof(pos));

            (*prc_count)+=4;
        }
    }

    return 0;
}

/**
 * opencv_recognize_circles
 * 参数
 * pcl_list：圆的列表，int[], int[0]-圆心x,int[1]-圆心y,int[2]-半径R
 * pcl_count: 圆的数量
 * range1<=R<=range2：识别圆的半径范围
 */
int opencv_recognize_circles(char* filename,int range1,int range2,void* pcl_list,int* pcl_count) {
    cv::Mat src = cv::imread(filename);
    if (src.empty()) {
        printf("打开图像[%s]失败!\n",filename);
        return -1;
    }

    // 转换为灰度图像
    cv::Mat gray;
    cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);

    // 对图像进行平滑处理以减少噪声
    cv::GaussianBlur(gray, gray, cv::Size(9, 9), 2, 2);

    // 存储检测到的圆
    std::vector<cv::Vec3f> circles;
    
    //设置检索圆圈的半径大小
    int min_radius=range1;
    int max_radius=range2;
    // 使用霍夫变换检测圆圈
    //cv::HoughCircles(gray, circles, 
    //                 cv::HOUGH_GRADIENT, 
    //                 1, 
    //                 gray.rows/16, 
    //                 100, //Canny边缘检测的最高阈值，更高的阈值导致更少的边缘像素被检测，适合高对比度的边缘图像，通常设置在100-200
    //                 30,  //霍夫变换累加器，即检测出一个圆所需要的最小累加器值，需要不低于此值的圆边缘的检测才会被认定为一个圆。
    //                 min_radius,max_radius);
    
    cv::HoughCircles(gray, circles, cv::HOUGH_GRADIENT, 1, gray.rows/16,100,130,min_radius,max_radius);

    // 在原图上绘制检测到的圆圈
    for (size_t i = 0; i < circles.size(); i++) {
        cv::Vec3i c = circles[i];
        memcpy((int*)pcl_list+i*3,&c[0],sizeof(int)*3);

        //pcl_list[3*i]=c[0];
        //pcl_list[3*i+1]=c[1];
        //pcl_list[3*i+1]=c[2];
    }
    *pcl_count=circles.size();

    return 0;
}

int opencv_recognize_circles(char* filename,void* pcl_list,int* pcl_count) {
    int default_range1=70;
    int default_range2=120;
    return opencv_recognize_circles(filename,default_range1,default_range2,pcl_list,pcl_count);
}

int opencv_savetemplateclips(char* filename,int x,int y,int cx,int cy,char* template_file) {
    cv::Mat img=cv::imread(filename);
    if(img.empty()) {
        printf("打开图片%s失败!\n",filename);
        return -1;
    }

    cv::Rect clipRect(x,y,cx,cy);
    cv::Mat clip=img(clipRect);
    cv::imwrite(template_file,clip);

    return 0;
}

int opencv_savetemplateclips(char* filename,int x,int y,int cx,int cy,PAGECODE page) {
    char template_imgs[][256]={
        "pics\\fortunegift_draw_jackpot_template.png",//中奖模板图
        "pics\\fortunegift_draw_noprize_template.png",//未中奖模板图
        "pics\\streamer_room_template.png",
        "pics\\fortunegift_detail_template.png",
        "pics\\diamond_detail_template.png",
        "pics\\fortunegift_afmaward_template.png",
    };

    PAGECODE pages[]={
        FORTUNEGIFT_JACKPOT,
        FORTUNEGIFT_NOPRIZE,
        STREAMER_ROOM,
        FORTUNEGIFT_DETAIL,
        DIAMOND_DETAIL,
        FORTUNEGIFT_AFMAWARD,
    };

    char path[256]={0};
    GetModulePath(path);
    for(int idx=0;idx<sizeof(template_imgs)/sizeof(char[256]);idx++) {
        char tmp[256]={0};
        sprintf(tmp,"%s%s",path,template_imgs[idx]);
        strcpy(template_imgs[idx],tmp);
    }

    for(int idx=0;idx<sizeof(pages)/sizeof(PAGECODE);idx++) {
        if(page==pages[idx]) {
            return opencv_savetemplateclips(filename,x,y,cx,cy,template_imgs[idx]);
        }
    }
    
    return -1;
}

pMajor initialmajor() {
    pMajor major=(pMajor)calloc(sizeof(Major),1);
    if(major==NULL) return NULL;
    
    //参数初始化
    int sections_size=sizeof(sections)/sizeof(TRZ);
    pTRZ trz=new TRZ[sections_size];
    if(!trz) return NULL;
    memcpy(trz,sections,sizeof(sections));
    major->trz=trz;

    major->trzproj_buffer=(char*)calloc(sizeof(char)*1024,1);
    major->trzproj_list=(char**)calloc(sizeof(char*)*50,1);
    if(!major->trzproj_buffer||!major->trzproj_list) {
        clearmajor(major);
        return NULL;
    }

    major->trzproj_list[0]=major->trzproj_buffer;
    major->activeproj_index=-1;

    return major;
}

void resetmajor(pMajor major) {
    if(major) {
        memset(major->host,0x00,sizeof(major->host));
        major->hasfg=major->alivecount=major->timestamp=major->fg_countdown=0;
        major->currpage=(PAGECODE)0;

        major->taskcount=0;
        major->taskstatus[0]=major->taskstatus[1]=major->taskstatus[2]=0;
        major->proto_checked=0;
    }
}

void clearmajor(pMajor major) {
    if(major) {
        if(major->trz) {

            ReleaseSections(major->trz);

            major->trz=NULL;
        }

        if(major->trzproj_buffer) {
            free(major->trzproj_buffer);
            major->trzproj_buffer=NULL;
        }

        if(major->trzproj_list) {
            free(major->trzproj_list);
            major->trzproj_list=NULL;
        }
        major->trzproj_count=0;

        free(major);
    }
}

// 撤销：原意为对.trz文件的修改，已在ScratchView中实现
// int Major_AlterTRZProj(pMajor major,PAGECODE page,pTRZ list,int listcount) {
//
// }

int Major_AddTRZProj(pMajor major,char* proj) {
    return Major_AddTRZProj(major,proj,0);
}

/*
 * Major_AddTRZProj
 * 功能：添加新的运行方案
 * 参数：
 * writebackindexfile: 0----无需回写TRZPROJINDEXFILE
 *                     !0---必须回写
 */
int Major_AddTRZProj(pMajor major,char* proj,int writebackindexfile) {
    /*
     * 在当前的Major中添加指定的proj
     * 添加major 需要做一些必要的检查：文件是否存在，目录是否存在 
     */
    if(!major||!major->trzproj_list) return -1;

    for(int idx=0;idx<major->trzproj_count;idx++) {
        if(strcmp(proj,major->trzproj_list[idx])==0) {
            /**
             * 方案已经存在
             */

            return 0;
        }
    }
    
    /**
     * 检查方案文件是否存在
     */
    char path[256]={0},projfolder[256]={0},projtrzfile[256]={0};
    GetModulePath(path);
    sprintf(projfolder,"%s%s\\",path,proj);
    if(!PathIsDirectory(projfolder)) return -1;

    sprintf(projtrzfile,"%s%s%s",projfolder,proj,TRZPROJ_EXTENSION);
    DWORD attrib=GetFileAttributes(projtrzfile);
    if(attrib==INVALID_FILE_ATTRIBUTES||(attrib&FILE_ATTRIBUTE_DIRECTORY)) return -1;

    if(writebackindexfile!=0&&major->trzproj_count<=0) {
        char idxfile[256]={0};
        sprintf(idxfile,"%s%s",path,TRZPROJINDEXFILE);
        FILE* file=fopen(idxfile,"a+");
        if(!file) return -1;

        /*
         * 读取记录数 字符串 8个字节
         * 如果文件不存在就需要创建，直接写
         */
        fprintf(file,"%-8s\n","1");
        fwrite(proj,strlen(proj),1,file);

        if(file) fclose(file);
    }

    /*
     * 加入proj列表
     */
    strcpy(major->trzproj_list[major->trzproj_count],proj);
    major->trzproj_count+=1;
    assert(major->trzproj_count<20);
    major->trzproj_list[major->trzproj_count]=major->trzproj_list[major->trzproj_count-1]+(1+strlen(major->trzproj_list[major->trzproj_count-1]));

    if(writebackindexfile!=0&&major->trzproj_count>0) {
        char idxfile[256]={0};
        sprintf(idxfile,"%s%s",path,TRZPROJINDEXFILE);
        FILE* file=fopen(idxfile,"a+");
        if(!file) return -1;
        fprintf(file,"%-8d\n",major->trzproj_count);

        fseek(file, 0, SEEK_END);
        fprintf(file,"\n%s",proj);

        fclose(file);
    }

    return 0;
}

int Major_SetCurrProj(pMajor major,char* proj) {
    if(!major||!major->trzproj_list||major->trzproj_count<=0) return -1;

    for(int idx=0;idx<major->trzproj_count;idx++) 
        if(0==strcmp(proj,major->trzproj_list[idx]))
            return Major_SetCurrProj(major,idx);
    return -1; 
}

/*
 * 默认需要从TRZProjFile中加载方案
 */
int Major_SetCurrProj(pMajor major,int idx) {
    return Major_SetCurrProj(major,idx,1);
}

/*
 * 若当前没有方案，且TRZProjFile尚不存在时，仅从从内存中更新proj ，
 * 则copyfromfile = 0
 */
int Major_SetCurrProj(pMajor major,int idx,int copyfromfile) {
    /*
     * 从文件中读出数据，并应用至 major->smps,major->trz 等
     */
    if(!major||idx>=major->trzproj_count||major->trzproj_list[idx]==NULL) return -1;

    major->activeproj_index=idx;

    if(copyfromfile!=1) return 0;

    char path[256]={0},trzproj_fullname[256]={0};
    GetModulePath(path);

    sprintf(trzproj_fullname,"%s%s\\%s%s",path,major->trzproj_list[idx],major->trzproj_list[idx],TRZPROJ_EXTENSION);
    FILE* file=fopen(trzproj_fullname,"rb");
    if(!file) return -1;

    TRZProjFile trzproj={0};
    fread(&trzproj,sizeof(TRZProjFile),1,file);
    fclose(file);

    /*
     * 此处存在一个隐患：
     * trz::pText 是一个指针，是无法进行拷贝的
     * 此处直接拷贝可能导致内存泄漏
     * 伏笔日期：2024年10月17日
     * -------------------------------------
     * 已试图修复
     */
    if(major->trz&&trzproj.trzs_count>0) {
        int trz_allocated=_msize(major->trz)/sizeof(TRZ);
        for(int idx=0;idx<trz_allocated;idx++) {
            if((major->trz+idx)->pText) {
                free((major->trz+idx)->pText);
                (major->trz+idx)->pText=NULL;
            }
        }

        memcpy(major->trz,&trzproj.trz,trzproj.trzs_count*sizeof(TRZ));
    }

    HWND scratchview=((pwfpj_param)(f->extra))->imgsratch;
    SendMessage(scratchview,MSG_SETSMPSIMGS,(WPARAM)major,(LPARAM)&trzproj);
    return 0;
}

int Major_LoadProj(pMajor major,char* proj) {
    if(!major) return -1;

    if(!proj) {
        /*
         * 从索引文件中加载
         */
        char filepath[256]={0};
        GetModulePath(filepath);
        strcat(filepath,TRZPROJINDEXFILE);
        FILE* file=fopen(filepath,"rb");
        if(!file) return -1;

        int projcount=0;
        fscanf(file,"%d",&projcount);
        assert(projcount<20);
        for(int idx=0;idx<projcount;idx++) {
            char projname[256]={0};
            fscanf(file,"%s",projname);

            if(0!=Major_AddTRZProj(major,projname)) {
                fclose(file);
                file=NULL;
                return -1;
            }
        }
        if(file) fclose(file);
        file=NULL;

        if(major->trzproj_count>0) {
            Major_SetCurrProj(major,0);
        }
    } else {
        /*
         * 检查已加载的方案中是否存在指定的proj
         */
        for(int idx=0;idx<major->trzproj_count;idx++) {
            if(strcmp(proj,major->trzproj_list[idx])==0) {
                return Major_SetCurrProj(major,proj);
            }
        }

        /*
         * 指定方案尚未加载
         */
        if(0!=Major_AddTRZProj(major,proj)) return -1;
        return Major_SetCurrProj(major,proj);
    }
}

int Major_SaveProjs(pMajor major) {
    if(!major) return -1;

    if(major->trzproj_count<=0) {
        /**
         * 将当前的ProjFile 保存
         */
        HWND scratchview=((pwfpj_param)(f->extra))->imgsratch;
        pScratchView psv=ScratchView_GetSettings(scratchview);
        if(psv&&psv->smps_count>0) {
            /*
             * 为当前的 trz/smps 创建TRZProjFile
             */
            
        }
        //创建默认proj
        major->trzproj_count=1;
        sprintf(major->trzproj_list[0],"%s","DefaultProj_Joe");
        major->trzproj_list[1]=major->trzproj_list[0]+(strlen(major->trzproj_list[0])+1);
        major->activeproj_index=0;

        SendMessage(scratchview,MSG_SAVEPROJ,(WPARAM)major,(LPARAM)major->trzproj_list[0]);
    }

    char path[256]={0};
    GetModulePath(path);

    char idxfile[256]={0};
    sprintf(idxfile,"%s%s",path,TRZPROJINDEXFILE);
    FILE* file=fopen(idxfile,"w");
    if(!file) return -1;

    fprintf(file,"%-8d",major->trzproj_count);
    for(int idx=0;idx<major->trzproj_count;idx++) {
        fprintf(file,"\n%s",major->trzproj_list[idx]);
    }
    fclose(file);

    return 0;
}

/*
 * 对于 福袋详情页面
 * 计划采取如下分析思路
 * 1、分析图形，过滤有效矩形
 * 2、按照面积排序，试图找出页面的三个较大面积的矩形（福袋详情、任务列表、任务按钮）
 *    理论上，最大的矩形属于福袋详情，位于 福袋详情标识图的下方
 *    找到福袋详情之后，计算其宽度，以此为基准，筛选宽度类似的其它矩形
 *    
 * 3、分析关注的信息
 *  3.1 找出页面识别标识的位置，从而定位倒计时的 top/bottom，并筛选出有效矩形
 *      将此区间的矩形按照left坐标排序，从中读取数值
 *  3.2 任务列表判断是一步任务还是两部任务...
 * 
 */
/*
 * 我在更新这段代码的时候，在网上看到一段视频
 * 一个医学博士，享受国务院津贴，带课题组，37岁，患三期肺癌，他知道自己没治了，身体很痛苦，内心更加痛苦。
 * 他说他知道自己的情况，他说以他所学和经历，医学根本没有奇迹，他曾跟随云游僧人两天，僧人劝他做有意义的事情，他的跟随只是浪费自己的生命。
 * 他从医一直想做好人，接诊期间累计替病人垫付40余万，无一不是握着他的手说有钱一定还上，他说他累计收回无头账仅仅不到两万。
 * 他在无多的时日里将房子卖掉，觉得钱款没有意义，遂转手将房款转给一个大学同学（女）完成理财指标，就在她几乎已经认为钱被卷走的时候，同学将钱打了回来，甚至还赚了不少...
 * 他强调内心不甘心，他还没有出成果，对不起津贴，没有子嗣，对不起父母，努力奋斗，对不起自己。
 * 他说他资助了一名山区学生，从9岁开始，现今已经大学毕业，正陪着他，他强调自己是个好人，怨念啊...
 * --------------------------------------------------------------------------------------
 * 命运弄人
 */
//面积由大到小
int cmpSize2(const void* a,const void* b) {
    int* pa=(int*)a;int* pb=(int*)b;
    return (abs(pb[2]-pb[0])*abs(pb[3]-pb[1])-abs(pa[2]-pa[0])*abs(pa[3]-pa[1]));
}
//top由低到高 （小到大) return a-b
int cmpPosTop(const void* a,const void* b) {
    return *((int*)a+1)-*((int*)b+1);
}
//left由小到大 return a-b
int cmpPosLeft(const void* a,const void* b) {
    return *((int*)a)-*((int*)b);
}
/*
 * fg_detail_ocr_prepare_dynamic
 * ------------------------------------
 * 该代码需要在福袋详情页面的内容分析前执行，用于确定详情页面的几个关键选区的位置
 */
int fg_detail_ocr_prepare_dynamic(pTRZ list,char* img) {
    /**
     * 更新并获取有效的DOU_FORTUNEGIFT_DETAIL_IMG区域坐标 
     */
    pTRZ trz_fdi=gettrzbycode(list,DOU_FORTUNEGIFT_DETAIL_IMG);
    trz_fdi->x=0;trz_fdi->y=0;
    if(0!=searchtemplate(img,DOU_FORTUNEGIFT_DETAIL_IMG,list)) return -1;

    int rcCount=0;
    int rcList[1000*4]={0};
    if(0!=opencv_recognize_rectangles(img,&rcList,&rcCount)) return -1;

    /*
     * 按照面积进行排序
     */
    rcCount/=4;
    qsort(rcList,rcCount,sizeof(int)*4,cmpSize2);
    /*
     * 检查rcList第一组是否位于trz_fdi 之下
     */
    if(rcList[1]<=trz_fdi->y+trz_fdi->height) {
        printf("设想中的页面识别标识应该位于识别的最大矩形（假定为福袋详情)的上方，但现在似乎并非如此\n");
        return -1;
    }

    /**
     * 筛选宽度类似的矩形
     * 并按照y/height 坐标排序
     */
    int width=rcList[2]-rcList[0];
    cv::Mat image=cv::imread(img);
    int imgwidth=image.cols;
    /* 
     * 1000 pixls ,偏差 30 pixls的冗余 即 0.015
     */
    float range_dense=0.050f;
    int width_range[2]={
        (width/(float)imgwidth-range_dense)*imgwidth,
        (width/(float)imgwidth+range_dense)*imgwidth
    };

    int filtered[1000*4]={0};
    int rcFiltered=0;
    for(int idx=0;idx<rcCount;idx++) {
        if(rcList[idx*4+2]-rcList[idx*4]>=width_range[0]&&
           rcList[idx*4+2]-rcList[idx*4]<=width_range[1]&&
           rcList[idx*4+1]>rcList[3]&&
           rcList[idx*4+3]-rcList[idx*4+1]>0) {
            /*
             * 可用的矩形
             */
            memcpy(&filtered[rcFiltered*4],&rcList[idx*4],sizeof(int)*4);
            rcFiltered++;
        }
    }
    /**
     * 这些矩形可能是嵌套交叉的，要排除掉嵌套交叉的
     * 按照 top 坐标排序，从小到大，如果相邻的两个矩形存在包含或者大面积交叉
     * 包含-选取较大的那一个 
     * 大面积交叉-选取并集 ？
     * 
     */
    qsort(filtered,rcFiltered,sizeof(int)*4,cmpPosTop);
    
    int filtered2[1000*4]={rcList[0],rcList[1],rcList[2],rcList[3]};
    int rcFiltered2=1;
    for(int idx=1;idx<rcFiltered;idx++) {
        if(abs(filtered[idx*4+1]-filtered2[(rcFiltered2-1)*4+1])<=range_dense*imgwidth*2||
           abs(filtered[idx*4+3]-filtered2[(rcFiltered2-1)*4+3])<=range_dense*imgwidth*2
        ) {
            //两者相交或包含,覆写
            int x1[2]={
                filtered[idx*4],filtered[idx*4+2]
            },
            y1[2]={
                filtered[idx*4+1],filtered[idx*4+3]
            },
            x2[2]={
                filtered2[(rcFiltered2-1)*4],filtered2[(rcFiltered2-1)*4+2]
            },
            y2[2]={
                filtered2[(rcFiltered2-1)*4+1],filtered2[(rcFiltered2-1)*4+3]
            };

            filtered2[(rcFiltered2-1)*4]=x1[0]<x2[0]?x1[0]:x2[0];
            filtered2[(rcFiltered2-1)*4+1]=y1[0]<y2[0]?y1[0]:y2[0];
            filtered2[(rcFiltered2-1)*4+2]=x1[1]>x2[1]?x1[1]:x2[1];
            filtered2[(rcFiltered2-1)*4+3]=y1[1]>y2[1]?y1[1]:y2[1];
        } else {
            rcFiltered2++;
            memcpy(&filtered2[(rcFiltered2-1)*4],&filtered[idx*4],sizeof(int)*4);
        }
    }

    if(rcFiltered2==3) {
        /*
         * 判断是一步任务还是两步任务
         */
        //回写 选区TRZ
        pTRZ trz_tk=gettrzbycode(list,DOU_FORTUNEGIFT_TASKRECT);
        if(!trz_tk) return -1;
        trz_tk->x=filtered2[1*4];
        trz_tk->y=filtered2[1*4+1];
        trz_tk->width=filtered2[1*4+2]-trz_tk->x;
        trz_tk->height=filtered2[1*4+3]-trz_tk->y;
        
        pTRZ trz_taskbtn=gettrzbycode(list,DOU_FORTUNEGIFT_DETAIL_EXECTASK);
        if(!trz_taskbtn) return -1;
        trz_taskbtn->x=filtered2[2*4];
        trz_taskbtn->y=filtered2[2*4+1];
        trz_taskbtn->width=filtered2[2*4+2]-trz_taskbtn->x;
        trz_taskbtn->height=filtered2[2*4+3]-trz_taskbtn->y;


        /*
         * 处理倒计时
         */
        int timefiltered[1000*4]={0};
        int rcTimeFiltered=0;
        for(int idx=0;idx<rcCount;idx++) {
            if(filtered2[1]>rcList[idx*4+3]&&
               trz_fdi->y+trz_fdi->height<rcList[idx*4+1]) {
                memcpy(&timefiltered[rcTimeFiltered*4],&rcList[idx*4],sizeof(int)*4);
                rcTimeFiltered++;
            }
        }
        qsort(timefiltered,rcTimeFiltered,sizeof(int)*4,cmpPosLeft);

        int right=0;
        int time_fetched=0;
        for(int idx=0;idx<rcTimeFiltered;idx++) {
            if(timefiltered[idx*4]>right) {
                time_fetched++;
                right=timefiltered[idx*4+2];
                if(time_fetched==1) {
                    pTRZ trz_hour=gettrzbycode(list,DOU_FORTUNEGIFT_COUNTDOWN_DETAIL);
                    trz_hour->x=timefiltered[idx*4];
                    trz_hour->y=timefiltered[idx*4+1];
                    trz_hour->width=timefiltered[idx*4+2]-timefiltered[idx*4];
                    trz_hour->height=timefiltered[idx*4+3]-timefiltered[idx*4+1];
                } else if(time_fetched==2) {
                    pTRZ trz_sec=gettrzbycode(list,DOU_FORTUNEGIFT_COUNTDOWN_DETAIL2);
                    trz_sec->x=timefiltered[idx*4];
                    trz_sec->y=timefiltered[idx*4+1];
                    trz_sec->width=timefiltered[idx*4+2]-timefiltered[idx*4];
                    trz_sec->height=timefiltered[idx*4+3]-timefiltered[idx*4+1];
                }
            }
            if(time_fetched>=2) break;
        }
        return 0;

    } else {
        /*
         * 设想的场景是此处只有两个矩形，任务矩形 和 执行任务按钮的矩形
         */
        return -1;
    }
}

/** 
 * watchlist_ocr
 * 对关注列表进行识别，筛选出正在直播的主播列表
 * 正在直播的主播列表：
 * 包含直播间的名称、直播列表的位置等
 * 直播间的名称列表，后续可能据此配置直播间的切换策略
 * 直播列表的位置，用于执行切换直播间的操作（划屏和点击等）
 */
int watchlist_ocr(pTRZ list,char* img) {
    /*
     * 处理思路：
     * 1、打开关注列表的图片，分析关注的直播间列表，筛选出直播的主播，记录列表、偏移
     * 2、通过某种策略选中一个符合要求的直播间，点击切换
     * --------------------------------------------
     * 
     * 当前的页面偏移，首先需要定义一个偏移基准，即页面最初的状态，
     * 紧接着筛选关注列表
     * 如果列表较长，需要滑动屏幕，一旦滑动，偏移即发生变化
     * 在滑动的过程中，确定关注列表中的所有直播间，并严格建立此时此刻的各个直播间列表与偏移的对应关系
     * 严格来讲，将向滚动条一样，建立pagesize/min/max/pos映射
     * 
     * --------------------------------------------
     * 
     */

    return 0;
}