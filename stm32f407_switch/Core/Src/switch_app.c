#include "switch_app.h"
#include "sys_info.h"
#include "lwip.h"
#include "delay.h"

uint8_t switch_PHY_detect(uint8_t portNum);
void switch_everyPHY_detect(void);
uint8_t switch_IC_connect_state(void);
void switch_port_VLAN_table(void);
void switch_port_link_sta(void);
void switch_port_duplex_sta(void);
void switch_port_speed_sta(void);
void switch_port_vlan_table_get(void);
void switch_reset(void);
void switch_port_egress_init(void);
void switch_port_egress_get(void);
void switch_port_egress_counter(void);
void switch_throughput(void);

str_switch         switch_data;
volatile uint16_t step_timer = 0;
str_port_count port_count[MAX_SWITCH_PORTS];
void switch_timer(uint8_t timer_ms)
{
    step_timer+=timer_ms;
}
/**
 * @brief 交换机初始化相关
 * @param 
 * @return 
 */
void switch_init(void)
{
    uint16_t i = 0;
    
    /******     初始化数据结构             ******/
    LOG_PRINT_INFO("System data struct init...\r\n");
    //数据初始清零
    memset(&switch_data,0,sizeof(switch_data));
    memset(&port_count,0,sizeof(port_count));
    
    switch_data.portmng.pvl_state = 1;
    //port link mode default 
    switch_data.portmng.lk_set[0]=PORT_LKS_100M_FULL_DUPLEX;
    //vlan table default
    for(i=1; i<MAX_SWITCH_PORTS; i++)
    {
        switch_data.portmng.lk_set[i]=PORT_LKS_AUTOCONNECT;
    }
    
    for(i=0; i<MAX_PVLT_MEM; i++)
    {
        switch_data.portmng.pvl_t[0] |= 0x01<<i;
    }
    
    
}
/**
 * @brief 将VLAN table转换成port VLAN table
 * @param 
 * @return 
 */
uint8_t pvl_table_to_port(void)
{
    uint8_t  i=0,j=0,n=0,m=0,state = 0;
    int32_t    t_value = 0,value = 0xffffffff;
    uint8_t mp[MAX_SWITCH_PORTS]={0};
    uint16_t    max_mb = 0;
    
    for(i=0; i<MAX_SWITCH_PORTS; i++)
    {
        if(i != 0)
        {
            value = 1;
            for(j=0; j<MAX_PVLT_MEM; j++)
            {
                t_value = switch_data.portmng.pvl_t[j]<<1;
                if(t_value&(0x01<<i))
                {
                    value |= t_value;
                }
            }
        }
        else
        {
            max_mb = ~(0xffff>>MAX_SWITCH_PORTS);
            max_mb = max_mb>>(16-MAX_SWITCH_PORTS);
            value = max_mb;
        }
        value = value&(~(0x0001<<i));
        if(value != switch_data.pvl_mem[i])
        {
            switch_data.pvl_mem[i] = value;
            m = 0;
            for(n=0; n<MAX_SWITCH_PORTS; n++)
            {
                if(value&(0x01<<n))
                {
                    mp[m] = n;
                    m++;
                }
            }
            state = switch_port_vlan_update(i,mp,m);
            if(state)
            {
                return state;
            }
        }
    }

    return 0;
}

/**
 * @brief 交换机初始化相关
 * @param 
 * @return 
 */
uint16_t switch_cfg(void)
{
    MSD_STATUS sta = 0;
    uint16_t i = 0,j= 0;

    /******     通过读取设备ID判断寄存器读写正常与否             ******/
    //Offset: 0x03 0x390 for the 88E6390 – PQFP package
    LOG_PRINT_INFO(" check switch IC connect...\r\n");
    sta = switch_IC_connect_state();
    if (sta != 0)
    {
        LOG_PRINT_INFO("Switch IC connect FAULTURE\r\n");
    }
    else
    {
        LOG_PRINT_INFO("Switch IC connect success\r\n");
    }

    /******     轮询每个端口的PHY连接是否正常             ******/
    LOG_PRINT_INFO(" check port x PHY connect...\r\n");
    switch_everyPHY_detect();

    /******     port0的phy detect字段需要软件设置             ******/
    LOG_PRINT_INFO(" setting port0 phy detect field...\r\n");
    if(switch_data.switch_port[0].from_reg.info.phy_detect)
    {
        msdSetAnyRegField(0,0,12,1,1);
    }
    else
    {
        msdSetAnyRegField(0,0,12,1,0);
    }
    /******     PHY初始化             ******/
    LOG_PRINT_INFO(" port x PHY init...\r\n");
    for(j = 0;j<MAX_SWITCH_PORTS;j++)
    {
        if(!(switch_data.portmng.lk_set[j]))
        {
            LOG_PRINT_INFO("Port %d is close, PHY init Failed\r\n",j);
            continue;
        }
        //进入Forwarding模式
        sta = Peridot_gprtSetPortState(j,Peridot_PORT_STATE_FORWARDING);
        if (sta != MSD_OK)
        {
            LOG_PRINT_ERROR("Phy register%d init status:%s\n",j, msdDisplayStatus(sta));
        }
    }

    /******     划分Port VLAN             ******/
    LOG_PRINT_INFO(" setting port VLAN table...\r\n");
    switch_port_VLAN_table();
    
    /******     限速设置 预留功能            ******/
    //switch_port_egress_init();
    
    /******     Power up            ******/
    LOG_PRINT_INFO(" PHY power up...\r\n");
    for(j = 0;j<MAX_SWITCH_PORTS;j++)
    {
        if(!(switch_data.portmng.lk_set[j]))
        {
            LOG_PRINT_INFO("Port %d is close, Power up Failed\r\n",j);
            continue;
        }
        for(i=0;i<32;i++)
        {
            delay_us(100);//rt_hw_us_delay(100);
        }
        Peridot_gphyPortPowerDown(j,MSD_FALSE);
    }

    delay_ms(1000);
    
    LOG_PRINT_INFO("The switch_cfg() END\r\n");

    return 0;
}


/**
 * @brief 交换机的主应用程序
 * @param 
 * @return 
 */
#define SWITCH_PERIOD    5
#define SWITCH_TASK_NUM    5
uint8_t switch_app(void)
{
    uint8_t  i = 0;
    uint16_t timer_perio = 0;
    static uint16_t cnt[SWITCH_TASK_NUM] = {0};
    static uint8_t step = 0;
    static uint16_t step_perio_back = 0;

    timer_perio = step_timer / (SWITCH_PERIOD);
    if (step_perio_back != timer_perio)
    {
        if(timer_perio < step_perio_back)
        {
            step_perio_back = timer_perio;
        }
        else step_perio_back++;
        switch(step)
        {
            case 0:
                /******     每隔1秒获取每个 port PHY的连接状态           ******/
                cnt[step]++;
                if(cnt[step]*SWITCH_PERIOD*SWITCH_TASK_NUM >= 1000)
                {
                    cnt[step] = 0;
                    switch_IC_connect_state();
                    switch_everyPHY_detect();
                    //LOG_PRINT_INFO("%d,%d,%d,%d\r\n",step,step_perio_back,timer_perio,step_timer);
                }
                if(step < SWITCH_TASK_NUM-1)step++;
                else step = 0;
                break;
            case 1:
                /******     每隔0.4秒获取每个 port link状态           ******/
                cnt[step]++;
                if(cnt[step]*SWITCH_PERIOD*SWITCH_TASK_NUM >= 400)
                {
                    cnt[step] = 0;
                    switch_port_link_sta();
                }
                if(step < SWITCH_TASK_NUM-1)step++;
                else step = 0;
                break;
            case 2:
                /******     每隔0.5秒获取每个 port 的双工及带宽           ******/
                cnt[step]++;
                if(cnt[step]*SWITCH_PERIOD*SWITCH_TASK_NUM >= 500)
                {
                    cnt[step] = 0;
                    switch_port_duplex_sta();
                    switch_port_speed_sta();
                    for(i=0; i<MAX_SWITCH_PORTS; i++)
                    {
                        if(switch_data.switch_port[i].from_reg.info.link_state)
                        {
                            switch(switch_data.switch_port[i].from_reg.info.port_speed)
                            {
                                case SPEED_10_MBPS:
                                    switch_data.lk_s[i] = PORT_LKS_10M_HALF_DUPLEX;
                                    break;
                                case SPEED_100_MBPS:
                                    switch_data.lk_s[i] = PORT_LKS_100M_HALF_DUPLEX;
                                    break;
                                case SPEED_200_MBPS:
                                    switch_data.lk_s[i] = PORT_LKS_200M_HALF_DUPLEX;
                                    break;
                                case SPEED_1000_MBPS:
                                    switch_data.lk_s[i] = PORT_LKS_1000M_FULL_DUPLEX;
                                    break;
                                default:
                                    switch_data.lk_s[i] = PORT_LKS_UNKNOWN;
                                    break;
                            }
                            if(switch_data.lk_s[i] < PORT_LKS_1000M_FULL_DUPLEX)
                            {
                                if(switch_data.switch_port[i].from_reg.info.duplex_mode == FULL_DUPLEX)
                                {
                                    switch_data.lk_s[i] += 1;
                                }
                            }
                        }
                        else
                        {
                            switch_data.lk_s[i] = PORT_LKS_DISCONNECT;
                        }            
                    }
                }
                if(step < SWITCH_TASK_NUM-1)step++;
                else step = 0;
                break;
            case 3:
                /******     每隔0.5秒 vlan table     ******/
                cnt[step]++;
                if(cnt[step]*SWITCH_PERIOD*SWITCH_TASK_NUM >= 500)
                {
                    cnt[step] = 0;
                    switch_port_vlan_table_get();
                    switch_port_VLAN_table();
                }
                if(step < SWITCH_TASK_NUM-1)step++;
                else step = 0;
                break;
            case 4:
                cnt[step]++;
                if(cnt[step]*SWITCH_PERIOD*SWITCH_TASK_NUM >= 1000)
                {
                    cnt[step] = 0;
                    switch_port_egress_counter();
                    switch_throughput();
                }
                if(step < SWITCH_TASK_NUM-1)step++;
                else step = 0;
                break;
            default:
                step=0;
            break;
        }
    }
    return 0;
}

void switch_reset(void)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
    delay_ms(30);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);
    delay_ms(500);
}

/**
 * @brief port VLAN update
 * @param 
 * @return 
 */
uint8_t switch_port_vlan_update(uint8_t port,uint8_t* meb,uint8_t memlen)
{
    uint8_t i = 0;

    if(port >= MAX_SWITCH_PORTS)
    {
        LOG_PRINT_INFO("switch_port_vlan_update: port too large\r\n");
        return 1;
    }

    for(i=0; i<memlen; i++)
    {
        if(meb[i] >= MAX_SWITCH_PORTS)
        {
            LOG_PRINT_INFO("switch_port_vlan_update: member port too large\r\n");
            return 2;
        }
    }

    switch_data.switch_port[port].vlan_table.update = 1;
    switch_data.switch_port[port].vlan_table.memlen = memlen;

    for(i=0; i<memlen; i++)
    {
        switch_data.switch_port[port].vlan_table.memport[i] = meb[i];
    }

    
    return 0;
}


/**
 * @brief 划分port VLAN
 * @param 
 * @return 
 */
void switch_port_VLAN_table(void)
{
    static uint8_t flag = 0;
    uint8_t j = 0,i = 0,n = 0,m = 0;
    uint8_t mbl = 0;
    MSD_U32 mb[16] = {0};

    if(switch_data.portmng.pvl_state)
    {
        flag = 0;
        for(j = 0;j<MAX_SWITCH_PORTS;j++)
        {
            if(switch_data.switch_port[j].vlan_table.update)
            {
                LOG_PRINT_INFO("write vlan table Port %d:[%d]---",j,switch_data.switch_port[j].vlan_table.memlen);
                for(m=0; m<switch_data.switch_port[j].vlan_table.memlen;m++)
                {
                    LOG_PRINT_INFO(" %d",switch_data.switch_port[j].vlan_table.memport[m]);
                }
                LOG_PRINT_INFO("\r\n");
                Peridot_gprtSetVlanPorts(j,switch_data.switch_port[j].vlan_table.memport,switch_data.switch_port[j].vlan_table.memlen);
                Peridot_gprtGetVlanPorts(j,mb,&mbl);
                if(mbl == switch_data.switch_port[j].vlan_table.memlen)
                {
                    LOG_PRINT_INFO("read vlan table Port %d:[%d]---",j,switch_data.switch_port[j].vlan_table.memlen);
                    for(m=0; m<mbl;m++)
                    {
                        LOG_PRINT_INFO(" %d",mb[m]);
                    }
                    LOG_PRINT_INFO("\r\n");
                    i=0;
                    while(i<mbl)
                    {
                        for(n=0; n<mbl; n++)
                        {
                            if(mb[i] == switch_data.switch_port[j].vlan_table.memport[n])
                            {
                                break;
                            }
                        }
                        if(n<mbl)
                        {
                            i++;
                        }
                        else
                        {
                            break;
                        }
                    }
                    if(i == mbl)
                    {
                        LOG_PRINT_INFO("port %d VLAN table setting success\r\n",j);
                        switch_data.switch_port[j].vlan_table.update = 0;
                    }
                    else
                    {
                        LOG_PRINT_WARIN("RW error2,port %d VLAN setting Failed\r\n",j);
                    }
                }
                else
                {
                    LOG_PRINT_WARIN("RW error2,port %d VLAN setting Failed\r\n",j);
                }
            }
        }
    }
    else if(flag == 0)
    {
        flag = 1;
        for(j = 0;j<MAX_SWITCH_PORTS;j++)
        {
            mbl = 0;
            m=0;
            while(m<MAX_SWITCH_PORTS)
            {
                if(m!=j)
                {
                    mb[mbl] = m;
                    mbl++;
                }
                m++;
            }
            Peridot_gprtSetVlanPorts(j,mb,mbl);
        }
    }
}


/**
 * @brief 端口VLAN查询
 * @param 
 * @return 
 */
void switch_port_vlan_table_get(void)
{
    uint8_t j = 0;

    for(j=0; j<MAX_SWITCH_PORTS; j++)
    {
        if(!(switch_data.portmng.lk_set[j]))
        {
            continue;
        }
        Peridot_gprtGetVlanPorts(j,switch_data.switch_port[j].from_reg.info.vlan_table.memport,\
                &switch_data.switch_port[j].from_reg.info.vlan_table.memlen);
    }
}


/**
 * @brief 端口速度查询
 * @param 
 * @return 
 */
void switch_port_speed_sta(void)
{
    uint8_t j = 0;
    PERIDOT_MSD_PORT_SPEED data = Peridot_PORT_SPEED_10_MBPS;

    for(j=0; j<MAX_SWITCH_PORTS; j++)
    {
        if(!(switch_data.portmng.lk_set[j]))
        {
            continue;
        }
        Peridot_gprtGetSpeed(j,&data);
        switch_data.switch_port[j].from_reg.info.port_speed = (em_PORT_SPEED)data;
    }

}


/**
 * @brief 获取端口双工状态
 * @param 
 * @return 
 */
void switch_port_duplex_sta(void)
{
    uint8_t j = 0;
    MSD_BOOL data = MSD_FALSE;

    for(j=0; j<MAX_SWITCH_PORTS; j++)
    {
        if(!(switch_data.portmng.lk_set[j]))
        {
            continue;
        }
        Peridot_gprtGetDuplexStatus(j,&data);
        switch_data.switch_port[j].from_reg.info.duplex_mode = (em_DUPLEX_MODE)data;
    }

}


/**
 * @brief 获取端口连接状态
 * @param 
 * @return 
 */
void switch_port_link_sta(void)
{
    uint8_t j = 0;
    MSD_BOOL data = MSD_FALSE;

    for(j=0; j<MAX_SWITCH_PORTS; j++)
    {
        if(!(switch_data.portmng.lk_set[j]))
        {
            continue;
        }
        Peridot_gprtGetLinkState(j,&data);
        switch_data.switch_port[j].from_reg.info.link_state = (em_LINK_STATE)data;
    }

}


/**
 * @brief 判断交换机IC连接是否正常
 * @param 
 * @return 
 */
uint8_t switch_IC_connect_state(void)
{
    MSD_STATUS sta;
    uint16_t data[32]={0};

    sta = msdGetAnyReg(0,3,&data[0]);
    if (sta != MSD_OK)
    {
        switch_data.IC_cnt_sta = 0;
        sta = 2;
    }
    else
    {
        if(((data[0]&0XFFF0) == 0x3900)||((data[0]&0XFFF0) == 0X0A10))
        {
            switch_data.IC_cnt_sta = 1;
            sta = 0;
        }
        else
        {
            switch_data.IC_cnt_sta = 0;
            sta = 1;
        }
    }

    return sta;
}


/**
 * @brief 检测每路PHY的连接状态
 * @param 
 * @return 
 */
void switch_everyPHY_detect(void)
{
    uint8_t j = 0;

    for(j = 0;j<MAX_SWITCH_PORTS;j++)
    {
        if(!(switch_data.portmng.lk_set[j]))
        {
            LOG_PRINT_WARIN("Port %d is close, PHY init Failed\r\n",j);
            continue;
        }
        switch_data.switch_port[j].from_reg.info.phy_detect = switch_PHY_detect(j);
    }
}


/**
 * @brief 检测端口PHY的连接状态
 * @param 
 * @return 
 */
uint8_t switch_PHY_detect(uint8_t portNum)
{
    uint16_t data = 0;
    MSD_STATUS sta = 0;

    sta = Peridot_msdReadPagedPhyReg(portNum,0,2,&data);
    if (sta != MSD_OK)
    {
        LOG_PRINT_INFO("Port %d PHY detect function read ERROR\r\n",portNum);
        return 0;
    }
    if(data != 0x0141)
    {
        LOG_PRINT_INFO("Port %d PHY detect Failed,data != 0x0141,data:%04x\r\n",portNum,data);
        return 0; 
    }
    LOG_PRINT_INFO("Port %d PHY detect SUCCESS\r\n",portNum);
    return 1;
}

/**
 * @brief 将VLAN table插入到数据缓存中
 * @param void 
 * @return void
 */
int8_t switch_vlantable_insert(uint16_t* table_data,uint8_t* table_num,uint8_t len)
{
    int8_t sta;
    uint8_t i;
    
    for(i=0; i<len; i++)
    {
        switch_data.portmng.pvl_t[*(table_num+i)] = table_data[i];
    }
    sta = pvl_table_to_port();
    if(sta)
    {
        return sta+50;
    }
    sta = portmng_file_update();
    if(sta)
    {
        return sta+100;
    }
    return 0;
}

/**
 * @brief 端口限速设置
 * @param void 
 * @return void
 */
void switch_port_egress_init(void)
{
    uint8_t j = 0;

    for(j = 0;j<MAX_SWITCH_PORTS;j++)
    {
        Peridot_grcSetEgressRate(j,PERIDOT_MSD_ELIMIT_LAYER2,1000000);
    }
    
}
/**
 * @brief 端口速率获取
 * @param void 
 * @return void
 */
void switch_port_egress_get(void)
{
    uint8_t j = 0;

    //LOG_PRINT_INFO("\r\n****\r\n");
    for(j = 0;j<MAX_SWITCH_PORTS;j++)
    {
        Peridot_grcGetEgressRate(j,&switch_data.switch_port[j].from_reg.egressrate_creg.mode,&switch_data.switch_port[j].from_reg.egressrate_creg.egress_rate);
    }
    
}
/**
 * @brief 端口速率获取
 * @param void 
 * @return void
 */
void switch_throughput(void)
{
    uint8_t j = 0;
    uint32_t value1 = 0,value2 = 0;
    
    for(j = 0;j<MAX_SWITCH_PORTS;j++)
    {
        //变量溢出自动循环
        value1 = switch_data.switch_port[j].from_reg.egress_counter.InGoodOctetsLo - port_count[j].InGoodOctetsLo;
        value2 = switch_data.switch_port[j].from_reg.egress_counter.InBadOctets - port_count[j].InBadOctets;
        switch_data.portmng.throughput[j].sectotal_ibyte = value1 + value2;
        port_count[j].InGoodOctetsHi = switch_data.switch_port[j].from_reg.egress_counter.InGoodOctetsHi;
        port_count[j].InGoodOctetsLo = switch_data.switch_port[j].from_reg.egress_counter.InGoodOctetsLo;
        port_count[j].InBadOctets = switch_data.switch_port[j].from_reg.egress_counter.InBadOctets;
        //
        value1 = switch_data.switch_port[j].from_reg.egress_counter.OutOctetsLo - port_count[j].OutOctetsLo;
        switch_data.portmng.throughput[j].sectotal_obyte=value1;
        switch_data.portmng.throughput[j].sectotal_byte = switch_data.portmng.throughput[j].sectotal_ibyte+switch_data.portmng.throughput[j].sectotal_obyte;
        port_count[j].OutOctetsHi = switch_data.switch_port[j].from_reg.egress_counter.OutOctetsHi;
        port_count[j].OutOctetsLo = switch_data.switch_port[j].from_reg.egress_counter.OutOctetsLo;
        //
        switch_data.portmng.throughput[j].ibad_val = switch_data.switch_port[j].from_reg.egress_counter.InBadOctets;
        //
        if(switch_data.portmng.throughput[j].sectotal_ibyte >= 100000)
        {
            switch_data.portmng.throughput[j].rxs_val = switch_data.portmng.throughput[j].sectotal_ibyte*8/(10.24*1024.0);
            switch_data.portmng.throughput[j].rxs_unit = 2;
        }
        else if(switch_data.portmng.throughput[j].sectotal_byte >= 1000)
        {
            switch_data.portmng.throughput[j].rxs_val = switch_data.portmng.throughput[j].sectotal_ibyte*8/(10.24);
            switch_data.portmng.throughput[j].rxs_unit = 1;
        }
        else
        {
            switch_data.portmng.throughput[j].rxs_val = switch_data.portmng.throughput[j].sectotal_ibyte*8;
            switch_data.portmng.throughput[j].rxs_unit = 0;
        }
        //
        if(port_count[j].InBadOctets >= 100000)
        {
            switch_data.portmng.throughput[j].ibad_val = port_count[j].InBadOctets/(10.24*1024.0);
            switch_data.portmng.throughput[j].ibad_unit = 2;
        }
        else if(port_count[j].InBadOctets >= 1000)
        {
            switch_data.portmng.throughput[j].ibad_val = port_count[j].InBadOctets/(10.24);
            switch_data.portmng.throughput[j].ibad_unit = 1;
        }
        else
        {
            switch_data.portmng.throughput[j].ibad_val = port_count[j].InBadOctets;
            switch_data.portmng.throughput[j].ibad_unit = 0;
        }
        //
        if(switch_data.portmng.throughput[j].sectotal_obyte >= 100000)
        {
            switch_data.portmng.throughput[j].txs_val = switch_data.portmng.throughput[j].sectotal_obyte*8/(10.24*1024.0);
            switch_data.portmng.throughput[j].txs_unit = 2;
        }
        else if(switch_data.portmng.throughput[j].sectotal_obyte >= 1000)
        {
            switch_data.portmng.throughput[j].txs_val = switch_data.portmng.throughput[j].sectotal_obyte*8/(10.24);
            switch_data.portmng.throughput[j].txs_unit = 1;
        }
        else
        {
            switch_data.portmng.throughput[j].txs_val = switch_data.portmng.throughput[j].sectotal_obyte*8;
            switch_data.portmng.throughput[j].txs_unit = 0;
        }
        //
        if(switch_data.portmng.throughput[j].sectotal_byte >= 100000)
        {
            switch_data.portmng.throughput[j].thrput_val = switch_data.portmng.throughput[j].sectotal_byte*8/(10.24*1024.0);
            switch_data.portmng.throughput[j].thrput_unit = 2;
        }
        else if(switch_data.portmng.throughput[j].sectotal_byte >= 1000)
        {
            switch_data.portmng.throughput[j].thrput_val = switch_data.portmng.throughput[j].sectotal_byte*8/(10.24);
            switch_data.portmng.throughput[j].thrput_unit = 1;
        }
        else
        {
            switch_data.portmng.throughput[j].thrput_val = switch_data.portmng.throughput[j].sectotal_byte*8;
            switch_data.portmng.throughput[j].thrput_unit = 0;
        }
    }
}
/**
 * @brief 获取每个端口的包数
 * @param void 
 * @return void
 */
void switch_port_egress_counter(void)
{
    uint8_t j = 0;
    
    for(j = 0;j<MAX_SWITCH_PORTS;j++)
    {
        Peridot_gstatsGetPortCounter(j,EGRESS_INGOOD_STAT_L,&switch_data.switch_port[j].from_reg.egress_counter.InGoodOctetsLo);
        Peridot_gstatsGetPortCounter(j,EGRESS_INGOOD_STAT_H,&switch_data.switch_port[j].from_reg.egress_counter.InGoodOctetsHi);
        Peridot_gstatsGetPortCounter(j,EGRESS_INBAD_STAT,&switch_data.switch_port[j].from_reg.egress_counter.InBadOctets);
        Peridot_gstatsGetPortCounter(j,EGRESS_OUT_STAT_L,&switch_data.switch_port[j].from_reg.egress_counter.OutOctetsLo);
        Peridot_gstatsGetPortCounter(j,EGRESS_OUT_STAT_H,&switch_data.switch_port[j].from_reg.egress_counter.OutOctetsHi);
    }
    
}
/* SHELL 命令行相关函数 */
#include <ctype.h>
/**
 * @brief 将16进制字符串转成整型
 * @param src 
 * @param data 
 * @return uint8_t 0 
 */
uint8_t hexstr_to_int(char *src, uint32_t* data)
{
    uint8_t d = 10;
    uint8_t p = 0;
    int8_t i = 0;
    uint8_t num = 0;
    uint8_t value[10] = {0};
    uint32_t dat = 0;

    LOG_PRINT_INFO("****start hex to int\r\n");
    if((src[0] == '0')&&('X'== toupper(src[1])))
    {
        p = 2;
        d = 16;
        LOG_PRINT_INFO("the str is hex\r\n");
    }

    i = p;
    while(src[i] != '\0')
    {
        if((src[i] >= '0') && (src[i] <= '9'))
        {
            value[num] = src[i]-0x30;
            LOG_PRINT_INFO(" %d",value[num]);
        }
        else if((src[i] >= 'a') && (src[i] <= 'f'))
        {
            value[num] = src[i]-'a'+10;
            LOG_PRINT_INFO(" %d",value[num]);
        }
        else if((src[i] >= 'A') && (src[i] <= 'F'))
        {
            value[num] = src[i]-'A'+10;
            LOG_PRINT_INFO(" %d",value[num]);
        }
        else
        {
            LOG_PRINT_INFO(" %c\r\n",src[i]);
            return 1;
        }
        num++;
        if(num > 8)
        {
            LOG_PRINT_INFO("\r\nnum > 8\r\n");
            return 2;
        }
        i++;
    }
    LOG_PRINT_INFO("\r\n");
    dat = 0;
    for(i=0; i<num; i++)
    {
        dat = dat*d + value[i];
    }

    *data = dat;
    LOG_PRINT_INFO("data:%d\r\n",*data);
    return 0;
}

/**
 * @brief 查询交换机运行参数
 * @param 
 * @return 
 */
void ifconfig(int argc, char **argv)
{
    uint8_t step = 0;
    int8_t res;
    
    if(!strcmp(argv[1],"ip"))
    {
        step = 1;
    }
    else if(!strcmp(argv[1],"mac"))
    {
        step = 2;
    }
    else if(!strcmp(argv[1],"gateway"))
    {
        step = 3;
    }
    else if(!strcmp(argv[1],"mask"))
    {
        step = 4;
    }
    else if(argc == 1)
    {
        step = 5;
    }
    else
    {
        LOG_PRINT_INFO("Parameter format error,ifconfig [ip/gateway/mask/no parameter] [parameter]\r\n");
        return;
    }
    switch(step)
    {
        case 1:
                res = strIP4_to_array(IP_ADDRESS,argv[2]);
                if(!res)
                {
                    res = binfo_file_update();
                    LOG_PRINT_INFO("ip set success:%d.%d.%d.%d",IP_ADDRESS[0],IP_ADDRESS[1],IP_ADDRESS[2],IP_ADDRESS[3]);
                    if(!res)
                    {
                        refresh_ip4();
                        LOG_PRINT_INFO("\r\n");
                    }
                    else
                    {
                        LOG_PRINT_INFO(",but ref to file failed:%d\r\n",res);
                    }
                }
                else
                {
                     LOG_PRINT_INFO("ip address format error!\r\n");
                }
            break;
        case 2:
                res = strMAC_to_array(MACAddr,argv[2]);
                if(!res)
                {
                    res = binfo_file_update();
                    LOG_PRINT_INFO("mac set success:%02x-%02x-%02x-%02x-%02x-%02x\r\n",MACAddr[0],MACAddr[1],MACAddr[2],MACAddr[3],MACAddr[4],MACAddr[5]);
                    if(!res)
                    {
                        refresh_ip4();
                        LOG_PRINT_INFO("\r\n");
                    }
                    else
                    {
                        LOG_PRINT_INFO(",but ref to file failed:%d\r\n",res);
                    }
                }
                else
                {
                     LOG_PRINT_INFO("mac address format error!\r\n");
                }
            break;
        case 3:
                res = strIP4_to_array(GATEWAY_ADDRESS,argv[2]);
                if(!res)
                {
                    res = binfo_file_update();
                    LOG_PRINT_INFO("gateway set success:%d.%d.%d.%d",GATEWAY_ADDRESS[0],GATEWAY_ADDRESS[1],GATEWAY_ADDRESS[2],GATEWAY_ADDRESS[3]);
                    if(!res)
                    {
                        refresh_ip4();
                        LOG_PRINT_INFO("\r\n");
                    }
                    else
                    {
                        LOG_PRINT_INFO(",but ref to file failed:%d\r\n",res);
                    }
                }
                else
                {
                     LOG_PRINT_INFO("gateway address format error!\r\n");
                }
            break;
        case 4:
                res = strIP4_to_array(NETMASK_ADDRESS,argv[2]);
                if(!res)
                {
                    res = binfo_file_update();
                    LOG_PRINT_INFO("mask set success:%d.%d.%d.%d",NETMASK_ADDRESS[0],NETMASK_ADDRESS[1],NETMASK_ADDRESS[2],NETMASK_ADDRESS[3]);
                    if(!res)
                    {
                        refresh_ip4();
                        LOG_PRINT_INFO("\r\n");
                    }
                    else
                    {
                        LOG_PRINT_INFO(",but ref to file failed:%d\r\n",res);
                    }
                }
                else
                {
                     LOG_PRINT_INFO("mask address format error!\r\n");
                }
            break;
        case 5:
            LOG_PRINT_INFO("\r\n");
            LOG_PRINT_INFO("\r\n");
            LOG_PRINT_INFO("Dev name:\t%s\r\n",dev_name);
            LOG_PRINT_INFO("mac:\t%02x-%02x-%02x-%02x-%02x-%02x\r\n",MACAddr[0],MACAddr[1],MACAddr[2],MACAddr[3],MACAddr[4],MACAddr[5]);
            LOG_PRINT_INFO("ip:\t%d.%d.%d.%d\r\n",IP_ADDRESS[0],IP_ADDRESS[1],IP_ADDRESS[2],IP_ADDRESS[3]);
            LOG_PRINT_INFO("mask:\t%d.%d.%d.%d\r\n",NETMASK_ADDRESS[0],NETMASK_ADDRESS[1],NETMASK_ADDRESS[2],NETMASK_ADDRESS[3]);
            LOG_PRINT_INFO("gateway:\t%d.%d.%d.%d\r\n",GATEWAY_ADDRESS[0],GATEWAY_ADDRESS[1],GATEWAY_ADDRESS[2],GATEWAY_ADDRESS[3]);
            LOG_PRINT_INFO("SW Version:\t%s\r\n",sw_ver);
            LOG_PRINT_INFO("HW Version:\t%s\r\n",hw_ver);
            LOG_PRINT_INFO("\r\n");
            LOG_PRINT_INFO("\r\n");
            break;
        default:
            break;
    }
}
//SHELL_EXPORT_CMD(ifconfig, ifconfig, query OR set system netif );
/**
 * @brief 查询交换机运行参数
 * @param 
 * @return 
 */
void q_switch_info(void)
{
    uint8_t  i = 0,j = 0;

    //IC 连接状态
    if(switch_data.IC_cnt_sta)
    {
        LOG_PRINT_INFO("\r\n\t IC connect: success\r\n");
    }
    else
    {
        LOG_PRINT_INFO("\r\n\t IC connect: failure\r\n");
    }
    for(j=0; j<MAX_SWITCH_PORTS; j++)
    {
        LOG_PRINT_INFO("\r\n/***** Port %d info ******/\r\n",j);
        //port的使能状态
        if(switch_data.portmng.lk_set[j])
        {
            LOG_PRINT_INFO("\t port state: enable\r\n");
        }
        else
        {
            LOG_PRINT_INFO("\t port state: disable\r\n");
        }
        //双工状态
        if(switch_data.switch_port[j].from_reg.info.duplex_mode == HALF_DUPLEX)
        {
            LOG_PRINT_INFO("\t duplex_mode: HALF_DUPLEX\r\n");
        }
        else if(switch_data.switch_port[j].from_reg.info.duplex_mode == FULL_DUPLEX)
        {
            LOG_PRINT_INFO("\t duplex_mode: FULL_DUPLEX\r\n");
        }
        else
        {
            LOG_PRINT_INFO("\t duplex_mode: Unknown\r\n");
        }
        //网络连接状态
        if(switch_data.switch_port[j].from_reg.info.link_state == LINK_DOWN)
        {
            LOG_PRINT_INFO("\t link_state: failure\r\n");
        }
        else if(switch_data.switch_port[j].from_reg.info.link_state == LINK_UP)
        {
            LOG_PRINT_INFO("\t link_state: success\r\n");
        }
        else
        {
            LOG_PRINT_INFO("\t link_state: Unknown\r\n");
        }
        //port phy detect
        if(switch_data.switch_port[j].from_reg.info.phy_detect)
        {
            LOG_PRINT_INFO("\t port PHY detect: success\r\n");
        }
        else
        {
            LOG_PRINT_INFO("\t port PHY detect: failure\r\n");
        }
        //端口连接速度
        switch (switch_data.switch_port[j].from_reg.info.port_speed)
        {
           case SPEED_10_MBPS:
               LOG_PRINT_INFO("\t port speed: 10M\r\n");
               break;
           case SPEED_100_MBPS:
               LOG_PRINT_INFO("\t port speed: 100M\r\n");
               break;
           case SPEED_200_MBPS:
               LOG_PRINT_INFO("\t port speed: 200M\r\n");
               break;
           case SPEED_1000_MBPS:
               LOG_PRINT_INFO("\t port speed: 1000M\r\n");
               break;
           case SPEED_2_5_GBPS:
               LOG_PRINT_INFO("\t port speed: 2.5G\r\n");
               break;
           case SPEED_10_GBPS:
               LOG_PRINT_INFO("\t port speed: 10G\r\n");
               break;
           default:
               LOG_PRINT_INFO("\t port speed: Unknow\r\n");
               break;
        }
        //端口VLAN table
        LOG_PRINT_INFO("\t port VLAN table:");
        for(i=0; i<switch_data.switch_port[j].from_reg.info.vlan_table.memlen; i++)
        {
            LOG_PRINT_INFO(" %d",switch_data.switch_port[j].from_reg.info.vlan_table.memport[i]);
        }
        LOG_PRINT_INFO("\r\n");
    }
}
//SHELL_EXPORT_CMD(q_switch_info, q_switch_info, query switch_ info);


/**
 * @brief 设置任意指定寄存器的值
 * @param 
 * @return 
 */
void s_anyreg(int argc, char **argv)
{
    uint8_t sta = 0;
    uint32_t port = 0;
    uint32_t page = 0;
    uint32_t regaddr = 0;
    uint32_t data = 0;

    if(!strcmp(argv[1],"phy"))
    {
        if(argc != 6)
        {
            LOG_PRINT_INFO("Parameter quantity error,need = 6,s_anyreg phy [port] [page] [regaddr] [data]\r\n");
            return;
        }
    }
    else if(!strcmp(argv[1],"port"))
    {
        if(argc != 5)
        {
            LOG_PRINT_INFO("Parameter quantity error,need = 5,s_anyreg port [port] [regaddr] [data]\r\n");
            return;
        }
    }
    else
    {
        LOG_PRINT_INFO("Parameter format error:\r\ns_anyreg phy [port] [page] [regaddr] [data]\r\n\ts_anyreg port [port] [regaddr] [data]\r\n");
        return;
    }

    if(argc == 5)
    {
        sta = hexstr_to_int(argv[2],&port);
        if(sta == 1)
        {
            LOG_PRINT_INFO("Failure,Parameter 2 format error\r\n");
            return;
        }
        else if(sta == 2)
        {
            LOG_PRINT_INFO("Failure,Parameter 2 too large\r\n");
            return;
        }
        if(port >= MAX_SWITCH_PORTS)
        {
            LOG_PRINT_INFO("Failure,Parameter port need less than %d,input:0x%04x\r\n",MAX_SWITCH_PORTS,port);
            return;
        }
        sta = hexstr_to_int(argv[3],&regaddr);
        if(sta == 1)
        {
            LOG_PRINT_INFO("Failure,Parameter 3 format error\r\n");
            return;
        }
        else if(sta == 2)
        {
            LOG_PRINT_INFO("Failure,Parameter 3 too large\r\n");
            return;
        }
        if(regaddr >= 32)
        {
            LOG_PRINT_INFO("Failure,Parameter regaddr need less than 32,input:0x%04x\r\n",regaddr);
            return;
        }
        sta = hexstr_to_int(argv[4],&data);
        if(sta == 1)
        {
            LOG_PRINT_INFO("Failure,Parameter parameter 4 format error\r\n");
            return;
        }
        else if(sta == 2)
        {
            LOG_PRINT_INFO("Failure,Parameter parameter 4 too large\r\n");
            return;
        }
        if(data >= 0xffff)
        {
            LOG_PRINT_INFO("Failure,Parameter data need less than 65535,input:%04x\r\n",data);
            return;
        }
        msdSetAnyReg(port,regaddr,data);
        LOG_PRINT_INFO("Success,The port %d reg %d data:0x%04x\r\n",port,regaddr,data);
    }

    if(argc == 6)
    {
        sta = hexstr_to_int(argv[2],&port);
        if(sta == 1)
        {
            LOG_PRINT_INFO("Failure,Parameter 2 format error\r\n");
            return;
        }
        else if(sta == 2)
        {
            LOG_PRINT_INFO("Failure,Parameter 2 too large\r\n");
            return;
        }
        if(port > 0x1f)
        {
            LOG_PRINT_INFO("Failure,Parameter port need less than %d,input:0x%04x\r\n",MAX_SWITCH_PORTS,port);
            return;
        }
        sta = hexstr_to_int(argv[3],&page);
        if(sta == 1)
        {
            LOG_PRINT_INFO("Failure,Parameter 3 format error\r\n");
            return;
        }
        else if(sta == 2)
        {
            LOG_PRINT_INFO("Failure,Parameter 3 too large\r\n");
            return;
        }
        if(port >= 32)
        {
            LOG_PRINT_INFO("Failure,Parameter page need less than 32,input:0x%04x\r\n",page);
            return;
        }
        sta = hexstr_to_int(argv[4],&regaddr);
        if(sta == 1)
        {
            LOG_PRINT_INFO("Failure,Parameter 4 format error\r\n");
            return;
        }
        else if(sta == 2)
        {
            LOG_PRINT_INFO("Failure,Parameter 4 too large\r\n");
            return;
        }
        if(regaddr >= 32)
        {
            LOG_PRINT_INFO("Failure,Parameter regaddr need less than 32,input:0x%04x\r\n",regaddr);
            return;
        }
        sta = hexstr_to_int(argv[5],&data);
        if(sta == 1)
        {
            LOG_PRINT_INFO("Failure,Parameter 5 format error\r\n");
            return;
        }
        else if(sta == 2)
        {
            LOG_PRINT_INFO("Failure,Parameter 5 too large\r\n");
            return;
        }
        if(data >= 0xffff)
        {
            LOG_PRINT_INFO("Failure,Parameter data need less than 65535,input:0x%04x\r\n",data);
            return;
        }
        Peridot_msdWritePagedPhyReg(port,page,regaddr,data);
        LOG_PRINT_INFO("Success,The port %d phy page %d reg %d data:0x%04x\r\n",port,page,regaddr,data);
    }
}
//SHELL_EXPORT_CMD(s_anyreg, s_anyreg, set any reg);


/**
 * @brief 查询任意指定寄存器的值
 * @param 
 * @return 
 */
void q_anyreg(int argc, char **argv)
{
    uint8_t i = 0,j=0;
    uint8_t port = 0;
    uint8_t page = 0;
    uint8_t regaddr = 0;
    uint16_t data = 0;

    if(!strcmp(argv[1],"phy"))
    {
        if(argc != 5)
        {
            LOG_PRINT_INFO("Parameter format error,q_anyreg phy [port] [page] [regaddr]\r\n");
            return;
        }
    }
    else if(!strcmp(argv[1],"port"))
    {
        if(argc != 4)
        {
            LOG_PRINT_INFO("Parameter format error,q_anyreg port [port] [regaddr]\r\n");
            return;
        }
    }
    else
    {
        LOG_PRINT_INFO("Parameter format error:\r\nq_anyreg phy [port] [page] [regaddr]\r\n\tq_anyreg port [port] [regaddr]\r\n");
        return;
    }

    //LOG_PRINT_INFO("argc:%d\r\n",argc);
    for(i=2; i<argc; i++)
    {
        j = 0;
        while(argv[i][j] != '\0')
        {
            //LOG_PRINT_INFO("argv[%d]:%c",i,argv[i][j]);
            if((argv[i][j] < '0') || (argv[i][j] > '9'))
            {
                LOG_PRINT_INFO("\r\n");
                LOG_PRINT_INFO("Parameter format error,q_phyreg [number] [number] [number]\r\n");
                return;
            }
            j++;
        }
        LOG_PRINT_INFO("\r\n");
    }

    if(argc == 4)
    {
        port = atol(argv[2]);
        regaddr = atol(argv[3]);
        msdGetAnyReg(port,regaddr,&data);
        LOG_PRINT_INFO("The port %d reg %d data:0x%04x\r\n",port,regaddr,data);
    }

    if(argc == 5)
    {
        port = atol(argv[2]);
        page = atol(argv[3]);
        regaddr = atol(argv[4]);
        Peridot_msdReadPagedPhyReg(port,page,regaddr,&data);
        LOG_PRINT_INFO("The port %d phy page %d reg %d data:0x%04x\r\n",port,page,regaddr,data);
    }
}
//SHELL_EXPORT_CMD_EX(q_anyreg, q_anyreg, query any reg,\
//q_anyreg [phy/port] [port] [page] [regaddr]);


/**
 * @brief 设置任意指定寄存器的指定位的值
 * @param 
 * @return 
 */
void s_anyregbit(int argc, char **argv)
{
    uint8_t sta = 0;
    uint32_t port = 0;
    uint32_t page = 0;
    uint32_t regaddr = 0;
    uint32_t data = 0,data_max = 1;
    uint32_t bit_star = 0;
    uint32_t bit_len = 0;
    uint8_t i = 0;

    if(!strcmp(argv[1],"phy"))
    {
        if(argc != 8)
        {
            LOG_PRINT_INFO("Parameter quantity error,need = 8,s_anyregbit phy [port] [page] [regaddr] [bit] [len] [data]\r\n");
            return;
        }
    }
    else if(!strcmp(argv[1],"port"))
    {
        if(argc != 7)
        {
            LOG_PRINT_INFO("Parameter quantity error,need = 7,s_anyregbit port [port] [regaddr] [bit] [len] [data]\r\n");
            return;
        }
    }
    else
    {
        LOG_PRINT_INFO("Parameter format error:\r\ns_anyregbit phy [port] [page] [regaddr] [bit] [len] [data]\r\n\ts_anyregbit port [port] [regaddr] [bit] [len] [data]\r\n");
        return;
    }
    //7个参数  表示对port寄存器进行操作
    if(argc == 7)
    {
        //参数2  指定port
        sta = hexstr_to_int(argv[2],&port);
        if(sta == 1)
        {
            LOG_PRINT_INFO("Failure,Parameter 2 format error\r\n");
            return;
        }
        else if(sta == 2)
        {
            LOG_PRINT_INFO("Failure,Parameter 2 too large\r\n");
            return;
        }
        if(port > 0x1f)
        {
            LOG_PRINT_INFO("Failure,Parameter port need less than %d,input:0x%04x\r\n",MAX_SWITCH_PORTS,port);
            return;
            
        }
        //参数3  指定寄存器
        sta = hexstr_to_int(argv[3],&regaddr);
        if(sta == 1)
        {
            LOG_PRINT_INFO("Failure,Parameter 3 format error\r\n");
            return;
        }
        else if(sta == 2)
        {
            LOG_PRINT_INFO("Failure,Parameter 3 too large\r\n");
            return;
        }
        if(regaddr >= 32)
        {
            LOG_PRINT_INFO("Failure,Parameter regaddr need less than 32,input:0x%04x\r\n",regaddr);
            return;
        }
        //参数4  指定开始的bit位
        sta = hexstr_to_int(argv[4],&bit_star);
        if(sta == 1)
        {
            LOG_PRINT_INFO("Failure,Parameter 4 format error\r\n");
            return;
        }
        else if(sta == 2)
        {
            LOG_PRINT_INFO("Failure,Parameter 4 too large\r\n");
            return;
        }
        if(bit_star >= 16)
        {
            LOG_PRINT_INFO("Failure,Parameter 4 bit value need less than 16\r\n");
            return;
        }
        //参数5  指定从开始的bit位起算共操作指定个bit
        sta = hexstr_to_int(argv[5],&bit_len);
        if(sta == 1)
        {
            LOG_PRINT_INFO("Failure,Parameter 5 format error\r\n");
            return;
        }
        else if(sta == 2)
        {
            LOG_PRINT_INFO("Failure,Parameter 5 too large\r\n");
            return;
        }
        if(bit_star+bit_len > 16)
        {
            LOG_PRINT_INFO("Failure,bit_star+bit_len need less than 16\r\n");
            return;
        }
        //参数5  向指定的bit位开始写入指定bit的数据
        sta = hexstr_to_int(argv[6],&data);
        if(sta == 1)
        {
            LOG_PRINT_INFO("Failure,Parameter 6 format error\r\n");
            return;
        }
        else if(sta == 2)
        {
            LOG_PRINT_INFO("Failure,Parameter 6 too large\r\n");
            return;
        }
        data_max = 1;
        for(i=0;i<bit_len;i++)
        {
            data_max *= 2;
        }
        if(data >= data_max)
        {
            LOG_PRINT_INFO("Failure,insert parameter bit length is %d,data need less than %d,input:0x%04x\r\n",bit_len,data_max,data);
            return;
        }
        msdSetAnyRegField(port,regaddr,bit_star,bit_len,data);
        LOG_PRINT_INFO("Success,The port %d reg %d bit_star %d bit_len %d data:0x%04x\r\n",port,regaddr,bit_star,bit_len,data);
    }

    if(argc == 8)
    {
        sta = hexstr_to_int(argv[2],&port);
        if(sta == 1)
        {
            LOG_PRINT_INFO("Failure,Parameter 2 format error\r\n");
            return;
        }
        else if(sta == 2)
        {
            LOG_PRINT_INFO("Failure,Parameter 2 too large\r\n");
            return;
        }
        if(port >= MAX_SWITCH_PORTS)
        {
            LOG_PRINT_INFO("Failure,Parameter port need less than %d,input:0x%04x\r\n",MAX_SWITCH_PORTS,port);
            return;
        }

        sta = hexstr_to_int(argv[3],&page);
        if(sta == 1)
        {
            LOG_PRINT_INFO("Failure,Parameter 3 format error\r\n");
            return;
        }
        else if(sta == 2)
        {
            LOG_PRINT_INFO("Failure,Parameter 3 too large\r\n");
            return;
        }
        if(port >= 32)
        {
            LOG_PRINT_INFO("Failure,Parameter page need less than 32,input:0x%04x\r\n",page);
            return;
        }

        sta = hexstr_to_int(argv[4],&regaddr);
        if(sta == 1)
        {
            LOG_PRINT_INFO("Failure,Parameter 4 format error\r\n");
            return;
        }
        else if(sta == 2)
        {
            LOG_PRINT_INFO("Failure,Parameter 4 too large\r\n");
            return;
        }
        if(regaddr >= 32)
        {
            LOG_PRINT_INFO("Failure,Parameter regaddr need less than 32,input:0x%04x\r\n",regaddr);
            return;
        }
        //参数4  指定开始的bit位
        sta = hexstr_to_int(argv[5],&bit_star);
        if(sta == 1)
        {
            LOG_PRINT_INFO("Failure,Parameter 5 format error\r\n");
            return;
        }
        else if(sta == 2)
        {
            LOG_PRINT_INFO("Failure,Parameter 5 too large\r\n");
            return;
        }
        if(bit_star >= 16)
        {
            LOG_PRINT_INFO("Failure,Parameter 5 bit value need less than 16\r\n");
            return;
        }
        //参数5  指定从开始的bit位起算共操作指定个bit
        sta = hexstr_to_int(argv[6],&bit_len);
        if(sta == 1)
        {
            LOG_PRINT_INFO("Failure,Parameter 6 format error\r\n");
            return;
        }
        else if(sta == 2)
        {
            LOG_PRINT_INFO("Failure,Parameter 6 too large\r\n");
            return;
        }
        if(bit_star+bit_len > 16)
        {
            LOG_PRINT_INFO("Failure,bit_star+bit_len need less than 16\r\n");
            return;
        }
        sta = hexstr_to_int(argv[7],&data);
        if(sta == 1)
        {
            LOG_PRINT_INFO("Failure,Parameter 5 format error\r\n");
            return;
        }
        else if(sta == 2)
        {
            LOG_PRINT_INFO("Failure,Parameter 5 too large\r\n");
            return;
        }
        data_max = 1;
        for(i=0;i<bit_len;i++)
        {
            data_max *= 2;
        }
        if(data >= data_max)
        {
            LOG_PRINT_INFO("Failure,insert parameter bit length is %d,data need less than %d,input:0x%04x\r\n",bit_len,data_max,data);
            return;
        }
        Peridot_msdSetPagedPhyRegField(port,page,regaddr,bit_star,bit_len,data);
        LOG_PRINT_INFO("Success,The port %d phy page %d reg %d bit_star %d bit_len %d data:0x%04x\r\n",port,page,regaddr,bit_star,bit_len,data);
    }
}
//SHELL_EXPORT_CMD_EX(s_anyregbit, s_anyregbit, set any reg any bit,\
//s_anyreg phy/port [port] [page] [regaddr] [data]);


/**
 * @brief 设置vlan table
 * @param 
 * @return 
 */
void s_pvlt(int argc, char *argv[])
{
    uint8_t sta = 0;
    uint32_t table = 0,member=0;
    uint16_t    max_mb = 0;
    
    max_mb = ~(0xffff>>MAX_PVLT_MEM);
    max_mb = max_mb>>(16-MAX_PVLT_MEM);
    
    if(argc != 3)
    {
        LOG_PRINT_INFO("Parameter format error:s_pvt [ 1 <=table<= %d ] [ 0 <=member<= 0x%04x ]\r\n",MAX_PVLT_MEM,max_mb);
        return;
    }

    sta = hexstr_to_int(argv[1],&table);
    if(sta == 1)
    {
        LOG_PRINT_INFO("Parameter format error:s_pvt [ 1 <=table<= %d ] [ 0 <=member<= 0x%04x ]\r\n",MAX_PVLT_MEM,max_mb);
        return;
    }
    else if(sta == 2)
    {
        LOG_PRINT_INFO("Parameter format error:s_pvt [ 1 <=table<=%d ] [ 0 <=member<= 0x%04x ]\r\n",MAX_PVLT_MEM,max_mb);
        return;
    }
    
    sta = hexstr_to_int(argv[2],&member);
    if(sta == 1)
    {
        LOG_PRINT_INFO("Parameter format error:s_pvt [ 1 <=table<= %d ] [ 0 <=member<= 0x%04x ]\r\n",MAX_PVLT_MEM,max_mb);
        return;
    }
    else if(sta == 2)
    {
        LOG_PRINT_INFO("Parameter format error:s_pvt [ 1 <=table<= %d ] [ 0 <=member<= 0x%04x ]\r\n",MAX_PVLT_MEM,max_mb);
        return;
    }
    
    if((table <= 0) || (table > MAX_PVLT_MEM))
    {
        LOG_PRINT_INFO("Failure,support vlan table number:1~%d\r\n",MAX_PVLT_MEM);
        return;
    }
    
    if(member > max_mb)
    {
        LOG_PRINT_INFO("Failure,max support vlan port member:%04x\r\n",max_mb);
        return;
    }

    table = table-1;
    sta = switch_vlantable_insert((uint16_t*)(&member),(uint8_t*)(&table),1);
    if(!sta)
    {
        LOG_PRINT_INFO("Parameter format check success,will write to regeister,View status information by s_dpl command\r\n");
    }
    else
    {
        LOG_PRINT_INFO("vlan table insert failed,error code:%d\r\n",sta);
    }
}
//SHELL_EXPORT_CMD_EX(s_pvlt, s_pvlt, set port VLAN table, \
//s_pvt [table] [member]);

/**
 * @brief 查询vlan table
 * @param 
 * @return 
 */
void q_pvlt(void)
{
    uint8_t i = 0,j = 0;
    char sp_buf[6]={0};
    char buf[MAX_SWITCH_PORTS*3]={0};
    uint16_t    max_mb = 0;
    
    max_mb = ~(0xffff>>MAX_PVLT_MEM);
    max_mb = max_mb>>(16-MAX_PVLT_MEM);
    
    for(i=0; i<MAX_PVLT_MEM; i++)
    {
        if(switch_data.portmng.pvl_t[i]&max_mb)
        {
            LOG_PRINT_INFO("table %d:",i+1);
        }
        else 
        {
            LOG_PRINT_INFO("table %d: NULL\r\n",i+1);
            continue;
        }
        for(j=0; j<MAX_SWITCH_PORTS; j++)
        {
            if(switch_data.portmng.pvl_t[i]&(0x01<<j))
            {
                sprintf (sp_buf, " %d ",j+1);
                strcat ( buf , sp_buf );
            }
        }
        LOG_PRINT_INFO("%s\r\n",buf);
        memset(buf,0,sizeof(buf));
    }

}
//SHELL_EXPORT_CMD_EX(q_pvlt, q_pvlt, query port VLAN table, \
//q_pvlt);

///**
// * @brief 设置DEBUG打印等级
// * @param 
// * @return 
// */
//void s_dpl(int argc, char **argv)
//{
//    uint16_t val;
//    LogLevel level;
//    
//    val = (*argv[1])-0x30;
//    if(val <= LOG_ALL)
//    {
//        level = (LogLevel)val;
//        logSetLevel(&uartLog,level);
//    }
//    else
//    {
//        LOG_PRINT_INFO("log level value must < %d\r\n",LOG_ALL+1);
//    }
//}
//SHELL_EXPORT_CMD_EX(s_dpl, s_dpl, set log level ,s_dpl [level]);

