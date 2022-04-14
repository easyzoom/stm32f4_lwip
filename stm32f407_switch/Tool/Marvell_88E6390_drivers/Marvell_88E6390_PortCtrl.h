/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-03-30     Lenovo       the first version
 */
#ifndef APPLICATIONS_MARVELL_88E6390_PORTCTRL_H_
#define APPLICATIONS_MARVELL_88E6390_PORTCTRL_H_

#include "Marvell_88E6390_type.h"
#include "Marvell_88E6390_HwAccess.h"
#include "Marvell_88E6390_regs.h"
#include "kubot_debug.h"
/****************************************************************************/
/* Exported Port Control Types                                                         */
/****************************************************************************/
/*
 *  typedef: enum PERIDOT_MSD_ELIMIT_MODE
 *
 *  Description: Enumeration of the port egress rate limit counting mode.
 *
 *  Enumerations:
 *      PERIDOT_MSD_ELIMIT_FRAME -
 *                Count the number of frames
 *      PERIDOT_MSD_ELIMIT_LAYER1 -
 *                Count all Layer 1 bytes: 
 *                Preamble (8bytes) + Frame's DA to CRC + IFG (12bytes)
 *      PERIDOT_MSD_ELIMIT_LAYER2 -
 *                Count all Layer 2 bytes: Frame's DA to CRC
 *      PERIDOT_MSD_ELIMIT_LAYER3 -
 *                Count all Layer 3 bytes: 
 *                Frame's DA to CRC - 18 - 4 (if frame is tagged)
 */
typedef enum
{
    PERIDOT_MSD_ELIMIT_FRAME = 0,
    PERIDOT_MSD_ELIMIT_LAYER1,
    PERIDOT_MSD_ELIMIT_LAYER2,
    PERIDOT_MSD_ELIMIT_LAYER3
} PERIDOT_MSD_ELIMIT_MODE;
/*
 * Typedef: enum PERIDOT_MSD_MTU_SIZE
 *
 * Description: Defines Jumbo Frame Size allowed to be tx and rx
 *
 * Fields:
 *      PERIDOT_MSD_MTU_SIZE_1522 - Rx and Tx frames with max byte of 1522.
 *      PERIDOT_MSD_MTU_SIZE_2048 - Rx and Tx frames with max byte of 2048.
 *      PERIDOT_MSD_MTU_SIZE_10240 - Rx and Tx frames with max byte of 10240.
 *
 */
typedef enum
{
    PERIDOT_MSD_MTU_SIZE_1522 = 0,
    PERIDOT_MSD_MTU_SIZE_2048,
    PERIDOT_MSD_MTU_SIZE_10240
} PERIDOT_MSD_MTU_SIZE;
/* Definition for the Port Duplex Mode */
typedef enum
{
    PERIDOT_MSD_DUPLEX_HALF,
    PERIDOT_MSD_DUPLEX_FULL,
    PERIDOT_MSD_DUPLEX_AUTO
} PERIDOT_MSD_PORT_FORCE_DUPLEX_MODE;
/*  typedef: enum PERIDOT_MSD_8021Q_MODE */
typedef enum
{
    PERIDOT_MSD_DISABLE = 0,
    PERIDOT_MSD_FALLBACK,
    PERIDOT_MSD_CHECK,
    PERIDOT_MSD_SECURE
} PERIDOT_MSD_8021Q_MODE;
/* Definition for full duplex flow control mode */
typedef enum
{
    Peridot_PORT_FC_TX_RX_ENABLED,
    Peridot_PORT_RX_ONLY,
    Peridot_PORT_TX_ONLY,
    Peridot_PORT_PFC_ENABLED
} PERIDOT_MSD_PORT_FC_MODE;
/* Definition for the forced Port Speed */
typedef enum
{
    Peridot_PORT_FORCE_SPEED_10M = 0x0,
    Peridot_PORT_FORCE_SPEED_100M,
    Peridot_PORT_FORCE_SPEED_200M,
    Peridot_PORT_FORCE_SPEED_1000M,
    Peridot_PORT_FORCE_SPEED_2_5G,
    Peridot_PORT_FORCE_SPEED_10G = 0x6,
    Peridot_PORT_DO_NOT_FORCE_SPEED
} PERIDOT_MSD_PORT_FORCED_SPEED_MODE;
typedef enum
{
    PERIDOT_PORT_FORCE_LINK_UP,
    PERIDOT_PORT_FORCE_LINK_DOWN,
    PERIDOT_PORT_DO_NOT_FORCE_LINK
} PERIDOT_MSD_PORT_FORCE_LINK_MODE;
/* Definition for the Port Speed */
typedef enum
{
    Peridot_PORT_SPEED_10_MBPS = 0,
    Peridot_PORT_SPEED_100_MBPS = 1,
    Peridot_PORT_SPEED_200_MBPS = 2,
    Peridot_PORT_SPEED_1000_MBPS = 3,
    Peridot_PORT_SPEED_2_5_GBPS = 4,
    Peridot_PORT_SPEED_10_GBPS = 6,
    Peridot_PORT_SPEED_UNKNOWN = 7
} PERIDOT_MSD_PORT_SPEED;

typedef enum
{
    Peridot_PORT_STATE_DISABLE = 0,
    Peridot_PORT_STATE_BLOCKINGLISTENING = 1,
    Peridot_PORT_STATE_LEARNING = 2,
    Peridot_PORT_STATE_FORWARDING = 3,
}PERIDOT_MSD_PORT_PORTSTATE;
/* definition for Trunking */
#define PERIDOT_IS_TRUNK_ID_VALID(_dev, _id)    (((_id) < 32) ? 1 : 0)

extern MSD_STATUS Peridot_gprtSetForceDefaultVid
(
IN MSD_LPORT     port,
IN MSD_BOOL      en
);
extern MSD_STATUS Peridot_gprtGetForceDefaultVid
(
IN  MSD_LPORT   port,
OUT MSD_BOOL    *en
);
extern MSD_STATUS Peridot_gvlnSetPortVid
(
    IN MSD_LPORT     port,
    IN MSD_U16       vid
);
extern MSD_STATUS Peridot_gvlnGetPortVid
(
    IN  MSD_LPORT port,
    OUT MSD_U16   *vid
);
extern MSD_STATUS Peridot_gvlnSetPortVlanDot1qMode
(
    IN MSD_LPORT     port,
    IN PERIDOT_MSD_8021Q_MODE    mode
);
extern MSD_STATUS Peridot_gvlnGetPortVlanDot1qMode
(
    IN  MSD_LPORT        port,
    OUT PERIDOT_MSD_8021Q_MODE   *mode
);
extern MSD_STATUS Peridot_gprtSetDiscardTagged
(
    IN MSD_LPORT     port,
    IN MSD_BOOL        mode
);
extern MSD_STATUS Peridot_gprtGetDiscardTagged
(
    IN  MSD_LPORT    port,
    OUT MSD_BOOL     *mode
);
extern MSD_STATUS Peridot_gprtSetUnicastFloodBlock
(
    IN MSD_LPORT     port,
    IN MSD_BOOL      mode
);
extern MSD_STATUS Peridot_gprtSetDiscardUntagged
(
    IN MSD_LPORT     port,
    IN MSD_BOOL        mode
);
extern MSD_STATUS Peridot_gprtGetDiscardUntagged
(
    IN  MSD_LPORT    port,
    OUT MSD_BOOL     *mode
);
extern MSD_STATUS Peridot_gprtSetUnicastFloodBlock
(
    IN MSD_LPORT     port,
    IN MSD_BOOL      mode
);
extern MSD_STATUS Peridot_gprtGetUnicastFloodBlock
(
    IN  MSD_LPORT    port,
    OUT MSD_BOOL     *mode
);
extern MSD_STATUS Peridot_gprtSetMulticastFloodBlock
(
    IN MSD_LPORT     port,
    IN MSD_BOOL        mode
);
extern MSD_STATUS Peridot_gprtGetMulticastFloodBlock
(
    IN  MSD_LPORT    port,
    OUT MSD_BOOL     *mode
);
extern MSD_STATUS Peridot_gprtGetDropOnLock
(
IN  MSD_LPORT     port,
OUT MSD_BOOL      *en
);
extern MSD_STATUS Peridot_gprtSetDropOnLock
(
IN  MSD_LPORT     port,
IN  MSD_BOOL      en
);
extern MSD_STATUS Peridot_gprtSetJumboMode
(
    IN  MSD_LPORT    port,
    IN  PERIDOT_MSD_MTU_SIZE   mode
);
extern MSD_STATUS Peridot_gprtGetJumboMode
(
    IN  MSD_LPORT    port,
    OUT PERIDOT_MSD_MTU_SIZE   *mode
);
extern MSD_STATUS Peridot_gprtSetVlanPorts
(
IN  MSD_LPORT  port,
IN  MSD_LPORT  *memPorts,
IN  MSD_U8  memPortsLen
);
extern MSD_STATUS Peridot_gprtGetVlanPorts
(
IN  MSD_LPORT  port,
OUT MSD_LPORT  *memPorts,
OUT MSD_U8  *memPortsLen
);
extern MSD_STATUS Peridot_gprtGetSpeed
(
    IN  MSD_LPORT  port,
    OUT PERIDOT_MSD_PORT_SPEED   *speed
);
extern MSD_STATUS Peridot_gprtGetDuplexStatus
(
    IN  MSD_LPORT  port,
    OUT MSD_BOOL   *mode
);
extern MSD_STATUS Peridot_gprtGetLinkState
(
    IN  MSD_LPORT  port,
    OUT MSD_BOOL   *state
);
extern MSD_STATUS Peridot_grcSetEgressRate
(
    IN MSD_LPORT        port,
    IN PERIDOT_MSD_ELIMIT_MODE mode,
    IN MSD_U32	rate
);
extern MSD_STATUS Peridot_grcGetEgressRate
(
    IN  MSD_LPORT		port,
    OUT PERIDOT_MSD_ELIMIT_MODE *mode,
	OUT MSD_U32	*rate
);
extern MSD_STATUS Peridot_grcSetELimitMode
(
    IN  MSD_LPORT			port,
    IN  PERIDOT_MSD_ELIMIT_MODE      mode
);
extern MSD_STATUS Peridot_grcGetELimitMode
(
    IN  MSD_LPORT		port,
    OUT PERIDOT_MSD_ELIMIT_MODE  *mode
);
/********以下函数非移植官方Linux驱动库，自己编写********/
extern MSD_STATUS Peridot_gprtSetPortState
(
    IN  MSD_LPORT  port,
    IN  PERIDOT_MSD_PORT_PORTSTATE   portstate
);
extern MSD_STATUS Peridot_gprtGetPortState
(
    IN  MSD_LPORT  port,
    OUT PERIDOT_MSD_PORT_PORTSTATE   *portstate
);
#endif /* APPLICATIONS_MARVELL_88E6390_PORTCTRL_H_ */
