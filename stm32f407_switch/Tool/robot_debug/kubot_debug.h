#ifndef __DEBUG_H
#define __DEBUG_H

#include <stdio.h>
#include <string.h>
#include "main.h"
/*
字背景颜色范围: 40--49     字颜色: 30--39
40: 黑                           30: 黑
41: 红                           31: 红
42: 绿                           32: 绿
43: 黄                           33: 黄
44: 蓝                           34: 蓝
45: 紫                           35: 紫
46: 深绿                         36: 深绿
47: 白色                         37: 白色
*/
#define  BLACK   "30"
#define  RED     "31"
#define  GREEN   "32"
#define  YELLOW  "33"
#define  BLUE    "34"
#define  MAGENTA "35"
#define  CYAN    "36"
#define  WHITE   "37"

#if 1
#define LOG_DEBUG(color,format,...) \
do{\
  printf("\033[40;");\
  printf(color);\
  printf("m");\
  printf("File: ");\
  printf(__FILE__);\
  printf(", Line: %05d: ",__LINE__);\
  printf(format, ##__VA_ARGS__);\
  printf("\033[0m\n");\
}while(0)
#else
#define LOG_DEBUG(format,...)
#endif

#ifdef MODULE_ENB_DEBUG_PRINT
#define LOG_PRINT(color,format,...)\
do{\
  printf("\033[40;");\
  printf(color);\
  printf("m");\
  printf(format, ##__VA_ARGS__);\
  printf("\033[0m");\
}while(0)
#else
#define LOG_PRINT(color,format,...)
#endif

#define LOG_PRINT_DEBUG(format, ...)\
        LOG_PRINT(YELLOW, format, ##__VA_ARGS__)
#define LOG_PRINT_ERROR(format, ...)\
        LOG_PRINT(RED, format, ##__VA_ARGS__)
#define LOG_PRINT_INFO(format, ...)\
        LOG_PRINT(MAGENTA, format, ##__VA_ARGS__)
#define LOG_PRINT_WARIN(format, ...)\
        LOG_PRINT(MAGENTA, format, ##__VA_ARGS__)

#endif /* __DEBUG_H */
