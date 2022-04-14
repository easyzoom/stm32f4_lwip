/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-03-31     Lenovo       the first version
 */

#include "Marvell_88E6390_HwAccess.h"


void marvell_delay_ms(MSD_U16 t_ms)
{
	delay_ms(t_ms);
}
/*******************************************************************************
*
*
* DESCRIPTION:
*       This function directly writes to a switch's register.
*
* INPUTS:
*       devAddr - device register.
*       regAddr - The register's address.
*       data    - The data to be written.
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
MSD_STATUS msdSetAnyReg
(
    IN  MSD_U8    devAddr,
    IN  MSD_U8    regAddr,
    IN  MSD_U16   data
)
{
    uint16_t   state;

    MSD_DBG_INFO("(LOG RW): devAddr 0x%02x, regAddr 0x%02x,\r\n",
              devAddr,regAddr);
    MSD_DBG_INFO("\tRW data: 0x%04x.\r\n", data);

    state = mdio_write_regs(devAddr,regAddr,data);
    
		if(state)
		{
			MSD_DBG_INFO("\tRW failure:%d\r\n", state);
			return MSD_FAIL;
		}
		
    return MSD_OK;
}
/*******************************************************************************
* msdGetAnyReg
*
* DESCRIPTION:
*       This function directly reads a switch's register.
*
* INPUTS:
*       devAddr - device register.
*       regAddr - The register's address.
*
* OUTPUTS:
*       data    - The read register's data.
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
MSD_STATUS msdGetAnyReg
(
    IN  MSD_U8    devAddr,
    IN  MSD_U8    regAddr,
    OUT MSD_U16   *data
)
{
		uint16_t state = 0;
	
    state = mdio_read_regs(devAddr,regAddr,data);
    MSD_DBG_INFO("(LOG RR): devAddr 0x%02x, regAddr 0x%02x,\r\n",
              devAddr,regAddr);
		if(state)
		{
			MSD_DBG_INFO("\tRR failure:%d\r\n", state);
			return MSD_FAIL;
		}
		MSD_DBG_INFO("\tRR data: 0x%04x.\r\n", *data);
		
    return MSD_OK;
}
/*******************************************************************************
* msdSetAnyRegField
*
* DESCRIPTION:
*       This function writes to specified field in a switch's register.
*
* INPUTS:
*       devAddr     - Device Address to write the register for.
*       regAddr     - The register's address.
*       fieldOffset - The field start bit index. (0 - 15)
*       fieldLength - Number of bits to write.
*       data        - Data to be written.
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
*       1.  The sum of fieldOffset & fieldLength parameters must be smaller-
*           equal to 16.
*
*******************************************************************************/
MSD_STATUS msdSetAnyRegField
(
    IN  MSD_U8    devAddr,
    IN  MSD_U8    regAddr,
    IN  MSD_U8    fieldOffset,
    IN  MSD_U8    fieldLength,
    IN  MSD_U16   data
)
{
    MSD_U16 mask;
    MSD_U16 tmpData;
    MSD_STATUS   retVal;

    retVal = msdGetAnyReg(devAddr, regAddr, &tmpData);
    if(retVal != MSD_OK)
    {
        return retVal;
    }

    MSD_CALC_MASK(fieldOffset,fieldLength,mask);

    /* Set the desired bits to 0.                       */
    tmpData &= ~mask;
    /* Set the given data into the above reset bits.    */
    tmpData |= ((data << fieldOffset) & mask);
    MSD_DBG_INFO("(LOG RW): devAddr 0x%02x, regAddr 0x%02x, ",
              devAddr,regAddr);
    MSD_DBG_INFO("fOff %d, fLen %d, data 0x%04x.\r\n", fieldOffset,
              fieldLength,data);

    retVal = msdSetAnyReg(devAddr, regAddr, tmpData);

    return retVal;
}
/*******************************************************************************
* msdGetAnyRegField
*
* DESCRIPTION:
*       This function reads a specified field from a switch's register.
*
* INPUTS:
*       devAddr     - device address to read the register for.
*       regAddr     - The register's address.
*       fieldOffset - The field start bit index. (0 - 15)
*       fieldLength - Number of bits to read.
*
* OUTPUTS:
*       data        - The read register field.
*
* RETURNS:
*       MSD_OK  - on success
*       MSD_FAIL  - on error
*       MSD_BAD_PARAM - if invalid parameter is given
*       MSD_NOT_SUPPORTED - device not support
*
* COMMENTS:
*       1.  The sum of fieldOffset & fieldLength parameters must be smaller-
*           equal to 16.
*
*******************************************************************************/
MSD_STATUS msdGetAnyRegField
(
    IN  MSD_U8    devAddr,
    IN  MSD_U8    regAddr,
    IN  MSD_U8    fieldOffset,
    IN  MSD_U8    fieldLength,
    OUT MSD_U16   *data
)
{
    MSD_U16 mask;            /* Bits mask to be read */
    MSD_U16 tmpData;
		uint16_t state;
	
		MSD_DBG_INFO("(LOG RR): devAddr 0x%02x, regAddr 0x%02x ",
              devAddr,regAddr);
    state = mdio_read_regs(devAddr, regAddr, &tmpData);
		if(state)
		{
			 MSD_DBG_INFO("\tRR failure\r\n");
			return MSD_FAIL;
		}
	
    MSD_CALC_MASK(fieldOffset,fieldLength,mask);

    tmpData = (tmpData & mask) >> fieldOffset;
    *data = tmpData;

    MSD_DBG_INFO("\tfOff %d, fLen %d, data 0x%04x.\n", fieldOffset, fieldLength, *data);

    return MSD_OK;
}
/*****************************************************************************
* msdSetSMIPhyReg
*
* DESCRIPTION:
*       This function indirectly write internal PHY register through SMI PHY command.
*
* INPUTS:
*       devAddr - The PHY address.
*       regAddr - The register address.
*       data - data to be written
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MSD_OK   - on success
*       MSD_FAIL - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MSD_STATUS Peridot_msdSetSMIPhyReg
(
    IN  MSD_U8    devAddr,
    IN  MSD_U8    regAddr,
    IN MSD_U16   data
)
{
    unsigned int timeOut;
    MSD_U16 smiReg;

    MSD_DBG_INFO("(LOG PERIDOT SMI PHY C22 RW): devAddr 0x%02x, regAddr 0x%02x, \r\n",
        devAddr, regAddr);
    MSD_DBG_INFO("\tdata 0x%04x.\r\n", data);

    /* first check that it is not busy */
    timeOut = MSD_SMI_ACCESS_LOOP; /* initialize the loop count */
    do
    {
        if(msdGetAnyReg(PERIDOT_GLOBAL2_DEV_ADDR,PERIDOT_QD_REG_SMI_PHY_CMD, &smiReg) != MSD_OK)
        {
            MSD_DBG_ERROR("\tread PERIDOT_QD_REG_SMI_PHY_CMD register returned: %s.\r\n", msdDisplayStatus(MSD_FAIL));
            return MSD_FAIL;
        }
        if(timeOut-- < 1 )
        {
            MSD_DBG("\tread PERIDOT_QD_REG_SMI_PHY_CMD register Timed Out\r\n");
            return MSD_FAIL;
        }
    } while (smiReg & MSD_SMI_BUSY);

    if(msdSetAnyReg(PERIDOT_GLOBAL2_DEV_ADDR,PERIDOT_QD_REG_SMI_PHY_DATA, data) != MSD_OK)
    {
        MSD_DBG_ERROR("\twrite PERIDOT_QD_REG_SMI_PHY_DATA register returned: %s.\r\n", msdDisplayStatus(MSD_FAIL));
        return MSD_FAIL;
    }
    if(devAddr == 0)
    {
        smiReg = MSD_SMI_BUSY | (devAddr << MSD_SMI_DEV_ADDR_BIT) | (MSD_SMI_EXTERNAL << MSD_SMI_FUNC_BIT) | (MSD_SMI_WRITE << MSD_SMI_OP_BIT) |
        (regAddr << MSD_SMI_REG_ADDR_BIT) | (MSD_SMI_CLAUSE22 << MSD_SMI_MODE_BIT);
    }
    else
    {
        smiReg = MSD_SMI_BUSY | (devAddr << MSD_SMI_DEV_ADDR_BIT) | (MSD_SMI_WRITE << MSD_SMI_OP_BIT) |
            (regAddr << MSD_SMI_REG_ADDR_BIT) | (MSD_SMI_CLAUSE22 << MSD_SMI_MODE_BIT);
    }
    if(msdSetAnyReg(PERIDOT_GLOBAL2_DEV_ADDR,PERIDOT_QD_REG_SMI_PHY_CMD, smiReg) != MSD_OK)
    {
        MSD_DBG_ERROR("\twrite PERIDOT_QD_REG_SMI_PHY_CMD register returned: %s.\r\n", msdDisplayStatus(MSD_FAIL));
        return MSD_FAIL;
    }

    return MSD_OK;
}
/*****************************************************************************
* msdGetSMIPhyReg
*
* DESCRIPTION:
*       This function indirectly read internal PHY register through SMI PHY command.
*
* INPUTS:
*       devAddr  - The PHY address to be read.
*       regAddr  - The register address to read.
*
* OUTPUTS:
*       data     - The storage where register date to be saved.
*
* RETURNS:
*       MSD_OK   - on success
*       MSD_FAIL - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MSD_STATUS Peridot_msdGetSMIPhyReg
(
    IN  MSD_U8    devAddr,
    IN  MSD_U8    regAddr,
    OUT  MSD_U16   *data
)
{
    unsigned int timeOut;
    MSD_U16 smiReg;

	
		MSD_DBG_INFO("(LOG PERIDOT SMI PHY C22 RW): devAddr 0x%02x, regAddr 0x%02x, \r\n",
        devAddr, regAddr);
    /* first check that it is not busy */
    timeOut = MSD_SMI_ACCESS_LOOP; /* initialize the loop count */

    do
    {
        if(msdGetAnyReg(PERIDOT_GLOBAL2_DEV_ADDR,PERIDOT_QD_REG_SMI_PHY_CMD, &smiReg) != MSD_OK)
        {
            MSD_DBG_ERROR("\tread PERIDOT_QD_REG_SMI_PHY_CMD register returned: %s.\r\n", msdDisplayStatus(MSD_FAIL));
            return MSD_FAIL;
        }
        if(timeOut-- < 1 )
        {
            MSD_DBG_ERROR("\tRead PERIDOT_QD_REG_SMI_PHY_CMD Timed Out\r\n");
            return MSD_FAIL;
        }
    } while (smiReg & MSD_SMI_BUSY);
    if(devAddr == 0)
    {
        smiReg = MSD_SMI_BUSY | (devAddr << MSD_SMI_DEV_ADDR_BIT) | (MSD_SMI_EXTERNAL << MSD_SMI_FUNC_BIT) | (MSD_SMI_READ_22 << MSD_SMI_OP_BIT) |
        (regAddr << MSD_SMI_REG_ADDR_BIT) | (MSD_SMI_CLAUSE22 << MSD_SMI_MODE_BIT);
    }
    else
    {
        smiReg =  MSD_SMI_BUSY | (devAddr << MSD_SMI_DEV_ADDR_BIT) | (MSD_SMI_READ_22 << MSD_SMI_OP_BIT) |
            (regAddr << MSD_SMI_REG_ADDR_BIT) | (MSD_SMI_CLAUSE22 << MSD_SMI_MODE_BIT);
    }

    if(msdSetAnyReg(PERIDOT_GLOBAL2_DEV_ADDR,PERIDOT_QD_REG_SMI_PHY_CMD, smiReg) != MSD_OK)
    {
        MSD_DBG_ERROR("\twrite PERIDOT_QD_REG_SMI_PHY_CMD register returned: %s.\r\n", msdDisplayStatus(MSD_FAIL));
        return MSD_FAIL;
    }

    timeOut = MSD_SMI_ACCESS_LOOP; /* initialize the loop count */

    do
    {
        if(msdGetAnyReg(PERIDOT_GLOBAL2_DEV_ADDR,PERIDOT_QD_REG_SMI_PHY_CMD, &smiReg) != MSD_OK)
        {
            MSD_DBG_ERROR("\tread PERIDOT_QD_REG_SMI_PHY_CMD register returned: %s.\r\n", msdDisplayStatus(MSD_FAIL));
            return MSD_FAIL;
        }
        if(timeOut-- < 1 )
        {
            MSD_DBG_ERROR("\tread TOPAZ_QD_REG_SMI_PHY_DATA register returned: %s.\r\n", msdDisplayStatus(MSD_FAIL));
            return MSD_FAIL;
        }
    } while (smiReg & MSD_SMI_BUSY);

    if(msdGetAnyReg(PERIDOT_GLOBAL2_DEV_ADDR,PERIDOT_QD_REG_SMI_PHY_DATA, &smiReg) != MSD_OK)
    {
        MSD_DBG("\tReading Phy register Failed\r\n");
        return MSD_FAIL;
    }
    *data = smiReg;

    
    MSD_DBG_INFO("\tdata 0x%04x.\r\n", *data);

    return MSD_OK;
}
/*******************************************************************************
* msdPhyReset
*
* DESCRIPTION:
*       This function performs softreset and waits until reset completion.
*
* INPUTS:
*       portNum     - Port number to write the register for.
*       u16Data     - data should be written into Phy control register.
*                      if this value is 0xFF, normal operation occcurs (read,
*                      update, and write back.)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MSD_OK   - on success
*       MSD_FAIL - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MSD_STATUS Peridot_msdPhyReset
(
    IN  MSD_U8        portNum,
    IN  MSD_U8       pageNum,
    IN  MSD_U16       u16Data
)
{
    MSD_U16 tmpData = 0;
    MSD_STATUS   retVal;
    MSD_U32 retryCount;
    MSD_BOOL    pd = MSD_FALSE;

    /*if((retVal=msdGetSMIPhyReg(dev,portNum,0,&tmpData))
           != MSD_OK)*/
    if((retVal=Peridot_msdReadPagedPhyReg(portNum, pageNum, 0,&tmpData))
           != MSD_OK)
    {
        MSD_DBG("Reading Register failed\r\n");
        return retVal;
    }

    if (tmpData & 0x800)
    {
        pd = MSD_TRUE;
    }

    if (u16Data != 0xFF)
    {
        tmpData = u16Data;
    }

    /* Set the desired bits to 0. */
    if (pd)
    {
        tmpData |= 0x800;
    }
    else
    {
      if(((tmpData&0x4000)==0)||(u16Data==0xFF)) /* setting loopback do not set reset */
        tmpData |= 0x8000;
    }

    /*if((retVal=msdSetSMIPhyReg(dev,portNum,0,tmpData))
        != MSD_OK)*/
    if((retVal=Peridot_msdWritePagedPhyReg(portNum, pageNum, 0,tmpData))
        != MSD_OK)
    {
        MSD_DBG("Writing to register failed\r\n");
        return retVal;
    }

    if (pd)
    {
        return MSD_OK;
    }

    for (retryCount = 0x1000; retryCount > 0; retryCount--)
    {
        /*if((retVal=msdGetSMIPhyReg(dev,portNum,0,&tmpData)) != MSD_OK)*/
        if((retVal=Peridot_msdReadPagedPhyReg(portNum, pageNum, 0,&tmpData)) != MSD_OK)
        {
            MSD_DBG("Reading register failed\r\n");
            return retVal;
        }
        if ((tmpData & 0x8000) == 0)
            break;
    }

    if (retryCount == 0)
    {
        MSD_DBG("Reset bit is not cleared\r\n");
        return MSD_FAIL;
    }

    return MSD_OK;
}
/*****************************************************************************
* msdSetSMIPhyRegField
*
* DESCRIPTION:
*       This function indirectly write internal PHY specified register field through SMI PHY command.
*
* INPUTS:
*       devAddr     - The PHY address to be read.
*       regAddr     - The register address to read.
*       fieldOffset - The field start bit index. (0 - 15)
*       fieldLength - Number of bits to write.
*       data        - register date to be written.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       MSD_TRUE   - on success
*       MSD_FALSE  - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MSD_STATUS Peridot_msdSetSMIPhyRegField
(
    IN  MSD_U8    devAddr,
    IN  MSD_U8    regAddr,
    IN  MSD_U8    fieldOffset,
    IN  MSD_U8    fieldLength,
    IN MSD_U16    data
)
{
    MSD_U16 mask;
    MSD_U16 tmpData;
    MSD_STATUS   retVal;

    retVal =  Peridot_msdGetSMIPhyReg(devAddr,regAddr,&tmpData);
    if(retVal != MSD_OK)
    {
        return retVal;
    }

    MSD_CALC_MASK(fieldOffset,fieldLength,mask);

    /* Set the desired bits to 0.                       */
    tmpData &= ~mask;
    /* Set the given data into the above reset bits.    */
    tmpData |= ((data << fieldOffset) & mask);
    MSD_DBG("Write to devAddr(%d): regAddr 0x%x, \r\n",
              devAddr,regAddr);
    MSD_DBG("fieldOff %d, fieldLen %d, data 0x%x.\r\n",fieldOffset,
              fieldLength,data);

    retVal = Peridot_msdSetSMIPhyReg(devAddr,regAddr,tmpData);

    return retVal;
}
/*****************************************************************************
* msdGetSMIPhyRegField
*
* DESCRIPTION:
*       This function indirectly read internal PHY specified register field through SMI PHY command.
*
* INPUTS:
*       devAddr     - The PHY address to be read.
*       regAddr     - The register address to read.
*       fieldOffset - The field start bit index. (0 - 15)
*       fieldLength - Number of bits to write.
*
* OUTPUTS:
*       data       - The storage where register date to be saved.
*
* RETURNS:
*       MSD_TRUE   - on success
*       MSD_FALSE  - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MSD_STATUS Peridot_msdGetSMIPhyRegField
(
    IN  MSD_U8    devAddr,
    IN  MSD_U8    regAddr,
    IN  MSD_U8    fieldOffset,
    IN  MSD_U8    fieldLength,
    OUT  MSD_U16   *data
)
{
    MSD_U16 mask;            /* Bits mask to be read */
    MSD_U16 tmpData;
    MSD_STATUS   retVal;

    retVal =  Peridot_msdGetSMIPhyReg(devAddr,regAddr,&tmpData);
    if (retVal != MSD_OK)
        return retVal;

    MSD_CALC_MASK(fieldOffset,fieldLength,mask);

    tmpData = (tmpData & mask) >> fieldOffset;
    *data = tmpData;
    MSD_DBG("Read from deviceAddr(%d): regAddr 0x%x, \r\n",
              devAddr,regAddr);
    MSD_DBG("fOff %d, fLen %d, data 0x%x.\r\n",fieldOffset,fieldLength,*data);

    return MSD_OK;
}
/*****************************************************************************
* msdGetSMIC45PhyReg
*
* DESCRIPTION:
*       This function indirectly read internal SERDES register through SMI PHY command.
*
* INPUTS:
*       devAddr - The device address.
*       phyAddr - The PHY address.
*       regAddr  - The register address to read.
*
* OUTPUTS:
*       data     - The storage where register data to be saved.
*
* RETURNS:
*       MSD_OK   - on success
*       MSD_FAIL - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MSD_STATUS Peridot_msdGetSMIC45PhyReg
(
    IN  MSD_U8     devAddr,
    IN  MSD_U8     phyAddr,
    IN  MSD_U16    regAddr,
    OUT MSD_U16    *data
)
{
    unsigned int timeOut;
    MSD_U16 smiReg;
    MSD_U16 tmp;

	
		MSD_DBG_INFO("(LOG PERIDOT SMI PHY C45 RR): devAddr 0x%02x, phyAddr 0x%02x, regAddr 0x%02x, \r\n",
        devAddr, phyAddr, regAddr);
    /* first check that it is not busy */
    timeOut = MSD_SMI_ACCESS_LOOP; /* initialize the loop count */

    do
    {
        if(msdGetAnyReg(PERIDOT_GLOBAL2_DEV_ADDR,PERIDOT_QD_REG_SMI_PHY_CMD, &smiReg) != MSD_OK)
        {
            MSD_DBG_ERROR("\tread PERIDOT_QD_REG_SMI_PHY_CMD register returned: %s.\r\n", msdDisplayStatus(MSD_FAIL));
            return MSD_FAIL;
        }
        if(timeOut-- < 1 )
        {
            MSD_DBG("\tread PERIDOT_QD_REG_SMI_PHY_CMD register Timed Out\r\n");
            return MSD_FAIL;
        }
    } while (smiReg & MSD_SMI_BUSY);


    if(msdSetAnyReg(PERIDOT_GLOBAL2_DEV_ADDR,PERIDOT_QD_REG_SMI_PHY_DATA, regAddr) != MSD_OK)
    {
        MSD_DBG_ERROR("\twrite PERIDOT_QD_REG_SMI_PHY_DATA register returned: %s.\r\n", msdDisplayStatus(MSD_FAIL));
        return MSD_FAIL;
    }
    
    smiReg = MSD_SMI_BUSY | (phyAddr << MSD_SMI_DEV_ADDR_BIT) | (MSD_SMI_WRITE_ADDR << MSD_SMI_OP_BIT) |
            (devAddr << MSD_SMI_REG_ADDR_BIT) | (MSD_SMI_CLAUSE45 << MSD_SMI_MODE_BIT);

    if(msdSetAnyReg(PERIDOT_GLOBAL2_DEV_ADDR,PERIDOT_QD_REG_SMI_PHY_CMD, smiReg) != MSD_OK)
    {
        MSD_DBG_ERROR("\twrite PERIDOT_QD_REG_SMI_PHY_CMD register returned: %s.\r\n", msdDisplayStatus(MSD_FAIL));
        return MSD_FAIL;
    }

    smiReg = MSD_SMI_BUSY | (phyAddr << MSD_SMI_DEV_ADDR_BIT) | (MSD_SMI_READ_45 << MSD_SMI_OP_BIT) |
            (devAddr << MSD_SMI_REG_ADDR_BIT) | (MSD_SMI_CLAUSE45 << MSD_SMI_MODE_BIT);

    if(msdSetAnyReg(PERIDOT_GLOBAL2_DEV_ADDR,PERIDOT_QD_REG_SMI_PHY_CMD, smiReg) != MSD_OK)
    {
        MSD_DBG_ERROR("\tWrite TOPAZ_QD_REG_SMI_PHY_CMD register Failed\r\n");
        return MSD_FAIL;
    }

    timeOut = MSD_SMI_ACCESS_LOOP; /* initialize the loop count */

    do
    {
        if(msdGetAnyReg(PERIDOT_GLOBAL2_DEV_ADDR,PERIDOT_QD_REG_SMI_PHY_CMD, &smiReg) != MSD_OK)
        {
            MSD_DBG_ERROR("\tread PERIDOT_QD_REG_SMI_PHY_CMD register returned: %s.\r\n", msdDisplayStatus(MSD_FAIL));
            return MSD_FAIL;
        }
        if(timeOut-- < 1 )
        {
            MSD_DBG_ERROR("\tread PERIDOT_QD_REG_SMI_PHY_CMD register Timed Out\r\n");
            return MSD_FAIL;
        }
    } while (smiReg & MSD_SMI_BUSY);


    if(msdGetAnyReg(PERIDOT_GLOBAL2_DEV_ADDR,PERIDOT_QD_REG_SMI_PHY_DATA, &tmp) != MSD_OK)
    {
        MSD_DBG_ERROR("\tread TOPAZ_QD_REG_SMI_PHY_DATA register returned: %s.\r\n", msdDisplayStatus(MSD_FAIL));
        return MSD_FAIL;
    }

    *data = tmp;

    MSD_DBG_INFO("\tdata 0x%04x.\r\n", *data);

    return MSD_OK;
}

/*****************************************************************************
* msdSetSMIC45PhyReg
*
* DESCRIPTION:
*       This function indirectly write internal SERDES register through SMI PHY command.
*
* INPUTS:
*       devAddr - The device address.
*       phyAddr - The PHY address.
*       regAddr - The register address.
*       data - data to be written
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MSD_OK   - on success
*       MSD_FAIL - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MSD_STATUS Peridot_msdSetSMIC45PhyReg
(
    IN MSD_U8    devAddr,
    IN MSD_U8    phyAddr,
    IN MSD_U16   regAddr,
    IN MSD_U16   data
)
{
    unsigned int timeOut;
    MSD_U16 smiReg;

    MSD_DBG_INFO("(LOG PERIDOT SMI PHY C45 RW): devAddr 0x%02x, phyAddr 0x%02x, regAddr 0x%02x, \r\n",
        devAddr, phyAddr, regAddr);
    MSD_DBG_INFO("\tdata 0x%04x.\r\n", data);

    /* first check that it is not busy */
    timeOut = MSD_SMI_ACCESS_LOOP; /* initialize the loop count */

    do
    {
        if(msdGetAnyReg(PERIDOT_GLOBAL2_DEV_ADDR,PERIDOT_QD_REG_SMI_PHY_CMD, &smiReg) != MSD_OK)
        {
            MSD_DBG_ERROR("\tread PERIDOT_QD_REG_SMI_PHY_CMD register returned: %s.\r\n", msdDisplayStatus(MSD_FAIL));
            return MSD_FAIL;
        }
        if(timeOut-- < 1 )
        {
            MSD_DBG_ERROR("\tread SMI Phy PERIDOT_QD_REG_SMI_PHY_CMD register Timed Out\r\n");
            return MSD_FAIL;
        }
    } while (smiReg & MSD_SMI_BUSY);


    if(msdSetAnyReg(PERIDOT_GLOBAL2_DEV_ADDR,PERIDOT_QD_REG_SMI_PHY_DATA, regAddr) != MSD_OK)
    {
        MSD_DBG_ERROR("\tWrite PERIDOT_QD_REG_SMI_PHY_DATA register returned: %s.\r\n", msdDisplayStatus(MSD_FAIL));
        return MSD_FAIL;
    }
    smiReg = MSD_SMI_BUSY | (phyAddr << MSD_SMI_DEV_ADDR_BIT) | (MSD_SMI_WRITE_ADDR << MSD_SMI_OP_BIT) |
            (devAddr << MSD_SMI_REG_ADDR_BIT) | (MSD_SMI_CLAUSE45 << MSD_SMI_MODE_BIT);

    if(msdSetAnyReg(PERIDOT_GLOBAL2_DEV_ADDR,PERIDOT_QD_REG_SMI_PHY_CMD, smiReg) != MSD_OK)
    {
        MSD_DBG_ERROR("\tWrite PERIDOT_QD_REG_SMI_PHY_CMD register returned: %s.\r\n", msdDisplayStatus(MSD_FAIL));
        return MSD_FAIL;
    }

    timeOut = MSD_SMI_ACCESS_LOOP; /* initialize the loop count */

    do
    {
        if(msdGetAnyReg(PERIDOT_GLOBAL2_DEV_ADDR,PERIDOT_QD_REG_SMI_PHY_CMD, &smiReg) != MSD_OK)
        {
            MSD_DBG_ERROR("\tread PERIDOT_QD_REG_SMI_PHY_CMD register returned: %s.\r\n", msdDisplayStatus(MSD_FAIL));
            return MSD_FAIL;
        }
        if(timeOut-- < 1 )
        {
            MSD_DBG("\tread SMI Phy PERIDOT_QD_REG_SMI_PHY_CMD register Timed Out\r\n");
            return MSD_FAIL;
        }
    } while (smiReg & MSD_SMI_BUSY);


    if(msdSetAnyReg(PERIDOT_GLOBAL2_DEV_ADDR,PERIDOT_QD_REG_SMI_PHY_DATA, data) != MSD_OK)
    {
        MSD_DBG_ERROR("\twrite PERIDOT_QD_REG_SMI_PHY_DATA register returned: %s.\r\n", msdDisplayStatus(MSD_FAIL));
        return MSD_FAIL;
    }
    smiReg = MSD_SMI_BUSY | (phyAddr << MSD_SMI_DEV_ADDR_BIT) | (MSD_SMI_WRITE << MSD_SMI_OP_BIT) |
            (devAddr << MSD_SMI_REG_ADDR_BIT) | (MSD_SMI_CLAUSE45 << MSD_SMI_MODE_BIT);

    if(msdSetAnyReg(PERIDOT_GLOBAL2_DEV_ADDR,PERIDOT_QD_REG_SMI_PHY_CMD, smiReg) != MSD_OK)
    {
        MSD_DBG_ERROR("write PERIDOT_QD_REG_SMI_PHY_CMD register returned: %s.\r\n", msdDisplayStatus(MSD_FAIL));
        return MSD_FAIL;
    }

    return MSD_OK;
}
MSD_STATUS Peridot_msdSetSMIPhyXMDIOReg
(
    IN MSD_U8  portNum,
    IN MSD_U8  devAddr,
    IN MSD_U16  regAddr,
    IN MSD_U16 data
)
{
    MSD_STATUS retVal;
    MSD_U16 tmpData;
    MSD_U8 hwPort;

    hwPort = portNum;
    if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
        MSD_DBG_ERROR("Bad Port: %d.\n", portNum);
        return MSD_BAD_PARAM;
    }

    /* Set MMD access control address is Address | devAddr */
    tmpData = MSD_PHY_MMD_ADDR | devAddr;
    retVal = Peridot_msdWritePagedPhyReg(hwPort, 0, 13, tmpData);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("ERROR to Set MMD access control address is Address | devAddr.\r\n");
        return retVal;
    }

    /* Set MMD access Data is regAddr */
    tmpData = regAddr;
    retVal = Peridot_msdWritePagedPhyReg(hwPort, 0, 14, tmpData);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("ERROR to Set MMD access Data is regAddr.\r\n");
        return retVal;
    }

    /* Set MMD access control address is Data no inc | devAddr */
    tmpData = MSD_PHY_MMD_DATA_NO_INC | devAddr;
    retVal = Peridot_msdWritePagedPhyReg(hwPort, 0, 13, tmpData);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("ERROR to Set MMD access control address is Data no inc | devAddr.\r\n");
        return retVal;
    }

    /* Write MMD access Data */
    retVal = Peridot_msdWritePagedPhyReg(hwPort, 0, 14, data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("ERROR to Write MMD access Data.\r\n");
        return retVal;
    }

    return MSD_OK;
}

MSD_STATUS Peridot_msdGetSMIPhyXMDIOReg
(
    IN MSD_U8  portNum,
    IN MSD_U8  devAddr,
    IN MSD_U16  regAddr,
    OUT MSD_U16 *data
)
{
    MSD_STATUS retVal;
    MSD_U16 tmpData;

    /* Set MMD access control address is Address | devAddr */
    tmpData = MSD_PHY_MMD_ADDR | devAddr;
    retVal = Peridot_msdWritePagedPhyReg(portNum, 0, 13, tmpData);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("ERROR to Set MMD access control address is Address | devAddr.\r\n");
        return retVal;
    }

    /* Set MMD access Data is regAddr */
    tmpData = regAddr;
    retVal = Peridot_msdWritePagedPhyReg(portNum, 0, 14, tmpData);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("ERROR to Set MMD access Data is regAddr.\r\n");
        return retVal;
    }

    /* Set MMD access control address is Data no inc | devAddr */
    tmpData = MSD_PHY_MMD_DATA_NO_INC | devAddr;
    retVal = Peridot_msdWritePagedPhyReg(portNum, 0, 13, tmpData);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("ERROR to Set MMD access control address is Data no inc | devAddr.\r\n");
        return retVal;
    }

    /* Read MMD access Data */
    retVal = Peridot_msdReadPagedPhyReg(portNum, 0, 14, &tmpData);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("ERROR to Read MMD access Data.\r\n");
        return retVal;
    }

    *data = tmpData;

    return MSD_OK;
}
/****************************************************************************/
/* SMI PHY Registers indirect R/W functions.                                         */
/****************************************************************************/
MSD_STATUS Peridot_msdSetPagedPhyRegField
(
    IN  MSD_U8    portNum,
    IN  MSD_U8    pageNum,
    IN  MSD_U8    regAddr,
    IN  MSD_U8    fieldOffset,
    IN  MSD_U8    fieldLength,
    IN  MSD_U16   data
)
{
    MSD_STATUS retVal;
    retVal = Peridot_msdSetSMIPhyReg(portNum, 22, pageNum);
    if(retVal != MSD_OK)
        return retVal;
    retVal = Peridot_msdSetSMIPhyRegField(portNum, regAddr, fieldOffset, fieldLength, data);
    return retVal;
}

MSD_STATUS Peridot_msdGetPagedPhyRegField
(
    IN  MSD_U8    portNum,
    IN  MSD_U8    pageNum,
    IN  MSD_U8    regAddr,
    IN  MSD_U8    fieldOffset,
    IN  MSD_U8    fieldLength,
    IN  MSD_U16   *data
)
{
    MSD_STATUS retVal;
    retVal = Peridot_msdSetSMIPhyReg(portNum, 22, pageNum);
    if(retVal != MSD_OK)
        return retVal;
    retVal = Peridot_msdGetSMIPhyRegField(portNum, regAddr, fieldOffset, fieldLength, data);
    return retVal;
}

MSD_STATUS Peridot_msdReadPagedPhyReg
(
    IN  MSD_U8    portNum,
    IN  MSD_U8    pageNum,
    IN  MSD_U8    regAddr,
    OUT MSD_U16   *data
)
{
    MSD_STATUS retVal;
    retVal = Peridot_msdSetSMIPhyReg(portNum, 22, pageNum);
    if(retVal != MSD_OK)
        return retVal;
    retVal = Peridot_msdGetSMIPhyReg(portNum, regAddr, data);
    return retVal;
}

MSD_STATUS Peridot_msdWritePagedPhyReg
(
    IN  MSD_U8    portNum,
    IN  MSD_U8    pageNum,
    IN  MSD_U8    regAddr,
    IN  MSD_U16   data
)
{

    MSD_STATUS retVal;
    retVal = Peridot_msdSetSMIPhyReg(portNum, 22, pageNum);
    if(retVal != MSD_OK)
        return retVal;
    retVal = Peridot_msdSetSMIPhyReg(portNum, regAddr, data);
    return retVal;
}
/********************Below is external PHY register access*******************/

/*****************************************************************************
* msdSetSMIExtPhyReg
*
* DESCRIPTION:
*       This function indirectly write external PHY register through SMI PHY command.
*
* INPUTS:
*       devAddr - The PHY address.
*       regAddr - The register address.
*       data - data to be written
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MSD_OK   - on success
*       MSD_FAIL - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MSD_STATUS Peridot_msdSetSMIExtPhyReg
(
IN  MSD_U8    devAddr,
IN  MSD_U8    regAddr,
IN MSD_U16   data
)
{
    unsigned int timeOut;
    MSD_U16 smiReg;

    MSD_DBG_INFO("(LOG PERIDOT SMI PHY C22 RW): devAddr 0x%02x, regAddr 0x%02x, \r\n",
        devAddr, regAddr);
    MSD_DBG_INFO("\tdata 0x%04x.\r\n", data);

    /* first check that it is not busy */
    timeOut = MSD_SMI_ACCESS_LOOP; /* initialize the loop count */

    do
    {
        if (msdGetAnyReg(PERIDOT_GLOBAL2_DEV_ADDR, PERIDOT_QD_REG_SMI_PHY_CMD, &smiReg) != MSD_OK)
        {
            MSD_DBG_ERROR("\tread PERIDOT_QD_REG_SMI_PHY_CMD register returned: %s.\r\n", msdDisplayStatus(MSD_FAIL));
            return MSD_FAIL;
        }
        if (timeOut-- < 1)
        {
            MSD_DBG("\tread PERIDOT_QD_REG_SMI_PHY_CMD register Timed Out\r\n");
            return MSD_FAIL;
        }
    } while (smiReg & MSD_SMI_BUSY);


    if (msdSetAnyReg(PERIDOT_GLOBAL2_DEV_ADDR, PERIDOT_QD_REG_SMI_PHY_DATA, data) != MSD_OK)
    {
        MSD_DBG_ERROR("\twrite PERIDOT_QD_REG_SMI_PHY_DATA register returned: %s.\r\n", msdDisplayStatus(MSD_FAIL));
        return MSD_FAIL;
    }
    smiReg = MSD_SMI_BUSY | (devAddr << MSD_SMI_DEV_ADDR_BIT) | (MSD_SMI_EXTERNAL << MSD_SMI_FUNC_BIT) | (MSD_SMI_WRITE << MSD_SMI_OP_BIT) |
        (regAddr << MSD_SMI_REG_ADDR_BIT) | (MSD_SMI_CLAUSE22 << MSD_SMI_MODE_BIT);

    if (msdSetAnyReg(PERIDOT_GLOBAL2_DEV_ADDR, PERIDOT_QD_REG_SMI_PHY_CMD, smiReg) != MSD_OK)
    {
        MSD_DBG_ERROR("\twrite PERIDOT_QD_REG_SMI_PHY_CMD register returned: %s.\r\n", msdDisplayStatus(MSD_FAIL));
        return MSD_FAIL;
    }

    return MSD_OK;
}

/*****************************************************************************
* msdGetSMIExtPhyReg
*
* DESCRIPTION:
*       This function indirectly read external PHY register through SMI PHY command.
*
* INPUTS:
*       devAddr  - The PHY address to be read.
*       regAddr  - The register address to read.
*
* OUTPUTS:
*       data     - The storage where register date to be saved.
*
* RETURNS:
*       MSD_OK   - on success
*       MSD_FAIL - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MSD_STATUS Peridot_msdGetSMIExtPhyReg
(
IN  MSD_U8    devAddr,
IN  MSD_U8    regAddr,
OUT  MSD_U16   *data
)
{
    unsigned int timeOut;
    MSD_U16 smiReg;

	
		MSD_DBG_INFO("(LOG PERIDOT SMI PHY C22 RW): devAddr 0x%02x, regAddr 0x%02x, \r\n",
        devAddr, regAddr);
    /* first check that it is not busy */
    timeOut = MSD_SMI_ACCESS_LOOP; /* initialize the loop count */

    do
    {
        if (msdGetAnyReg(PERIDOT_GLOBAL2_DEV_ADDR, PERIDOT_QD_REG_SMI_PHY_CMD, &smiReg) != MSD_OK)
        {
            MSD_DBG_ERROR("\tread PERIDOT_QD_REG_SMI_PHY_CMD register returned: %s.\r\n", msdDisplayStatus(MSD_FAIL));
            return MSD_FAIL;
        }
        if (timeOut-- < 1)
        {
            MSD_DBG_ERROR("\tRead PERIDOT_QD_REG_SMI_PHY_CMD Timed Out\r\n");
            return MSD_FAIL;
        }
    } while (smiReg & MSD_SMI_BUSY);

    smiReg = MSD_SMI_BUSY | (devAddr << MSD_SMI_DEV_ADDR_BIT) | (MSD_SMI_EXTERNAL << MSD_SMI_FUNC_BIT) | (MSD_SMI_READ_22 << MSD_SMI_OP_BIT) |
        (regAddr << MSD_SMI_REG_ADDR_BIT) | (MSD_SMI_CLAUSE22 << MSD_SMI_MODE_BIT);

    if (msdSetAnyReg(PERIDOT_GLOBAL2_DEV_ADDR, PERIDOT_QD_REG_SMI_PHY_CMD, smiReg) != MSD_OK)
    {
        MSD_DBG_ERROR("\twrite PERIDOT_QD_REG_SMI_PHY_CMD register returned: %s.\r\n", msdDisplayStatus(MSD_FAIL));
        return MSD_FAIL;
    }

    timeOut = MSD_SMI_ACCESS_LOOP; /* initialize the loop count */

    do
    {
        if (msdGetAnyReg(PERIDOT_GLOBAL2_DEV_ADDR, PERIDOT_QD_REG_SMI_PHY_CMD, &smiReg) != MSD_OK)
        {
            MSD_DBG_ERROR("\tread PERIDOT_QD_REG_SMI_PHY_CMD register returned: %s.\r\n", msdDisplayStatus(MSD_FAIL));
            return MSD_FAIL;
        }
        if (timeOut-- < 1)
        {
            MSD_DBG_ERROR("\tread TOPAZ_QD_REG_SMI_PHY_DATA register returned: %s.\r\n", msdDisplayStatus(MSD_FAIL));
            return MSD_FAIL;
        }
    } while (smiReg & MSD_SMI_BUSY);

    if (msdGetAnyReg(PERIDOT_GLOBAL2_DEV_ADDR, PERIDOT_QD_REG_SMI_PHY_DATA, &smiReg) != MSD_OK)
    {
        MSD_DBG("\tReading Phy register Failed\r\n");
        return MSD_FAIL;
    }
    *data = smiReg;

    MSD_DBG_INFO("data 0x%04x.\r\n", *data);

    return MSD_OK;
}

MSD_STATUS Peridot_msdWritePagedExtPhyReg
(
IN  MSD_U8    portNum,
IN  MSD_U8    pageNum,
IN  MSD_U8    regAddr,
IN  MSD_U16   data
)
{

    MSD_STATUS retVal;
    retVal = Peridot_msdSetSMIExtPhyReg(portNum, 22, pageNum);
    if (retVal != MSD_OK)
        return retVal;
    retVal = Peridot_msdSetSMIExtPhyReg(portNum, regAddr, data);
    return retVal;
}

MSD_STATUS Peridot_msdReadPagedExtPhyReg
(
IN  MSD_U8    portNum,
IN  MSD_U8    pageNum,
IN  MSD_U8    regAddr,
OUT MSD_U16   *data
)
{
    MSD_STATUS retVal;
    retVal = Peridot_msdSetSMIExtPhyReg(portNum, 22, pageNum);
    if (retVal != MSD_OK)
        return retVal;
    retVal = Peridot_msdGetSMIExtPhyReg(portNum, regAddr, data);
    return retVal;
}

/*****************************************************************************
* msdSetSMIExtPhyXMDIOReg
*
* DESCRIPTION:
*       This function indirectly write external PHY XMDIO register through SMI PHY command.
*
* INPUTS:
*       portNum  - The PHY address to be write.
*       devAddr  - The PHY address to be write.
*       regAddr  - The register address to write.
*       data     - The storage where register data to be saved.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MSD_OK   - on success
*       MSD_FAIL - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MSD_STATUS Peridot_msdSetSMIExtPhyXMDIOReg
(
IN MSD_U8  portNum,
IN MSD_U8  devAddr,
IN MSD_U16  regAddr,
IN MSD_U16 data
)
{
    MSD_STATUS retVal;
    MSD_U16 tmpData;

    /* Set MMD access control address is Address | devAddr */
    tmpData = MSD_PHY_MMD_ADDR | devAddr;
    retVal = Peridot_msdWritePagedExtPhyReg(portNum, 0, 13, tmpData);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("ERROR to Set MMD access control address is Address | devAddr.\r\n");
        return retVal;
    }

    /* Set MMD access Data is regAddr */
    tmpData = regAddr;
    retVal = Peridot_msdWritePagedExtPhyReg(portNum, 0, 14, tmpData);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("ERROR to Set MMD access Data is regAddr.\r\n");
        return retVal;
    }

    /* Set MMD access control address is Data no inc | devAddr */
    tmpData = MSD_PHY_MMD_DATA_NO_INC | devAddr;
    retVal = Peridot_msdWritePagedExtPhyReg(portNum, 0, 13, tmpData);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("ERROR to Set MMD access control address is Data no inc | devAddr.\r\n");
        return retVal;
    }

    /* Write MMD access Data */
    retVal = Peridot_msdWritePagedExtPhyReg(portNum, 0, 14, data);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("ERROR to Write MMD access Data.\r\n");
        return retVal;
    }

    return MSD_OK;
}

/*****************************************************************************
* msdGetSMIExtPhyXMDIOReg
*
* DESCRIPTION:
*       This function indirectly read external PHY XMDIO register through SMI PHY command.
*
* INPUTS:
*       portNum  - The PHY address to be read.
*       devAddr  - The device address to be read.
*       regAddr  - The register address to read.
*
* OUTPUTS:
*       data     - The storage where register data to be saved.
*
* RETURNS:
*       MSD_OK   - on success
*       MSD_FAIL - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MSD_STATUS Peridot_msdGetSMIExtPhyXMDIOReg
(
IN MSD_U8  portNum,
IN MSD_U8  devAddr,
IN MSD_U16  regAddr,
OUT MSD_U16 *data
)
{
    MSD_STATUS retVal;
    MSD_U16 tmpData;

    /* Set MMD access control address is Address | devAddr */
    tmpData = MSD_PHY_MMD_ADDR | devAddr;
    retVal = Peridot_msdWritePagedExtPhyReg(portNum, 0, 13, tmpData);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("ERROR to Set MMD access control address is Address | devAddr.\r\n");
        return retVal;
    }

    /* Set MMD access Data is regAddr */
    tmpData = regAddr;
    retVal = Peridot_msdWritePagedExtPhyReg(portNum, 0, 14, tmpData);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("ERROR to Set MMD access Data is regAddr.\r\n");
        return retVal;
    }

    /* Set MMD access control address is Data no inc | devAddr */
    tmpData = MSD_PHY_MMD_DATA_NO_INC | devAddr;
    retVal = Peridot_msdWritePagedExtPhyReg(portNum, 0, 13, tmpData);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("ERROR to Set MMD access control address is Data no inc | devAddr.\r\n");
        return retVal;
    }

    /* Read MMD access Data */
    retVal = Peridot_msdReadPagedExtPhyReg(portNum, 0, 14, &tmpData);
    if (retVal != MSD_OK)
    {
        MSD_DBG_ERROR("ERROR to Read MMD access Data.\r\n");
        return retVal;
    }

    *data = tmpData;

    return MSD_OK;
}
