/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-03-30     Lenovo       the first version
 */

/*
 * 几个注意的点：
 * 1、88E6390的寄存器寻址分为直接寻址和间接寻址两种；
 * 2、直接寻址是基于MDIO CLAUSE 22的协议格式；
 * 3、间接寻址支持MDIO CLAUSE 22和MDIO CLAUSE 45；
 * 4、ADDR[4:0]都上拉或者悬空，才进入直接寻址模式,其他都是间接寻址；
 * 5、间接寻址用于多芯片共用MDIO接口，仅通过两个SMI寄存器访问PHY及其他功能寄存器（0x00:SMI Command Register 0x01:SMI Data Register）；
 * */
#include <Marvell_88E6390_PortCtrl.h>
static MSD_STATUS Peridot_readFlowCtrlReg
(
    IN MSD_LPORT    port,
    IN MSD_U8   pointer,
    OUT MSD_U8  *data
);
static MSD_STATUS Peridot_writeFlowCtrlReg
(
    IN MSD_LPORT    port,
    IN MSD_U8   pointer,
    IN MSD_U8   data
);
#define PERIDOT_MSD_GET_RATE_LIMIT_PER_FRAME(_frames, _dec)    \
        ((_frames)?(1000000000 / (16 * (_frames)) + ((1000000000 % (16 * (_frames)) > (16*(_frames)/2))?1:0)):0)

#define PERIDOT_MSD_GET_RATE_LIMIT_PER_BYTE(_kbps, _dec)    \
        ((_kbps)?((8000000*(_dec)) / (16 * (_kbps)) + ((8000000*(_dec)) % (16 * (_kbps))?1:0)):0)
/*******************************************************************************
* Peridot_gprtSetForceDefaultVid
*
* DESCRIPTION:
*       This routine Set the port force default vlan id.
*
* INPUTS:
*       port - logical port number to set.
*       en   - enable force port default vlan id.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MSD_OK  - on success
*       MSD_FAIL  - on error
*       MSD_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MSD_STATUS Peridot_gprtSetForceDefaultVid
(
IN MSD_LPORT     port,
IN MSD_BOOL      en
)
{
    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U8           phyPort;        /* Physical port.               */
    MSD_U8           phyAddr;
    MSD_U16          data;

    phyPort = port;
    phyAddr = port;

    MSD_DBG_INFO("Peridot_gprtSetForceDefaultVID Called.\r\n");
    if (phyPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("\tFailed (Bad Port).\r\n");
        return MSD_BAD_PARAM;
    }

    MSD_BOOL_2_BIT(en, data);

    retVal = msdSetAnyRegField(phyAddr, PERIDOT_QD_REG_PVID, 12, 1, data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("\tERROR to write Peridot_QD_REG_PVID Register.\r\n");
        return retVal;
    }
    MSD_DBG_INFO("Peridot_gprtSetForceDefaultVID Exit.\r\n");
    return MSD_OK;
}
/*******************************************************************************
* Peridot_gprtGetForceDefaultVid
*
* DESCRIPTION:
*       This routine Get the port force default vlan id.
*
* INPUTS:
*       port - logical port number to set.
*
* OUTPUTS:
*       en  - enable port force default vlan id.
*
* RETURNS:
*       MSD_OK  - on success
*       MSD_FAIL  - on error
*       MSD_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MSD_STATUS Peridot_gprtGetForceDefaultVid
(
IN  MSD_LPORT   port,
OUT MSD_BOOL    *en
)
{
    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U16          data;           /* The register's read data.    */
    MSD_U8           phyPort;        /* Physical port.               */
    MSD_U8           phyAddr;

    MSD_DBG_INFO("Peridot_gprtGetForceDefaultVID Called.\r\n");

    phyPort = port;
    phyAddr = port;
    if (phyPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("\tFailed (Bad Port).\r\n");
        return MSD_BAD_PARAM;
    }

    retVal = msdGetAnyRegField(phyAddr, PERIDOT_QD_REG_PVID, 12, 1, &data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("\tERROR to read Peridot_QD_REG_PVID Register.\r\n");
        return retVal;
    }

    MSD_BIT_2_BOOL(data, *en);

    MSD_DBG_INFO("\tPeridot_gprtGetForceDefaultVID Exit.\r\n");
    return MSD_OK;
}
/*******************************************************************************
* Peridot_gvlnSetPortVid
*
* DESCRIPTION:
*       This routine Set the port default vlan id.
*
* INPUTS:
*       port - logical port number to set.
*       vid  - the port vlan id.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MSD_OK  - on success
*       MSD_FAIL  - on error
*       MSD_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MSD_STATUS Peridot_gvlnSetPortVid
(
    IN MSD_LPORT     port,
    IN MSD_U16       vid
)
{
    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U8           phyPort;        /* Physical port.               */
    MSD_U8          phyAddr;

    MSD_DBG_INFO("Peridot_gvlnSetPortVid Called.\r\n");
    phyPort = port;
    phyAddr = port;
    if (phyPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("\tFailed (Bad Port).\r\n");
        return MSD_BAD_PARAM;
    }

    if (vid > 0xFFF)
    {
        MSD_DBG_ERROR("\tBad vid. It should be within [0, 0xFFF]\r\n");
        return MSD_BAD_PARAM;
    }

    retVal = msdSetAnyRegField(phyAddr,PERIDOT_QD_REG_PVID,0,12, vid);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("\tERROR to write PERIDOT_QD_REG_PVID Register.\r\n");
        return retVal;
    }
    MSD_DBG_INFO("Peridot_gvlnSetPortVid Exit.\r\n");
    return MSD_OK;
}
/*******************************************************************************
* Peridot_gvlnGetPortVid
*
* DESCRIPTION:
*       This routine Get the port default vlan id.
*
* INPUTS:
*       port - logical port number to set.
*
* OUTPUTS:
*       vid  - the port vlan id.
*
* RETURNS:
*       MSD_OK  - on success
*       MSD_FAIL  - on error
*       MSD_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MSD_STATUS Peridot_gvlnGetPortVid
(
    IN  MSD_LPORT port,
    OUT MSD_U16   *vid
)
{
    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U16          data;           /* The register's read data.    */
    MSD_U8           phyPort;        /* Physical port.               */
    MSD_U8          phyAddr;

    MSD_DBG_INFO("\tPeridot_gvlnGetPortVid Called.\r\n");

    phyPort = port;
    phyAddr = port;
    if (phyPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("\tFailed (Bad Port).\r\n");
        return MSD_BAD_PARAM;
    }

    retVal = msdGetAnyRegField(phyAddr, PERIDOT_QD_REG_PVID, 0, 12, &data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("\tERROR to read PERIDOT_QD_REG_PVID Register.\r\n");
        return retVal;
    }

    *vid = data;

    MSD_DBG_INFO("Peridot_gvlnGetPortVid Exit.\r\n");
    return MSD_OK;
}
/********************************************************************
* Peridot_gvlnSetPortVlanDot1qMode
*
* DESCRIPTION:
*       This routine sets the IEEE 802.1q mode for this port
*
* INPUTS:
*       port    - logical port number to set.
*       mode     - 802.1q mode for this port
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MSD_OK  - on success
*       MSD_FAIL  - on error
*       MSD_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MSD_STATUS Peridot_gvlnSetPortVlanDot1qMode
(
    IN MSD_LPORT     port,
    IN PERIDOT_MSD_8021Q_MODE    mode
)
{
    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U8           phyPort;        /* Physical port.               */
    MSD_U8          phyAddr;

    MSD_DBG_INFO("Peridot_gvlnSetPortVlanDot1qMode Called.\r\n");

    phyPort = port;
    phyAddr = port;
    if (phyPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("\tFailed (Bad Port).\r\n");
        return MSD_BAD_PARAM;
    }

    switch (mode)
    {
    case PERIDOT_MSD_DISABLE:
    case PERIDOT_MSD_FALLBACK:
    case PERIDOT_MSD_CHECK:
    case PERIDOT_MSD_SECURE:
        break;
    default:
        MSD_DBG_ERROR("\tFailed (Bad Mode).\r\n");
        return MSD_BAD_PARAM;
    }

    retVal = msdSetAnyRegField(phyAddr,PERIDOT_QD_REG_PORT_CONTROL2,10,2,(MSD_U16)mode );
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("\tERROR to write PERIDOT_QD_REG_PORT_CONTROL2 Register.\r\n");
        return retVal;
    }

    MSD_DBG_INFO("\tPeridot_gvlnSetPortVlanDot1qMode Exit.\r\n");
    return MSD_OK;
}
/*******************************************************************************
* Peridot_gvlnGetPortVlanDot1qMode
*
* DESCRIPTION:
*       This routine Peridot_gets the IEEE 802.1q mode for this port.
*
* INPUTS:
*       port     - logical port number to Peridot_get.
*
* OUTPUTS:
*       mode     - 802.1q mode for this port
*
* RETURNS:
*       MSD_OK  - on success
*       MSD_FAIL  - on error
*       MSD_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MSD_STATUS Peridot_gvlnGetPortVlanDot1qMode
(
    IN  MSD_LPORT        port,
    OUT PERIDOT_MSD_8021Q_MODE   *mode
)
{
    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U16          data;           /* The register's read data.    */
    MSD_U8           phyPort;        /* Physical port.               */
    MSD_U8          phyAddr;

    MSD_DBG_INFO("Peridot_gvlnGetPortVlanDot1qMode Called.\r\n");

    phyPort = port;
    phyAddr = port;
    if (phyPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("\tFailed (Bad Port).\r\n");
        return MSD_BAD_PARAM;
    }

    if(mode == NULL)
    {
        MSD_DBG_ERROR("\tFailed(bad mode input).\r\n");
        return MSD_BAD_PARAM;
    }

    retVal = msdGetAnyRegField(phyAddr,PERIDOT_QD_REG_PORT_CONTROL2,10,2, &data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("\tERROR to read PERIDOT_QD_REG_PORT_CONTROL2 Register.\r\n");
        return retVal;
    }

    *mode = (PERIDOT_MSD_8021Q_MODE)data;
    MSD_DBG_INFO("Peridot_gvlnGetPortVlanDot1qMode Exit.\r\n");
    return MSD_OK;
}
/*******************************************************************************
* Peridot_gprtSetDiscardTagged
*
* DESCRIPTION:
*       When this bit is set to a one, all non-MGMT frames that are processed as
*       Tagged will be discarded as they enter this switch port. Priority only
*       tagged frames (with a VID of 0x000) are considered tagged.
*
* INPUTS:
*       port - the logical port number.
*       mode - MSD_TRUE to discard tagged frame, MSD_FALSE otherwise
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MSD_OK  - on success
*       MSD_FAIL  - on error
*       MSD_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MSD_STATUS Peridot_gprtSetDiscardTagged
(
    IN MSD_LPORT     port,
    IN MSD_BOOL        mode
)
{
    MSD_U16          data;
    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U8           hwPort;         /* the physical port number     */
    MSD_U8          phyAddr;

    MSD_DBG_INFO("Peridot_gprtSetDiscardTagged Called.\r\n");

    /* translate LPORT to hardware port */
    hwPort = port;
    phyAddr = port;
    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("\tFailed (Bad Port).\r\n");
        return MSD_BAD_PARAM;
    }

    /* translate BOOL to binary */
    MSD_BOOL_2_BIT(mode, data);

    /* Set DiscardTagged. */
    retVal = msdSetAnyRegField(phyAddr, PERIDOT_QD_REG_PORT_CONTROL2, 9, 1, data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("\tERROR to write PERIDOT_QD_REG_PORT_CONTROL2 Register.\r\n");
        return retVal;
    }

    MSD_DBG_INFO("Peridot_gprtSetDiscardTagged Exit.\r\n");
    return retVal;
}
/*******************************************************************************
* Peridot_gprtGetDiscardTagged
*
* DESCRIPTION:
*       This routine Peridot_gets DiscardTagged bit for the given port
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       mode  - MSD_TRUE if DiscardTagged bit is set, MSD_FALSE otherwise
*
* RETURNS:
*       MSD_OK  - on success
*       MSD_FAIL  - on error
*       MSD_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MSD_STATUS Peridot_gprtGetDiscardTagged
(
    IN  MSD_LPORT    port,
    OUT MSD_BOOL     *mode
)
{
    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U8           hwPort;         /* the physical port number     */
    MSD_U16          data;           /* to keep the read valve       */
    MSD_U8          phyAddr;

    MSD_DBG_INFO("Peridot_gprtGetDiscardTagged Called.\r\n");

    /* translate LPORT to hardware port */
    hwPort = port;
    phyAddr = port;
    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("\tFailed (Bad Port).\r\n");
        return MSD_BAD_PARAM;
    }

    /* Get the DiscardTagged. */
    retVal = msdGetAnyRegField(phyAddr, PERIDOT_QD_REG_PORT_CONTROL2, 9, 1, &data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("\tERROR to read PERIDOT_QD_REG_PORT_CONTROL2 Register.\r\n");
        return retVal;
    }
    MSD_BIT_2_BOOL(data, *mode);

    MSD_DBG_INFO("\tPeridot_gprtGetDiscardTagged Exit.\r\n");
    return MSD_OK;
}
/*******************************************************************************
* Peridot_gprtSetDiscardUntagged
*
* DESCRIPTION:
*       When this bit is set to a one, all non-MGMT frames that are processed as
*       Untagged will be discarded as they enter this switch port. Priority only
*       tagged frames (with a VID of 0x000) are considered tagged.
*
* INPUTS:
*       port - the logical port number.
*       mode - MSD_TRUE to discard untagged frame, MSD_FALSE otherwise
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MSD_OK  - on success
*       MSD_FAIL  - on error
*       MSD_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MSD_STATUS Peridot_gprtSetDiscardUntagged
(
    IN MSD_LPORT     port,
    IN MSD_BOOL        mode
)
{
    MSD_U16          data;
    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U8           hwPort;         /* the physical port number     */
    MSD_U8          phyAddr;

    MSD_DBG_INFO("Peridot_gprtSetDiscardUntagged Called.\r\n");

    /* translate LPORT to hardware port */
    hwPort = port;
    phyAddr = port;
    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("\tFailed (Bad Port).\n");
        return MSD_BAD_PARAM;
    }

    /* translate BOOL to binary */
    MSD_BOOL_2_BIT(mode, data);

    /* Set DiscardUnTagged. */
    retVal = msdSetAnyRegField(phyAddr, PERIDOT_QD_REG_PORT_CONTROL2, 8, 1, data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("\tERROR to write PERIDOT_QD_REG_PORT_CONTROL2 Register.\r\n");
        return retVal;
    }

    MSD_DBG_INFO("Peridot_gprtSetDiscardUntagged Exit.\r\n");
    return retVal;
}
/*******************************************************************************
* Peridot_gprtGetDiscardUntagged
*
* DESCRIPTION:
*       This routine Peridot_gets DiscardUntagged bit for the given port
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       mode  - MSD_TRUE if DiscardUntagged bit is set, MSD_FALSE otherwise
*
* RETURNS:
*       MSD_OK  - on success
*       MSD_FAIL  - on error
*       MSD_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MSD_STATUS Peridot_gprtGetDiscardUntagged
(
    IN  MSD_LPORT    port,
    OUT MSD_BOOL     *mode
)
{
    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U8           hwPort;         /* the physical port number     */
    MSD_U16          data;           /* to keep the read valve       */
    MSD_U8          phyAddr;

    MSD_DBG_INFO("Peridot_gprtGetDiscardUnTagged Called.\r\n");

    /* translate LPORT to hardware port */
    hwPort = port;
    phyAddr = port;
    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("\tFailed (Bad Port).\r\n");
        return MSD_BAD_PARAM;
    }

    /* Get the DiscardUnTagged. */
    retVal = msdGetAnyRegField(phyAddr, PERIDOT_QD_REG_PORT_CONTROL2, 8, 1, &data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("\tERROR to read PERIDOT_QD_REG_PORT_CONTROL2 Register.\r\n");
        return retVal;
    }

    MSD_BIT_2_BOOL(data, *mode);

    MSD_DBG_INFO("Peridot_gprtGetDiscardUnTagged Exit.\r\n");
    return MSD_OK;
}
/*******************************************************************************
* Peridot_gprtSetUnicastFloodBlock
*
* DESCRIPTION:
*       This routine set Forward Unknown mode of a switch port.
*       When this mode is set to MSD_TRUE, normal switch operation occurs.
*       When this mode is set to MSD_FALSE, unicast frame with unknown DA addresses
*       will not egress out this port.
*
* INPUTS:
*       port - the logical port number.
*       mode - MSD_TRUE for normal switch operation or MSD_FALSE to do not egress out the unknown DA unicast frames
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MSD_OK  - on success
*       MSD_FAIL  - on error
*       MSD_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MSD_STATUS Peridot_gprtSetUnicastFloodBlock
(
    IN MSD_LPORT     port,
    IN MSD_BOOL      mode
)
{
    MSD_U16          data;
    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U8           hwPort;         /* the physical port number     */
    MSD_U8          phyAddr;

    MSD_DBG_INFO("Peridot_gprtSetUnicastFloodBlock Called.\r\n");

    /* translate BOOL to binary */
    MSD_BOOL_2_BIT(mode, data);
    data ^= 1;

    /* translate LPORT to hardware port */
    hwPort = port;
    phyAddr = port;
    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("\tFailed (Bad Port).\r\n");
        return MSD_BAD_PARAM;
    }

    retVal = msdSetAnyRegField(phyAddr, PERIDOT_QD_REG_PORT_CONTROL, 2, 1, data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("\tERROR to write PERIDOT_QD_REG_PORT_CONTROL Register.\r\n");
        return retVal;
    }

    MSD_DBG_INFO("Peridot_gprtSetUnicastFloodBlock Exit.\r\n");
    return retVal;
}
/*******************************************************************************
* Peridot_gprtGetUnicastFloodBlock
*
* DESCRIPTION:
*       This routine Peridot_gets Forward Unknown mode of a switch port.
*       When this mode is set to MSD_TRUE, normal switch operation occurs.
*       When this mode is set to MSD_FALSE, unicast frame with unknown DA addresses
*       will not egress out this port.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       mode - MSD_TRUE for normal switch operation or MSD_FALSE to do not egress out the unknown DA unicast frames
*
* RETURNS:
*       MSD_OK  - on success
*       MSD_FAIL  - on error
*       MSD_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MSD_STATUS Peridot_gprtGetUnicastFloodBlock
(
    IN  MSD_LPORT    port,
    OUT MSD_BOOL     *mode
)
{
    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U8           hwPort;         /* the physical port number     */
    MSD_U16          data;           /* to keep the read valve       */
    MSD_U8          phyAddr;

    MSD_DBG_INFO("Peridot_gprtGetUnicastFloodBlock Called.\r\n");

    /* translate LPORT to hardware port */
    hwPort = port;
    phyAddr = port;
    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("\tFailed (Bad Port).\r\n");
        return MSD_BAD_PARAM;
    }

    retVal = msdGetAnyRegField(phyAddr, PERIDOT_QD_REG_PORT_CONTROL, 2, 1, &data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("\tERROR to read PERIDOT_QD_REG_PORT_CONTROL Register.\r\n");
        return retVal;
    }
    data ^= 1;

    /* translate binary to BOOL  */
    MSD_BIT_2_BOOL(data, *mode);

    MSD_DBG_INFO("Peridot_gprtGetUnicastFloodBlock Exit.\r\n");
    return MSD_OK;
}
/*******************************************************************************
* Peridot_gprtSetMulticastFloodBlock
*
* DESCRIPTION:
*       When this bit is set to a one, normal switch operation will occurs and
*       multicast frames with unknown DA addresses are allowed to egress out this
*       port (assuming the VLAN settings allow the frame to egress this port too).
*       When this bit is cleared to a zero, multicast frames with unknown DA
*       addresses will not egress out this port.
*
* INPUTS:
*       port - the logical port number.
*       mode - MSD_TRUE for normal switch operation or MSD_FALSE to do not egress out the unknown DA multicast frames
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MSD_OK  - on success
*       MSD_FAIL  - on error
*       MSD_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MSD_STATUS Peridot_gprtSetMulticastFloodBlock
(
    IN MSD_LPORT     port,
    IN MSD_BOOL        mode
)
{
    MSD_U16          data;
    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U8           hwPort;         /* the physical port number     */
    MSD_U8          phyAddr;

    MSD_DBG_INFO("Peridot_gprtSetMulticastFloodBlock Called.\r\n");

    /* translate LPORT to hardware port */
    hwPort = port;
    phyAddr = port;
    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("\tFailed (Bad Port).\r\n");
        return MSD_BAD_PARAM;
    }

    /* translate BOOL to binary */
    MSD_BOOL_2_BIT(mode, data);

    data ^= 1;

    /* Set DefaultForward. */
    retVal = msdSetAnyRegField(phyAddr, PERIDOT_QD_REG_PORT_CONTROL, 3, 1, data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("\tERROR to write PERIDOT_QD_REG_PORT_CONTROL Register.\r\n");
        return retVal;
    }

    MSD_DBG_INFO("Peridot_gprtSetMulticastFloodBlock Exit.\r\n");
    return retVal;
}
/*******************************************************************************
* Peridot_gprtGetMulticastFloodBlock
*
* DESCRIPTION:
*       This routine Peridot_gets DefaultForward bit for the given port
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       mode - MSD_TRUE for normal switch operation or MSD_FALSE to do not egress out the unknown DA multicast frames
*
* RETURNS:
*       MSD_OK  - on success
*       MSD_FAIL  - on error
*       MSD_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MSD_STATUS Peridot_gprtGetMulticastFloodBlock
(
    IN  MSD_LPORT    port,
    OUT MSD_BOOL     *mode
)
{
    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U8           hwPort;         /* the physical port number     */
    MSD_U8          phyAddr;
    MSD_U16          data;           /* to keep the read valve       */

    MSD_DBG_INFO("Peridot_gprtGetMulticastFloodBlock Called.\r\n");

    /* translate LPORT to hardware port */
    hwPort = port;
    phyAddr = port;
    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("\tFailed (Bad Port).\r\n");
        return MSD_BAD_PARAM;
    }

    /* Get the DefaultForward. */
    retVal = msdGetAnyRegField(phyAddr, PERIDOT_QD_REG_PORT_CONTROL, 3, 1, &data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("\tERROR to read PERIDOT_QD_REG_PORT_CONTROL Register.\r\n");
        return retVal;
    }

    data ^= 1;

    MSD_BIT_2_BOOL(data, *mode);

    MSD_DBG_INFO("Peridot_gprtGetMulticastFloodBlock Exit.\r\n");
    return MSD_OK;
}
/*******************************************************************************
* Peridot_gprtGetDropOnLock
*
* DESCRIPTION:
*       This routine gets the DropOnLock mode of specified port on specified device
*
* INPUTS:
*       port - logical port number
*
* OUTPUTS:
*       en - MSD_TRUE, if enabled,
*             MSD_FALSE, otherwise.
*
* RETURNS:
*       MSD_OK  - on success
*       MSD_FAIL  - on error
*       MSD_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MSD_STATUS Peridot_gprtGetDropOnLock
(
IN  MSD_LPORT     port,
OUT MSD_BOOL      *en
)
{
    MSD_U16          data;
    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U8           hwPort;         /* the physical port number     */
    MSD_U8          phyAddr;

    MSD_DBG_INFO("Peridot_gprtGetDropOnLock Called.\r\n");

    /* translate LPORT to hardware port */
    hwPort = port;
    phyAddr = port;
    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("\tFailed (Bad Port).\r\n");
        return MSD_BAD_PARAM;
    }

    data = 0;

    retVal = msdGetAnyRegField(phyAddr, PERIDOT_QD_REG_PORT_CONTROL, 14, 1, &data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("\tERROR to read PERIDOT_QD_REG_PORT_CONTROL Register.\r\n");
        return retVal;
    }

    MSD_BIT_2_BOOL(data, *en);

    MSD_DBG_INFO("Peridot_gprtGetDropOnLock Exit.\r\n");
    return MSD_OK;
}
/*******************************************************************************
* Peridot_gprtSetDropOnLock
*
* DESCRIPTION:
*       This routine sets the Drop on Lock. When set to one, Ingress frames
*       will be discarded if their SA field is not in the ATU's address database
*       of specified port on specified device
*
* INPUTS:
*       port - logical port number
*       en - MSD_TRUE, to enable the mode,
*             MSD_FALSE, otherwise.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       MSD_OK  - on success
*       MSD_FAIL  - on error
*       MSD_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MSD_STATUS Peridot_gprtSetDropOnLock
(
IN  MSD_LPORT     port,
IN  MSD_BOOL      en
)
{
    MSD_U16          data;           /* Used to poll the data */
    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U8           hwPort;         /* the physical port number     */
    MSD_U8          phyAddr;

    MSD_DBG_INFO("Peridot_gprtSetDropOnLock Called.\r\n");

    /* translate LPORT to hardware port */
    hwPort = port;
    phyAddr = port;
    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("\tFailed (Bad Port).\r\n");
        return MSD_BAD_PARAM;
    }

    /* translate BOOL to binary */
    MSD_BOOL_2_BIT(en, data);

    retVal = msdSetAnyRegField(phyAddr, PERIDOT_QD_REG_PORT_CONTROL, 14, 2, data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("\tERROR to write PERIDOT_QD_REG_PORT_CONTROL Register.\r\n");
        return retVal;
    }

    MSD_DBG_INFO("Peridot_gprtSetDropOnLock Exit.\r\n");
    return MSD_OK;
}
/*******************************************************************************
* Peridot_gprtSetJumboMode
*
* DESCRIPTION:
*       This routine Set the max frame size allowed to be received and transmitted
*       from or to a given port.
*
* INPUTS:
*       port - the logical port number
*       mode - PERIDOT_MSD_MTU_SIZE (1522, 2048, or 10240)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MSD_OK  - on success
*       MSD_FAIL  - on error
*       MSD_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MSD_STATUS Peridot_gprtSetJumboMode
(
    IN  MSD_LPORT    port,
    IN  PERIDOT_MSD_MTU_SIZE   mode
)
{
    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U8           hwPort;         /* the physical port number     */
    MSD_U8          phyAddr;

    MSD_DBG_INFO("Peridot_gprtSetJumboMode Called.\r\n");

    if (mode > PERIDOT_MSD_MTU_SIZE_10240)
    {
        MSD_DBG_ERROR("\tBad Parameter\r\n");
        return MSD_BAD_PARAM;
    }

    /* translate LPORT to hardware port */
    hwPort = port;
    phyAddr = port;
    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("\tFailed (Bad Port).\r\n");
        return MSD_BAD_PARAM;
    }

    switch (mode)
    {
    case PERIDOT_MSD_MTU_SIZE_1522:
    case PERIDOT_MSD_MTU_SIZE_2048:
    case PERIDOT_MSD_MTU_SIZE_10240:
        break;
    default:
        MSD_DBG_ERROR("\tFailed (Bad Mode).\r\n");
        return MSD_BAD_PARAM;
    }

    /* Set the Jumbo Fram Size bit.               */
    retVal = msdSetAnyRegField(phyAddr, PERIDOT_QD_REG_PORT_CONTROL2, 12, 2, (MSD_U16)mode);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("\tERROR to write PERIDOT_QD_REG_PORT_CONTROL2 Register.\r\n");
        return retVal;
    }

    MSD_DBG_INFO("Peridot_gprtSetJumboMode Exit.\r\n");
    return MSD_OK;
}
/*******************************************************************************
* Peridot_gprtGetJumboMode
*
* DESCRIPTION:
*       This routine Peridot_gets the max frame size allowed to be received and transmitted
*       from or to a given port.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       mode - PERIDOT_MSD_MTU_SIZE (1522, 2048, or 10240)
*
* RETURNS:
*       MSD_OK  - on success
*       MSD_FAIL  - on error
*       MSD_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MSD_STATUS Peridot_gprtGetJumboMode
(
    IN  MSD_LPORT    port,
    OUT PERIDOT_MSD_MTU_SIZE   *mode
)
{
    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U8           hwPort;         /* the physical port number     */
    MSD_U8          phyAddr;
    MSD_U16          data;           /* to keep the read valve       */

    MSD_DBG_INFO("Peridot_gsysGetJumboMode Called.\r\n");

    /* translate LPORT to hardware port */
    hwPort = port;
    phyAddr = port;
    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("\tFailed (Bad Port).\r\n");
        return MSD_BAD_PARAM;
    }

    /* Get Jumbo Frame Mode.            */
    retVal = msdGetAnyRegField(phyAddr, PERIDOT_QD_REG_PORT_CONTROL2, 12, 2, &data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("\tERROR to read PERIDOT_QD_REG_PORT_CONTROL2 Register.\r\n");
        return retVal;
    }

    *mode = (PERIDOT_MSD_MTU_SIZE)data;

    MSD_DBG_INFO("Peridot_gsysGetJumboMode Exit.\r\n");
    return MSD_OK;
}
/*******************************************************************************
* Peridot_gprtSetTrunkPort
*
* DESCRIPTION:
*       This function enables/disables and sets the trunk ID.
*
* INPUTS:
*       port - the logical port number.
*       en - MSD_TRUE to make the port be a member of a trunk with the given trunkId.
*             MSD_FALSE, otherwise.
*       trunkId - valid ID is 0 ~ 31
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MSD_OK  - on success
*       MSD_FAIL  - on error
*       MSD_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MSD_STATUS Peridot_gprtSetTrunkPort
(
    IN MSD_LPORT     port,
    IN MSD_BOOL      en,
    IN MSD_U32       trunkId
)
{
    MSD_U16          data;
    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U8           hwPort;         /* the physical port number     */
    MSD_U8          phyAddr;

    MSD_DBG_INFO("Peridot_gprtSetTrunkPort Called.\r\n");

    /* translate LPORT to hardware port */
    hwPort = port;
    phyAddr = port;
    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("\tFailed (Bad Port).\r\n");
        return MSD_BAD_PARAM;
    }

    /* translate BOOL to binary */
    MSD_BOOL_2_BIT(en, data);

    if(en == MSD_TRUE)
    {
        /* need to enable trunk. so check the trunkId */
        if (!PERIDOT_IS_TRUNK_ID_VALID(dev, trunkId))
        {
            MSD_DBG_ERROR("\tFailed(bad trunkId).\r\n");
            return MSD_BAD_PARAM;
        }

        /* Set TrunkId. */
        retVal = msdSetAnyRegField(phyAddr, PERIDOT_QD_REG_PORT_CONTROL1, 8, 5, (MSD_U16)trunkId);
        if (retVal != MSD_OK)
        {
            MSD_DBG_ERROR("\tERROR to write PERIDOT_QD_REG_PORT_CONTROL1 Register.\r\n");
            return retVal;
        }
    }

    /* Set TrunkPort bit. */
    retVal = msdSetAnyRegField(phyAddr, PERIDOT_QD_REG_PORT_CONTROL1, 14, 1, data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("\tERROR to write PERIDOT_QD_REG_PORT_CONTROL1 Register.\r\n");
        return retVal;
    }

    MSD_DBG_INFO("Peridot_gprtSetTrunkPort Exit.\r\n");
    return retVal;
}
/*******************************************************************************
* Peridot_gprtGetTrunkPort
*
* DESCRIPTION:
*       This function returns trunk state of the port.
*       When trunk is disabled, trunkId field won't have valid value.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       en - MSD_TRUE, if the port is a member of a trunk,
*             MSD_FALSE, otherwise.
*       trunkId - 0 ~ 31, valid only if en is MSD_TRUE
*
* RETURNS:
*       MSD_OK  - on success
*       MSD_FAIL  - on error
*       MSD_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MSD_STATUS Peridot_gprtGetTrunkPort
(
    IN MSD_LPORT     port,
    OUT MSD_BOOL     *en,
    OUT MSD_U32        *trunkId
)
{
    MSD_U16          data;
    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U8           hwPort;         /* the physical port number     */
    MSD_U8          phyAddr;

    MSD_DBG_INFO("Peridot_gprtGetTrunkPort Called.\r\n");

    /* translate LPORT to hardware port */
    hwPort = port;
    phyAddr = port;
    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("\tFailed (Bad Port).\r\n");
        return MSD_BAD_PARAM;
    }

    data = 0;
    retVal = msdGetAnyRegField(phyAddr, PERIDOT_QD_REG_PORT_CONTROL1, 14, 1, &data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("\tERROR to read PERIDOT_QD_REG_PORT_CONTROL1 Register.\r\n");
        return retVal;
    }

    MSD_BIT_2_BOOL(data, *en);

    retVal = msdGetAnyRegField(phyAddr, PERIDOT_QD_REG_PORT_CONTROL1, 8, 5, &data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("\tERROR to read PERIDOT_QD_REG_PORT_CONTROL1 Register.\r\n");
        return retVal;
    }

    *trunkId = (MSD_U32)data;

    MSD_DBG_INFO("Peridot_gprtGetTrunkPort Exit.\r\n");
    return MSD_OK;
}
/*******************************************************************************
* Peridot_gprtSetFlowCtrl
*
* DESCRIPTION:
*       This routine enable/disable port flow control and set flow control mode
*          mode - PORT_FC_TX_RX_ENABLED,
*                 PORT_RX_ONLY,
*                 PORT_TX_ONLY,
*                 PORT_PFC_ENABLED
*
* INPUTS:
*       port - the logical port number.
*       en - enable/disable the flow control
*       mode - flow control mode
*
* OUTPUTS:
*       None
*
* RETURNS:
*       MSD_OK  - on success
*       MSD_FAIL  - on error
*       MSD_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*        Set ForcedFC=1, FCValue = enable/disable, FCMode
*
*******************************************************************************/
MSD_STATUS Peridot_gprtSetFlowCtrl
(
    IN  MSD_LPORT    port,
    IN  MSD_BOOL     en,
    IN  PERIDOT_MSD_PORT_FC_MODE      mode
)
{
    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U8          tmpData;

    MSD_DBG_INFO("Peridot_gprtSetFlowCtrl Called.\r\n");

    switch (mode)
    {
    case Peridot_PORT_FC_TX_RX_ENABLED:
    case Peridot_PORT_TX_ONLY:
    case Peridot_PORT_RX_ONLY:
    case Peridot_PORT_PFC_ENABLED:
        break;
    default:
        MSD_DBG_ERROR("\tFailed (Bad Mode).\r\n");
        return MSD_BAD_PARAM;
    }

    retVal = Peridot_readFlowCtrlReg(port, 0x10, &tmpData);
    if(retVal != MSD_OK)
    {
        MSD_DBG_ERROR("\tPeridot_readFlowCtrlReg port:%x, reg:%x, returned: %s.\r\n", port, 0x10, msdDisplayStatus(retVal));
        return retVal;
    }

    tmpData &= ~0x07;
    tmpData = (MSD_U8)((tmpData | (1 << 3 ) |(en == MSD_TRUE?1:0) << 2) | mode);

    retVal = Peridot_writeFlowCtrlReg(port, 0x10, tmpData);
    if(retVal != MSD_OK)
    {
        MSD_DBG_ERROR("\tPeridot_writeFlowCtrlReg port:%x, reg:%x, returned: %s.\r\n", port, 0x10, msdDisplayStatus(retVal));
        return retVal;
    }

    MSD_DBG_INFO("Peridot_gprtSetFlowCtrl Exit.\r\n");
    return retVal;
}

/*******************************************************************************
* Peridot_gprtGetFlowCtrl
*
* DESCRIPTION:
*       This routine Peridot_get switch port flow control enable/disable status and return
*       flow control mode
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       en - enable/disable the flow control
*       mode - flow control mode
*
* RETURNS:
*       MSD_OK  - on success
*       MSD_FAIL  - on error
*       MSD_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MSD_STATUS Peridot_gprtGetFlowCtrl
(
    IN  MSD_LPORT    port,
    OUT MSD_BOOL     *en,
    OUT PERIDOT_MSD_PORT_FC_MODE      *mode
)
{
    MSD_U8          tmpData;
    MSD_STATUS       retVal;

    MSD_DBG_INFO("Peridot_gprtGetFlowCtrl Called.\r\n");

    retVal = Peridot_readFlowCtrlReg(port, 0x10, &tmpData);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("\tPeridot_readFlowCtrlReg port:%x, reg:%x, returned: %s.\r\n", port, 0x10, msdDisplayStatus(retVal));
        return retVal;
    }

    *en = (MSD_BOOL)((tmpData >> 2) & 0x1);
    *mode = (PERIDOT_MSD_PORT_FC_MODE)(tmpData & 0x3);

    MSD_DBG_INFO("Peridot_gprtGetFlowCtrl Exit.\r\n");
    return retVal;
}
/****************************************************************************/
/* 璁剧疆鍙屽伐妯″紡                                                     */
/****************************************************************************/
MSD_STATUS Peridot_gprtSetDuplex
(
    IN  MSD_LPORT  port,
    OUT PERIDOT_MSD_PORT_FORCE_DUPLEX_MODE   mode
)
{
    MSD_U16          data;
    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U8           hwPort;         /* the physical port number     */
    MSD_U8          phyAddr;

    MSD_DBG_INFO("Peridot_gprtSetDuplex Called.\r\n");
    /* translate LPORT to hardware port */
    hwPort = port;
    phyAddr = hwPort;

    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("\tFailed (Bad Port).\r\n");
        return MSD_BAD_PARAM;
    }

    switch (mode)
    {
    case PERIDOT_MSD_DUPLEX_HALF:
        data = 0x1;
        break;
    case PERIDOT_MSD_DUPLEX_FULL:
        data = 0x3;
        break;
    case PERIDOT_MSD_DUPLEX_AUTO:
        data = 0x0;
        break;
    default:
        MSD_DBG_ERROR("\tFailed (Bad mode).\r\n");
        return MSD_BAD_PARAM;
//        break;
    }

    retVal = msdSetAnyRegField(phyAddr, PERIDOT_QD_REG_PHY_CONTROL, 2, 2, data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("\tERROR to read PERIDOT_QD_REG_PORT_STATUS Register.\r\n");
        return retVal;
    }

    MSD_DBG_INFO("Peridot_gprtSetDuplex Exit.\r\n");
    return MSD_OK;
}
/****************************************************************************/
/* 璇诲彇鍙屽伐妯″紡                                                      */
/****************************************************************************/
MSD_STATUS Peridot_gprtGetDuplex
(
    IN  MSD_LPORT  port,
    OUT PERIDOT_MSD_PORT_FORCE_DUPLEX_MODE  *mode
)
{
    MSD_U16          data;
    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U8           hwPort;         /* the physical port number     */
    MSD_U8          phyAddr;

    MSD_DBG_INFO("Peridot_gprtGetDuplex Called.\r\n");
    /* translate LPORT to hardware port */
    hwPort = port;
    phyAddr = hwPort;

    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("\tFailed (Bad Port).\r\n");
        return MSD_BAD_PARAM;
    }

    retVal = msdGetAnyRegField(phyAddr, PERIDOT_QD_REG_PHY_CONTROL, 2, 2, &data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("\tERROR to read PERIDOT_QD_REG_PORT_STATUS Register.\r\n");
        return retVal;
    }

    switch (data)
    {
    case 0x1:
        *mode = PERIDOT_MSD_DUPLEX_HALF;
        break;
    case 0x3:
        *mode = PERIDOT_MSD_DUPLEX_FULL;
        break;
    case 0:
    case 0x2:
        *mode = PERIDOT_MSD_DUPLEX_AUTO;
        break;
    default:
        MSD_DBG_ERROR("\tFailed (Bad mode).\r\n");
        return MSD_BAD_PARAM;
//        break;
    }


    MSD_DBG_INFO("Peridot_gprtGetDuplex Exit.\r\n");
    return MSD_OK;
}
/*******************************************************************************
* Peridot_gprtGetDuplexStatus
*
* DESCRIPTION:
*       This routine retrives the port duplex mode.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       mode - MSD_TRUE for Full-duplex  or MSD_FALSE for Half-duplex
*
* RETURNS:
*       MSD_OK  - on success
*       MSD_FAIL  - on error
*       MSD_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MSD_STATUS Peridot_gprtGetDuplexStatus
(
    IN  MSD_LPORT  port,
    OUT MSD_BOOL   *mode
)
{
    MSD_U16          data;
    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U8           hwPort;         /* the physical port number     */
    MSD_U8          phyAddr;

    MSD_DBG_INFO("Peridot_gprtGetDuplexStatus Called.\r\n");
    /* translate LPORT to hardware port */
    hwPort = port;
    phyAddr = hwPort;

    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("\tFailed (Bad Port).\r\n");
        return MSD_BAD_PARAM;
    }

    /* Get the force flow control bit.  */
    retVal = msdGetAnyRegField(phyAddr, PERIDOT_QD_REG_PORT_STATUS, 10, 1, &data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("\tERROR to read PERIDOT_QD_REG_PORT_STATUS Register.\r\n");
        return retVal;
    }

    /* translate binary to BOOL  */
    MSD_BIT_2_BOOL(data, *mode);

    MSD_DBG_INFO("Peridot_gprtGetDuplexStatus Exit.\r\n");
    return MSD_OK;
}
/****************************************************************************/
/* 鑾峰彇寮哄埗閾炬帴鐘舵�?                                                      */
/****************************************************************************/
MSD_STATUS Peridot_gprtGetForceLink
(
    IN  MSD_LPORT  port,
    OUT PERIDOT_MSD_PORT_FORCE_LINK_MODE   *mode
)
{
    MSD_U16          data;
    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U8           hwPort;         /* the physical port number     */
    MSD_U8          phyAddr;

    MSD_DBG_INFO("Peridot_gprtGetForceLink Called.\r\n");
    /* translate LPORT to hardware port */
    hwPort = port;
    phyAddr = hwPort;

    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("\tFailed (Bad Port).\r\n");
        return MSD_BAD_PARAM;
    }

    retVal = msdGetAnyRegField(phyAddr, PERIDOT_QD_REG_PHY_CONTROL, 4, 2, &data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("\tERROR to read PERIDOT_QD_REG_PORT_STATUS Register.\r\n");
        return retVal;
    }

    if ((data & 0x1) == 0)
        *mode = PERIDOT_PORT_DO_NOT_FORCE_LINK;
    else if ((data & 0x3) == 3)
        *mode = PERIDOT_PORT_FORCE_LINK_UP;
    else if ((data & 0x3) == 1)
        *mode = PERIDOT_PORT_FORCE_LINK_DOWN;

    MSD_DBG_INFO("Peridot_gprtGetForceLink Exit.\r\n");
    return MSD_OK;
}
/****************************************************************************/
/* 璁剧疆寮哄埗閾炬帴鐘舵�?                                                      */
/****************************************************************************/
MSD_STATUS Peridot_gprtSetForceLink
(
IN  MSD_LPORT  port,
OUT PERIDOT_MSD_PORT_FORCE_LINK_MODE  mode
)
{
    MSD_U16          data;
    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U8           hwPort;         /* the physical port number     */
    MSD_U8          phyAddr;

    MSD_DBG_INFO("Peridot_gprtSetForceLink Called.\r\n");
    /* translate LPORT to hardware port */
    hwPort = port;
    phyAddr = hwPort;

    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("\tFailed (Bad Port).\r\n");
        return MSD_BAD_PARAM;
    }

    switch (mode)
    {
    case PERIDOT_PORT_DO_NOT_FORCE_LINK:
        data = 0;
        break;
    case PERIDOT_PORT_FORCE_LINK_UP:
        data = 3;
        break;
    case PERIDOT_PORT_FORCE_LINK_DOWN:
        data = 1;
        break;
    default:
        MSD_DBG_ERROR("\tFailed (Bad mpde).\r\n");
        return MSD_BAD_PARAM;
    }

    retVal = msdSetAnyRegField(phyAddr, PERIDOT_QD_REG_PHY_CONTROL, 4, 2, data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("\tERROR to read PERIDOT_QD_REG_PORT_STATUS Register.\r\n");
        return retVal;
    }

    MSD_DBG_INFO("Peridot_gprtSetForceLink Exit.\r\n");
    return MSD_OK;
}
/*******************************************************************************
* Peridot_gprtGetLinkState
*
* DESCRIPTION:
*       This routine retrives the link state.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       state - MSD_TRUE for Up  or MSD_FALSE for Down
*
* RETURNS:
*       MSD_OK  - on success
*       MSD_FAIL  - on error
*       MSD_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MSD_STATUS Peridot_gprtGetLinkState
(
    IN  MSD_LPORT  port,
    OUT MSD_BOOL   *state
)
{
    MSD_U16          data;           /* Used to poll the SWReset bit */
    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U8           hwPort;         /* the physical port number     */
    MSD_U8          phyAddr;

    MSD_DBG_INFO("Peridot_gprtGetLinkState Called.\r\n");
    /* translate LPORT to hardware port */
    hwPort = port;
    phyAddr = hwPort;

    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("\tFailed (Bad Port).\r\n");
        return MSD_BAD_PARAM;
    }

    /* Get the force flow control bit.  */
    retVal = msdGetAnyRegField(phyAddr, PERIDOT_QD_REG_PORT_STATUS, 11, 1, &data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("\tERROR to read PERIDOT_QD_REG_PORT_STATUS Register.\r\n");
        return retVal;
    }

    /* translate binary to BOOL  */
    MSD_BIT_2_BOOL(data, *state);

    MSD_DBG_INFO("Peridot_gprtGetLinkState Exit.\r\n");
    return MSD_OK;
}
/*******************************************************************************
* Peridot_gprtSetForceSpeed
*
* DESCRIPTION:
*       This routine forces switch MAC speed.
*
* INPUTS:
*       port - the logical port number.
*       mode - PERIDOT_MSD_PORT_FORCED_SPEED_MODE (10, 100, 200, 1000, 2.5g, 10g,or No Speed Force)
*
* OUTPUTS:
*       None
*
* RETURNS:
*       MSD_OK  - on success
*       MSD_FAIL  - on error
*       MSD_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MSD_STATUS Peridot_gprtSetForceSpeed
(
IN MSD_LPORT     port,
IN PERIDOT_MSD_PORT_FORCED_SPEED_MODE  mode
)
{
    MSD_STATUS       retVal;
    MSD_U8           hwPort;
    MSD_U8          phyAddr;
    MSD_U16         data1;
    MSD_U16         data2;
    MSD_U16         linkStatus;

    MSD_DBG_INFO("Peridot_gprtSetForceSpeed Called.\r\n");

    /* translate LPORT to hardware port */
    hwPort = port;
    phyAddr = hwPort;

    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("\tFailed (Bad Port).\r\n");
        return MSD_BAD_PARAM;
    }

    /* Set the ForceSpeed bit.  */
    if (mode == Peridot_PORT_DO_NOT_FORCE_SPEED)
    {
        /*Get the force link status and then force link down*/
        if ((retVal = msdGetAnyRegField(phyAddr, PERIDOT_QD_REG_PHY_CONTROL, 4, 2, &linkStatus)) != MSD_OK)
        {
            MSD_DBG_ERROR("\tGet link status Failed\r\n");
            return retVal;
        }
        if ((retVal = msdSetAnyRegField(phyAddr, PERIDOT_QD_REG_PHY_CONTROL, 4, 2, 1)) != MSD_OK)
        {
            MSD_DBG_ERROR("\tForce Link down failed\r\n");
            return retVal;
        }

        if ((retVal = msdSetAnyRegField(phyAddr, PERIDOT_QD_REG_PHY_CONTROL, 13, 1, 0)) != MSD_OK)
        {
            MSD_DBG_ERROR("\tSet Force Speed bit error\r\n");
            return retVal;
        }
    }
    else
    {
        switch (mode)
        {
        case Peridot_PORT_FORCE_SPEED_10M:
            data1 = 0;
            data2 = 0;
            break;
        case Peridot_PORT_FORCE_SPEED_100M:
            data1 = 0;
            data2 = 1;
            break;
        case Peridot_PORT_FORCE_SPEED_200M:
            if (port != 0)
            {
                MSD_DBG_ERROR("\tFailed (Bad Port), Only port 0 support 200M.\r\n");
                return MSD_NOT_SUPPORTED;
            }
            data1 = 1;
            data2 = 1;
            break;
        case Peridot_PORT_FORCE_SPEED_1000M:
            data1 = 0;
            data2 = 2;
            break;
        case Peridot_PORT_FORCE_SPEED_2_5G:
            if ((port != 9) && (port != 10))
            {
                MSD_DBG_ERROR("\tFailed (Bad Port), Only port 9, 10 support 2.5G.\r\n");
                return MSD_NOT_SUPPORTED;
            }
            data1 = 1;
            data2 = 3;
            break;
        case Peridot_PORT_FORCE_SPEED_10G:
            if ((port != 9) && (port != 10))
            {
                MSD_DBG_ERROR("\tFailed (Bad Port), Only port 9, 10 support 10G.\r\n");
                return MSD_NOT_SUPPORTED;
            }
            data1 = 0;
            data2 = 3;
            break;
        default:
            MSD_DBG_ERROR("\tFailed (Bad Speed).\r\n");
            return MSD_BAD_PARAM;
        }

        /*Get the force link status and then force link down*/
        if ((retVal = msdGetAnyRegField(phyAddr, PERIDOT_QD_REG_PHY_CONTROL, 4, 2, &linkStatus)) != MSD_OK)
        {
            MSD_DBG_ERROR("\tGet link status Failed\r\n");
            return retVal;
        }
        if ((retVal = msdSetAnyRegField(phyAddr, PERIDOT_QD_REG_PHY_CONTROL, 4, 2, 1)) != MSD_OK)
        {
            MSD_DBG_ERROR("\tForce Link down failed\r\n");
            return retVal;
        }

        /* Set the ForceSpeed bit.  */
        if ((retVal = msdSetAnyRegField(phyAddr, PERIDOT_QD_REG_PHY_CONTROL, 13, 1, 1)) != MSD_OK)
        {
            MSD_DBG_ERROR("\tSet Force Speed bit error\r\n");
            return retVal;
        }

        if ((retVal = msdSetAnyRegField(phyAddr, PERIDOT_QD_REG_PHY_CONTROL, 12, 1, data1)) != MSD_OK)
        {
            MSD_DBG_ERROR("\tSet Alternate Speed bit error\r\n");
            return retVal;
        }
        if ((retVal = msdSetAnyRegField(phyAddr, PERIDOT_QD_REG_PHY_CONTROL, 0, 2, data2)) != MSD_OK)
        {
            MSD_DBG_ERROR("\tSet Speed bit error\r\n");
            return retVal;
        }
    }

    /*Set back the force link status*/
    if ((retVal = msdSetAnyRegField(phyAddr, PERIDOT_QD_REG_PHY_CONTROL, 4, 2, linkStatus)) != MSD_OK)
    {
        MSD_DBG_ERROR("\tSet back force link status error\r\n");
        return retVal;
    }

    MSD_DBG_INFO("Peridot_gprtSetForceSpeed Called.\r\n");
    return retVal;
}
/*******************************************************************************
* Peridot_gprtGetForceSpeed
*
* DESCRIPTION:
*       This routine retrieves switch MAC Force Speed value
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       mode - PERIDOT_MSD_PORT_FORCED_SPEED_MODE (10, 100, 200, 1000, 2.5g, 10g,or No Speed Force)
*
* RETURNS:
*       MSD_OK  - on success
*       MSD_FAIL  - on error
*       MSD_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MSD_STATUS Peridot_gprtGetForceSpeed
(
IN  MSD_LPORT     port,
OUT PERIDOT_MSD_PORT_FORCED_SPEED_MODE   *mode
)
{
    MSD_U16          data1;
    MSD_U16          data2;
    MSD_STATUS       retVal;
    MSD_U8           hwPort;
    MSD_U8          phyAddr;

    MSD_DBG_INFO("Peridot_gprtGetForceSpeed Called.\r\n");

    /* translate LPORT to hardware port */
    hwPort = port;
    phyAddr = hwPort;

    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("\tFailed (Bad Port).\r\n");
        return MSD_BAD_PARAM;
    }

    /* Get the ForceSpeed bit.  */
    if ((retVal = msdGetAnyRegField(phyAddr, PERIDOT_QD_REG_PHY_CONTROL, 13, 1, &data1)) != MSD_OK)
    {
        MSD_DBG_ERROR("\tGet force Speed bit error\r\n");
        return retVal;
    }

    if (data1 == 0)
    {
        *mode = Peridot_PORT_DO_NOT_FORCE_SPEED;
    }
    else
    {
        if ((retVal = msdGetAnyRegField(phyAddr, PERIDOT_QD_REG_PHY_CONTROL, 12, 1, &data1)) != MSD_OK)
        {
            MSD_DBG_ERROR("\tGet Alternate Speed bit error\r\n");
            return retVal;
        }
        if ((retVal = msdGetAnyRegField(phyAddr, PERIDOT_QD_REG_PHY_CONTROL, 0, 2, &data2)) != MSD_OK)
        {
            MSD_DBG_ERROR("Get Speed bit error\r\n");
            return retVal;
        }
        if (data1 == 1)
        {
            switch (data2)
            {
            case 0:
                *mode = Peridot_PORT_FORCE_SPEED_10M;
                break;
            case 1:
                *mode = Peridot_PORT_FORCE_SPEED_200M;
                break;
            case 2:
                *mode = Peridot_PORT_FORCE_SPEED_1000M;
                break;
            case 3:
                *mode = Peridot_PORT_FORCE_SPEED_2_5G;
                break;
            default:
                MSD_DBG_ERROR("\tGet invalid speed from hardware\n.");
                return MSD_FAIL;
            }
        }
        else
        {
            switch (data2)
            {
            case 0:
                *mode = Peridot_PORT_FORCE_SPEED_10M;
                break;
            case 1:
                *mode = Peridot_PORT_FORCE_SPEED_100M;
                break;
            case 2:
                *mode = Peridot_PORT_FORCE_SPEED_1000M;
                break;
            case 3:
                *mode = Peridot_PORT_FORCE_SPEED_10G;
                break;
            default:
                MSD_DBG_ERROR("\tGet invalid speed from hardware\n.");
                return MSD_FAIL;
            }
        }
    }

    MSD_DBG_INFO("Peridot_gprtGetForceSpeed Exit.\r\n");
    return retVal;
}
/*******************************************************************************
* Peridot_gprtGetSpeed
*
* DESCRIPTION:
*       This routine retrives the port speed.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       mode - PERIDOT_MSD_PORT_SPEED type.
*                (PORT_SPEED_10_MBPS,PORT_SPEED_100_MBPS, PORT_SPEED_1000_MBPS,
*                etc.)
*
* RETURNS:
*       MSD_OK  - on success
*       MSD_FAIL  - on error
*       MSD_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MSD_STATUS Peridot_gprtGetSpeed
(
    IN  MSD_LPORT  port,
    OUT PERIDOT_MSD_PORT_SPEED   *speed
)
{
    MSD_U16          data, speedEx;
    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U8           hwPort;         /* the physical port number     */
    MSD_U8          phyAddr;

    MSD_DBG_INFO("Peridot_gprtGetSpeed Called.\r\n");

    /* translate LPORT to hardware port */
    hwPort = port;
    phyAddr = hwPort;

    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("\tFailed (Bad Port).\r\n");
        return MSD_BAD_PARAM;
    }

    retVal = msdGetAnyReg(phyAddr, PERIDOT_QD_REG_PORT_STATUS, &data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("\tERROR to read PERIDOT_QD_REG_PORT_STATUS Register.\r\n");
        return retVal;
    }
    /* Reg0_13 + Reg0[9:8]
        000 - 10M
        001 - 100M
        101 - 200M
        010 - 1000M
        011 - 10G
        111 - 2.5G
    */
    speedEx = (MSD_U16)(((data & 0x2000) >> 11) | ((data & 0x300) >> 8));

    switch (speedEx)
    {
        case 0:
            *speed = Peridot_PORT_SPEED_10_MBPS;
            break;
        case 1:
            *speed = Peridot_PORT_SPEED_100_MBPS;
            break;
        case 5:
            *speed = Peridot_PORT_SPEED_200_MBPS;
            break;
        case 2:
            *speed = Peridot_PORT_SPEED_1000_MBPS;
            break;
        case 7:
            *speed = Peridot_PORT_SPEED_2_5_GBPS;
            break;
        case 3:
            *speed = Peridot_PORT_SPEED_10_GBPS;
            break;
        default:
            *speed = Peridot_PORT_SPEED_UNKNOWN;
            break;
    }

    MSD_DBG_INFO("Peridot_gprtGetSpeed Called.\r\n");
    return MSD_OK;
}
/*******************************************************************************
* Peridot_gprtGetVlanPorts
*
* DESCRIPTION:
*       this routine Get port based vlan table of a specified port
*
* INPUTS:
*       devNum  - physical devie number
*       portNum - logical port number
*
* OUTPUTS:
*       memPorts - vlan ports
*       memPortsLen - number of vlan ports
*
* RETURNS:
*       MSD_OK  - on success
*       MSD_FAIL  - on error
*       MSD_BAD_PARAM - if invalid parameter is given
*       MSD_NOT_SUPPORTED - device not support
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MSD_STATUS Peridot_gprtGetVlanPorts
(
IN  MSD_LPORT  port,
OUT MSD_LPORT  *memPorts,
OUT MSD_U8  *memPortsLen
)
{
    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U8           hwPort;         /* the physical port number     */
    MSD_U16          data;           /* to keep the read valve       */
    MSD_U8          phyAddr;
    MSD_U8           i;
    MSD_32           portLen = 0;

    MSD_DBG_INFO("Peridot_gprtGetVlanPorts Called.\r\n");

    /* translate LPORT to hardware port */
    hwPort = port;
    phyAddr = hwPort;

    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("\tFailed (Bad Port).\r\n");
        return MSD_BAD_PARAM;
    }

    retVal = msdGetAnyRegField(phyAddr, PERIDOT_QD_REG_PORT_VLAN_MAP, 0, 11, &data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("\tGet PERIDOT_QD_REG_PORT_VLAN_MAP register error\r\n");
        return retVal;
    }
    for (i = 0; i < MSD_MAX_SWITCH_PORTS; i++)
    {
        if (data & (1 << i))
        {
            memPorts[portLen++] = i;
        }
    }

    *memPortsLen = (MSD_U8)portLen;

    MSD_DBG_INFO("Peridot_gprtGetVlanPorts Exit.\r\n");
    return MSD_OK;
}
/*******************************************************************************
* Peridot_gprtSetVlanPorts
*
* DESCRIPTION:
*       this routine Set port based vlan table of a specified port
*
* INPUTS:
*       portNum - logical port number
*       memPorts - vlan ports to set
*       memPortsLen - number of vlan ports
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MSD_OK  - on success
*       MSD_FAIL  - on error
*       MSD_BAD_PARAM - if invalid parameter is given
*       MSD_NOT_SUPPORTED - device not support
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MSD_STATUS Peridot_gprtSetVlanPorts
(
IN  MSD_LPORT  port,
IN  MSD_LPORT  *memPorts,
IN  MSD_U8  memPortsLen
)
{
    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U8           hwPort;         /* the physical port number     */
    MSD_U16          data;           /* to keep the read valve       */
    MSD_U8          phyAddr;
    MSD_U8           i;
    MSD_U8          numOfPorts;
    MSD_DBG_INFO("Peridot_gprtSetVlanPorts Called.\r\n");

    /* translate LPORT to hardware port */
    hwPort = port;
    phyAddr = hwPort;
    numOfPorts = MSD_MAX_SWITCH_PORTS;
    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("\tFailed (Bad Port).\r\n");
        return MSD_BAD_PARAM;
    }

    if (memPortsLen > numOfPorts)
    {
        MSD_DBG_ERROR("\tFailed (Bad num of vlan ports).\r\n");
        return MSD_BAD_PARAM;
    }

    data = 0;
    for (i = 0; i < memPortsLen; i++)
    {
        if (memPorts[i] < numOfPorts)
            data |= (1 << memPorts[i]);
        else
        {
            MSD_DBG_ERROR("\tFailed (Bad member port).\r\n");
            return MSD_BAD_PARAM;
        }
    }

    retVal = msdSetAnyRegField(phyAddr, PERIDOT_QD_REG_PORT_VLAN_MAP, 0, 11, data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("\tSet PERIDOT_QD_REG_PORT_VLAN_MAP register error\r\n");
        return retVal;
    }

    MSD_DBG_INFO("Peridot_gprtSetVlanPorts Exit.\r\n");
    return MSD_OK;
}
/*******************************************************************************
* Peridot_grcSetEgressRate
*
* DESCRIPTION:
*       This routine sets the port's egress data limit.
*        
*
* INPUTS:
*       port      - logical port number.
*       rateType  - egress data rate limit (MSD_ERATE_TYPE union type). 
*                    union type is used to support multiple devices with the
*                    different formats of egress rate.
*                    MSD_ERATE_TYPE has the following fields:
*                        kbRate - rate in kbps that should used with the PERIDOT_MSD_ELIMIT_MODE of 
*                                PERIDOT_MSD_ELIMIT_LAYER1,
*                                PERIDOT_MSD_ELIMIT_LAYER2, or 
*                                PERIDOT_MSD_ELIMIT_LAYER3 (see Peridot_grcSetELimitMode)
*                            64kbps ~ 1Mbps    : increments of 64kbps,
*                            1Mbps ~ 100Mbps   : increments of 1Mbps, and
*                            100Mbps ~ 1000Mbps: increments of 10Mbps
*                            1Gbps ~ 5Gbps: increments of 100Mbps
*                            Therefore, the valid values are:
*                                64, 128, 192, 256, 320, 384,..., 960,
*                                1000, 2000, 3000, 4000, ..., 100000,
*                                110000, 120000, 130000, ..., 1000000
*                                1100000, 1200000, 1300000, ..., 5000000.
*                        fRate - frame per second that should used with PERIDOT_MSD_ELIMIT_MODE of 
*                                PERIDOT_MSD_PIRL_ELIMIT_FRAME
*                            Valid values are between 3815 and 14880000
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MSD_OK  - on success
*       MSD_FAIL  - on error
*       MSD_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*		None.
*
*******************************************************************************/
MSD_STATUS Peridot_grcSetEgressRate
(
    IN MSD_LPORT        port,
    IN PERIDOT_MSD_ELIMIT_MODE mode,
    IN MSD_U32	rate
)
{
    MSD_STATUS    retVal;         /* Functions return value.      */
    MSD_U16        data;
    MSD_U32        eDec;
    MSD_U8        hwPort,phyAddr;        /* Physical port.               */

    MSD_DBG_INFO("Peridot_grcSetEgressRate Called.\r\n");

    hwPort = port;
	phyAddr = hwPort;
	if (hwPort >= MSD_MAX_SWITCH_PORTS)
	{
        MSD_DBG_ERROR("Failed (Bad Port).\r\n");
		return MSD_BAD_PARAM;
	}

    switch (mode)
    {
    case PERIDOT_MSD_ELIMIT_FRAME:
    case PERIDOT_MSD_ELIMIT_LAYER1:
    case PERIDOT_MSD_ELIMIT_LAYER2:
    case PERIDOT_MSD_ELIMIT_LAYER3:
        break;
    default:
        MSD_DBG_ERROR("Failed (Bad Mode).\r\n");
        return MSD_BAD_PARAM;
    }

    if((retVal = Peridot_grcSetELimitMode(port,mode)) != MSD_OK)
    {
        MSD_DBG_ERROR("Peridot_grcSetELimitMode returned: %s.\r\n", msdDisplayStatus(retVal));
        return retVal;
    }

    if (mode == PERIDOT_MSD_ELIMIT_FRAME)    
    {
        if (rate == 0) /* disable egress rate limit */
        {
            eDec = 0;
            data = 0;
        }
        else if((rate < 3815)  || (rate > 14880000))
        {
            MSD_DBG_ERROR("Failed (Bad Rate).\r\n");
            return MSD_BAD_PARAM;
        }
        else
        {
            eDec = 1;
            data = (MSD_U16)PERIDOT_MSD_GET_RATE_LIMIT_PER_FRAME(rate,eDec);
        }
    }
    else
    {
        if(rate == 0)
        {
            eDec = 0;
        }
        else if(rate < 1000)    /* less than 1Mbps */
        {
            /* it should be divided by 64 */
            if (rate % 64)
            {
                MSD_DBG_ERROR("Failed (Bad Rate).\r\n");
                return MSD_BAD_PARAM;
            }
            eDec = rate/64;
        }
        else if(rate <= 100000)    /* less than or equal to 100Mbps */
        {
            /* it should be divided by 1000 */
            if (rate % 1000)
            {
                MSD_DBG_ERROR("Failed (Bad Rate).\r\n");
                return MSD_BAD_PARAM;
            }
            eDec = rate/1000;
        }
        else if(rate <= 1000000)    /* less than or equal to 1000Mbps */
        {
            /* it should be divided by 10000 */
            if (rate % 10000)
            {
                MSD_DBG_ERROR("Failed (Bad Rate).\r\n");
                return MSD_BAD_PARAM;
            }
            eDec = rate/10000;
        }
        else if(rate <= 5000000)    /* less than or equal to 10Gbps */
        {
            /* it should be divided by 100000 */
            if (rate % 100000)
            {
                MSD_DBG_ERROR("Failed (Bad Rate).\r\n");
                return MSD_BAD_PARAM;
            }
            eDec = rate/100000;
        }
        else
        {
            MSD_DBG_ERROR("Failed (Bad Rate).\r\n");
            return MSD_BAD_PARAM;
        }

        if(rate == 0)
        {
            data = 0;
        }
        else
        {
            data = (MSD_U16)PERIDOT_MSD_GET_RATE_LIMIT_PER_BYTE(rate,eDec);
        }
    }

	retVal = msdSetAnyRegField(phyAddr, PERIDOT_QD_REG_EGRESS_RATE_CTRL, 0, 7, (MSD_U16)eDec);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("ERROR to write PERIDOT_QD_REG_EGRESS_RATE_CTRL Register.\r\n");
        return retVal;
    }

	retVal = msdSetAnyRegField(phyAddr, PERIDOT_QD_REG_EGRESS_RATE_CTRL2, 0, 14, (MSD_U16)data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("ERROR to write PERIDOT_QD_REG_EGRESS_RATE_CTRL2 Register.\r\n");
        return retVal;
    }

    MSD_DBG_INFO("Peridot_grcSetEgressRate Exit.\r\n");
    return MSD_OK;
}

/*******************************************************************************
* Peridot_grcGetEgressRate
*
* DESCRIPTION:
*       This routine Peridot_gets the port's egress data limit.
*
* INPUTS:
*       port    - logical port number.
*
* OUTPUTS:
*       rateType  - egress data rate limit (MSD_ERATE_TYPE union type).
*                    union type is used to support multiple devices with the
*                    different formats of egress rate.
*                    MSD_ERATE_TYPE has the following fields:
*                        kbRate - rate in kbps that should used with the PERIDOT_MSD_ELIMIT_MODE of
*                                PERIDOT_MSD_ELIMIT_LAYER1,
*                                PERIDOT_MSD_ELIMIT_LAYER2, or
*                                PERIDOT_MSD_ELIMIT_LAYER3 (see Peridot_grcSetELimitMode)
*                            64kbps ~ 1Mbps    : increments of 64kbps,
*                            1Mbps ~ 100Mbps   : increments of 1Mbps, and
*                            100Mbps ~ 1000Mbps: increments of 10Mbps
*                            1Gbps ~ 5Gbps: increments of 100Mbps
*                            Therefore, the valid values are:
*                                64, 128, 192, 256, 320, 384,..., 960,
*                                1000, 2000, 3000, 4000, ..., 100000,
*                                110000, 120000, 130000, ..., 1000000
*                                1100000, 1200000, 1300000, ..., 5000000.
*                        fRate - frame per second that should used with PERIDOT_MSD_ELIMIT_MODE of
*                                PERIDOT_MSD_PIRL_ELIMIT_FRAME
*                            Valid values are between 3815 and 14880000
*
* RETURNS:
*       MSD_OK  - on success
*       MSD_FAIL  - on error
*       MSD_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*		None.
*
*******************************************************************************/
MSD_STATUS Peridot_grcGetEgressRate
(
    IN  MSD_LPORT		port,
    OUT PERIDOT_MSD_ELIMIT_MODE *mode,
	OUT MSD_U32	*rate
)
{
    MSD_STATUS    retVal;         /* Functions return value.      */
    MSD_U16        eRate, eDec;
    MSD_U8        hwPort,phyAddr;        /* Physical port.               */

    MSD_DBG_INFO("Peridot_grcGetEgressRate Called.\r\n");

    hwPort = port;
	phyAddr = hwPort;
	
	if (hwPort >= MSD_MAX_SWITCH_PORTS)
	{
        MSD_DBG_ERROR("Failed (Bad Port).\r\n");
		return MSD_BAD_PARAM;
	}

    if((retVal = Peridot_grcGetELimitMode(port,mode)) != MSD_OK)
    {
        MSD_DBG_ERROR("Peridot_grcGetELimitMode returned: %s.\r\n", msdDisplayStatus(retVal));
        return retVal;
    }

    retVal = msdGetAnyRegField(phyAddr,PERIDOT_QD_REG_EGRESS_RATE_CTRL,0,7,&eDec);
    if(retVal != MSD_OK)
    {
        MSD_DBG_ERROR("ERROR to read PERIDOT_QD_REG_EGRESS_RATE_CTRL Register.\r\n");
        return retVal;
    }

    retVal = msdGetAnyRegField(phyAddr,PERIDOT_QD_REG_EGRESS_RATE_CTRL2,0,14,&eRate );
    if(retVal != MSD_OK)
    {
        MSD_DBG_ERROR("ERROR to read PERIDOT_QD_REG_EGRESS_RATE_CTRL2 Register.\r\n");
        return retVal;
    }

    if (*mode == PERIDOT_MSD_ELIMIT_FRAME)    
    {
        *rate = PERIDOT_MSD_GET_RATE_LIMIT_PER_FRAME(eRate, eDec);
    }
    else
    {
        /* Count Per Byte */
        *rate = PERIDOT_MSD_GET_RATE_LIMIT_PER_BYTE(eRate, eDec);
    }

    MSD_DBG_INFO("Peridot_grcGetEgressRate Exit.\r\n");
    return MSD_OK;
}


/*******************************************************************************
* Peridot_grcSetELimitMode
*
* DESCRIPTION:
*       This routine sets Egress Rate Limit counting mode.
*       The supported modes are as follows:
*            PERIDOT_MSD_ELIMIT_FRAME -
*                Count the number of frames
*            PERIDOT_MSD_ELIMIT_LAYER1 -
*                Count all Layer 1 bytes: 
*                Preamble (8bytes) + Frame's DA to CRC + IFG (12bytes)
*            PERIDOT_MSD_ELIMIT_LAYER2 -
*                Count all Layer 2 bytes: Frame's DA to CRC
*            PERIDOT_MSD_ELIMIT_LAYER3 -
*                Count all Layer 1 bytes: 
*                Frame's DA to CRC - 18 - 4 (if frame is tagged)
*
* INPUTS:
*       port - logical port number
*       mode - PERIDOT_MSD_ELIMIT_MODE enum type
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MSD_OK  - on success
*       MSD_FAIL  - on error
*       MSD_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*		None. 
*
*******************************************************************************/
MSD_STATUS Peridot_grcSetELimitMode
(
    IN  MSD_LPORT			port,
    IN  PERIDOT_MSD_ELIMIT_MODE      mode
)
{
    MSD_U16            data;
    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U8           hwPort;        /* Physical port.               */
	MSD_U8			phyAddr;

    MSD_DBG_INFO("Peridot_grcSetELimitMode Called.\r\n");

    hwPort = port;
	if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("Failed (Bad Port).\r\n");
		return MSD_BAD_PARAM;
	}

	phyAddr = hwPort;
    data = (MSD_U16)mode & 0x3;

    /* Set the Elimit mode.            */
    retVal = msdSetAnyRegField(phyAddr, PERIDOT_QD_REG_EGRESS_RATE_CTRL2, 14, 2, data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("ERROR to read PERIDOT_QD_REG_EGRESS_RATE_CTRL2 Register.\r\n");
        return retVal;
    }

    MSD_DBG_INFO("Peridot_grcSetELimitMode Exit.\r\n");
    return MSD_OK;
}

/*******************************************************************************
* Peridot_grcGetELimitMode
*
* DESCRIPTION:
*       This routine Peridot_gets Egress Rate Limit counting mode.
*       The supported modes are as follows:
*            PERIDOT_MSD_ELIMIT_FRAME -
*                Count the number of frames
*            PERIDOT_MSD_ELIMIT_LAYER1 -
*                Count all Layer 1 bytes: 
*                Preamble (8bytes) + Frame's DA to CRC + IFG (12bytes)
*            PERIDOT_MSD_ELIMIT_LAYER2 -
*                Count all Layer 2 bytes: Frame's DA to CRC
*            PERIDOT_MSD_ELIMIT_LAYER3 -
*                Count all Layer 1 bytes: 
*                Frame's DA to CRC - 18 - 4 (if frame is tagged)
*
* INPUTS:
*       port - logical port number
*
* OUTPUTS:
*       mode - PERIDOT_MSD_ELIMIT_MODE enum type
*
* RETURNS:
*       MSD_OK  - on success
*       MSD_FAIL  - on error
*       MSD_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*		None.
*
*******************************************************************************/
MSD_STATUS Peridot_grcGetELimitMode
(
    IN  MSD_LPORT		port,
    OUT PERIDOT_MSD_ELIMIT_MODE  *mode
)
{
    MSD_U16            data;
    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U8           hwPort;        /* Physical port.               */
	MSD_U8			phyAddr;

    MSD_DBG_INFO("Peridot_grcGetELimitMode Called.\r\n");

    hwPort = port;
	if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("Failed (Bad Port).\r\n");
		return MSD_BAD_PARAM;
	}
	phyAddr = hwPort;

    /* Get the Elimit mode.            */
    retVal = msdGetAnyRegField(phyAddr, PERIDOT_QD_REG_EGRESS_RATE_CTRL2, 14, 2, &data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("ERROR to read PERIDOT_QD_REG_EGRESS_RATE_CTRL2 Register.\r\n");
        return retVal;
    }

    *mode = (PERIDOT_MSD_ELIMIT_MODE)data;

    MSD_DBG_INFO("Peridot_grcGetELimitMode Exit.\r\n");
    return MSD_OK;
}
/****************************************************************************/
/* Internal functions.                                                      */
/****************************************************************************/
static MSD_STATUS Peridot_writeFlowCtrlReg
(
    IN MSD_LPORT    port,
    IN MSD_U8   pointer,
    IN MSD_U8   data
)
{
    MSD_STATUS    retVal;         /* Functions return value.      */
    MSD_U8        hwPort;         /* the physical port number     */
    MSD_U8       phyAddr;
    MSD_U16   count, tmpData;
    MSD_DBG_INFO("Peridot_writeFlowCtrlReg Called.\r\n");

    /* translate LPORT to hardware port */
    hwPort = port;
    phyAddr = port;
    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        return MSD_BAD_PARAM;
    }
    count = 5;
    tmpData = 1;
    while(tmpData ==1)
    {
        retVal = msdGetAnyRegField(phyAddr, PERIDOT_QD_REG_LIMIT_PAUSE_CONTROL, 15, 1, &tmpData);
        if(retVal != MSD_OK)
        {
            return retVal;
        }
        if ((count--) == 0)
        {
            return MSD_FAIL;
        }
    }

    tmpData =  (MSD_U16)((1 << 15) | ((pointer&0x7F) << 8) | (MSD_U16)data);

    retVal = msdSetAnyReg(phyAddr, PERIDOT_QD_REG_LIMIT_PAUSE_CONTROL, tmpData);
    if(retVal != MSD_OK)
    {
        MSD_DBG_INFO("\tFailed.\r\n");
        return retVal;
    }

    return retVal;

}
static MSD_STATUS Peridot_readFlowCtrlReg
(
    IN MSD_LPORT    port,
    IN MSD_U8   pointer,
    OUT MSD_U8  *data
)
{
    MSD_STATUS    retVal;         /* Functions return value.      */
    MSD_U8           hwPort;         /* the physical port number     */
    MSD_U8          phyAddr;
    MSD_U16            tmpData;
    int count=0x10;

    /* translate LPORT to hardware port */
    hwPort = port;
    phyAddr = port;
    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        return MSD_BAD_PARAM;
    }
    do {
        retVal = msdGetAnyReg(phyAddr, PERIDOT_QD_REG_LIMIT_PAUSE_CONTROL, &tmpData);
        if (retVal != MSD_OK)
        {
            return retVal;
        }
        if ((count--) == 0)
        {
            return MSD_FAIL;
        }
    } while (tmpData & 0x8000);

    tmpData =  (MSD_U16)(((pointer&0x7F) << 8) | 0);
    retVal = msdSetAnyReg(phyAddr, PERIDOT_QD_REG_LIMIT_PAUSE_CONTROL, tmpData);
    if(retVal != MSD_OK)
    {
       return retVal;
    }

    do {
        retVal = msdGetAnyReg(phyAddr, PERIDOT_QD_REG_LIMIT_PAUSE_CONTROL, &tmpData);
        if(retVal != MSD_OK)
        {
            return retVal;
        }
        if((count--)==0)
        {
            return MSD_FAIL;
        }
    } while (tmpData&0x8000);

    *data = (MSD_U8)(tmpData & 0xff);

    return retVal;
}
/****************************************************************************/
/* 端口状态控制                                                             */
/****************************************************************************/
MSD_STATUS Peridot_gprtSetPortState
(
    IN  MSD_LPORT  port,
    IN  PERIDOT_MSD_PORT_PORTSTATE   portstate
)
{
    MSD_U16          data;
    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U8           hwPort;         /* the physical port number     */
    MSD_U8          phyAddr;

    MSD_DBG_INFO("Peridot_gprtSetPortState Called.\r\n");
    /* translate LPORT to hardware port */
    hwPort = port;
    phyAddr = hwPort;

    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("\tFailed (Bad Port).\r\n");
        return MSD_BAD_PARAM;
    }

    switch (portstate)
    {
    case Peridot_PORT_STATE_DISABLE:
        data = 0x0;
        break;
    case Peridot_PORT_STATE_BLOCKINGLISTENING:
        data = 0x1;
        break;
    case Peridot_PORT_STATE_LEARNING:
        data = 0x2;
        break;
    case Peridot_PORT_STATE_FORWARDING:
        data = 0x3;
        break;
    default:
        MSD_DBG_ERROR("\tFailed (Bad mode).\r\n");
        return MSD_BAD_PARAM;
//        break;
    }

    retVal = msdSetAnyRegField(phyAddr, PERIDOT_QD_REG_PORT_CONTROL, 0, 2, data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("\tERROR to read PERIDOT_QD_REG_PORT_CONTROL Register.\r\n");
        return retVal;
    }

    MSD_DBG_INFO("Peridot_gprtSetPortState Exit.\r\n");
    return MSD_OK;
}
/****************************************************************************/
/* 读取端口状态                                                             */
/****************************************************************************/
MSD_STATUS Peridot_gprtGetPortState
(
    IN  MSD_LPORT  port,
    OUT PERIDOT_MSD_PORT_PORTSTATE   *portstate
)
{
    MSD_U16          data;
    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U8           hwPort;         /* the physical port number     */
    MSD_U8          phyAddr;

    MSD_DBG_INFO("Peridot_gprtGetPortState Called.\r\n");
    /* translate LPORT to hardware port */
    hwPort = port;
    phyAddr = hwPort;

    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("\tFailed (Bad Port).\r\n");
        return MSD_BAD_PARAM;
    }

    retVal = msdGetAnyRegField(phyAddr, PERIDOT_QD_REG_PORT_CONTROL, 0, 2, &data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("\tERROR to read PERIDOT_QD_REG_PORT_CONTROL Register.\r\n");
        return retVal;
    }

    switch (data)
    {
    case 0x0:
        *portstate = Peridot_PORT_STATE_DISABLE;
        break;
    case 0x1:
        *portstate = Peridot_PORT_STATE_BLOCKINGLISTENING;
        break;
    case 0x2:
        *portstate = Peridot_PORT_STATE_LEARNING;
        break;
    case 0x3:
        *portstate = Peridot_PORT_STATE_FORWARDING;
        break;
    default:
        MSD_DBG_ERROR("\tFailed (Bad mode).\r\n");
        return MSD_BAD_PARAM;
//        break;
    }


    MSD_DBG_INFO("Peridot_gprtGetPortState Exit.\r\n");
    return MSD_OK;
}
