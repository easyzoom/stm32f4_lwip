#include "sys_info.h"
#include "fs_api.h"
#include "cJSON.h"
#include "kubot_debug.h"
#include "ethernetif.h"
#include <ctype.h>
#include "lwip.h"
#include "switch_app.h"
#include "crc.h"

char dev_name[16]={SW_DEV_NAME};
char sw_ver[26]={SW_SOFTWARE_VERSION};
char hw_ver[20]={SW_HARDWARE_VERSION};


str_sysfile_ram file_ram_bsif;
str_sysfile_ram file_ram_pmng;

int8_t checkMACformat(const char* src);
int8_t strMAC_to_array(uint8_t* dec,const char* src);
int8_t checkIPformat(const char* src);
int8_t strIP4_to_array(uint8_t* dec,const char* src);


/**
 * @brief 系统参数配置
 * 
 * @param [in] state 文件挂载状态
 * @return void
 */
uint8_t sys_info_config(uint8_t state)
{
    FILE_STR fp;
    int32_t res = 0;
    int32_t size;
     
    //basic info default
    IP_ADDRESS[0] = 192;
    IP_ADDRESS[1] = 168;
    IP_ADDRESS[2] = 0;
    IP_ADDRESS[3] = 100;
    NETMASK_ADDRESS[0] = 255;
    NETMASK_ADDRESS[1] = 255;
    NETMASK_ADDRESS[2] = 255;
    NETMASK_ADDRESS[3] = 0;
    GATEWAY_ADDRESS[0] = 192;
    GATEWAY_ADDRESS[1] = 168;
    GATEWAY_ADDRESS[2] = 0;
    GATEWAY_ADDRESS[3] = 1;
    MACAddr[0] = 0x70;
    MACAddr[1] = 0xB3;
    MACAddr[2] = 0xD5;
    MACAddr[3] = 0x0B;
    MACAddr[4] = 0x78;
    MACAddr[5] = 0x61;
    switch_init();
    file_ram_bsif.flag = 0;
    sprintf(file_ram_bsif.file,"file save to ram Init,ram size:%d BYTE.",sizeof(file_ram_bsif.file));
    file_ram_pmng.flag = 0;
    sprintf(file_ram_pmng.file,"file save to ram Init,ram size:%d BYTE.",sizeof(file_ram_pmng.file));
    
    //file system mount success
    if(!state)
    {
        //basic info file
        res = fs_api_fopen(&fp,FILE_BASIC_INFO,"r");
        if(res)
        {
            LOG_PRINT_ERROR("FILE_BASIC_INFO open failed\r\n");
        }
        else
        {
            size = fs_api_fsize(&fp);
            res = fs_api_fclose(&fp);
            LOG_PRINT_DEBUG("FILE_BASIC_INFO open success:%d BYTE\r\n",size);
            if(size > 0 && size < 256)
            {
                res = binfo_data_update(size);
                if(res)
                {

                }
                else
                {
                    ;
                }
            }
            else
            {
                LOG_PRINT_ERROR("FILE_BASIC_INFO size error,Use default parameters\r\n");
                res = binfo_file_update();
                if(res)
                {

                }
                else
                {
                    ;
                }
            }
        }
        //vlan table file
        res = fs_api_fopen(&fp,FILE_PORT_MANAGE,"r");
        if(res)
        {
            LOG_PRINT_ERROR("FILE_PORT_MANAGE open failed\r\n");
        }
        else
        {
            size = fs_api_fsize(&fp);
            res = fs_api_fclose(&fp);
            LOG_PRINT_DEBUG("FILE_PORT_MANAGE open success:%d BYTE\r\n",size);
            if(size > 0 && size < 256)
            {
                res = portmng_parameter_update(size);
                if(res)
                {
                    
                }
                else
                {
                    ;
                }
            }
            else
            {
                LOG_PRINT_ERROR("FILE_PORT_MANAGE size error,Use default parameters\r\n");
                res = portmng_file_update();
                if(res)
                {
                    
                }
                else
                {
                    ;
                }
            }
        }
    }
    else
    {
        ;
    }
    
    pvl_table_to_port();
    
    return state;
}

/**
 * @brief 更新basic文件中的数据
 * 
 * @param void
 * @return 0 成功 <0 错误
 */
int8_t binfo_file_update(void)
{
    FILE_STR fp;
    cJSON *cj_binfo;
    char *out;
    char sp_buf[64]={0};
    int32_t res = 0;
    int32_t size_w = 0,size_r = 0;
    int8_t state = 0;
    
    res = fs_api_fopen(&fp,FILE_BASIC_INFO,"wr");
    if(res)
    {
        LOG_PRINT_ERROR("FILE_BASIC_INFO open failure\r\n");
        state = -1;
    }
    else
    {
        cj_binfo = cJSON_CreateObject();
        if(cj_binfo)
        {
            cJSON_AddStringToObject(cj_binfo,DEV_NAME,dev_name);
            sprintf(sp_buf,"%02x-%02x-%02x-%02x-%02x-%02x",MACAddr[0],MACAddr[1],MACAddr[2],MACAddr[3],MACAddr[4],MACAddr[5]);
            cJSON_AddStringToObject(cj_binfo,MAC_SNAME,sp_buf);

            sprintf(sp_buf,"%d.%d.%d.%d",IP_ADDRESS[0],IP_ADDRESS[1],IP_ADDRESS[2],IP_ADDRESS[3]);
            cJSON_AddStringToObject(cj_binfo,IP_SNAME,sp_buf);

            sprintf(sp_buf,"%d.%d.%d.%d",NETMASK_ADDRESS[0],NETMASK_ADDRESS[1],NETMASK_ADDRESS[2],NETMASK_ADDRESS[3]);
            cJSON_AddStringToObject(cj_binfo,NETMASK_SNAME,sp_buf);

            sprintf(sp_buf,"%d.%d.%d.%d",GATEWAY_ADDRESS[0],GATEWAY_ADDRESS[1],GATEWAY_ADDRESS[2],GATEWAY_ADDRESS[3]);
            cJSON_AddStringToObject(cj_binfo,GATEWAY_SNAME,sp_buf);

            cJSON_AddStringToObject(cj_binfo,SW_SNAME,sw_ver);
            cJSON_AddStringToObject(cj_binfo,HW_SNAME,hw_ver);
            out = cJSON_Print(cj_binfo); 
            cJSON_Delete(cj_binfo);
            if(out)
            {
                size_w = strlen(out);
                LOG_PRINT_INFO("%s,%d\n",out,size_w);
                size_r = fs_api_fwrite(&fp,size_w,out);
                fs_api_truncate(&fp,size_w);
                if(size_w != size_r)
                {
                    state = -2;
                }
                if(size_w <= sizeof(file_ram_bsif.file))
                {
                    file_ram_bsif.flag = 1;
                    strcpy(file_ram_bsif.file,out);
                }
                else
                {
                    file_ram_bsif.flag = -1;
                }    
                cJSON_free(out);
            }
            else
            {
                state = -3;
            }
        }
        else
        {
            res = fs_api_fclose(&fp);
            state = -4;
        }
        res = fs_api_fclose(&fp);
    }
    
    return state;
}
/**
 * @brief 从ram数组中更新file
 * 
 * @param void
 * @return 0：成功 <0：错误  >0：部分成功 
 *           成功表示成功修改且掉电不丢失
 */
int8_t binfo_file_update_from_rambuf(char *buf,uint16_t len)
{
    FILE_STR fp;
    int32_t res = 0;
    cJSON *json,*json_desc,*json_mac,*json_ip,*json_netmask,*json_gateway,*json_sw,*json_hw;
    int32_t size_w = 0,size_r = 0;
    char sp_buf[64]={0};
    char *out;
    uint8_t set_state = 0;
    uint8_t err_state = 0;
    
    if(len > sizeof(file_ram_bsif.file))
    {
        return -1;
    }
    
    json = cJSON_Parse(buf);
    if(json)
    {
        json_desc = cJSON_GetObjectItem(json,DEV_NAME);
        //LOG_PRINT_INFO("json_desc:%s\r\n",json_desc->valuestring);
        json_mac = cJSON_GetObjectItem(json,MAC_SNAME);
        //LOG_PRINT_INFO("json_mac:%s\r\n",json_mac->valuestring);
        json_ip = cJSON_GetObjectItem(json,IP_SNAME);
        //LOG_PRINT_INFO("json_ip:%s\r\n",json_ip->valuestring);
        json_netmask = cJSON_GetObjectItem(json,NETMASK_SNAME);
        //LOG_PRINT_INFO("json_netmask:%s\r\n",json_netmask->valuestring);
        json_gateway = cJSON_GetObjectItem(json,GATEWAY_SNAME);
        //LOG_PRINT_INFO("json_gateway:%s\r\n",json_gateway->valuestring);
        json_sw = cJSON_GetObjectItem(json,SW_SNAME);
        //LOG_PRINT_INFO("json_sw:%s\r\n",json_sw->valuestring);
        json_hw = cJSON_GetObjectItem(json,HW_SNAME);
        //LOG_PRINT_INFO("json_hw:%s\r\n",json_hw->valuestring);
        if(!json_desc)
        {
            json_desc = cJSON_AddStringToObject(json,DEV_NAME,dev_name);
            if(!json_desc)
            {
                cJSON_Delete(json);
                return -14;
            }
        }
        else
        {
            if(strlen(json_desc->valuestring) > sizeof(dev_name))
            {
                res = cJSON_ReplaceItemInObject(json,DEV_NAME,cJSON_CreateString(dev_name));
                if(!res)
                {
                    cJSON_Delete(json);
                    return -7;
                }
                err_state |= 0x01;
            }
            else
            {
                set_state |= 0x01;
            }
        }
        if(!json_mac)
        {
            sprintf(sp_buf,"%02x-%02x-%02x-%02x-%02x-%02x",MACAddr[0],MACAddr[1],MACAddr[2],MACAddr[3],MACAddr[4],MACAddr[5]);
            json_mac = cJSON_AddStringToObject(json,MAC_SNAME,sp_buf);
            if(!json_mac)
            {
                cJSON_Delete(json);
                return -15;
            }
        }
        else
        {
            res = strMAC_to_array(MACAddr,json_mac->valuestring);
            if(res)
            {
                sprintf(sp_buf,"%02x-%02x-%02x-%02x-%02x-%02x",MACAddr[0],MACAddr[1],MACAddr[2],MACAddr[3],MACAddr[4],MACAddr[5]);
                res = cJSON_ReplaceItemInObject(json,MAC_SNAME,cJSON_CreateString(sp_buf));
                if(!res)
                {
                    cJSON_Delete(json);
                    return -8;
                }
                err_state |= 0x02;
            }
            else
            {
                set_state |= 0x02;
            }
        }
        if(!json_ip)
        {
            sprintf(sp_buf,"%d.%d.%d.%d",IP_ADDRESS[0],IP_ADDRESS[1],IP_ADDRESS[2],IP_ADDRESS[3]);
            json_ip = cJSON_AddStringToObject(json,IP_SNAME,sp_buf);
            if(!json_ip)
            {
                cJSON_Delete(json);
                return -16;
            }
        }
        else
        {
            res = strIP4_to_array(IP_ADDRESS,json_ip->valuestring);
            if(res)
            {
                sprintf(sp_buf,"%d.%d.%d.%d",IP_ADDRESS[0],IP_ADDRESS[1],IP_ADDRESS[2],IP_ADDRESS[3]);
                res = cJSON_ReplaceItemInObject(json,IP_SNAME,cJSON_CreateString(sp_buf));
                if(!res)
                {
                    cJSON_Delete(json);
                    return -9;
                }
                err_state |= 0x04;
            }
            else
            {
                set_state |= 0x04;
            }
        }
        if(!json_netmask)
        {
            sprintf(sp_buf,"%d.%d.%d.%d",NETMASK_ADDRESS[0],NETMASK_ADDRESS[1],NETMASK_ADDRESS[2],NETMASK_ADDRESS[3]);
            json_netmask = cJSON_AddStringToObject(json,NETMASK_SNAME,sp_buf);
            if(!json_netmask)
            {
                cJSON_Delete(json);
                return -17;
            }
        }
        else
        {
            res = strIP4_to_array(NETMASK_ADDRESS,json_netmask->valuestring);
            if(res)
            {
                sprintf(sp_buf,"%d.%d.%d.%d",NETMASK_ADDRESS[0],NETMASK_ADDRESS[1],NETMASK_ADDRESS[2],NETMASK_ADDRESS[3]);
                res = cJSON_ReplaceItemInObject(json,NETMASK_SNAME,cJSON_CreateString(sp_buf));
                if(!res)
                {
                    cJSON_Delete(json);
                    return -10;
                }
                err_state |= 0x08;
            }
            else
            {
                set_state |= 0x08;
            }
        }
        if(!json_gateway)
        {
            sprintf(sp_buf,"%d.%d.%d.%d",GATEWAY_ADDRESS[0],GATEWAY_ADDRESS[1],GATEWAY_ADDRESS[2],GATEWAY_ADDRESS[3]);
            json_gateway = cJSON_AddStringToObject(json,GATEWAY_SNAME,sp_buf);
            if(!json_gateway)
            {
                cJSON_Delete(json);
                return -18;
            }
        }
        else
        {
            res = strIP4_to_array(GATEWAY_ADDRESS,json_gateway->valuestring);
            if(res)
            {
                sprintf(sp_buf,"%d.%d.%d.%d",GATEWAY_ADDRESS[0],GATEWAY_ADDRESS[1],GATEWAY_ADDRESS[2],GATEWAY_ADDRESS[3]);
                res = cJSON_ReplaceItemInObject(json,GATEWAY_SNAME,cJSON_CreateString(sp_buf));
                if(!res)
                {
                    cJSON_Delete(json);
                    return -11;
                }
                err_state |= 0x10;
            }
            else
            {
                set_state |= 0x10;
            }
        }
        if(!json_sw)
        {
            json_sw = cJSON_AddStringToObject(json,SW_SNAME,sw_ver);
            if(!json_sw)
            {
                cJSON_Delete(json);
                return -19;
            }
        }
        else
        {
            res = cJSON_ReplaceItemInObject(json, SW_SNAME, cJSON_CreateString(SW_SOFTWARE_VERSION));
            if(!res)
            {
                cJSON_Delete(json);
                return -12;
            }
        }
        if(!json_hw)
        {
            json_hw = cJSON_AddStringToObject(json,HW_SNAME,hw_ver);
            if(!json_hw)
            {
                cJSON_Delete(json);
                return -20;
            }
        }
        else
        {
            res = cJSON_ReplaceItemInObject(json, HW_SNAME, cJSON_CreateString(SW_HARDWARE_VERSION));
            if(!res)
            {
                cJSON_Delete(json);
                return -13;
            }
        }
        if(set_state)
        {
            out = cJSON_Print(json);
            cJSON_Delete(json);
            size_w = strlen(out);
            LOG_PRINT_INFO("%s,%d\n",out,size_w);
            if(size_w >= sizeof(file_ram_bsif.file))
            {
                cJSON_free(out);
                return -6;
            }
            strcpy(file_ram_bsif.file,out);
            res = fs_api_fopen(&fp,FILE_BASIC_INFO,"wr");
            if(res)
            {
                cJSON_free(out);
                fs_api_fclose(&fp);
                return -2;
            }
            size_r = fs_api_fwrite(&fp,size_w,out);
            fs_api_truncate(&fp,size_w);
            if(size_r != size_w)
            {
                cJSON_free(out);
                fs_api_fclose(&fp);
                return -3;
            }
            cJSON_free(out);
            fs_api_fclose(&fp);
        }
        else
        {
            LOG_PRINT_WARIN("binfo file format error\r\n");
            cJSON_Delete(json);
            return -4;
        }
    }
    else
    {
        return -5;
    }

    return err_state;
}

/**
 * @brief 从basic文件中更新数据
 * 
 * @param [in] size 文件大小
 * @return 0 成功 <0 错误
 */
int8_t binfo_data_update(uint32_t size)
{
    FILE_STR fp;
    int32_t res = 0;
    int8_t state = 0;
    char* f_data = NULL;
    cJSON *json,*json_desc,*json_mac,*json_ip,*json_netmask,*json_gateway,*json_sw,*json_hw;
    char *out;
    int32_t size_w = 0;
    
    f_data = (char*)malloc(size+1);
    if(f_data == NULL)
    {
        LOG_PRINT_ERROR("FILE_BASIC_INFO data buff malloc failure\r\n");
        state = -1;
    }
    else
    {
        res = fs_api_fopen(&fp,FILE_BASIC_INFO,"r");
        if(res)
        {
            LOG_PRINT_ERROR("FILE_BASIC_INFO open failure\r\n");
            state = -2;
        }
        else
        {
            res = fs_api_fread(&fp,size,f_data);
            if (res >= 0)
            {
                json = cJSON_Parse(f_data);
                if(json)
                {
                    out = cJSON_Print(json); 
                    if(out)
                    {
                        size_w = strlen(out);
                        if(size_w <= sizeof(file_ram_bsif.file))
                        {
                            file_ram_bsif.flag = 1;
                            strcpy(file_ram_bsif.file,out);
                        }
                        else
                        {
                            file_ram_bsif.flag = -1;
                        }
                        LOG_PRINT_INFO("%s,%d\r\n",out,size_w);
                        cJSON_free(out);
                    }
                    json_desc = cJSON_GetObjectItem(json,DEV_NAME);
                    //LOG_PRINT_INFO("json_desc:%s\r\n",json_desc->valuestring);
                    json_mac = cJSON_GetObjectItem(json,MAC_SNAME);
                    //LOG_PRINT_INFO("json_mac:%s\r\n",json_mac->valuestring);
                    json_ip = cJSON_GetObjectItem(json,IP_SNAME);
                    //LOG_PRINT_INFO("json_ip:%s\r\n",json_ip->valuestring);
                    json_netmask = cJSON_GetObjectItem(json,NETMASK_SNAME);
                    //LOG_PRINT_INFO("json_netmask:%s\r\n",json_netmask->valuestring);
                    json_gateway = cJSON_GetObjectItem(json,GATEWAY_SNAME);
                    //LOG_PRINT_INFO("json_gateway:%s\r\n",json_gateway->valuestring);
                    json_sw = cJSON_GetObjectItem(json,SW_SNAME);
                    //LOG_PRINT_INFO("json_sw:%s\r\n",json_sw->valuestring);
                    json_hw = cJSON_GetObjectItem(json,HW_SNAME);
                    //LOG_PRINT_INFO("json_hw:%s\r\n",json_hw->valuestring);
                    if(json_desc && json_mac && json_ip && json_netmask &&\
                        json_gateway && json_sw && json_hw)
                        {
                            //strcpy(dev_name,json_desc->valuestring);
                            sprintf(dev_name,"%s",json_desc->valuestring);
                            LOG_PRINT_INFO("dev_name:%s\r\n",dev_name);
                            strMAC_to_array(MACAddr,json_mac->valuestring);
                            LOG_PRINT_INFO("MACAddr:%d-%d-%d-%d-%d-%d\r\n",MACAddr[0],MACAddr[1],MACAddr[2],MACAddr[3],MACAddr[4],MACAddr[5]);
                            strIP4_to_array(IP_ADDRESS,json_ip->valuestring);
                            LOG_PRINT_INFO("IP_ADDRESS:%d.%d.%d.%d\r\n",IP_ADDRESS[0],IP_ADDRESS[1],IP_ADDRESS[2],IP_ADDRESS[3]);
                            strIP4_to_array(NETMASK_ADDRESS,json_netmask->valuestring);
                            LOG_PRINT_INFO("NETMASK_ADDRESS:%d.%d.%d.%d\r\n",NETMASK_ADDRESS[0],NETMASK_ADDRESS[1],NETMASK_ADDRESS[2],NETMASK_ADDRESS[3]);
                            strIP4_to_array(GATEWAY_ADDRESS,json_gateway->valuestring);
                            LOG_PRINT_INFO("GATEWAY_ADDRESS:%d.%d.%d.%d\r\n",GATEWAY_ADDRESS[0],GATEWAY_ADDRESS[1],GATEWAY_ADDRESS[2],GATEWAY_ADDRESS[3]);
                            strcpy(sw_ver,json_sw->valuestring);
                            LOG_PRINT_INFO("sw_ver:%s\r\n",sw_ver);
                            strcpy(hw_ver,json_hw->valuestring);
                            LOG_PRINT_INFO("hw_ver:%s\r\n",hw_ver);
                        }
                        else
                        {
                            state = -3;
                        }
                        cJSON_Delete(json);
                }
                else
                {
                    state = -4;
                }
            }
            else
            {
                state = -3;
            }
            res = fs_api_fclose(&fp);
        }
        free(f_data);
    }

    return state;
}

/**
 * @brief 更新vlan table文件中的数据
 * 
 * @param void
 * @return 0 成功 <0 错误
 */
int8_t portmng_file_update(void)
{
    FILE_STR fp;
    cJSON *cj_pm_f,*cj_pm,*cj_pvls,*cj_pvlt,*cj_plkset;
    char *out;
    int32_t res = 0;
    int32_t size_w = 0,size_r = 0;
    uint16_t crc = 0;
    
    res = fs_api_fopen(&fp,FILE_PORT_MANAGE,"wr");
    if(res)
    {
        LOG_PRINT_ERROR("fs_api_fopen(&fp,FILE_PORT_MANAGE,\"wr\") failed\r\n");
        return -1;
    }
    
    cj_pm = cJSON_CreateObject();
    if(!cj_pm)
    {
        LOG_PRINT_ERROR("cj_pm = cJSON_CreateObject() failed\r\n");
        res = fs_api_fclose(&fp);
        return -2;
    }
    
    cj_pvls = cJSON_AddNumberToObject(cj_pm,PVL_STATE,switch_data.portmng.pvl_state);
    if(!cj_pvls)
    {
        LOG_PRINT_ERROR("cj_pvls = cJSON_AddNumberToObject(cj_ppm,PVL_STATE,switch_data.portmng.pvl_state) failed\r\n");
        cJSON_Delete(cj_pm);
        res = fs_api_fclose(&fp);
        return -3;
    }
    
    cj_pvlt = cJSON_CreateIntArray(switch_data.portmng.pvl_t, MAX_PVLT_MEM);
    if(!cj_pvlt)
    {
        LOG_PRINT_ERROR("cj_pvlt = cJSON_CreateIntArray(switch_data.portmng.pvl_t, MAX_PVLT_MEM) failed\r\n");
        cJSON_Delete(cj_pm);
        res = fs_api_fclose(&fp);
        return -4;
    }
    
    res = cJSON_AddItemToObject(cj_pm, "pvlt", cj_pvlt);
    if(!res)
    {
        LOG_PRINT_ERROR("cJSON_AddItemToObject(cj_ppm, \"pvlt\", cj_pvlt) failed\r\n");
        cJSON_Delete(cj_pm);
        res = fs_api_fclose(&fp);
        cJSON_Delete(cj_pvlt);
        return -5;
    }
    
    cj_plkset = cJSON_CreateIntArray(switch_data.portmng.lk_set, MAX_SWITCH_PORTS);
    if(!cj_plkset)
    {
        LOG_PRINT_ERROR("cj_plkset = cJSON_CreateIntArray(switch_data.portmng.lk_set, MAX_SWITCH_PORTS) failed\r\n");
        cJSON_Delete(cj_pm);
        res = fs_api_fclose(&fp);
        cJSON_Delete(cj_pvlt);
        return -6;
    }
    
    res = cJSON_AddItemToObject(cj_pm, "plks", cj_plkset);
    if(!res)
    {
        LOG_PRINT_ERROR("res = cJSON_AddItemToObject(cj_ppm, \"plks\", cj_plkset) failed\r\n");
        cJSON_Delete(cj_pm);
        res = fs_api_fclose(&fp);
        cJSON_Delete(cj_pvlt);
        cJSON_Delete(cj_plkset);
        return -7;
    }
    
    out = cJSON_PrintUnformatted(cj_pm); 
    if(!out)
    {
        LOG_PRINT_ERROR("out = cJSON_PrintUnformatted(cj_ppm) failed\r\n");
        cJSON_Delete(cj_pm);
        res = fs_api_fclose(&fp);
        cJSON_Delete(cj_pvlt);
        cJSON_Delete(cj_plkset);
        return -8;
    }
//    size_w = strlen(out);
//    LOG_PRINT_INFO("%s,%d\n",out,size_w);
    crc = crc16((uint8_t*)out,strlen(out),crc16Table[CRC16_TABLE_CODE]);
    cJSON_free(out);
    
    cj_pm_f = cJSON_CreateObject();
    if(!cj_pm_f)
    {
        LOG_PRINT_ERROR("cj_pm_f = cJSON_CreateObject() failed\r\n");
        res = fs_api_fclose(&fp);
        return -9;
    }
    
    res = cJSON_AddItemToObject(cj_pm_f, "pm", cj_pm);
    if(!res)
    {
        LOG_PRINT_ERROR("cJSON_AddItemToObject(cj_pvl, \"crc16\", cj_crc) failed\r\n");
        cJSON_Delete(cj_pm);
        res = fs_api_fclose(&fp);
        cJSON_Delete(cj_pm_f);
        return -10;
    }
    
    if(!cJSON_AddNumberToObject(cj_pm_f,CHECK_CRC,crc))
    {
        LOG_PRINT_ERROR("cj_crc = cJSON_CreateObject() failed\r\n");
        res = fs_api_fclose(&fp);
        cJSON_Delete(cj_pm_f);
        return -12;
    }
    
    out = cJSON_Print(cj_pm_f); 
    cJSON_Delete(cj_pm_f);
    if(!out)
    {
        LOG_PRINT_ERROR("out = cJSON_Print(cj_pm_f) failed\r\n");
        res = fs_api_fclose(&fp);
        return -14;
    }
    
    size_w = strlen(out);
    LOG_PRINT_INFO("%s,%d\n",out,size_w);
    if(size_w <= sizeof(file_ram_pmng.file))
    {
        file_ram_pmng.flag = 1;
        strcpy(file_ram_pmng.file,out);
    }
    else
    {
        file_ram_pmng.flag = -1;
    }
    size_r = fs_api_fwrite(&fp,size_w,out);
    fs_api_truncate(&fp,size_w);
    cJSON_free(out);
    if(size_w != size_r)
    {
        LOG_PRINT_ERROR("size_w != size_r failed\r\n");
        res = fs_api_fclose(&fp);
        return -15;
    }
    
    res = fs_api_fclose(&fp);
    
    return 0;
}
/**
 * @brief 更新从ram数组中更新file
 * 
 * @param void
 * @return 0 成功 <0 错误
 */
int8_t portmng_file_update_from_rambuf(char *buf,uint16_t len)
{
    FILE_STR fp;
    cJSON *cj_pm_f,*cj_pm,*cj_crc;
    char *out;
    int32_t res = 0;
    uint16_t crc = 0;
    int32_t size_w = 0,size_r = 0;
    
    LOG_PRINT_INFO("%s,%d\n",buf,len);
    if(len > sizeof(file_ram_pmng.file))
    {
        return -1;
    }
    
    cj_pm_f = cJSON_Parse(buf);
    if (!cj_pm_f)
    {
        return -2;
    }
    
    cj_pm = cJSON_GetObjectItem(cj_pm_f, "pm");
    if (!cj_pm)
    {
        cJSON_Delete(cj_pm_f);
        return -3;
    }
    
    out = cJSON_PrintUnformatted(cj_pm); 
    if(!out)
    {
        cJSON_Delete(cj_pm_f);
        return -4;
    }
    size_w = strlen(out);
    LOG_PRINT_INFO("%s,%d\n",out,size_w);
    crc = crc16((uint8_t*)out,size_w,crc16Table[CRC16_TABLE_CODE]);
    cJSON_free(out);
    
    cj_crc = cJSON_GetObjectItem(cj_pm_f, CHECK_CRC);
    cJSON_Delete(cj_pm_f);
    if (!cj_crc)
    {
        return -5;
    }
    if(crc == cj_crc->valueint)
    {
        if(size_w >= sizeof(file_ram_pmng.file))
        {
            return -9;
        }
        memcpy(file_ram_pmng.file,buf,len);
        res = fs_api_fopen(&fp,FILE_PORT_MANAGE,"wr");
        if(res)
        {
            fs_api_fclose(&fp);
            return -6;
        }
        size_r = fs_api_fwrite(&fp,len,buf);
        fs_api_truncate(&fp,len);
        if(size_r != len)
        {
            fs_api_fclose(&fp);
            return -7;
        }
        fs_api_fclose(&fp);
    }
    else
    {
        LOG_PRINT_WARIN("portmng file CRC error:%d!=%d\r\n",crc,cj_crc->valueint);
        return -8;
    }
    
    return 0;
}
/**
 * @brief 从vlan table文件中更新数据
 * 
 * @param [in] size 文件大小
 * @return 0 成功 <0 错误
 */
int8_t portmng_parameter_update(uint32_t size)
{
    FILE_STR fp;
    int32_t res = 0;
    char* f_data = NULL;
    cJSON *cj_pm_f,*cj_pm,*cj_pvls,*cj_pvlt,*cj_pvlt_m,*cj_plkset,*cj_plkset_m,*cj_crc;
    char *out;
    int32_t size_w = 0;
    uint16_t crc = 0;
    uint32_t value = 0;
    uint8_t i=0;
    
    f_data = (char*)malloc(size+1);
    if(f_data == NULL)
    {
        LOG_PRINT_ERROR("malloc(%d) failed\r\n",size);
        return -1;
    }
    
    res = fs_api_fopen(&fp,FILE_PORT_MANAGE,"r");
    if(res)
    {
        LOG_PRINT_ERROR("%d = fs_api_fopen(&fp,%s,\"r\") failed\r\n",res,FILE_PORT_MANAGE);
        free(f_data);
        return -2;
    }
    
    res = fs_api_fread(&fp,size,f_data);
    if (res < 0)
    {
        LOG_PRINT_ERROR("%d = fs_api_fread(&fp,%d,f_data) failed\r\n",res,size);
        free(f_data);
        fs_api_fclose(&fp);
        return -3;
    }
    
    cj_pm_f = cJSON_Parse(f_data);
    if (!cj_pm_f)
    {
        LOG_PRINT_ERROR("cj_pm_f = cJSON_Parse(f_data) failed\r\n");
        free(f_data);
        fs_api_fclose(&fp);
        return -4;
    }
    
    out = cJSON_Print(cj_pm_f); 
    if(!out)
    {
        LOG_PRINT_ERROR("cJSON_PrintUnformatted(cj_pm_f) failed\r\n");
        free(f_data);
        fs_api_fclose(&fp);
        cJSON_Delete(cj_pm_f);
        return -5;
    }
    
    size_w = strlen(out);
    if(size_w <= sizeof(file_ram_pmng.file))
    {
        file_ram_pmng.flag = 1;
        strcpy(file_ram_pmng.file,out);
    }
    else
    {
        file_ram_pmng.flag = -1;
        sprintf(file_ram_pmng.file,"file save to ram failed,ram size:%d BYTE,file size:%d BYTE.",sizeof(file_ram_pmng.file),size_w);
    }
    LOG_PRINT_INFO("%s,%d\n",out,size_w);
    cJSON_free(out);
    
    cj_pm = cJSON_GetObjectItem(cj_pm_f, "pm");
    if (!cj_pm)
    {
        LOG_PRINT_ERROR("cJSON_GetObjectItem(cj_pm_f, \"pm\") failed\r\n");
        free(f_data);
        fs_api_fclose(&fp);
        cJSON_Delete(cj_pm_f);
        return -6;
    }
    
    out = cJSON_PrintUnformatted(cj_pm); 
    if(!out)
    {
        LOG_PRINT_ERROR("cJSON_PrintUnformatted(cj_pm) failed\r\n");
        free(f_data);
        fs_api_fclose(&fp);
        cJSON_Delete(cj_pm_f);
        return -7;
    }
    size_w = strlen(out);
    //LOG_PRINT_INFO("%s,%d\n",out,size_w);
    crc = crc16((uint8_t*)out,strlen(out),crc16Table[CRC16_TABLE_CODE]);
    cJSON_free(out);
    
    cj_crc = cJSON_GetObjectItem(cj_pm_f, CHECK_CRC);
    if (!cj_crc)
    {
        LOG_PRINT_ERROR("cJSON_GetObjectItem(cj_pm_f, %s) failed\r\n",CHECK_CRC);
        free(f_data);
        fs_api_fclose(&fp);
        cJSON_Delete(cj_pm_f);
        return -8;
    }
    
    if(crc == cj_crc->valueint)
    {
        cj_pvls = cJSON_GetObjectItem(cj_pm, PVL_STATE);
        if (!cj_pvls)
        {
            LOG_PRINT_ERROR("cj_pvls = cJSON_GetObjectItem(cj_pm, %s) failed\r\n",PVL_STATE);
            free(f_data);
            fs_api_fclose(&fp);
            cJSON_Delete(cj_pm_f);
            return -9;
        }
        if(cj_pvls->type != cJSON_Number)
        {
            LOG_PRINT_ERROR("pvl state format error\r\n");
            free(f_data);
            fs_api_fclose(&fp);
            cJSON_Delete(cj_pm_f);
            return -10;
        }
        switch_data.portmng.pvl_state = cj_pvls->valueint;
        
        cj_pvlt = cJSON_GetObjectItem(cj_pm, "pvlt");
        if (!cj_pvlt)
        {
            LOG_PRINT_ERROR("cj_pvlt = cJSON_GetObjectItem(cj_pm, \"pvlt\") failed\r\n");
            free(f_data);
            fs_api_fclose(&fp);
            cJSON_Delete(cj_pm_f);
            return -9;
        }
        value = cJSON_GetArraySize(cj_pvlt);
        for(i=0; i<value; i++)
        {
            cj_pvlt_m = cJSON_GetArrayItem(cj_pvlt,i);
            if (!cj_pvlt_m)
            {
                LOG_PRINT_ERROR("cJSON_GetArrayItem(cj_pvlt,%d) failed\r\n",i);
                free(f_data);
                fs_api_fclose(&fp);
                cJSON_Delete(cj_pm_f);
                return -10;
            }
            if(cj_pvlt_m->type != cJSON_Number)
            {
                LOG_PRINT_ERROR("pvl table format error\r\n");
                free(f_data);
                fs_api_fclose(&fp);
                cJSON_Delete(cj_pm_f);
                return -11;
            }
            switch_data.portmng.pvl_t[i] = cj_pvlt_m->valueint;
        }
        
        cj_plkset = cJSON_GetObjectItem(cj_pm, "plks");
        if (!cj_plkset)
        {
            LOG_PRINT_ERROR("cj_plkset = cJSON_GetObjectItem(cj_pm, \"plks\") failed\r\n");
            free(f_data);
            fs_api_fclose(&fp);
            cJSON_Delete(cj_pm_f);
            return -12;
        }
        value = cJSON_GetArraySize(cj_plkset);
        for(i=0; i<value; i++)
        {
            cj_plkset_m = cJSON_GetArrayItem(cj_plkset,i);
            if (!cj_plkset_m)
            {
                LOG_PRINT_ERROR("cJSON_GetArrayItem(cj_plkset,%d) failed\r\n",i);
                free(f_data);
                fs_api_fclose(&fp);
                cJSON_Delete(cj_pm_f);
                return -13;
            }
            if(cj_plkset_m->type != cJSON_Number)
            {
                LOG_PRINT_ERROR("plks format error\r\n");
                free(f_data);
                fs_api_fclose(&fp);
                cJSON_Delete(cj_pm_f);
                return -11;
            }
                switch_data.portmng.lk_set[i] = cj_plkset_m->valueint;
        }
    }

    free(f_data);
    fs_api_fclose(&fp);
    cJSON_Delete(cj_pm_f);

    return 0;
}

/**
 * @brief 将IP字符串转换成4个整数
 * 
 * @param [in] *src 字符串
 * @param [in] *dec 转换后的IP数值存放缓存，不应小于4byte
 * @return 0 成功 <0 错误
 */
int8_t strIP4_to_array(uint8_t* dec,const char* src)
{
    int8_t state;
    uint8_t value = 0;

    state = checkIPformat(src);

    if(state) return state;

    do
    {
        if(*src == '.')
        {
            value = 0;
            dec++;
        }
        else
        {
            value = value * 10 + *src - '0';
            *dec = value;
        }
        src++;
    }while(*src != '\0');

    return 0;
}

/**
 * @brief 判断IP4格式的正确性 U8.U8.U8.U8
* 
 * @param [in] *src 字符串
 * @return 0 成功 <0 错误
 */
int8_t checkIPformat(const char* src)
{
    uint8_t len,i;
    uint8_t pcont = 0;
    uint8_t icont_buf[4] = {0};
    uint16_t value = 0;
    
    len = strlen(src);

    if(len < 7 || len > 15)
    {
        return -1;        
    }

    for(i=0; i<len; i++)
    {
        if((src[i] < '0')||(src[i] > '9'))
        {

            if((src[i] != '.')||(i == 0)||(i == (len-1)))
            {
                return -2;            
            }
            value = 0;
            pcont++;
            if(pcont > 3)
            {
                return -2;            
            }
        }
        else
        {
            icont_buf[pcont]++;
            if(icont_buf[pcont] > 3)
            {
                return -2;            
            }
            
            value = value * 10 + src[i] - '0';
            if(value > 255)
            {
                return -2;            
            }
        }
    }

    return 0;
}


/**
 * @brief 将MAC字符串转换成6个整数
 * 

 * @param [in] *src 字符串
 * @param [in] *dec 转换后的MAC数值存放缓存，不应小于6byte
 * @return 0 成功 <0 错误
 */
int8_t strMAC_to_array(uint8_t* dec,const char* src)
{
    int8_t state;
    uint8_t value = 0;
    uint8_t type = 0;

    state = checkMACformat(src);

    if(state) return state;

    do
    {
        if(*src == '-')
        {
            value = 0;
            dec++;
        }
        else
        {
            if((*src  >= '0') && (*src  <= '9'))
            {
                type = 0x30;
            }
            else if((*src >= 'a') && (*src <= 'f'))
            {
                type = 'a'-10;
            }
            else if((*src >= 'A') && (*src <= 'F'))
            {
                type = 'A'-10;
            }
            value = (value << 4) + *src - type;
            *dec = value;
        }
        src++;
    }while(*src != '\0');

    return 0;
}

/**
 * @brief 判断MAC格式的正确性 HEX-HEX-HEX-HEX-HEX-HEX
 * 
 * @param [in] *src 字符串
 * @return 0 成功 <0 错误
 */
int8_t checkMACformat(const char* src)
{
    uint8_t len,i;
    uint8_t pcont = 0;
    uint8_t icont_buf[7] = {0};
    
    len = strlen(src);

    if(len != 17)
    {
        return -1;        
    }

    for(i=0; i<len; i++)
    {
        if(((src[i] < '0')||(src[i] > '9'))&&((toupper(src[i]) < 'A') || (toupper(src[i]) > 'F')))
        {
            if((src[i] != '-')||(i == 0)||(i == (len-1)))
            {
                return -2;            
            }
            pcont++;
            if(pcont > 6)
            {
                return -2;            
            }
        }
        else
        {
            icont_buf[pcont]++;
            if(icont_buf[pcont] > 2)
            {
                return -2;            
            }
        }
    }

    return 0;
}

