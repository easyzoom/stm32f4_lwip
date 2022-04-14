/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-03-31     Lenovo       the first version
 */
#ifndef APPLICATIONS_MARVELL_88E6390_TYPE_H_
#define APPLICATIONS_MARVELL_88E6390_TYPE_H_

#include <stdint.h>
#include "sys_info.h"
#include "kubot_debug.h"

#undef IN
#define IN
#undef OUT
#define OUT
#undef INOUT
#define INOUT


#ifndef NULL
#define NULL ((void*)0)
#endif

typedef void  MSD_VOID;
typedef char  MSD_8;
typedef short MSD_16;
typedef long  MSD_32;

typedef uint8_t  MSD_U8;
typedef uint16_t MSD_U16;
typedef uint32_t  MSD_U32;
typedef uint32_t  MSD_UINT;
typedef uint64_t  MSD_U64;

typedef double MSD_DOUBLE;
typedef unsigned int  MSD_ADV_VCT_PEAKDET_HYST;

typedef enum
{
  MSD_FALSE = 0,
  MSD_TRUE = 1
} MSD_BOOL;

//Íø¿ÚÊýÁ¿
#define     MSD_MAX_SWITCH_PORTS             SW_PORTS_NUMBER

/* status / error codes */
typedef int MSD_STATUS;

#define MSD_OK              (0x00)   /* Operation succeeded                   */
#define MSD_FAIL            (0x01)   /* Operation failed                    */
#define MSD_BAD_PARAM       (0x04)   /* Illegal parameter in function called  */
#define MSD_NO_SUCH         (0x0D)   /* No such item                    */
#define MSD_NOT_SUPPORTED   (0x10)   /* This request is not support           */
#define MSD_ALREADY_EXIST   (0x1B)   /* Tried to create existing item         */
#define MSD_BAD_CPU_PORT    (0x20)   /* Input CPU Port is not valid physical port number */
#define MSD_FEATURE_NOT_ENABLE  (0x40) /*The feature not been enabled when do operation*/

typedef MSD_U32 MSD_LPORT;
typedef MSD_U32 MSD_ETYPE;
typedef MSD_U32 MSD_SEM;

#define MSD_DBG_ERROR(format, ...)\
        LOG_PRINT(YELLOW, format, ##__VA_ARGS__)
#define MSD_DBG_INFO(format, ...)\
        LOG_PRINT(RED, format, ##__VA_ARGS__)
#define MSD_DBG(format, ...)\
        LOG_PRINT(MAGENTA, format, ##__VA_ARGS__)

/* The following macro converts a binary    */
/* value (of 1 bit) to a boolean one.       */
/* 0 --> MSD_FALSE                           */
/* 1 --> MSD_TRUE                            */
#define MSD_BIT_2_BOOL(binVal,boolVal)                                  \
            (boolVal) = (((binVal) == 0) ? MSD_FALSE : MSD_TRUE)
/* The following macro converts a boolean   */
/* value to a binary one (of 1 bit).        */
/* MSD_FALSE --> 0                           */
/* MSD_TRUE --> 1                            */
#define MSD_BOOL_2_BIT(boolVal,binVal)                                  \
            (binVal) = (((boolVal) == MSD_FALSE) ? 0 : 1)

#define msdDisplayStatus(status)                                                         \
                        (status == MSD_OK) ? "MSD_OK" :                                  \
                        (status == MSD_FAIL) ? "MSD_FAIL" :                              \
                        (status == MSD_BAD_PARAM) ? "MSD_BAD_PARAM" :                    \
                        (status == MSD_NO_SUCH) ? "MSD_NO_SUCH" :                        \
                        (status == MSD_NOT_SUPPORTED) ? "MSD_NOT_SUPPORTED" :            \
                        (status == MSD_ALREADY_EXIST) ? "MSD_ALREADY_EXIST" :            \
                        (status == MSD_FEATURE_NOT_ENABLE) ? "MSD_FEATURE_NOT_ENABLE" :  \
                        "UnknowStatus"

#endif /* APPLICATIONS_MARVELL_88E6390_TYPE_H_ */
