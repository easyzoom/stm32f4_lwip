/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-04-01     Lenovo       the first version
 */
/********************************************************************************
* Peridot_msdPhyCtrl.c
*
* DESCRIPTION:
*       API implementation for switch internal Copper PHY control.
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*******************************************************************************/

#include "Marvell_88E6390_PhyCtrl.h"

/*
* This routine set Auto-Negotiation Ad Register for Copper
*/
static
MSD_STATUS Peridot_gigCopperSetAutoMode
(
    IN MSD_U8 hwPort,
    IN PERIDOT_MSD_PHY_AUTO_MODE mode
)
{
    MSD_U16             u16Data, u16Data1;

    if (Peridot_msdReadPagedPhyReg(hwPort, 0, PERIDOT_QD_PHY_AUTONEGO_AD_REG, &u16Data) != MSD_OK)
    {
        return MSD_FAIL;
    }

    /* Mask out all auto mode related bits. */
    u16Data &= ~PERIDOT_QD_PHY_MODE_AUTO_AUTO;

    if (Peridot_msdReadPagedPhyReg(hwPort, 0, PERIDOT_QD_PHY_AUTONEGO_1000AD_REG, &u16Data1) != MSD_OK)
    {
        return MSD_FAIL;
    }

    /* Mask out all auto mode related bits. */
    u16Data1 &= ~(PERIDOT_QD_GIGPHY_1000T_FULL | PERIDOT_QD_GIGPHY_1000T_HALF);

    switch (mode)
    {
        case PERIDOT_SPEED_AUTO_DUPLEX_AUTO:
            u16Data |= PERIDOT_QD_PHY_MODE_AUTO_AUTO;
        case PERIDOT_SPEED_1000_DUPLEX_AUTO:
            u16Data1 |= PERIDOT_QD_GIGPHY_1000T_FULL | PERIDOT_QD_GIGPHY_1000T_HALF;
            break;
        case PERIDOT_SPEED_AUTO_DUPLEX_FULL:
            u16Data |= PERIDOT_QD_PHY_MODE_AUTO_FULL;
            u16Data1 |= PERIDOT_QD_GIGPHY_1000T_FULL;
            break;
        case PERIDOT_SPEED_1000_DUPLEX_FULL:
            u16Data1 |= PERIDOT_QD_GIGPHY_1000T_FULL;
            break;
        case PERIDOT_SPEED_1000_DUPLEX_HALF:
            u16Data1 |= PERIDOT_QD_GIGPHY_1000T_HALF;
            break;
        case PERIDOT_SPEED_AUTO_DUPLEX_HALF:
            u16Data |= PERIDOT_QD_PHY_MODE_AUTO_HALF;
            u16Data1 |= PERIDOT_QD_GIGPHY_1000T_HALF;
            break;
        case PERIDOT_SPEED_100_DUPLEX_AUTO:
            u16Data |= PERIDOT_QD_PHY_MODE_100_AUTO;
            break;
        case PERIDOT_SPEED_10_DUPLEX_AUTO:
            u16Data |= PERIDOT_QD_PHY_MODE_10_AUTO;
            break;
        case PERIDOT_SPEED_100_DUPLEX_FULL:
            u16Data |= PERIDOT_QD_PHY_100_FULL;
            break;
        case PERIDOT_SPEED_100_DUPLEX_HALF:
            u16Data |= PERIDOT_QD_PHY_100_HALF;
            break;
        case PERIDOT_SPEED_10_DUPLEX_FULL:
            u16Data |= PERIDOT_QD_PHY_10_FULL;
            break;
        case PERIDOT_SPEED_10_DUPLEX_HALF:
            u16Data |= PERIDOT_QD_PHY_10_HALF;
            break;
        default:
            return MSD_BAD_PARAM;
    }

    /* Write to Phy AutoNegotiation Advertisement Register.  */
    if (Peridot_msdWritePagedPhyReg(hwPort, 0, PERIDOT_QD_PHY_AUTONEGO_AD_REG, u16Data) != MSD_OK)
    {
        return MSD_FAIL;
    }

    /* Write to Phy AutoNegotiation 1000B Advertisement Register.  */
    if (Peridot_msdWritePagedPhyReg(hwPort, 0, PERIDOT_QD_PHY_AUTONEGO_1000AD_REG, u16Data1) != MSD_OK)
    {
        return MSD_FAIL;
    }

    return MSD_OK;
}
/*
* This routine sets Auto Mode and Reset the phy
*/
static
MSD_STATUS phySetAutoMode
(
    IN MSD_U8 hwPort,
    IN PERIDOT_MSD_PHY_AUTO_MODE mode
)
{
    MSD_U16         u16Data;
    MSD_STATUS    status;

    if ((status = Peridot_gigCopperSetAutoMode(hwPort, mode)) != MSD_OK)
    {
        return status;
    }

    /* Read to Phy Control Register.  */
    if (Peridot_msdReadPagedPhyReg(hwPort, 0, PERIDOT_QD_PHY_CONTROL_REG, &u16Data) != MSD_OK)
        return MSD_FAIL;

    u16Data |= PERIDOT_QD_PHY_AUTONEGO;

    /* Write to Phy Control Register.  */
    if (Peridot_msdWritePagedPhyReg(hwPort, 0, PERIDOT_QD_PHY_CONTROL_REG, u16Data) != MSD_OK)
        return MSD_FAIL;


    return Peridot_msdPhyReset(hwPort, 0, 0xFF);
}
/*******************************************************************************
* Peridot_gphyReset
*
* DESCRIPTION:
*       This routine preforms PHY reset.
*       After reset, phy will be in Autonegotiation mode.
*
* INPUTS:
*       port - The logical port number
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MSD_OK   - on success
*       MSD_FAIL - on error
*       MSD_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       data sheet register 0.15 - Reset
*       data sheet register 0.13 - Speed(LSB)
*       data sheet register 0.12 - Autonegotiation
*       data sheet register 0.8  - Duplex Mode
*       data sheet register 0.6  - Speed(MSB)
*
*       If DUT is in power down or loopback mode, port will link down,
*       in this case, no need to do software reset to restart AN.
*       When port recover from link down, AN will restart automatically.
*
*******************************************************************************/
MSD_STATUS Peridot_gphyReset
(
    IN MSD_LPORT  port
)
{
    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U8           hwPort;         /* the physical port number     */
    MSD_U16  pageReg;

    MSD_DBG_INFO("Peridot_gphySetPhyReset Called.\r\r\n");

    /* translate LPORT to hardware port */
    hwPort = port;
    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("\tBad Port: %u.\r\n", (unsigned int)port);
        return MSD_BAD_PARAM;
    }

    if ((retVal = Peridot_msdGetSMIPhyRegField(hwPort, 22, 0, 8, &pageReg)) != MSD_OK)
    {
        MSD_DBG_ERROR("\tread Phy Page Register return %s.\r\n", msdDisplayStatus(retVal));
        return retVal;
    }

    /* set Auto Negotiation AD Register */
    retVal = phySetAutoMode(hwPort, PERIDOT_SPEED_AUTO_DUPLEX_AUTO);
    if(retVal != MSD_OK)
    {
        MSD_DBG_ERROR("\tPeridot_phySetAutoMode PERIDOT_SPEED_AUTO_DUPLEX_AUTO returned: %s.\r\n", msdDisplayStatus(retVal));
        return retVal;
    }

    if ((retVal = Peridot_msdSetSMIPhyRegField(hwPort, 22, 0, 8, pageReg)) != MSD_OK)
    {
        MSD_DBG_ERROR("\twrite back Phy Page Register %s.\r\n", msdDisplayStatus(retVal));
        return retVal;
    }

    MSD_DBG_INFO("Peridot_gphyReset Exit.\r\n");
    return retVal;
}
/*******************************************************************************
* Peridot_serdesReset
*
* DESCRIPTION:
*       This routine preforms internal serdes reset.
*       different devcie have different register location for the reset bit
*
* INPUTS:
*       devNum  - physical devie number
*       portNum - The logical PHY port number
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
*       Peridot: Port9 and Port10, Device4 Register0x1000
*                Port2 to Port7 (offset 0x12 - 0x17), Device4 Register0x2000

*
*
*******************************************************************************/
MSD_STATUS Peridot_serdesReset
(
    IN MSD_LPORT  port
)
{
    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U8           hwPort;         /* the physical port number     */
    MSD_U16  data;

    MSD_DBG_INFO("Peridot_serdesReset Called.\r\n");

    /* translate LPORT to hardware port */
    hwPort = port;
    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("\tBad Port: %u.\r\n", (unsigned int)port);
        return MSD_BAD_PARAM;
    }

    if ((retVal = Peridot_msdGetSMIC45PhyReg(4, hwPort, 0x2000, &data)) != MSD_OK)
    {
        MSD_DBG_ERROR("\tread 1000BASE-X/SGMI Control register error %s.\r\n", msdDisplayStatus(retVal));
        return retVal;
    }

    data = data | 0x8000;

    if ((retVal = Peridot_msdSetSMIC45PhyReg(4, hwPort, 0x2000, data)) != MSD_OK)
    {
        MSD_DBG_ERROR("\twrite 1000BASE-X/SGMI Control register error %s.\r\n", msdDisplayStatus(retVal));
        return retVal;
    }

    /* Only one of the bits need to be set as they are the same in hardware */
    /*if ((retVal = Peridot_msdGetSMIC45PhyReg(dev, 4, hwPort, 0x1000, &data)) != MSD_OK)
    {
        MSD_DBG_ERROR(("read 10GBase-R PCS Control register error %s.\r\n", msdDisplayStatus(retVal)));
        msdSemGive(dev->devNum, dev->phyRegsSem);
        return retVal;
    }

    data = data | 0x8000;

    if ((retVal = Peridot_msdSetSMIC45PhyReg(dev, 4, hwPort, 0x1000, data)) != MSD_OK)
    {
        MSD_DBG_ERROR(("write 10GBase-R PCS Control register error %s.\r\n", msdDisplayStatus(retVal)));
        msdSemGive(dev->devNum, dev->phyRegsSem);
        return retVal;
    }*/

    MSD_DBG_INFO("Peridot_serdesReset Exit.\r\n");
    return retVal;
}

/*******************************************************************************
* Peridot_gphySetPortLoopback
*
* DESCRIPTION:
*       Enable/Disable Internal Port Loopback.
*       For 10/100 Fast Ethernet PHY, speed of Loopback is determined as follows:
*         If Auto-Negotiation is enabled, this routine disables Auto-Negotiation and
*         forces speed to be 10Mbps.
*         If Auto-Negotiation is disabled, the forced speed is used.
*         Disabling Loopback simply clears bit 14 of control register(0.14). Therefore,
*         it is recommended to call gprtSetPortAutoMode for PHY configuration after
*         Loopback test.
*       For 10/100/1000 Gigagbit Ethernet PHY, speed of Loopback is determined as follows:
*         If Auto-Negotiation is enabled and Link is active, the current speed is used.
*         If Auto-Negotiation is disabled, the forced speed is used.
*         All other cases, default MAC Interface speed is used. Please refer to the data
*         sheet for the information of the default MAC Interface speed.
*
*
* INPUTS:
*       port - The logical port number, unless SERDES device is accessed
*              The physical address, if SERDES device is accessed
*       enable - If MSD_TRUE, enable loopback mode
*                If MSD_FALSE, disable loopback mode
*
* OUTPUTS:
*         None.
*
* RETURNS:
*       MSD_OK - on success
*       MSD_FAIL - on error
*       MSD_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       data sheet register 0.14 - Loop_back
*
*******************************************************************************/
MSD_STATUS Peridot_gphySetPortLoopback
(
    IN MSD_LPORT  port,
    IN MSD_BOOL   enable
)
{
    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U8           hwPort;         /* the physical port number     */
    MSD_U16             u16Data;
    MSD_U16  pageReg;

    MSD_DBG_INFO("Peridot_gphySetPortLoopback Called.\r\n");

    /* translate LPORT to hardware port */
    hwPort = port;
    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("\tBad Port: %u.\r\n", (unsigned int)port);
        return MSD_BAD_PARAM;
    }

    if ((retVal = Peridot_msdGetSMIPhyRegField(hwPort, 22, 0, 8, &pageReg)) != MSD_OK)
    {
        MSD_DBG_ERROR("\tread Phy Page Register %s.\r\n", msdDisplayStatus(retVal));
        return retVal;
    }

    MSD_BOOL_2_BIT(enable,u16Data);

    /* Write to Phy Control Register.  */
    retVal = Peridot_msdSetPagedPhyRegField(hwPort,0,PERIDOT_QD_PHY_CONTROL_REG,14,1,u16Data);
    if(retVal != MSD_OK)
    {
        MSD_DBG_ERROR("\twrite Phy Control Register  %s.\r\n", msdDisplayStatus(retVal));
        return retVal;
    }

    if ((retVal = Peridot_msdSetSMIPhyRegField(hwPort, 22, 0, 8, pageReg)) != MSD_OK)
    {
        MSD_DBG_ERROR("\twrite back Phy Page Register %s.\r\n", msdDisplayStatus(retVal));
        return retVal;
    }

    MSD_DBG_INFO("Peridot_gphySetPortLoopback Exit.\r\n");
    return retVal;
}

/*******************************************************************************
* Peridot_gphySetPortSpeed
*
* DESCRIPTION:
*       Sets speed for a specific logical port. This function will keep the duplex
*       mode and loopback mode to the previous value, but disable others, such as
*       Autonegotiation.
*
* INPUTS:
*       port -  The logical port number, unless SERDES device is accessed
*               The physical address, if SERDES device is accessed
*       speed - port speed.
*               PERIDOT_PHY_SPEED_10_MBPS for 10Mbps
*               PERIDOT_PHY_SPEED_100_MBPS for 100Mbps
*               PERIDOT_PHY_SPEED_1000_MBPS for 1000Mbps
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MSD_OK - on success
*       MSD_FAIL - on error
*       MSD_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       data sheet register 0.13 - Speed Selection (LSB)
*       data sheet register 0.6  - Speed Selection (MSB)
*
*       If DUT is in power down or loopback mode, port will link down,
*       in this case, no need to do software reset to force take effect .
*       When port recover from link down, configure will take effect automatically.
*
*******************************************************************************/
MSD_STATUS Peridot_gphySetPortSpeed
(
    IN MSD_LPORT  port,
    IN PERIDOT_MSD_PHY_SPEED speed
)
{
    MSD_U8           hwPort;         /* the physical port number     */
    MSD_U16             u16Data;
    MSD_STATUS        retVal;
    MSD_U16  pageReg;

    MSD_DBG_INFO("Peridot_gphySetPortSpeed Called.\r\n");

    /* translate LPORT to hardware port */
    hwPort = port;
    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("\tBad Port: %u.\r\n", (unsigned int)port);
        return MSD_BAD_PARAM;
    }

    if ((retVal = Peridot_msdGetSMIPhyRegField(hwPort, 22, 0, 8, &pageReg)) != MSD_OK)
    {
        MSD_DBG_ERROR("\tread Phy Page Register %s.\r\n", msdDisplayStatus(retVal));
        return retVal;
    }

    if(Peridot_msdReadPagedPhyReg(hwPort,0,PERIDOT_QD_PHY_CONTROL_REG,&u16Data) != MSD_OK)
    {
        MSD_DBG_ERROR("\tread Phy Control Register %s.\r\n", msdDisplayStatus(retVal));
        return MSD_FAIL;
    }

    switch(speed)
    {
        case PERIDOT_PHY_SPEED_10_MBPS:
            u16Data = u16Data & (PERIDOT_QD_PHY_LOOPBACK | PERIDOT_QD_PHY_DUPLEX);
            break;
        case PERIDOT_PHY_SPEED_100_MBPS:
            u16Data = (u16Data & (PERIDOT_QD_PHY_LOOPBACK | PERIDOT_QD_PHY_DUPLEX)) | PERIDOT_QD_PHY_SPEED;
            break;
        case PERIDOT_PHY_SPEED_1000_MBPS:
            u16Data = (u16Data & (PERIDOT_QD_PHY_LOOPBACK | PERIDOT_QD_PHY_DUPLEX)) | PERIDOT_QD_PHY_SPEED_MSB;
            break;
        default:
            MSD_DBG_ERROR("\tBad speed: %u.\r\n", speed);
            return MSD_BAD_PARAM;
    }

    retVal = Peridot_msdPhyReset(hwPort, 0, u16Data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("\tPeridot_hwPhyReset returned: %s.\r\n", msdDisplayStatus(retVal));
        return retVal;
    }

    if ((retVal = Peridot_msdSetSMIPhyRegField(hwPort, 22, 0, 8, pageReg)) != MSD_OK)
    {
        MSD_DBG_ERROR("\twrite back Phy Page Register %s.\r\n", msdDisplayStatus(retVal));
        return retVal;
    }

    MSD_DBG_INFO("Peridot_gphySetPortSpeed Exit.\r\n");
    return retVal;
}

/*******************************************************************************
* gprtPortPowerDown
*
* DESCRIPTION:
*        Enable/disable (power down) on specific logical port.
*        Phy configuration remains unchanged after Power down.
*
* INPUTS:
*       port -  The logical port number, unless SERDES device is accessed
*               The physical address, if SERDES device is accessed
*       state - MSD_TRUE: power down
*                MSD_FALSE: normal operation
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MSD_OK  - on success
*       MSD_FAIL - on error
*       MSD_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       data sheet register 0.11 - Power Down
*
*******************************************************************************/
MSD_STATUS Peridot_gphyPortPowerDown
(
    IN MSD_LPORT  port,
    IN MSD_BOOL   state
)
{
    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U8           hwPort;         /* the physical port number     */
    MSD_U16          u16Data;
    MSD_U16  pageReg;

    MSD_DBG_INFO("Peridot_gprtPortPowerDown Called.\r\n");

    /* translate LPORT to hardware port */
    hwPort = port;
    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_INFO("\tBad Port: %u.\r\n", port);
        return MSD_BAD_PARAM;
    }

    if ((retVal = Peridot_msdGetSMIPhyRegField(hwPort, 22, 0, 8, &pageReg)) != MSD_OK)
    {
        MSD_DBG_ERROR("\tNot able to read Phy Page Register.\r\n");
        return retVal;
    }

    MSD_BOOL_2_BIT(state,u16Data);

    if((retVal=Peridot_msdSetPagedPhyRegField(hwPort,0,PERIDOT_QD_PHY_CONTROL_REG,11,1,u16Data)) != MSD_OK)
    {
        MSD_DBG_ERROR("\tread Phy Control Register %s.\r\n", msdDisplayStatus(retVal));

        return retVal;
    }

    if ((retVal = Peridot_msdSetSMIPhyRegField(hwPort, 22, 0, 8, pageReg)) != MSD_OK)
    {
        MSD_DBG_ERROR("\twrite back Phy Page Register %s.\r\n", msdDisplayStatus(retVal));
        return retVal;
    }

    MSD_DBG_INFO("\tPeridot_gprtPortPowerDown Exit.\r\n");
    return MSD_OK;
}
/*******************************************************************************
* Peridot_gphySetPortDuplexMode
*
* DESCRIPTION:
*       Sets duplex mode for a specific logical port. This function will keep
*       the speed and loopback mode to the previous value, but disable others,
*       such as Autonegotiation.
*
* INPUTS:
*       port -  The logical port number, unless SERDES device is accessed
*                The physical address, if SERDES device is accessed
*       dMode - dulpex mode
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MSD_OK  - on success
*       MSD_FAIL - on error
*       MSD_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       data sheet register 0.8 - Duplex Mode
*
*       If DUT is in power down or loopback mode, port will link down,
*       in this case, no need to do software reset to force take effect .
*       When port recover from link down, configure will take effect automatically.
*
*******************************************************************************/
MSD_STATUS Peridot_gphySetPortDuplexMode
(
    IN MSD_LPORT  port,
    IN MSD_BOOL   dMode
)
{
    MSD_U8           hwPort;         /* the physical port number     */
    MSD_U16             u16Data;
    MSD_STATUS        retVal;
    MSD_U16  pageReg;

    MSD_DBG_INFO("Peridot_gphySetPortDuplexMode Called.\r\n");

    /* translate LPORT to hardware port */
    hwPort = port;
    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("Bad Port: %u.\r\n", (unsigned int)port);
        return MSD_BAD_PARAM;
    }

    if ((retVal = Peridot_msdGetSMIPhyRegField(hwPort, 22, 0, 8, &pageReg)) != MSD_OK)
    {
        MSD_DBG_ERROR("read Phy Page Register %s.\r\n", msdDisplayStatus(retVal));
        return retVal;
    }

    if(Peridot_msdReadPagedPhyReg(hwPort,0,PERIDOT_QD_PHY_CONTROL_REG,&u16Data) != MSD_OK)
    {
        MSD_DBG_ERROR("read Phy Control Register %s.\r\n", msdDisplayStatus(retVal));
        return MSD_FAIL;
    }

    if(dMode)
    {
        u16Data = (u16Data & (PERIDOT_QD_PHY_LOOPBACK | PERIDOT_QD_PHY_SPEED | PERIDOT_QD_PHY_SPEED_MSB)) | PERIDOT_QD_PHY_DUPLEX;
    }
    else
    {
        u16Data = u16Data & (PERIDOT_QD_PHY_LOOPBACK | PERIDOT_QD_PHY_SPEED | PERIDOT_QD_PHY_SPEED_MSB);
    }

    /* Write to Phy Control Register.  */
    retVal = Peridot_msdPhyReset(hwPort, 0, u16Data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("Peridot_msdPhyReset returned: %s.\r\n", msdDisplayStatus(retVal));
        return retVal;
    }

    if ((retVal = Peridot_msdSetSMIPhyRegField(hwPort, 22, 0, 8, pageReg)) != MSD_OK)
    {
        MSD_DBG_ERROR("write back Phy Page Register %s.\r\n", msdDisplayStatus(retVal));
        return retVal;
    }

    MSD_DBG_INFO("Peridot_gphySetPortDuplexMode Exit.\r\n");
    return retVal;
}
/*******************************************************************************
* Peridot_gphySetPortAutoMode
*
* DESCRIPTION:
*       This routine sets up the port with given Auto Mode.
*       Supported mode is as follows:
*        - Auto for both speed and duplex.
*        - Auto for speed only and Full duplex.
*        - Auto for speed only and Half duplex.
*        - Auto for duplex only and speed 1000Mbps.
*        - Auto for duplex only and speed 100Mbps.
*        - Auto for duplex only and speed 10Mbps.
*        - Speed 1000Mbps and Full duplex.
*        - Speed 1000Mbps and Half duplex.
*        - Speed 100Mbps and Full duplex.
*        - Speed 100Mbps and Half duplex.
*        - Speed 10Mbps and Full duplex.
*        - Speed 10Mbps and Half duplex.
*
*
* INPUTS:
*       port -  The logical port number, unless SERDES device is accessed
*               The physical address, if SERDES device is accessed
*       mode -  Auto Mode to be written
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MSD_OK   - on success
*       MSD_FAIL - on error
*       MSD_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       data sheet register 4.6, 4.5 Autonegotiation Advertisement for 10BT
*       data sheet register 4.8, 4.7 Autonegotiation Advertisement for 100BT
*       data sheet register 9.9, 9.8 Autonegotiation Advertisement for 1000BT
*
*       If DUT is in power down or loopback mode, port will link down,
*       in this case, no need to do software reset to restart AN.
*       When port recover from link down, AN will restart automatically.
*
*******************************************************************************/
MSD_STATUS Peridot_gphySetPortAutoMode
(
    IN MSD_LPORT  port,
    IN PERIDOT_MSD_PHY_AUTO_MODE mode
)
{

    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U8           hwPort;         /* the physical port number     */
    MSD_U16  pageReg;

    MSD_DBG_INFO("Peridot_gphySetPortAutoMode Called.\r\n");

    /* translate LPORT to hardware port */
    hwPort = port;
    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("Bad Port: %u.\r\n", (unsigned int)port);
        return MSD_BAD_PARAM;
    }

    if ((retVal = Peridot_msdGetSMIPhyRegField(hwPort, 22, 0, 8, &pageReg)) != MSD_OK)
    {
        MSD_DBG_ERROR("read Phy Page Register %s.\r\n", msdDisplayStatus(retVal));
        return retVal;
    }

    retVal = phySetAutoMode(hwPort, mode);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("Peridot_phySetAutoMode returned: %s.\r\n", msdDisplayStatus(retVal));
        return retVal;
    }

    if ((retVal = Peridot_msdSetSMIPhyRegField(hwPort, 22, 0, 8, pageReg)) != MSD_OK)
    {
        MSD_DBG_ERROR("write back Phy Page Register %s.\r\n", msdDisplayStatus(retVal));
        return retVal;
    }

    MSD_DBG_INFO("Peridot_gphySetPortAutoMode Exit.\r\n");
    return retVal;

}

MSD_STATUS Peridot_gphySetEEE
(
    IN MSD_LPORT portNum,
    IN MSD_BOOL en
)
{
    MSD_STATUS retVal;
    MSD_U8 hwPort;
    MSD_U16 data;

    MSD_DBG_INFO("Peridot_gphySetEEE Called.\r\n");

    /* translate LPORT to hardware port */
    hwPort = portNum;
    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("Bad Port: %u.\r\n", (unsigned int)portNum);
        return MSD_BAD_PARAM;
    }
    switch (en)
    {
        case MSD_TRUE:
            data = 0x06;
            break;
        case MSD_FALSE:
            data = 0;
            break;
        default:
            data = 0x06;
            break;
    }

    /*enable/Disable the EEE for 100Base-T and 1000Base-T*/
    retVal = Peridot_msdSetSMIPhyXMDIOReg((MSD_U8)hwPort, 7, 60, data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("enable/Disable the EEE for 100Base-T and 1000Base-T %s.\r\n", msdDisplayStatus(retVal));
        return retVal;
    }
    /*Software reset*/
    retVal = Peridot_msdReadPagedPhyReg((MSD_U8)hwPort, 0, 0, &data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("get status.\n %s", msdDisplayStatus(retVal));
        return retVal;
    }
    data = data | 0x8000;
    retVal = Peridot_msdWritePagedPhyReg((MSD_U8)hwPort, 0, 0, data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("Software reset %s.\r\n", msdDisplayStatus(retVal));
        return retVal;
    }

    MSD_DBG_INFO("Peridot_gphySetEEE Exit.\r\n");
    return retVal;
}


MSD_STATUS Peridot_gphySetFlowControlEnable
(
    IN MSD_LPORT portNum,
    IN MSD_BOOL en
)
{
    MSD_STATUS retVal;
    MSD_U8 hwPort;
    MSD_U16 data, tempData;

    MSD_DBG_INFO("Peridot_gphySetFlowControlEnable Called.\r\n");

    /* translate LPORT to hardware port */
    hwPort = portNum;
    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("Bad Port: %u.\r\n", (unsigned int)portNum);
        return MSD_BAD_PARAM;
    }
    switch (en)
    {
    case MSD_TRUE:
        data = 0x1;
        break;
    case MSD_FALSE:
        data = 0x0;
        break;
    default:
        data = 0x1;
        break;
    }

    /*enable/Disable the flow control page 0 reg 4 bit 10*/
    retVal = Peridot_msdReadPagedPhyReg((MSD_U8)hwPort, 0, 4, &tempData);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("get status %s.\r\n", msdDisplayStatus(retVal));
        return retVal;
    }

    data = (tempData & 0xFBFF) | (data << 10);

    retVal = Peridot_msdWritePagedPhyReg((MSD_U8)hwPort, 0, 4, data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("enable/Disable flow control %s.\r\n", msdDisplayStatus(retVal));
        return retVal;
    }
    /*Software reset*/
    retVal = Peridot_msdReadPagedPhyReg((MSD_U8)hwPort, 0, 0, &data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("get status %s.\r\n", msdDisplayStatus(retVal));
        return retVal;
    }
    data = data | 0x8000;
    retVal = Peridot_msdWritePagedPhyReg((MSD_U8)hwPort, 0, 0, data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("Software reset %s.\r\n", msdDisplayStatus(retVal));
        return retVal;
    }

    MSD_DBG_INFO("Peridot_gphySetFlowControlEnable Exit.\r\n");
    return retVal;
}

/*******************************************************************************
* Peridot_gphySetIntEnable
*
* DESCRIPTION:
*       This routine enables or disables one PHY Interrupt, When an interrupt occurs,
*       the corresponding bit is set and remains set until register 19 is read. When
*       interrupt enable bits are not set in register 18, interrupt status bits in
*       register 19 are still set when the corresponding interrupt events occur.
*       If a certain interrupt event is not enabled for the INTn pin, it will still
*       be indicated by the corresponding Interrupt status bits if the interrupt event occurs.
*       The unselected events will not cause the INTn pin to be activated.
*
* INPUTS:
*       devNum  - physical devie number
*       portNum - The logical PHY port number
*       en      - the data of interrupt to enable/disable. any combination of below
*                 MSD_PHY_AUTO_NEG_ERROR
*                 MSD_PHY_SPEED_CHANGED
*                 MSD_PHY_DUPLEX_CHANGED
*                 MSD_PHY_PAGE_RECEIVED
*                 MSD_PHY_AUTO_NEG_COMPLETED
*                 MSD_PHY_LINK_STATUS_CHANGED
*                 MSD_PHY_SYMBOL_ERROR
*                 MSD_PHY_FALSE_CARRIER
*                 MSD_PHY_MDI_CROSSOVER_CHANGED
*                 MSD_PHY_DOWNSHIFT
*                 MSD_PHY_ENERGY_DETECT
*                 MSD_PHY_FLP_EXCHANGE_COMPLETE
*                 MSD_PHY_DTE_POWER_DETECTION
*                 MSD_PHY_POLARITY_CHANGED
*                 MSD_PHY_JABBER
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
*
*******************************************************************************/
MSD_STATUS Peridot_gphySetIntEnable
(
IN MSD_LPORT portNum,
IN MSD_U16 en
)
{
    MSD_STATUS retVal;
    MSD_U8 hwPort;

    MSD_DBG_INFO("Peridot_gphySetIntEnable Called.\r\n");

    /* translate LPORT to hardware port */
    hwPort = portNum;
    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("Bad Port: %u.\r\n", (unsigned int)portNum);
        return MSD_BAD_PARAM;
    }

    /*enable/Disable page 0 reg 18 interrupt enable*/
    retVal = Peridot_msdWritePagedPhyReg((MSD_U8)hwPort, 0, 18, (MSD_U16)en);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("enable/Disable interrupt %s.\r\n", msdDisplayStatus(retVal));
        return retVal;
    }

    MSD_DBG_INFO("Peridot_gphySetIntEnable Exit.\r\n");

    return retVal;
}

/*******************************************************************************
* Peridot_gphyGetIntEnable
*
* DESCRIPTION:
*       This routine gets one PHY Interrupt enable/disable, When an interrupt occurs,
*       the corresponding bit is set and remains set until register 19 is read. When
*       interrupt enable bits are not set in register 18, interrupt status bits in
*       register 19 are still set when the corresponding interrupt events occur.
*       If a certain interrupt event is not enabled for the INTn pin, it will still
*       be indicated by the corresponding Interrupt status bits if the interrupt event occurs.
*       The unselected events will not cause the INTn pin to be activated.
*
* INPUTS:
*       devNum  - physical devie number
*       portNum - The logical PHY port number
*
* OUTPUTS:
*       en      - the data of interrupt to enable/disable.
*
* RETURNS:
*       MSD_OK  - on success
*       MSD_FAIL  - on error
*       MSD_BAD_PARAM - if invalid parameter is given
*       MSD_NOT_SUPPORTED - device not support
*
* COMMENTS:
*
*******************************************************************************/
MSD_STATUS Peridot_gphyGetIntEnable
(
IN MSD_LPORT portNum,
OUT MSD_U16 *en
)
{
    MSD_STATUS retVal;
    MSD_U8 hwPort;
    MSD_U16 data;

    MSD_DBG_INFO("Peridot_gphyGetIntEnable Called.\r\n");

    /* translate LPORT to hardware port */
    hwPort = portNum;
    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("Bad Port: %u.\r\n", (unsigned int)portNum);
        return MSD_BAD_PARAM;
    }


    /*enable/Disable page 0 reg 18 interrupt enable*/
    retVal = Peridot_msdReadPagedPhyReg((MSD_U8)hwPort, 0, 18, &data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("enable/Disable interrupt %s.\r\n", msdDisplayStatus(retVal));
        return retVal;
    }

    *en = (MSD_U16)data;

    MSD_DBG_INFO("Peridot_gphyGetIntEnable Exit.\r\n");

    return retVal;
}
/*******************************************************************************
* Peridot_gphyGetIntStatus
*
* DESCRIPTION:
*       This routine gets one PHY Interrupt status, When an interrupt occurs,
*       the corresponding bit is set and remains set until register 19 is read. When
*       interrupt enable bits are not set in register 18, interrupt status bits in
*       register 19 are still set when the corresponding interrupt events occur.
*       If a certain interrupt event is not enabled for the INTn pin, it will still
*       be indicated by the corresponding Interrupt status bits if the interrupt event occurs.
*       The unselected events will not cause the INTn pin to be activated.
*
* INPUTS:
*       devNum  - physical devie number
*       portNum - The logical PHY port number
*
* OUTPUTS:
*       en      - the data of interrupt to enable/disable.
*
* RETURNS:
*       MSD_OK  - on success
*       MSD_FAIL  - on error
*       MSD_BAD_PARAM - if invalid parameter is given
*       MSD_NOT_SUPPORTED - device not support
*
* COMMENTS:
*
*******************************************************************************/
MSD_STATUS Peridot_gphyGetIntStatus
(
IN MSD_LPORT portNum,
OUT MSD_U16 *stat
)
{
    MSD_STATUS retVal;
    MSD_U8 hwPort;
    MSD_U16 data;

    MSD_DBG_INFO("Peridot_gphyGetIntStatus Called.\r\n");

    /* translate LPORT to hardware port */
    hwPort = portNum;
    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("Bad Port: %u.\r\n", (unsigned int)portNum);
        return MSD_BAD_PARAM;
    }

    /*enable/Disable page 0 reg 19 interrupt status*/
    retVal = Peridot_msdReadPagedPhyReg((MSD_U8)hwPort, 0, 19, &data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("enable/Disable interrupt %s.\r\n", msdDisplayStatus(retVal));
        return retVal;
    }

    *stat = (MSD_U16)data;

    MSD_DBG_INFO("Peridot_gphyGetIntStatus Exit.\r\n");

    return retVal;
}

#if 0
MSD_STATUS Peridot_gphyGetAnyReg
(
    IN MSD_LPORT  port,
    OUT MSD_U16 *data
)
{

    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U8           hwPort;         /* the physical port number     */
    MSD_U16  pageReg;

    MSD_DBG_INFO("Peridot_gphyGetAnyReg Called.\r\n");

    /* translate LPORT to hardware port */
    hwPort = port;
    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("Bad Port: %u.\r\n", (unsigned int)port));
        return MSD_BAD_PARAM;
    }

    if ((retVal = Peridot_msdGetSMIPhyRegField(hwPort, 22, 0, 8, &pageReg)) != MSD_OK)
    {
        MSD_DBG_ERROR("read Phy Page Register %s.\r\n", msdDisplayStatus(retVal)));
        return retVal;
    }

    retVal = phySetAutoMode(hwPort, mode);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("Peridot_phySetAutoMode returned: %s.\r\n", msdDisplayStatus(retVal)));
        return retVal;
    }

    if ((retVal = Peridot_msdSetSMIPhyRegField(hwPort, 22, 0, 8, pageReg)) != MSD_OK)
    {
        MSD_DBG_ERROR("write back Phy Page Register %s.\r\n", msdDisplayStatus(retVal)));
        return retVal;
    }

    MSD_DBG_INFO("Peridot_gphySetPortAutoMode Exit.\r\n");
    return retVal;

}
#endif
