#include "lwip/debug.h"
#include "lwip/tcp.h"

#include "fs.h"
#include "ethernetif.h"
#include "lwip.h"
#include "sys_info.h"
#include "switch_app.h"
#include <string.h>
#include <stdlib.h>   
#include "httpd_structs.h"
#include "httpd.h"

//CGI函数
const char* LOGIN_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char* MONITOR_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char* PVL_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char* BUCKUP_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char* RECOVER_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char* FMMENU_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char* SYSSETTING_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char* AJAX_MINITOR_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);

//SSI函数
static void req_basic_info_get( char * pcBuf, int iBufLen );
static void req_monitor_get( char * pcBuf, int iBufLen );
static void req_pvl_get( char * pcBuf, int iBufLen );
static void req_pmngcfg_get( char * pcBuf, int iBufLen );
static void req_bkprcv_get( char * pcBuf, int iBufLen );
static void req_password_get( char * pcBuf, int iBufLen );
static void req_swpage_get( char * pcBuf, int iBufLen );
static void req_sysst_get( char * pcBuf, int iBufLen );
static void ajax_monitor_get( char * pcBuf, int iBufLen );
    
//JS语言标注
#define JAVASCRIPT_HEADER   "<script type='text/javascript' language='JavaScript'><!--\n"
#define JAVASCRIPT_FOOTER   "//--></script>\n"

//弹窗反馈
char cgi_to_ssi_buf[64]={"\"\""};
//链接
#define INDEX_PAGE_SET_CGI_RSP_URL                 "/index.shtml"
#define SWITCH_PAGE_SET_CGI_RSP_URL             "/switch.shtml"
#define BASIC_INFO_PAGE_SET_CGI_RSP_URL         "/basic_info.shtml"
#define DATA_MONITOR_PAGE_SET_CGI_RSP_URL         "/data_monitor.shtml"
#define PVLAN_SET_PAGE_SET_CGI_RSP_URL             "/vlan_set.shtml"
#define RECOVER_PAGE_SET_CGI_RSP_URL             "/backup_recover.shtml"
#define BUCKUP_CFG_SET_CGI_RSP_URL              "/HR_LINK.cfg"
#define SYSSETTING_PAGE_SET_CGI_RSP_URL             "/sys_setting.shtml"
#define RESETWAIT_PAGE_SET_CGI_RSP_URL             "/reset_wait.shtml"
#define JUMP_PAGE_SET_CGI_RSP_URL             "/jump.shtml"
#define AJAX_MONITOR_SET_CGI_RSP_URL            "/monitor.ajax"

//句柄数
#define NUM_CONFIG_SSI_TAGS     (sizeof(g_pcConfigSSITags) / sizeof (char *))
#define NUM_CONFIG_CGI_URIS     (sizeof(g_psConfigCGIURIs) / sizeof(tCGI))
//打包缓存
char ssi_buff[96]={0};
char cgi_buff[16]={0};
//SSI关键字
#define    SSITAGS_INFOGET        "infoGet"
#define    SSITAGS_MONITOR        "monitor"
#define    SSITAGS_PVLSET        "pvl_set"
#define    SSITAGS_PMNGCFG        "pmng_cfg"
#define    SSITAGS_BKPRCV        "bkprcv"
#define    SSITAGS_PASSWORD    "pw_state"
#define    SSITAGS_SWPAGE        "sw_page"
#define    SSITAGS_SYSST        "sysst_page"
#define    SSITAGS_AJAX_MONITOR    "ajax_monitor"
//备份文件传输采用自定义方法
#define    BUCKUPRECOVE_BSIF_TAG    "<!--#bsif_cfg-->\r\n"
#define    BUCKUPRECOVE_PMNG_TAG    "<!--#"SSITAGS_PMNGCFG"-->\r\n"
#define    BUCKUPRECOVE_CUT_TAG    "</>"

//cgi句柄
static const tCGI g_psConfigCGIURIs[]= 
{
    {"/login.cgi",LOGIN_CGI_Handler},
    {"/monitor.cgi",MONITOR_CGI_Handler},
    {"/pvl_set.cgi",PVL_CGI_Handler},
    {"/buckup.cgi",BUCKUP_CGI_Handler},
    {"/recover.cgi",RECOVER_CGI_Handler},
    {"/fm_menu.cgi",FMMENU_CGI_Handler},
    {"/sys_setting.cgi",SYSSETTING_CGI_Handler},
    {"/ajax_minitor.cgi",AJAX_MINITOR_CGI_Handler},
    
};

//SSI关键字
static const char *g_pcConfigSSITags[] =
{
    SSITAGS_INFOGET,
    SSITAGS_MONITOR,
    SSITAGS_PVLSET, 
    SSITAGS_PMNGCFG,
    SSITAGS_BKPRCV,
    SSITAGS_PASSWORD,
    SSITAGS_SWPAGE,
    SSITAGS_SYSST,
    SSITAGS_AJAX_MONITOR,
};
enum ssi_index_s
{
    SSI_INDEX_INFO_GET = 0, //该表对应g_pcConfigSSITags[]的排序
    SSI_INDEX_MONITOR, 
    SSI_INDEX_PVL_GET,
    SSI_INDEX_CFG_PMNG_GET,
    SSI_INDEX_BKPRCV_GET,
    SSI_PASSWORD_GET,
    SSI_SWPAGE_GET,
    SSI_SYSST_GET,
    SSITAGS_AJAX_MONITOR_GET,
} ;

//POST传输方式URL数据结构
typedef struct 
{
    const char*     res_url;
    int8_t    index;
}str_htppd_post_procs;

//Web用户管理数据结构
typedef struct 
{
    char username[12];
    char password[12];
    uint8_t state;
    uint32_t time;
}str_lwip_login;

//全局变量定义
str_htppd_post_procs    htppd_post_procs;
str_lwip_login lwip_login;
uint8_t web_login = 0;

//登录状态管理三个函数
void web_login_tag(uint8_t state)
{
    lwip_login.state = state;    
}
void web_login_monitor(void)
{
    if(lwip_login.state)
    {
        lwip_login.time++;
        if(lwip_login.time > 10*60*1000)
        {
            lwip_login.time = 0;
            lwip_login.state = 0;
        }
    }
}
uint8_t web_login_state(void)
{
    lwip_login.time = 0;
    return lwip_login.state;
}
//当web客户端请求浏览器的时候,使用此函数被CGI handler调用
static int FindCGIParameter(const char *pcToFind,char *pcParam[],int iNumParams)
{
    int iLoop;
    for(iLoop = 0;iLoop < iNumParams;iLoop ++ )
    {
        if(strcmp(pcToFind,pcParam[iLoop]) == 0)
        {
            return (iLoop); //返回iLOOP
        }
    }
    return (-1);
}

//CGI LOGIN控制句柄
const char* LOGIN_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    int s_index = 0;
    int8_t state = 0;
    uint8_t i = 0;
    
    if(web_login_state()==0)
    {
        if(iNumParams)
        {
            s_index = FindCGIParameter("btnli",pcParam,iNumParams);  
            if (s_index != -1)
            {
                s_index = FindCGIParameter("user",pcParam,iNumParams); 
                if (s_index != -1)
                {
                    if(strcmp(pcValue[s_index],lwip_login.username) != 0)
                    {
                        sprintf(cgi_to_ssi_buf,"1");
                        state = 1;
                    }
                }
                else
                {
                    sprintf(cgi_to_ssi_buf,"1");
                        state = 1;
                }
                s_index = FindCGIParameter("password",pcParam,iNumParams); 
                if (s_index != -1)
                {
                    if(strcmp(pcValue[s_index],lwip_login.password) != 0)
                    {
                        sprintf(cgi_to_ssi_buf,"2");
                        state = 2;
                    }    
                }
                else
                {
                    sprintf(cgi_to_ssi_buf,"2");
                    state = 2;
                }
            }
            else if(iNumParams)
            {
                sprintf(cgi_to_ssi_buf,"3");
                state = 3;
            }
        }
        else
        {
            sprintf(cgi_to_ssi_buf,"4");
            state = 4;
        }
    }
    else if(iNumParams)
    {
        
        sprintf(cgi_to_ssi_buf,"5");
        state = 5;
    }
    for(i=0;i<iNumParams;i++)
    {
        memset(pcParam[i],0,strlen(pcParam[i]));
        memset(pcValue[i],0,strlen(pcValue[i]));
    }
    if(state)
    {
        web_login_tag(0);
        return INDEX_PAGE_SET_CGI_RSP_URL;
    }
    web_login_tag(1);
    sprintf(cgi_to_ssi_buf,"1");
    return SWITCH_PAGE_SET_CGI_RSP_URL;
}
//CGI MONITOR控制句柄
const char* MONITOR_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    iIndex = FindCGIParameter("clear",pcParam,iNumParams);  
    
    if(web_login_state())
    {
        if (iIndex != -1)
        {
            sprintf(cgi_to_ssi_buf,"\"success\"");
        }        
        else
        {
            sprintf(cgi_to_ssi_buf,"\"failure\"");
        }
        return DATA_MONITOR_PAGE_SET_CGI_RSP_URL;
    }

    sprintf(cgi_to_ssi_buf,"4");
    return JUMP_PAGE_SET_CGI_RSP_URL;

}
//CGI 端口VLAN控制句柄
const char* PVL_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    uint8_t i = 0;
    int16_t value = 0;
    uint16_t t_data = 0;
    uint8_t res = 0;
    uint8_t tb1_buf[MAX_SWITCH_PORTS];
    uint16_t tb2_buf[MAX_SWITCH_PORTS]={0};
    
    if(web_login_state())
    {
        sprintf(cgi_to_ssi_buf,"\"success\"");
        iIndex = FindCGIParameter("pvl_add",pcParam,iNumParams);  
        if (iIndex != -1)
        {
            iIndex = FindCGIParameter("psl",pcParam,iNumParams);  
            if (iIndex != -1)
            {
                value = atoi(pcValue[iIndex]);
                if((value > 0) && (value < MAX_SWITCH_PORTS))
                {
                    for(i=1; i<MAX_SWITCH_PORTS; i++)
                    {
                        sprintf(cgi_buff,"mpvl%d",i);
                        iIndex = FindCGIParameter(cgi_buff,pcParam,iNumParams);  
                        if (iIndex != -1)
                        {
                            t_data|=0x0001<<(i-1);
                        }
                    }
                    value-=1;
                    res = switch_vlantable_insert(&t_data,(uint8_t*)(&value),1);
                    if(res)
                    {
                        sprintf(cgi_to_ssi_buf,"\"vlan table insert failed:E%d\"",res);
                    }
                }
                else
                {
                    sprintf(cgi_to_ssi_buf,"\"port number error\"");
                }
            }
            else
            {
                sprintf(cgi_to_ssi_buf,"\"vlan table error\"");
            }
        }        
        else
        {
            iIndex = FindCGIParameter("pvl_e",pcParam,iNumParams);  
            if (iIndex != -1)
            {
                value = atoi(pcValue[iIndex]);
                if(value==0 || value==1)
                {
                    switch_data.portmng.pvl_state = value;
                    res = portmng_file_update();
                    if(res)
                    {
                        sprintf(cgi_to_ssi_buf,"\"pvlan close failed:E%d\"",res);
                    }
                }
            }
            else
            {
                iIndex = FindCGIParameter("pvlan_del",pcParam,iNumParams);
                if (iIndex != -1)
                {
                    t_data = 0;
                    for(i=0; i<iNumParams; i++)
                    {
                        if(strcmp(pcParam[i],"selVlans") == 0)
                        {
                            value = atoi(pcValue[i]);
                            if((value > 0) && (value < MAX_SWITCH_PORTS))
                            {
                                tb1_buf[t_data] = value-1;
                                t_data++;
                            }
                            else
                            {
                                sprintf(cgi_to_ssi_buf,"\"delete failure:table %d\"",value);
                            }
                        }
                    }
                    res = switch_vlantable_insert(tb2_buf,tb1_buf,t_data);
                    if(res)
                    {
                        sprintf(cgi_to_ssi_buf,"\"vlan table insert failed:E%d\"",res);
                    }
                }
                else
                {
                    sprintf(cgi_to_ssi_buf,"\"failure\"");
                }
            }
        }
        return PVLAN_SET_PAGE_SET_CGI_RSP_URL;
    }
    
    sprintf(cgi_to_ssi_buf,"4");
    return JUMP_PAGE_SET_CGI_RSP_URL;
    
}
//CGI 导出配置文件句柄
const char* BUCKUP_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    if(web_login_state())
    {
        return BUCKUP_CFG_SET_CGI_RSP_URL;
    }
    
    sprintf(cgi_to_ssi_buf,"4");
    return JUMP_PAGE_SET_CGI_RSP_URL;
    
}
//CGI 导入配置文件句柄
const char* RECOVER_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    char *p=NULL;
    char *p_s=NULL;
    char *p_e=NULL;
    char *buf=NULL;
    int16_t len=0;
    int8_t res;
    char str_buf[32]={0};
    
    if(web_login_state())
    {    
        sprintf(cgi_to_ssi_buf,"\"pmng_cfg no changed ");
        strcat(cgi_to_ssi_buf,",bsif_cfg no changed.\"");
        
        if((iIndex==-1) && (iNumParams==0))
        {
            sprintf(cgi_to_ssi_buf,"\"Failed,config file too large!\"");
        }
        else if((iIndex!=-1) && (iNumParams==-1))
        {
            sprintf(cgi_to_ssi_buf,"\"Failed,Handler error!\"");
        }
        else if(iNumParams>0)
        {
            p = strstr(pcParam[0],"Content-Disposition:");
            if (p != NULL)
            {
                p+=20;
                while(*p == '\0')
                {
                    p++;
                }
                
                p_s = p;
                p_s = strstr(p_s,BUCKUPRECOVE_PMNG_TAG);
                if(p_s != NULL)
                {
                    p_s+=strlen(BUCKUPRECOVE_PMNG_TAG);
                    p_e = strstr(p_s,BUCKUPRECOVE_CUT_TAG);
                    if(p_e != NULL)
                    {
                        len = p_e-p_s;
                        buf = malloc(len+1);
                        if(buf != NULL)
                        {
                            memset(buf,0,len+1);
                            memcpy(buf,p_s,len);
                            res = portmng_file_update_from_rambuf(buf,len);
                            if(res)
                            {
                                sprintf(cgi_to_ssi_buf,"\"pmng_cfg failed:E%d",res);
                            }
                            else
                            {
                                sprintf(cgi_to_ssi_buf,"\"pmng_cfg success");
                            }
                            free(buf);
                        }
                        else
                        {
                            sprintf(cgi_to_ssi_buf,"\"pmng_cfg malloc failed");
                        }
                    }
                    else
                    {
                        sprintf(cgi_to_ssi_buf,"\"pmng_cfg </> don't found");
                    }
                }
                else
                {
                    sprintf(cgi_to_ssi_buf,"\"No pmng_cfg file");
                }
                
                p_s = p;
                p_s = strstr(p_s,BUCKUPRECOVE_BSIF_TAG);
                if(p_s != NULL)
                {
                    p_s+=strlen(BUCKUPRECOVE_BSIF_TAG);
                    p_e = strstr(p_s,BUCKUPRECOVE_CUT_TAG);
                    if(p_e != NULL)
                    {
                        len = p_e-p_s;
                        buf = malloc(len+1);
                        if(buf != NULL)
                        {
                            memset(buf,0,len+1);
                            memcpy(buf,p_s,len);
                            LOG_PRINT_INFO("bsif_cfg FILE:%s,%d\n",buf,len);
                            res = binfo_file_update_from_rambuf(buf,len);
                            if(res)
                            {
                                if(res > 0)
                                {
                                    sprintf(str_buf,",bsif_cfg failed item:0x%02x\"",res);
                                }
                                else
                                {
                                    sprintf(str_buf,",bsif_cfg Error:E%d\"",res);
                                }
                                strcat(cgi_to_ssi_buf,str_buf);
                            }
                            else
                            {
                                strcat(cgi_to_ssi_buf,",bsif_cfg success.\"");
                            }
                            free(buf);
                        }
                        else
                        {
                            strcat(cgi_to_ssi_buf,",bsif_cfg malloc failed.\"");
                        }
                    }
                    else
                    {
                        strcat(cgi_to_ssi_buf,",bsif_cfg </> don't found.\"");
                    }
                }
                else
                {
                    strcat(cgi_to_ssi_buf,",No bsif_cfg file .\"");
                }
            }
        }
        
        return RECOVER_PAGE_SET_CGI_RSP_URL;
    }
    
    sprintf(cgi_to_ssi_buf,"4");
    return JUMP_PAGE_SET_CGI_RSP_URL;
    
}
//菜单界面切换
const char* FMMENU_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    if(web_login_state())
    {
        iIndex = FindCGIParameter("mn_bsif",pcParam,iNumParams);  
        if (iIndex != -1)
        {
            sprintf(cgi_to_ssi_buf,"1");
            return SWITCH_PAGE_SET_CGI_RSP_URL;
        }
        
        iIndex = FindCGIParameter("mn_monitor",pcParam,iNumParams);  
        if (iIndex != -1)
        {
            sprintf(cgi_to_ssi_buf,"2");
            return SWITCH_PAGE_SET_CGI_RSP_URL;
        }
        
        iIndex = FindCGIParameter("mn_pvlt",pcParam,iNumParams); 
        if (iIndex != -1)
        {
            sprintf(cgi_to_ssi_buf,"3");
            return SWITCH_PAGE_SET_CGI_RSP_URL;
        }
        
        iIndex = FindCGIParameter("mn_set",pcParam,iNumParams);  
        if (iIndex != -1)
        {
            sprintf(cgi_to_ssi_buf,"4");
            return SWITCH_PAGE_SET_CGI_RSP_URL;
        }
        
        iIndex = FindCGIParameter("mn_bkrv",pcParam,iNumParams);
        if (iIndex != -1)
        {
            sprintf(cgi_to_ssi_buf,"5");
            return SWITCH_PAGE_SET_CGI_RSP_URL;
        }
        
        iIndex = FindCGIParameter("mn_logout",pcParam,iNumParams);  
        if (iIndex != -1)
        {
            web_login_tag(0);
            sprintf(cgi_to_ssi_buf,"5");
            return INDEX_PAGE_SET_CGI_RSP_URL;
        }
        sprintf(cgi_to_ssi_buf,"10");
        web_login_tag(0);
        return INDEX_PAGE_SET_CGI_RSP_URL;
    }
    sprintf(cgi_to_ssi_buf,"4");
    return JUMP_PAGE_SET_CGI_RSP_URL;
}
//系统设置
const char* SYSSETTING_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    int8_t state = 0;
    
    if(web_login_state())
    {
        sprintf(cgi_to_ssi_buf,"\"success,reboot to take effect\"");
        iIndex = FindCGIParameter("ip_address",pcParam,iNumParams);  
        if (iIndex != -1)
        {
            state = strIP4_to_array(IP_ADDRESS,pcValue[iIndex]);
            if(state)
            {
                sprintf(cgi_to_ssi_buf,"\"failed,IP address format error\"");
                return SYSSETTING_PAGE_SET_CGI_RSP_URL;
            }
        }
        else
        {
            sprintf(cgi_to_ssi_buf,"\"failed:");
            strcat(cgi_to_ssi_buf,"ip");
        }
        
        iIndex = FindCGIParameter("ip_netmask",pcParam,iNumParams);  
        if (iIndex != -1)
        {
            state = strIP4_to_array(NETMASK_ADDRESS,pcValue[iIndex]);
            if(state)
            {
                sprintf(cgi_to_ssi_buf,"\"failed,mask error\"");
                return SYSSETTING_PAGE_SET_CGI_RSP_URL;
            }
        }
        else
        {
            strcat(cgi_to_ssi_buf,",mask");
        }
        
        iIndex = FindCGIParameter("nm_gateway",pcParam,iNumParams);  
        if (iIndex != -1)
        {
            state = strIP4_to_array(GATEWAY_ADDRESS,pcValue[iIndex]);
            if(state)
            {
                sprintf(cgi_to_ssi_buf,"\"failed,gateway error\"");
                return SYSSETTING_PAGE_SET_CGI_RSP_URL;
            }
        }
        else
        {
            strcat(cgi_to_ssi_buf,",gateway");
        }
        
        iIndex = FindCGIParameter("nm_mac",pcParam,iNumParams);  
        if (iIndex != -1)
        {
            state = strMAC_to_array(MACAddr,pcValue[iIndex]);
            if(state)
            {
                sprintf(cgi_to_ssi_buf,"\"failed,mac error\"");
            }
            else
            {
                state = binfo_file_update();
                if(state)
                {
                    sprintf(cgi_to_ssi_buf,"\"failed,save error\"");
                }
            }
            return SYSSETTING_PAGE_SET_CGI_RSP_URL;
        }
        else
        {
            strcat(cgi_to_ssi_buf,",mac\"");
        }
        
        iIndex = FindCGIParameter("reset",pcParam,iNumParams); 
        if (iIndex != -1)
        {
            web_login_tag(0);
            sprintf(cgi_to_ssi_buf,"");
            //200ms后复位
            set_reboot(200);
            return RESETWAIT_PAGE_SET_CGI_RSP_URL;
        }
        
        iIndex = FindCGIParameter("default",pcParam,iNumParams); 
        if (iIndex != -1)
        {
            web_login_tag(0);
            sprintf(cgi_to_ssi_buf,"");
            sys_default();
            return RESETWAIT_PAGE_SET_CGI_RSP_URL;
        }
        
        return SYSSETTING_PAGE_SET_CGI_RSP_URL;
    }
    
    sprintf(cgi_to_ssi_buf,"4");
    return JUMP_PAGE_SET_CGI_RSP_URL;
    
}
//AJAX monitor界面
const char* AJAX_MINITOR_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    if(web_login_state())
    {
        return AJAX_MONITOR_SET_CGI_RSP_URL;
    }
    
    sprintf(cgi_to_ssi_buf,"4");
    return JUMP_PAGE_SET_CGI_RSP_URL;
}
//SSI的Handler句柄
static uint16_t SSIHandler(int iIndex,char *pcInsert,int iInsertLen)
{
    switch(iIndex)
    {
        case SSI_INDEX_INFO_GET: 
                req_basic_info_get( pcInsert, iInsertLen );
                break;
        case SSI_INDEX_MONITOR:
                req_monitor_get( pcInsert, iInsertLen );
                break;
        case SSI_INDEX_PVL_GET:
                req_pvl_get( pcInsert, iInsertLen );
                break;
        case SSI_INDEX_CFG_PMNG_GET:
                req_pmngcfg_get( pcInsert, iInsertLen );
                break;
        case SSI_INDEX_BKPRCV_GET:
                req_bkprcv_get( pcInsert, iInsertLen );
                break;
        //index.shtml弹窗
        case SSI_PASSWORD_GET:
                req_password_get( pcInsert, iInsertLen );
                break;
        //switch.shtml弹窗
        case SSI_SWPAGE_GET:
                req_swpage_get( pcInsert, iInsertLen );
            break;
        case SSI_SYSST_GET:
                req_sysst_get( pcInsert, iInsertLen );
            break;
        case SSITAGS_AJAX_MONITOR_GET:
            ajax_monitor_get( pcInsert, iInsertLen );
            break;
        default:break;
    }
    return strlen(pcInsert);
}

//SSI句柄初始化
void httpd_ssi_init(void)
{  
    //配置SSI句柄
    http_set_ssi_handler(SSIHandler,g_pcConfigSSITags,NUM_CONFIG_SSI_TAGS);
}

//CGI句柄初始化
void httpd_cgi_init(void)
{ 
    //配置CGI句柄
    http_set_cgi_handlers(g_psConfigCGIURIs, NUM_CONFIG_CGI_URIS);
    memset(&lwip_login,0,sizeof(lwip_login));
    //固化默认密码
    sprintf(lwip_login.username,"%s","admin");
    sprintf(lwip_login.password,"%s","hairou");
    
}
//basic_info.shtml数据插入
static void req_basic_info_get( char * pcBuf, int iBufLen )
{
    strcpy ( pcBuf , JAVASCRIPT_HEADER );

    sprintf ( ssi_buff, "nf.desc=\"%s\";\n",dev_name);   
    strcat ( pcBuf , ssi_buff );
    
    sprintf ( ssi_buff, "nf.mac=\"%02x-%02x-%02x-%02x-%02x-%02x\";\n",MACAddr[0],MACAddr[1],MACAddr[2],MACAddr[3],MACAddr[4],MACAddr[5]);
    strcat ( pcBuf , ssi_buff );
     
    sprintf ( ssi_buff, "nf.ip=\"%d.%d.%d.%d\";\n",IP_ADDRESS[0],IP_ADDRESS[1],IP_ADDRESS[2],IP_ADDRESS[3]);
    strcat ( pcBuf , ssi_buff ); 
    
    sprintf ( ssi_buff, "nf.nm=\"%d.%d.%d.%d\";\n",NETMASK_ADDRESS[0],NETMASK_ADDRESS[1],NETMASK_ADDRESS[2],NETMASK_ADDRESS[3]);
    strcat ( pcBuf , ssi_buff );
    
    sprintf ( ssi_buff, "nf.gw=\"%d.%d.%d.%d\";\n",GATEWAY_ADDRESS[0],GATEWAY_ADDRESS[1],GATEWAY_ADDRESS[2],GATEWAY_ADDRESS[3]);
    strcat ( pcBuf , ssi_buff );
    
    sprintf ( ssi_buff, "nf.sv=\"%s\";\n",sw_ver);
    strcat ( pcBuf , ssi_buff );
        
    sprintf ( ssi_buff, "nf.hv=\"%s\";\n",hw_ver);
    strcat ( pcBuf , ssi_buff );

    LOG_PRINT_INFO("req_basic_info_get:%d Byte\n",strlen(pcBuf));
    strcat ( pcBuf , JAVASCRIPT_FOOTER );

}
//data_monitor.shtml数据插入
static void req_monitor_get( char * pcBuf, int iBufLen )
{
    uint8_t j = 0,i=0;

    sprintf ( ssi_buff, "\nvar tip=%s;\n",cgi_to_ssi_buf);   
    strcat ( pcBuf , ssi_buff );
    sprintf ( cgi_to_ssi_buf, "\"\"");
    
    sprintf ( ssi_buff, "var pmax = %d;\n",MAX_SWITCH_PORTS-1);   
    strcat ( pcBuf , ssi_buff );
    
    sprintf ( ssi_buff, "var d_nf = {\n");   
    strcat ( pcBuf , ssi_buff );
    
    sprintf ( ssi_buff, "sw_s:[");   
    strcat ( pcBuf , ssi_buff );
    
    for(i=0,j=1; j<MAX_SWITCH_PORTS; j++)
    {
        if(switch_data.portmng.lk_set[j])
        {
            ssi_buff[i] = '1';
        }
        else
        {
            ssi_buff[i] = '0';
        }
        i++;
        if(j < (MAX_SWITCH_PORTS-1))
        {
            ssi_buff[i] = ',';
        }
        else
        {
            ssi_buff[i] = ']';
            ssi_buff[i+1] = ',';
            ssi_buff[i+2] = '\n';
            ssi_buff[i+3] = '\0';
            break;
        }
        i++;
    }
    strcat ( pcBuf , ssi_buff );
    
    sprintf ( ssi_buff, "lk_s:[");   
    strcat ( pcBuf , ssi_buff );
    for(i=0,j=1; j<MAX_SWITCH_PORTS; j++)
    {
        ssi_buff[i] = switch_data.lk_s[j]+0x30;
        i++;
        if(j < (MAX_SWITCH_PORTS-1))
        {
            ssi_buff[i] = ',';
        }
        else
        {
            ssi_buff[i] = ']';
            ssi_buff[i+1] = ',';
            ssi_buff[i+2] = '\n';
            ssi_buff[i+3] = '\0';
            break;
        }
        i++;
    }
    strcat ( pcBuf , ssi_buff );
    
    sprintf ( ssi_buff, "lk_set:[");   
    strcat ( pcBuf , ssi_buff );
    for(i=0,j=1; j<MAX_SWITCH_PORTS; j++)
    {
        ssi_buff[i] = switch_data.portmng.lk_set[j]+0x30;
        i++;
        if(j < (MAX_SWITCH_PORTS-1))
        {
            ssi_buff[i] = ',';
        }
        else
        {
            ssi_buff[i] = ']';
            ssi_buff[i+1] = ',';
            ssi_buff[i+2] = '\n';
            ssi_buff[i+3] = '\0';
            break;
        }
        i++;
    }
    strcat ( pcBuf , ssi_buff );
    
    sprintf ( ssi_buff, "txs:[");   
    strcat ( pcBuf , ssi_buff );
    for(i=0,j=1; j<MAX_SWITCH_PORTS; j++)
    {
        sprintf ( ssi_buff, "%.2f",switch_data.portmng.throughput[j].txs_val/100.0); 
        strcat ( pcBuf , ssi_buff );
        if(j < (MAX_SWITCH_PORTS-1))
        {
            ssi_buff[0] = ',';
            ssi_buff[1] = switch_data.portmng.throughput[j].txs_unit+0x30;
            ssi_buff[2] = ',';
            ssi_buff[3] = '\0';
            strcat ( pcBuf , ssi_buff );
        }
        else
        {
            ssi_buff[0] = ',';
            ssi_buff[1] = switch_data.portmng.throughput[j].txs_unit+0x30;
            ssi_buff[2] = ']';
            ssi_buff[3] = ',';
            ssi_buff[4] = '\n';
            ssi_buff[5] = '\0';
            strcat ( pcBuf , ssi_buff );
            break;
        }
    }
    
    sprintf ( ssi_buff, "rxs:[");   
    strcat ( pcBuf , ssi_buff );
    for(i=0,j=1; j<MAX_SWITCH_PORTS; j++)
    {
        sprintf ( ssi_buff, "%.2f",switch_data.portmng.throughput[j].rxs_val/100.0); 
        strcat ( pcBuf , ssi_buff );
        if(j < (MAX_SWITCH_PORTS-1))
        {
            ssi_buff[0] = ',';
            ssi_buff[1] = switch_data.portmng.throughput[j].rxs_unit+0x30;
            ssi_buff[2] = ',';
            ssi_buff[3] = '\0';
            strcat ( pcBuf , ssi_buff );
        }
        else
        {
            ssi_buff[0] = ',';
            ssi_buff[1] = switch_data.portmng.throughput[j].rxs_unit+0x30;
            ssi_buff[2] = ']';
            ssi_buff[3] = ',';
            ssi_buff[4] = '\n';
            ssi_buff[5] = '\0';
            strcat ( pcBuf , ssi_buff );
            break;
        }
    }
    
    sprintf ( ssi_buff, "};");   
    strcat ( pcBuf , ssi_buff );
    
    LOG_PRINT_INFO("req_monitor_get:%d Byte\n",strlen(pcBuf));

}
//ajax_monitor数据插入
static void ajax_monitor_get( char * pcBuf, int iBufLen )
{
    uint8_t j = 0,i=0;
    
    sprintf ( ssi_buff, ";%d;",MAX_SWITCH_PORTS-1);   
    strcpy ( pcBuf , ssi_buff );    
    
    for(i=0,j=1; j<MAX_SWITCH_PORTS; j++)
    {
        if(switch_data.portmng.lk_set[j])
        {
            ssi_buff[i] = '1';
        }
        else
        {
            ssi_buff[i] = '0';
        }
        i++;
        if(j < (MAX_SWITCH_PORTS-1))
        {
            ssi_buff[i] = ',';
        }
        else
        {
            ssi_buff[i] = ';';
            ssi_buff[i+1] = '\0';
            break;
        }
        i++;
    }
    strcat ( pcBuf , ssi_buff );
    
    for(i=0,j=1; j<MAX_SWITCH_PORTS; j++)
    {
        ssi_buff[i] = switch_data.lk_s[j]+0x30;
        i++;
        if(j < (MAX_SWITCH_PORTS-1))
        {
            ssi_buff[i] = ',';
        }
        else
        {
            ssi_buff[i] = ';';
            ssi_buff[i+1] = '\0';
            break;
        }
        i++;
    }
    strcat ( pcBuf , ssi_buff );
    
    for(i=0,j=1; j<MAX_SWITCH_PORTS; j++)
    {
        ssi_buff[i] = switch_data.portmng.lk_set[j]+0x30;
        i++;
        if(j < (MAX_SWITCH_PORTS-1))
        {
            ssi_buff[i] = ',';
        }
        else
        {
            ssi_buff[i] = ';';
            ssi_buff[i+1] = '\0';
            break;
        }
        i++;
    }
    strcat ( pcBuf , ssi_buff );
    
    for(i=0,j=1; j<MAX_SWITCH_PORTS; j++)
    {
        sprintf ( ssi_buff, "%.2f",switch_data.portmng.throughput[j].txs_val/100.0); 
        strcat ( pcBuf , ssi_buff );
        ssi_buff[0] = ',';
        ssi_buff[1] = switch_data.portmng.throughput[j].txs_unit+0x30;
        if(j < (MAX_SWITCH_PORTS-1))
        {
            ssi_buff[2] = ',';
        }
        else
        {
            ssi_buff[2] = ';';
        }
        ssi_buff[3] = '\0';
        strcat ( pcBuf , ssi_buff );
    }
    
    for(i=0,j=1; j<MAX_SWITCH_PORTS; j++)
    {
        sprintf ( ssi_buff, "%.2f",switch_data.portmng.throughput[j].rxs_val/100.0); 
        strcat ( pcBuf , ssi_buff );
        ssi_buff[0] = ',';
        ssi_buff[1] = switch_data.portmng.throughput[j].rxs_unit+0x30;
        if(j < (MAX_SWITCH_PORTS-1))
        {    
            ssi_buff[2] = ',';
        }
        else
        {
            ssi_buff[2] = ';';
        }
        ssi_buff[3] = '\0';
        strcat ( pcBuf , ssi_buff );
    } 
    
    for(i=0,j=1; j<MAX_SWITCH_PORTS; j++)
    {
        sprintf ( ssi_buff, "%.2f",switch_data.portmng.throughput[j].ibad_val/100.0); 
        strcat ( pcBuf , ssi_buff );
        ssi_buff[0] = ',';
        ssi_buff[1] = switch_data.portmng.throughput[j].ibad_unit+0x30;
        if(j < (MAX_SWITCH_PORTS-1))
        {
            ssi_buff[2] = ',';
        }
        else
        {
            ssi_buff[2] = ';';
        }
        ssi_buff[3] = '\0';
        strcat ( pcBuf , ssi_buff );
    } 
    
    LOG_PRINT_INFO("ajax_monitor_get:%d Byte\n",strlen(pcBuf));

}
static void req_password_get( char * pcBuf, int iBufLen )
{
    strcpy ( pcBuf , JAVASCRIPT_HEADER );
    if(strcmp(cgi_to_ssi_buf,"\"\"") == 0)
    {
        sprintf(cgi_to_ssi_buf,"9");
    }
    sprintf ( ssi_buff, "\npw_s=%s;\n",cgi_to_ssi_buf);   
    strcat ( pcBuf , ssi_buff );
    sprintf ( cgi_to_ssi_buf, "\"\"");
    strcat ( pcBuf , JAVASCRIPT_FOOTER );
    LOG_PRINT_INFO("req_password_get:%d Byte\n",strlen(pcBuf));
}
static void req_swpage_get( char * pcBuf, int iBufLen )
{
    strcpy ( pcBuf , JAVASCRIPT_HEADER );
    sprintf ( ssi_buff, "\nsw_s=%s;\n",cgi_to_ssi_buf);   
    strcat ( pcBuf , ssi_buff );
    sprintf ( cgi_to_ssi_buf, "\"\"");
    strcat ( pcBuf , JAVASCRIPT_FOOTER );
    LOG_PRINT_INFO("req_swpage_get:%d Byte\n",strlen(pcBuf));
}
//插入系统设置初始参数
static void req_sysst_get( char * pcBuf, int iBufLen )
{
    strcpy ( pcBuf , JAVASCRIPT_HEADER );
    
    sprintf ( ssi_buff, "\nvar tip=%s;\n",cgi_to_ssi_buf);   
    strcat ( pcBuf , ssi_buff );
    sprintf ( cgi_to_ssi_buf, "\"\"");
     
    sprintf ( ssi_buff, "nf.ip=\"%d.%d.%d.%d\";\n",IP_ADDRESS[0],IP_ADDRESS[1],IP_ADDRESS[2],IP_ADDRESS[3]);
    strcat ( pcBuf , ssi_buff ); 
    
    sprintf ( ssi_buff, "nf.nm=\"%d.%d.%d.%d\";\n",NETMASK_ADDRESS[0],NETMASK_ADDRESS[1],NETMASK_ADDRESS[2],NETMASK_ADDRESS[3]);
    strcat ( pcBuf , ssi_buff );
    
    sprintf ( ssi_buff, "nf.gw=\"%d.%d.%d.%d\";\n",GATEWAY_ADDRESS[0],GATEWAY_ADDRESS[1],GATEWAY_ADDRESS[2],GATEWAY_ADDRESS[3]);
    strcat ( pcBuf , ssi_buff );

    sprintf ( ssi_buff, "nf.mac=\"%02x-%02x-%02x-%02x-%02x-%02x\";\n",MACAddr[0],MACAddr[1],MACAddr[2],MACAddr[3],MACAddr[4],MACAddr[5]);
    strcat ( pcBuf , ssi_buff );
    
    LOG_PRINT_INFO("req_sysst_get:%d Byte\n",strlen(pcBuf));
    strcat ( pcBuf , JAVASCRIPT_FOOTER );

}
#include <ctype.h>
/**
 * @brief 插入vlan相关参数
 * @param pcBuf 数据缓存
 * @param iBufLen 数据长度 
 * @return uint8_t 0 
 */
static void req_pvl_get( char * pcBuf, int iBufLen )
{
    uint8_t i=0,j=0;
    
    sprintf ( ssi_buff, "\nvar tip=%s;\n",cgi_to_ssi_buf);   
    strcat ( pcBuf , ssi_buff );
    sprintf ( cgi_to_ssi_buf, "\"\"");
    
    sprintf ( ssi_buff, "\nvar pvl_dt = {\n");   
    strcat ( pcBuf , ssi_buff );
    
    sprintf ( ssi_buff, "vl_s:%d,\n",switch_data.portmng.pvl_state);   
    strcat ( pcBuf , ssi_buff );
    
    sprintf ( ssi_buff, "pmax:%d,\n",MAX_PVLT_MEM);   
    strcat ( pcBuf , ssi_buff );
    
    sprintf ( ssi_buff, "t_num:%d,\n",MAX_PVLT_MEM);   
    strcat ( pcBuf , ssi_buff );
    
    sprintf ( ssi_buff, "vl_tb:[");   
    strcat ( pcBuf , ssi_buff );
    for(i=0,j=0; j<MAX_PVLT_MEM; j++)
    {
        ssi_buff[i] = j+1+'0';
        i++;
        if(j < MAX_PVLT_MEM-1)
        {
            ssi_buff[i] = ',';
        }
        else
        {
            ssi_buff[i] = ']';
            ssi_buff[i+1] = ',';
            ssi_buff[i+2] = '\n';
            ssi_buff[i+3] = '\0';
            break;
        }
        i++;
    }
    strcat ( pcBuf , ssi_buff );
    
    sprintf ( ssi_buff, "mbrs:[");   
    strcat ( pcBuf , ssi_buff );
    for(j=0; j<MAX_PVLT_MEM; j++)
    {
        
        if(j < MAX_PVLT_MEM-1)
        {
            sprintf(ssi_buff,"0x%02x,",switch_data.portmng.pvl_t[j]);
            strcat ( pcBuf , ssi_buff );
        }
        else
        {
            sprintf(ssi_buff,"0x%02x]\n",switch_data.portmng.pvl_t[j]);
            strcat ( pcBuf , ssi_buff );
            break;
        }
    }
    
    sprintf ( ssi_buff, "};");   
    strcat ( pcBuf , ssi_buff );
    
    LOG_PRINT_INFO("req_pvl_get:%d Byte\n",strlen(pcBuf));
}

/**
 * @brief 插入port_manage文件
 * @param pcBuf 数据缓存
 * @param iBufLen 数据长度 
 * @return uint8_t 0 
 */
static void req_pmngcfg_get( char * pcBuf, int iBufLen )
{
    sprintf (ssi_buff,"\r\n");
    strcpy(pcBuf,ssi_buff);
    strcat(pcBuf,file_ram_pmng.file);
    sprintf (ssi_buff,"</>\r\n%s",BUCKUPRECOVE_BSIF_TAG);   
    strcat ( pcBuf , ssi_buff);
    strcat ( pcBuf , file_ram_bsif.file );
    sprintf (ssi_buff,"</>");   
    strcat ( pcBuf , ssi_buff);
    LOG_PRINT_INFO("req_pmngcfg_get:%d Byte\n",strlen(pcBuf));
}
/**
 * @brief 导入配置后，插入返回结果
 * @param pcBuf 数据缓存
 * @param iBufLen 数据长度 
 * @return uint8_t 0 
 */
static void req_bkprcv_get( char * pcBuf, int iBufLen )
{
    sprintf ( ssi_buff, "\nvar tip=%s;\n",cgi_to_ssi_buf);   
    strcpy ( pcBuf , ssi_buff );
    sprintf ( cgi_to_ssi_buf, "\"\"");
    LOG_PRINT_INFO("req_bkprcv_get:%d Byte\n",strlen(pcBuf));
}

/****
*
*
*POST数据处理三剑客
*
*
*****/
#include "httpd.h"
err_t httpd_post_begin(void *connection, const char *uri, const char *http_request,
                       u16_t http_request_len, int content_len, char *response_uri,
                       u16_t response_uri_len, u8_t *post_auto_wnd)
{

    int i = 0;

    if(!uri || (uri[0] == '\0')) 
    {
        return ERR_ARG;
    }

    htppd_post_procs.index = -1;
    htppd_post_procs.res_url = NULL;

    if (NUM_CONFIG_CGI_URIS && httpd_cgis) 
    {
        for (i = 0; i < NUM_CONFIG_CGI_URIS; i++) 
        {
            if (strcmp(uri, httpd_cgis[i].pcCGIName) == 0) 
            {
                htppd_post_procs.index = i;
                break;
            }
        }
    }

    if(htppd_post_procs.index == -1)
    {
        return ERR_ARG;
    }

    return ERR_OK;
    
}
#define LWIP_HTTPD_POST_MAX_PAYLOAD_LEN     ((2*(FILE_MAX_BYTE))+256)
static char http_post_payload[LWIP_HTTPD_POST_MAX_PAYLOAD_LEN];
static u16_t http_post_payload_len = 0;
err_t httpd_post_receive_data(void *connection, struct pbuf *p)
{
    struct http_state *hs = (struct http_state *)connection;
    struct pbuf *q = p;
    int count;
    
    u32_t http_post_payload_full_flag = 0;

    //兼容分包整合
    while(q != NULL)
    {
        if(http_post_payload_len + q->len <= LWIP_HTTPD_POST_MAX_PAYLOAD_LEN) 
        {
            MEMCPY(http_post_payload+http_post_payload_len, q->payload, q->len);
            http_post_payload_len += q->len;
        }
        else 
        {
            http_post_payload_full_flag = 1;
            break;
        }
        q = q->next;
    }
    // 释放pbuf
    pbuf_free(p); 

    count = 0;
    if(http_post_payload_full_flag)
    {
        http_post_payload_full_flag = 0;
        http_post_payload_len = 0;
        htppd_post_procs.index = -1;
    }
    else if(hs->post_content_len_left == 0) 
    {
       
        if(htppd_post_procs.index != -1)
        {
            // 解析
            count = extract_uri_parameters(hs, http_post_payload);  
            http_post_payload_len = 0;
        }
        else 
        {
            http_post_payload_len = 0;
            count = -1;
        }
    }
    // 调用CGI句柄
    htppd_post_procs.res_url = httpd_cgis[htppd_post_procs.index].pfnCGIHandler(htppd_post_procs.index, count, hs->params,
                                             hs->param_vals); 
    return ERR_OK;
    
}
 
void httpd_post_finished(void *connection, char *response_uri, u16_t response_uri_len)
{
    if(htppd_post_procs.res_url != NULL)
    {
        strncpy(response_uri,htppd_post_procs.res_url ,response_uri_len);
    }
}
