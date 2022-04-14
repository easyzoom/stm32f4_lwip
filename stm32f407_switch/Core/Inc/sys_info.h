#ifndef __SYS_INFO_H__
#define __SYS_INFO_H__


#include <stdint.h>

//系统支持的网口数量，SW_PORTS_NUMBER = 外接网口数量 + 1(此为MCU连接交换机的Port0,外接网口从Port1开始)
#define SW_PORTS_NUMBER                     5
#define SW_SOFTWARE_VERSION                 "V1.1 Build "__DATE__" "    //不大于26 byte
#define SW_HARDWARE_VERSION                 "ROB042_MB201_V1.0"         //不大于20 byte
#define SW_DEV_NAME                         "ROB042_MB201"              //不大于16 byte

#define DEV_NAME                "desc"
#define MAC_SNAME               "mac"
#define IP_SNAME                "ip"
#define NETMASK_SNAME           "nm"
#define GATEWAY_SNAME           "gw"
#define SW_SNAME                "sw_ver"
#define HW_SNAME                "hw_ver"
#define PVL_STATE               "pvl_s"
#define PVLT_GR                 "tb"
#define CHECK_CRC               "ck_crc"




#define CRC16_TABLE_CODE        5
#define FILE_MAX_BYTE           256

typedef struct
{
    int16_t     flag;
    char        file[FILE_MAX_BYTE];
}str_sysfile_ram;

extern str_sysfile_ram file_ram_bsif;
extern str_sysfile_ram file_ram_pmng;

extern char dev_name[16];
extern char sw_ver[26];
extern char hw_ver[20];


typedef struct
{
    char desc[16];
    char mac[18];
    char ip[16];
    char netmask[16];
    char gateway[16];
    char sw_ver[16];
    char hw_ver[16];
}str_basic_info;


typedef struct
{
    str_basic_info basic_info;
}str_sys_info;


extern uint8_t sys_info_config(uint8_t state);
extern int8_t binfo_file_update(void);
extern int8_t binfo_file_update_from_rambuf(char *buf,uint16_t len);
extern int8_t binfo_data_update(uint32_t size);
extern int8_t portmng_parameter_update(uint32_t size);
extern int8_t portmng_file_update(void);
extern int8_t portmng_file_update_from_rambuf(char *buf,uint16_t len);
extern int8_t strIP4_to_array(uint8_t* dec,const char* src);
extern int8_t checkIPformat(const char* src);
extern int8_t strMAC_to_array(uint8_t* dec,const char* src);
extern int8_t checkMACformat(const char* src);

#endif /*__SYS_INFO_H__*/


