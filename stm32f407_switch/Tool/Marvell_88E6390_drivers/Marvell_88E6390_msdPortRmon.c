/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-08-16     Lenovo       the first version
 */
/*******************************************************************************
* Peridot_msdPortRmon.c
*
* DESCRIPTION:
*       API definitions for RMON counters
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*******************************************************************************/

#include "Marvell_88E6390_msdPortRmon.h"

/* Definitions for MIB Counter */
/*
#define PERIDOT_MSD_STATS_NO_OP               0x0
#define PERIDOT_MSD_STATS_FLUSH_ALL           0x1
#define PERIDOT_MSD_STATS_FLUSH_PORT          0x2
#define PERIDOT_MSD_STATS_READ_COUNTER        0x4
#define PERIDOT_MSD_STATS_CAPTURE_PORT        0x5
*/

/*
 *    Type definition for MIB counter operation
*/
typedef enum 
{
	Peridot_STATS_FLUSH_ALL = 1,            /* Flush all counters for all ports */
	Peridot_STATS_FLUSH_PORT = 2,           /* Flush all counters for a port */
	Peridot_STATS_READ_COUNTER = 4,         /* Read a specific counter from a port */
	Peridot_STATS_READ_ALL = 5,              /* Read all counters from a port */
} PERIDOT_MSD_STATS_OPERATION;

/****************************************************************************/
/* STATS operation function declaration.                                    */
/****************************************************************************/
static MSD_STATUS Peridot_statsOperationPerform
(
    IN   PERIDOT_MSD_STATS_OPERATION   statsOp,
    IN   MSD_U8                port,
	IN   PERIDOT_MSD_STATS_COUNTERS    counter,
    OUT  MSD_VOID              *statsData
);

static MSD_STATUS Peridot_statsReadCounter
(
    IN   MSD_U16         port,
    IN   MSD_U32         counter,
    OUT  MSD_U32         *statsData
);

/*******************************************************************************
* Peridot_gstatsFlushAll
*
* DESCRIPTION:
*       Flush All RMON counters for all ports.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MSD_OK   - on success
*       MSD_FAIL - on error
*
* COMMENTS:
*       None
*
*******************************************************************************/
MSD_STATUS Peridot_gstatsFlushAll(void)
{
    MSD_STATUS           retVal;

    MSD_DBG_INFO("Peridot_gstatsFlushAll Called.\n");

    retVal = Peridot_statsOperationPerform(Peridot_STATS_FLUSH_ALL,0,Peridot_STATS_InGoodOctetsLo,NULL);
    if(retVal != MSD_OK)
    {
		MSD_DBG_ERROR("Peridot_statsOperationPerform FLUSH_ALL returned: %s.\n", msdDisplayStatus(retVal));
        return retVal;
    }

    MSD_DBG_INFO("Peridot_gstatsFlushAll Exit.\n");
	
    return MSD_OK;

}


/*******************************************************************************
* Peridot_gstatsFlushPort
*
* DESCRIPTION:
*       Flush All RMON counters for a given port.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MSD_OK - on success,
*       MSD_FAIL - on error.
*       MSD_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None
*
*******************************************************************************/
MSD_STATUS Peridot_gstatsFlushPort
(

    IN MSD_LPORT      port
)
{
    MSD_STATUS    retVal;
    MSD_U8        hwPort;         /* physical port number         */

    MSD_DBG_INFO("Peridot_gstatsFlushPort Called.\n");

    /* translate logical port to physical port */
    hwPort = port;
	if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
		MSD_DBG_ERROR("Bad Port: %u.\n", (unsigned int)port);
        return MSD_BAD_PARAM;
    }

    retVal = Peridot_statsOperationPerform(Peridot_STATS_FLUSH_PORT,hwPort,Peridot_STATS_InGoodOctetsLo,NULL);
    if(retVal != MSD_OK)
    {
		MSD_DBG_ERROR("Peridot_statsOperationPerform FLUSH_PORT returned: %s.\n", msdDisplayStatus(retVal));
        return retVal;
    }

    MSD_DBG_INFO("Peridot_gstatsFlushPort Exit.\n");
    return MSD_OK;

}

/*******************************************************************************
* Peridot_gstatsGetPortCounter
*
* DESCRIPTION:
*        This routine Peridot_gets a specific counter of the given port
*
* INPUTS:
*        port - the logical port number.
*        counter - the counter which will be read
*
* OUTPUTS:
*        statsData - points to 32bit data storage for the MIB counter
*
* RETURNS:
*       MSD_OK - on success,
*       MSD_FAIL - on error.
*       MSD_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*        None
*
*******************************************************************************/
MSD_STATUS Peridot_gstatsGetPortCounter
(
    IN  MSD_LPORT        port,
    IN  PERIDOT_MSD_STATS_COUNTERS    counter,
    OUT MSD_U32            *statsData
)
{
    MSD_STATUS    retVal;
    MSD_U8        hwPort;         /* physical port number         */

    MSD_DBG_INFO("Peridot_gstatsGetPortCounter Called.\n");

    /* translate logical port to physical port */
    hwPort = port;
	if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
		MSD_DBG_ERROR("Bad Port: %u.\n", (unsigned int)port);
        return MSD_BAD_PARAM;
    }

    if (counter > PERIDOT_MSD_TYPE_BANK + 0x1F || (counter > 0x1f && counter < PERIDOT_MSD_TYPE_BANK))
    {
        MSD_DBG_ERROR("Bad counter: %d.\n", (int)counter);
        return MSD_BAD_PARAM;
    }

	retVal = Peridot_statsOperationPerform(Peridot_STATS_READ_COUNTER, hwPort, counter, (MSD_VOID*)statsData);
    if(retVal != MSD_OK)
    {
		MSD_DBG_ERROR("Peridot_statsOperationPerform READ_COUNTER returned: %s.\n", msdDisplayStatus(retVal));
        return retVal;
    }

    MSD_DBG_INFO("Peridot_gstatsGetPortCounter Exit.\n");
    return MSD_OK;

}


/*******************************************************************************
* Peridot_gstatsGetPortAllCounters
*
* DESCRIPTION:
*       This routine Peridot_gets all RMON counters of the given port
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       statsCounterSet - points to PERIDOT_MSD_STATS_COUNTER_SET for the MIB counters
*
* RETURNS:
*       MSD_OK - on success,
*       MSD_FAIL - on error.
*       MSD_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None
*
*******************************************************************************/
MSD_STATUS Peridot_gstatsGetPortAllCounters
(
    IN  MSD_LPORT        port,
    OUT PERIDOT_MSD_STATS_COUNTER_SET    *statsCounterSet
)
{
    MSD_STATUS    retVal;
    MSD_U8        hwPort;         /* physical port number         */

    MSD_DBG_INFO("Peridot_gstatsGetPortAllCounters Called.\n");

    /* translate logical port to physical port */
    hwPort = port;
	if (hwPort >= MSD_MAX_SWITCH_PORTS)
    {
		MSD_DBG_ERROR("Bad Port: %u.\n", (unsigned int)port);
        return MSD_BAD_PARAM;
    }

    retVal = Peridot_statsOperationPerform(Peridot_STATS_READ_ALL,hwPort,Peridot_STATS_InGoodOctetsLo,(MSD_VOID*)statsCounterSet);
    if(retVal != MSD_OK)
    {
		MSD_DBG_ERROR("Peridot_statsOperationPerform READ_ALL returned: %s.\n", msdDisplayStatus(retVal));
        return retVal;
    }

    MSD_DBG_INFO("Peridot_gstatsGetPortAllCounters Exit.\n");
    return MSD_OK;

}

/*******************************************************************************
* Peridot_gstatsGetHistogramMode
*
* DESCRIPTION:
*        This routine Peridot_gets the Histogram Counters Mode.
*
* INPUTS:
*        None.
*
* OUTPUTS:
*        mode - Histogram Mode (PERIDOT_MSD_COUNT_RX_ONLY, PERIDOT_MSD_COUNT_TX_ONLY, 
*                    and PERIDOT_MSD_COUNT_RX_TX)
*
* RETURNS:
*       MSD_OK - on success,
*       MSD_FAIL - on error.
*       MSD_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None
*
*******************************************************************************/
MSD_STATUS Peridot_gstatsGetHistogramMode
(
    OUT PERIDOT_MSD_HISTOGRAM_MODE    *mode
)
{
    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U16          data;           /* The register's read data.    */

    MSD_DBG_INFO("Peridot_gstatsGetHistogramMode Called.\n");
	
    if(mode == NULL)
    {
		MSD_DBG_ERROR("Mode is NULL.\n");
        return MSD_BAD_PARAM;
    }

    /* Get the Histogram mode bit.                */
	retVal = msdGetAnyRegField(PERIDOT_GLOBAL1_DEV_ADDR,PERIDOT_QD_REG_GLOBAL_CONTROL2,6,2,&data);
    if(retVal != MSD_OK)
    {
		MSD_DBG_ERROR("Read histogram mode bit returned: %s.\n", msdDisplayStatus(retVal));
        return retVal;
    }

    /* Software definition starts from 0 ~ 3, while hardware supports the values from 1 to 3 */
	switch(data)
	{
	    case 0x1:
			*mode = PERIDOT_MSD_COUNT_RX_ONLY;
			break;
		case 0x2:
			*mode = PERIDOT_MSD_COUNT_TX_ONLY;
			break;
		case 0x3:
			*mode = PERIDOT_MSD_COUNT_RX_TX;
			break;
        default:
			MSD_DBG_ERROR("Read back Bad Mode: %u.\n", (unsigned int)data);
            return MSD_BAD_PARAM;
	}

    MSD_DBG_INFO("Peridot_gstatsGetHistogramMode Exit.\n");
    return MSD_OK;
}

/*******************************************************************************
* Peridot_gstatsSetHistogramMode
*
* DESCRIPTION:
*        This routine sets the Histogram Counters Mode.
*
* INPUTS:
*        mode - Histogram Mode (PERIDOT_MSD_COUNT_RX_ONLY, PERIDOT_MSD_COUNT_TX_ONLY, 
*                    and PERIDOT_MSD_COUNT_RX_TX)
*
* OUTPUTS:
*        None.
*
* RETURNS:
*       MSD_OK - on success,
*       MSD_FAIL - on error.
*       MSD_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*        None.
*
*******************************************************************************/
MSD_STATUS Peridot_gstatsSetHistogramMode
(
    IN PERIDOT_MSD_HISTOGRAM_MODE        mode
)
{
    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U16          data;           /* The register's read data.    */

    MSD_DBG_INFO("Peridot_gstatsSetHistogramMode Called.\n");

    switch (mode)
    {
        case PERIDOT_MSD_COUNT_RX_ONLY:
			data = 1;
			break;
        case PERIDOT_MSD_COUNT_TX_ONLY:
			data = 2;
			break;
        case PERIDOT_MSD_COUNT_RX_TX:
			data = 3;
            break;
        default:
			MSD_DBG_ERROR("Bad Mode: %u.\n", (unsigned int)mode);
            return MSD_BAD_PARAM;
    }

    /*data = (MSD_U16)mode;*/

    /* Set the Histogram mode bit.                */
	retVal = msdSetAnyRegField(PERIDOT_GLOBAL1_DEV_ADDR,PERIDOT_QD_REG_GLOBAL_CONTROL2,6,2,data);
    if(retVal != MSD_OK)
    {
		MSD_DBG_ERROR("Write histogram mode bit returned: %s.\n", msdDisplayStatus(retVal));
        return retVal;
    }

    MSD_DBG_INFO("Peridot_gstatsSetHistogramMode Exit.\n");
    return MSD_OK;
}
/****************************************************************************/
/* Internal use functions.                                                  */
/****************************************************************************/
static MSD_STATUS Peridot_statsReadCounter
(
    IN   MSD_U16         port,
    IN   MSD_U32         counter,
    OUT  MSD_U32         *statsData
);
/*******************************************************************************
* Peridot_statsOperationPerform
*
* DESCRIPTION:
*       This function is used by all stats control functions, and is responsible
*       to write the required operation into the stats registers.
*
* INPUTS:
*       statsOp       - The stats operation bits to be written into the stats
*                     operation register.
*       port        - port number
*       counter     - counter to be read if it's read operation
*
* OUTPUTS:
*       statsData   - points to the data storage where the MIB counter will be saved.
*
* RETURNS:
*       MSD_OK on success,
*       MSD_FAIL otherwise.
*
* COMMENTS:
*		None.
*
*******************************************************************************/

static MSD_STATUS Peridot_statsOperationPerform
(
    IN   PERIDOT_MSD_STATS_OPERATION   statsOp,
    IN   MSD_U8                port,
    IN   PERIDOT_MSD_STATS_COUNTERS    counter,
    OUT  MSD_VOID              *statsData
)
{
    MSD_STATUS       retVal;         /* Functions return value.      */
    MSD_U16          data;			/* Data to be set into the      */
    MSD_U32	tmpCounter;
    MSD_U32	startCounter;
    MSD_U32	lastCounter;
    MSD_U16	portNum;
	MSD_U8 	bank;
	MSD_U16 count;
	
	portNum = (port + 1) << 5;

    /* Wait until the stats in ready. */
    data = 1;
	count = 0;
    while(data == 1)
    {
        retVal = msdGetAnyRegField(PERIDOT_GLOBAL1_DEV_ADDR,PERIDOT_QD_REG_STATS_OPERATION,15,1,&data);
        if(retVal != MSD_OK)
        {
            return retVal;
        }
		if(count++ > 1000)
		{
			msdSetAnyRegField(PERIDOT_GLOBAL1_DEV_ADDR,PERIDOT_QD_REG_STATS_OPERATION,15,1,0);
			//return MSD_FAIL;
		}
		marvell_delay_ms(2);
    }

	retVal = msdGetAnyReg(PERIDOT_GLOBAL1_DEV_ADDR,PERIDOT_QD_REG_STATS_OPERATION,&data);
	if(retVal != MSD_OK)
	{
		return retVal;
	}
	data &= 0xfff;
    /* Set the STAT Operation register */
    switch (statsOp)
    {
        case Peridot_STATS_FLUSH_ALL:
            data |= (1 << 15) | (Peridot_STATS_FLUSH_ALL << 12);
            retVal = msdSetAnyReg(PERIDOT_GLOBAL1_DEV_ADDR,PERIDOT_QD_REG_STATS_OPERATION,data);

            return retVal;

        case Peridot_STATS_FLUSH_PORT:
			data &= 0xc1f;
            data |= (1 << 15) | (Peridot_STATS_FLUSH_PORT << 12) | portNum ;
            retVal = msdSetAnyReg(PERIDOT_GLOBAL1_DEV_ADDR,PERIDOT_QD_REG_STATS_OPERATION,data);
            return retVal;

		case Peridot_STATS_READ_COUNTER:
            retVal = Peridot_statsReadCounter(portNum, (MSD_U32)counter, (MSD_U32*)statsData);
            if(retVal != MSD_OK)
            {
                return retVal;
            }
            break;

        case Peridot_STATS_READ_ALL:
			{
				int bankSize = 2;
				for(bank=0; bank<bankSize; bank++)
				{
					lastCounter = (bank == 0) ? (MSD_U32)Peridot_STATS_Late : (MSD_U32)Peridot_STATS_OutMGMT;
					startCounter = (bank==0)?(MSD_U32)Peridot_STATS_InGoodOctetsLo : (MSD_U32)Peridot_STATS_InDiscards;

					if(bank==1)
						statsData = (MSD_U32 *)statsData + Peridot_STATS_Late +1;
					for(tmpCounter=startCounter; tmpCounter<=lastCounter; tmpCounter++)
					{
						retVal = Peridot_statsReadCounter(portNum, tmpCounter,((MSD_U32*)statsData+tmpCounter-startCounter));
						if(retVal != MSD_OK)
						{
							return retVal;
						}
					}
				}
			}
			break;
			
        default:
            return MSD_FAIL;
    }

    return MSD_OK;
}

/*******************************************************************************
* Peridot_statsReadCounter
*
* DESCRIPTION:
*       This function is used to read a captured counter.
*
* INPUTS:
*       port        - port number
*       counter     - counter to be read if it's read operation
*
* OUTPUTS:
*       statsData   - points to the data storage where the MIB counter will be saved.
*
* RETURNS:
*       MSD_OK - on success,
*       MSD_FAIL - on error.
*
* COMMENTS:
*        If Semaphore is used, Semaphore should be acquired before this function call.
*******************************************************************************/
static MSD_STATUS Peridot_statsReadCounter
(
    IN   MSD_U16         port,
    IN   MSD_U32         counter,
    OUT  MSD_U32         *statsData
)
{
    MSD_STATUS   retVal;         /* Functions return value.            */
    MSD_U16      data;/* Data to be set into the  register. */ 
    MSD_U16    counter3_2;     /* Counter Register Bytes 3 & 2       */
    MSD_U16    counter1_0;     /* Counter Register Bytes 1 & 0       */
	MSD_U16 count;
	
    data = 1;
	count = 0;
    while(data == 1)
    {
        retVal = msdGetAnyRegField(PERIDOT_GLOBAL1_DEV_ADDR,PERIDOT_QD_REG_STATS_OPERATION,15,1,&data);
        if(retVal != MSD_OK)
        {
            return retVal;
        }
		if(count++ > 1000)
		{
			msdSetAnyRegField(PERIDOT_GLOBAL1_DEV_ADDR,PERIDOT_QD_REG_STATS_OPERATION,15,1,0);
			return MSD_FAIL;
		}
		marvell_delay_ms(2);
	}
	
    data = (MSD_U16)((1 << 15) | (Peridot_STATS_READ_COUNTER << 12) | port | (counter&0x1f) );
	if (counter & PERIDOT_MSD_TYPE_BANK)
	{
		data |= (1<<10);
	}

    retVal = msdSetAnyReg(PERIDOT_GLOBAL1_DEV_ADDR,PERIDOT_QD_REG_STATS_OPERATION,data);
    if(retVal != MSD_OK)
    {
        return retVal;
    }

    data = 1;
	count = 0;
    while(data == 1)
    {
        retVal = msdGetAnyRegField(PERIDOT_GLOBAL1_DEV_ADDR,PERIDOT_QD_REG_STATS_OPERATION,15,1,&data);
        if(retVal != MSD_OK)
        {
            return retVal;
        }
		if(count++ > 1000)
		{
			msdSetAnyRegField(PERIDOT_GLOBAL1_DEV_ADDR,PERIDOT_QD_REG_STATS_OPERATION,15,1,0);
			return MSD_FAIL;
		}
		marvell_delay_ms(2);
	}

	retVal = msdGetAnyReg(PERIDOT_GLOBAL1_DEV_ADDR, PERIDOT_QD_REG_STATS_COUNTER3_2, &counter3_2);
    if(retVal != MSD_OK)
    {
        return retVal;
    }

	retVal = msdGetAnyReg(PERIDOT_GLOBAL1_DEV_ADDR, PERIDOT_QD_REG_STATS_COUNTER1_0, &counter1_0);
    if(retVal != MSD_OK)
    {
        return retVal;
    }

    *statsData = (counter3_2 << 16) | counter1_0;

    return MSD_OK;

}
