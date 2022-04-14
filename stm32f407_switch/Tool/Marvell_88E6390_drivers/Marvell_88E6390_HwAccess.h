/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-03-31     Lenovo       the first version
 */
#ifndef APPLICATIONS_MARVELL_88E6390_HWACCESS_H_
#define APPLICATIONS_MARVELL_88E6390_HWACCESS_H_

#include "Marvell_88E6390_type.h"
#include "Marvell_88E6390_regs.h"
#include "mdio.h"
#include "delay.h"
/* This macro calculates the mask for partial read /    */
/* write of register's data.                            */
#define MSD_CALC_MASK(fieldOffset,fieldLen,mask)        \
            if((fieldLen + fieldOffset) >= 16)      \
                mask = (0 - (1 << fieldOffset));    \
            else                                    \
                mask = (((1 << (fieldLen + fieldOffset))) - (1 << fieldOffset));


#define MSD_SMI_ACCESS_LOOP        1000
#define MSD_SMI_TIMEOUT            2
/***********************************************************************
*  Internal Phy Clause 45 Register access *
***********************************************************************/
#define   MSD_PHY_MMD_ADDR          0x0000
#define   MSD_PHY_MMD_DATA_NO_INC   0x4000
#define   MSD_PHY_MMD_DATA_RW_INC   0x8000
#define   MSD_PHY_MMD_DATA_WR_INC   0xc000
/* Bit definition for MSD_REG_SMI_COMMAND */
#define MSD_SMI_BUSY                0x8000
#define MSD_SMI_MODE                0x1000
#define MSD_SMI_MODE_BIT            12
#define MSD_SMI_FUNC_BIT            13
#define MSD_SMI_FUNC_SIZE            2
#define MSD_SMI_OP_BIT              10
#define MSD_SMI_OP_SIZE              2
#define MSD_SMI_DEV_ADDR_BIT         5
#define MSD_SMI_DEV_ADDR_SIZE        5
#define MSD_SMI_REG_ADDR_BIT         0
#define MSD_SMI_REG_ADDR_SIZE        5

#define MSD_SMI_CLAUSE45            0
#define MSD_SMI_CLAUSE22            1

#define MSD_SMI_WRITE_ADDR          0x00
#define MSD_SMI_WRITE               0x01
#define MSD_SMI_READ_22             0x02
#define MSD_SMI_READ_45             0x03
#define MSD_SMI_READ_INC            0x02

#define MSD_SMI_INTERNAL            0x0
#define MSD_SMI_EXTERNAL            0x1
#define MSD_SMI_SMISETUP            0x2

extern void marvell_delay_ms(MSD_U16 t_ms);
extern MSD_STATUS msdSetAnyReg
(
    IN  MSD_U8    devAddr,
    IN  MSD_U8    regAddr,
    IN  MSD_U16   data
);
extern MSD_STATUS msdGetAnyReg
(
    IN  MSD_U8    devAddr,
    IN  MSD_U8    regAddr,
    OUT MSD_U16   *data
);
extern MSD_STATUS msdSetAnyRegField
(
    IN  MSD_U8    devAddr,
    IN  MSD_U8    regAddr,
    IN  MSD_U8    fieldOffset,
    IN  MSD_U8    fieldLength,
    IN  MSD_U16   data
);
extern MSD_STATUS msdGetAnyRegField
(
    IN  MSD_U8    devAddr,
    IN  MSD_U8    regAddr,
    IN  MSD_U8    fieldOffset,
    IN  MSD_U8    fieldLength,
    OUT MSD_U16   *data
);
extern MSD_STATUS Peridot_msdReadPagedPhyReg
(
    IN  MSD_U8    portNum,
    IN  MSD_U8    pageNum,
    IN  MSD_U8    regAddr,
    OUT MSD_U16   *data
);
extern MSD_STATUS Peridot_msdWritePagedPhyReg
(
    IN  MSD_U8    portNum,
    IN  MSD_U8    pageNum,
    IN  MSD_U8    regAddr,
    IN  MSD_U16   data
);
extern MSD_STATUS Peridot_msdPhyReset
(
    IN  MSD_U8        portNum,
    IN  MSD_U8       pageNum,
    IN  MSD_U16       u16Data
);
extern MSD_STATUS Peridot_msdSetSMIPhyRegField
(
    IN  MSD_U8    devAddr,
    IN  MSD_U8    regAddr,
    IN  MSD_U8    fieldOffset,
    IN  MSD_U8    fieldLength,
    IN MSD_U16    data
);
extern MSD_STATUS Peridot_msdGetSMIPhyRegField
(
    IN  MSD_U8    devAddr,
    IN  MSD_U8    regAddr,
    IN  MSD_U8    fieldOffset,
    IN  MSD_U8    fieldLength,
    OUT  MSD_U16   *data
);
extern MSD_STATUS Peridot_msdGetSMIC45PhyReg
(
    IN  MSD_U8     devAddr,
    IN  MSD_U8     phyAddr,
    IN  MSD_U16    regAddr,
    OUT MSD_U16    *data
);
extern MSD_STATUS Peridot_msdSetSMIC45PhyReg
(
    IN MSD_U8    devAddr,
    IN MSD_U8    phyAddr,
    IN MSD_U16   regAddr,
    IN MSD_U16   data
);
extern MSD_STATUS Peridot_msdSetPagedPhyRegField
(
    IN  MSD_U8    portNum,
    IN  MSD_U8    pageNum,
    IN  MSD_U8    regAddr,
    IN  MSD_U8    fieldOffset,
    IN  MSD_U8    fieldLength,
    IN  MSD_U16   data
);
extern MSD_STATUS Peridot_msdGetPagedPhyRegField
(
    IN  MSD_U8    portNum,
    IN  MSD_U8    pageNum,
    IN  MSD_U8    regAddr,
    IN  MSD_U8    fieldOffset,
    IN  MSD_U8    fieldLength,
    IN  MSD_U16   *data
);
extern MSD_STATUS Peridot_msdGetSMIPhyXMDIOReg
(
    IN MSD_U8  portNum,
    IN MSD_U8  devAddr,
    IN MSD_U16  regAddr,
    OUT MSD_U16 *data
);
extern MSD_STATUS Peridot_msdSetSMIPhyXMDIOReg
(
    IN MSD_U8  portNum,
    IN MSD_U8  devAddr,
    IN MSD_U16  regAddr,
    IN MSD_U16 data
);
extern MSD_STATUS Peridot_msdSetSMIExtPhyReg
(
IN  MSD_U8    devAddr,
IN  MSD_U8    regAddr,
IN MSD_U16   data
);
extern MSD_STATUS Peridot_msdGetSMIExtPhyReg
(
IN  MSD_U8    devAddr,
IN  MSD_U8    regAddr,
OUT  MSD_U16   *data
);
extern MSD_STATUS Peridot_msdWritePagedExtPhyReg
(
IN  MSD_U8    portNum,
IN  MSD_U8    pageNum,
IN  MSD_U8    regAddr,
IN  MSD_U16   data
);
extern MSD_STATUS Peridot_msdReadPagedExtPhyReg
(
IN  MSD_U8    portNum,
IN  MSD_U8    pageNum,
IN  MSD_U8    regAddr,
OUT MSD_U16   *data
);
extern MSD_STATUS Peridot_msdSetSMIExtPhyXMDIOReg
(
IN MSD_U8  portNum,
IN MSD_U8  devAddr,
IN MSD_U16  regAddr,
IN MSD_U16 data
);
extern MSD_STATUS Peridot_msdGetSMIExtPhyXMDIOReg
(
IN MSD_U8  portNum,
IN MSD_U8  devAddr,
IN MSD_U16  regAddr,
OUT MSD_U16 *data
);
extern MSD_STATUS Peridot_msdSetSMIPhyReg
(
    IN  MSD_U8    devAddr,
    IN  MSD_U8    regAddr,
    IN MSD_U16   data
);
extern MSD_STATUS Peridot_msdGetSMIPhyReg
(
    IN  MSD_U8    devAddr,
    IN  MSD_U8    regAddr,
    OUT  MSD_U16   *data
);
#endif /* APPLICATIONS_MARVELL_88E6390_HWACCESS_H_ */
