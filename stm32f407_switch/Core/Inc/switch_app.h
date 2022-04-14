#ifndef APPLICATIONS_SWITCH_APP_H_
#define APPLICATIONS_SWITCH_APP_H_

#include "kubot_debug.h"
#include "main.h"
#include "Marvell_88E6390_inc.h"

#define MAX_SWITCH_PORTS    (SW_PORTS_NUMBER)
#define MAX_PVLT_MEM        (SW_PORTS_NUMBER-1)

#define    EGRESS_INGOOD_STAT_L    Peridot_STATS_InGoodOctetsLo
#define    EGRESS_INGOOD_STAT_H    Peridot_STATS_InGoodOctetsHi
#define    EGRESS_INBAD_STAT        Peridot_STATS_InBadOctets
#define    EGRESS_OUT_STAT_L        Peridot_STATS_OutOctetsLo
#define    EGRESS_OUT_STAT_H        Peridot_STATS_OutOctetsHi

typedef enum
{
    PORT_LKS_DISCONNECT = 0,
    PORT_LKS_AUTOCONNECT = 1,
    PORT_LKS_10M_HALF_DUPLEX= 2,
    PORT_LKS_10M_FULL_DUPLEX= 3,
    PORT_LKS_100M_HALF_DUPLEX = 4,
    PORT_LKS_100M_FULL_DUPLEX = 5,
    PORT_LKS_200M_HALF_DUPLEX = 6,
    PORT_LKS_200M_FULL_DUPLEX = 7,
    PORT_LKS_1000M_FULL_DUPLEX = 8,
    PORT_LKS_UNKNOWN = 9
} PORT_LINK_INFO;

typedef enum
{
    LINK_DOWN = 0,
    LINK_UP = 1,
}em_LINK_STATE;

typedef enum
{
    HALF_DUPLEX = 0,
    FULL_DUPLEX = 1,
}em_DUPLEX_MODE;

typedef enum
{
    SPEED_10_MBPS = 0,
    SPEED_100_MBPS = 1,
    SPEED_200_MBPS = 2,
    SPEED_1000_MBPS = 3,
    SPEED_2_5_GBPS = 4,
    SPEED_10_GBPS = 6,
    SPEED_UNKNOWN = 7
} em_PORT_SPEED;

typedef struct
{
    uint32_t    InGoodOctetsLo;
    uint32_t     InGoodOctetsHi;
    uint32_t     InBadOctets;
    uint32_t    OutOctetsLo;   
    uint32_t    OutOctetsHi;   
}str_port_count;

typedef struct
{
    uint32_t sectotal_ibyte;
    uint32_t sectotal_obyte;
    uint32_t sectotal_byte;
    uint32_t ibad_val;
    uint8_t ibad_unit;
    uint32_t txs_val;
    uint8_t txs_unit;
    uint32_t rxs_val;
    uint8_t rxs_unit;
    uint32_t thrput_val;
    uint8_t thrput_unit;
}str_throughput;

typedef struct
{
    uint8_t    pvl_state;
    int32_t pvl_t[MAX_SWITCH_PORTS];
    int32_t lk_set[MAX_SWITCH_PORTS];
    str_throughput throughput[MAX_SWITCH_PORTS];
    
}str_portmng;

typedef struct
{
    uint8_t update;                              //1：待设置，0：已设置
    uint8_t memlen;                              //网口下划分vlan的网口数量
    uint32_t memport[MAX_SWITCH_PORTS];    //vlan网口号
}str_port_VLAN_member;


typedef struct
{
    em_LINK_STATE               link_state; //连接状态
    em_PORT_SPEED               port_speed; //连接速度
    em_DUPLEX_MODE              duplex_mode;//双工状态
    uint8_t                     phy_detect; //phy连接状态
    str_port_VLAN_member        vlan_table; //寄存器读取的VLAN成员
}str_switch_link_info;

typedef struct
{
    PERIDOT_MSD_ELIMIT_MODE mode;
    uint32_t egress_rate;
}str_port_egress;

typedef struct
{
    str_switch_link_info        info;       //通过寄存器读取
    str_port_count                 egress_counter;
    str_port_egress                egressrate_creg;
}str_switch_from_reg_info;

typedef struct
{
    str_switch_from_reg_info    from_reg;   //通过寄存器读取的状态及参数放这里面
    str_port_VLAN_member        vlan_table; //待设置进寄存器的VLAN成员
}str_switch_port;

typedef struct
{
    uint8_t                     IC_cnt_sta;
    str_portmng                    portmng;
    uint8_t                        lk_s[MAX_SWITCH_PORTS];
    uint16_t                    pvl_mem[MAX_SWITCH_PORTS];
    str_switch_port             switch_port[MAX_SWITCH_PORTS];
}str_switch;




extern str_switch switch_data;

extern void switch_timer(uint8_t timer_ms);
extern void switch_init(void);
extern uint16_t switch_cfg(void);
extern uint8_t pvl_table_to_port(void);
extern uint8_t switch_app(void);
extern uint8_t switch_port_vlan_update(uint8_t port,uint8_t* meb,uint8_t memlen);
extern int8_t switch_vlantable_insert(uint16_t* table_data,uint8_t* table_num,uint8_t len);
extern void switch_reset(void);
extern uint8_t hexstr_to_int(char *src, uint32_t* data);
#endif /* APPLICATIONS_SWITCH_APP_H_ */
