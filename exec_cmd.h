/*
 * 整个过程实际上可以使用 标准C的 popen 完成，但是
 * 这里用win32的pipe自己尝试了一把
 */

#ifndef EXEC_CMD_HEADER
#define EXEC_CMD_HEADER

#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>

/*
 * 功能：子进程标准输出的内容进行分析回调
 * 参数：
 * --------------------------------------
 *  pEP     pExecPipes 指针
 *  param   附加参数
 */
typedef void (*subproc_analysis)(int exitcode,void* pEP,void* param);
typedef struct exec_pipes{
    HANDLE subproc_in_wt;
    HANDLE subproc_in_rd;
    HANDLE subproc_out_wt;
    HANDLE subproc_out_rd;
    
    char* output_buffer;
    size_t length;
    
    HANDLE subproc;
    HANDLE subthread;
    DWORD subproc_id;
    
    subproc_analysis analysis;
    //void* param; //参数外置

    time_t begin;
    time_t end;
    int ret;

    char* input_buffer;
    size_t input_len;

    //char* output_buffer;
    size_t output_len;
    
}RExecPipes,*pExecPipes;

int initialsubproc(pExecPipes p,char* cmd_line);
int initialsubproc(pExecPipes p);
void releasesubproc(pExecPipes p);

int subproc_execmd(char* cmd_line,char* cmd_buffer);
int subproc_execmd(char* cmd_line,char* cmd_buffer,void* cb_param);
int subproc_execmd(char* cmd_line,char* cmd_buffer,subproc_analysis analysis,void* cb_param);

/*
 * 日志格式
 * guid \0 40+1
 * exec_begin_time_t\0 yyyy-mm-dd HH:MI:SS  19+1
 * exec_end_time_t\0 19+1
 * ret\0  8+1 ( 16进制)
 * content_length  8+1 :content length的长度为 length 自身 8+1， input_len_string 8+1 ， input_length + 1 , output_len_string 8+1 , output_length + 1
 * ----------------------------------
 * input_len \0
 * output_len \0
 * input string \0
 * output_string \0
 * 
 * 可以发现， ·-----------·之上的部分，为固定长度，后面的长度，可以写入一个数值
 * 以提升读取时的访问速度
 * 
 */
typedef struct exec_cmd_log_header {
    char guid[41];
    char begin[20];
    char end[20];
    int ret;
    int content_length;
} CmdExecLogHeader,CELHeader,*pCELHeader;

typedef struct exec_cmd_log {
    CELHeader header;
    char input[256];
    char output[256];
    //char* buffer;
    // char* input;
    // char* output;
    // char* buffer;
} CmdExecLog,CEL,*pCEL;

int FetchCELRecord(char* filename,pCEL list,int* pfetch_count,int count,const char* guid);
const char* CELHeaderParse(const char* plt,pCELHeader header);
const char* CELParse(const char* plt,pCEL cel);
void logpipe_exec(int exitcode,void* pEPParam,void* param);

#endif
