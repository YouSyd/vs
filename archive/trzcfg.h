/*
 * TRZ 选区配置界面
 */

#include "trzcfg_ctrl.h"

#include "dialog.h"
#include "frame.h"
#include "button.h"

#include "tesseract_test.h"
#include "img_scratch.h"

INT_PTR WINAPI TRZCfgHandle(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
int InitialCtrls(HWND hwnd);

extern TRZ trzcfg;
