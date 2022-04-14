/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-04-01     Lenovo       the first version
 */
#ifndef APPLICATIONS_MARVELL_88E6390_PHYCTRL_H_
#define APPLICATIONS_MARVELL_88E6390_PHYCTRL_H_

#include "Marvell_88E6390_type.h"
#include "Marvell_88E6390_HwAccess.h"
#include "Marvell_88E6390_regs.h"
#include "kubot_debug.h"
/****************************************************************************/
/* Exported Phy Control Types                                               */
/****************************************************************************/

/*
* typedef: enum PERIDOT_MSD_PHY_AUTO_MODE
*
* Description: Enumeration of Autonegotiation mode.
*    Auto for both speed and duplex.
*    Auto for speed only and Full duplex.
*    Auto for speed only and Half duplex. (1000Mbps is not supported)
*    Auto for duplex only and speed 1000Mbps.
*    Auto for duplex only and speed 100Mbps.
*    Auto for duplex only and speed 10Mbps.
*    1000Mbps Full duplex.
*    100Mbps Full duplex.
*    100Mbps Half duplex.
*    10Mbps Full duplex.
*    10Mbps Half duplex.
*/

typedef enum
{
    PERIDOT_SPEED_AUTO_DUPLEX_AUTO,
    PERIDOT_SPEED_1000_DUPLEX_AUTO,
    PERIDOT_SPEED_100_DUPLEX_AUTO,
    PERIDOT_SPEED_10_DUPLEX_AUTO,
    PERIDOT_SPEED_AUTO_DUPLEX_FULL,
    PERIDOT_SPEED_AUTO_DUPLEX_HALF,
    PERIDOT_SPEED_1000_DUPLEX_FULL,
    PERIDOT_SPEED_1000_DUPLEX_HALF,
    PERIDOT_SPEED_100_DUPLEX_FULL,
    PERIDOT_SPEED_100_DUPLEX_HALF,
    PERIDOT_SPEED_10_DUPLEX_FULL,
    PERIDOT_SPEED_10_DUPLEX_HALF
}PERIDOT_MSD_PHY_AUTO_MODE;
/*
* typedef: enum PERIDOT_MSD_PHY_SPEED
*
* Description: Enumeration of Phy Speed
*
* Enumerations:
*    PERIDOT_PHY_SPEED_10_MBPS   - 10Mbps
*    PERIDOT_PHY_SPEED_100_MBPS    - 100Mbps
*    PERIDOT_PHY_SPEED_1000_MBPS - 1000Mbps
*/
typedef enum
{
    PERIDOT_PHY_SPEED_10_MBPS,
    PERIDOT_PHY_SPEED_100_MBPS,
    PERIDOT_PHY_SPEED_1000_MBPS
} PERIDOT_MSD_PHY_SPEED;

extern MSD_STATUS Peridot_gphyReset
(
    IN MSD_LPORT  port
);
extern MSD_STATUS Peridot_serdesReset
(
    IN MSD_LPORT  port
);
extern MSD_STATUS Peridot_gphySetPortLoopback
(
    IN MSD_LPORT  port,
    IN MSD_BOOL   enable
);
extern MSD_STATUS Peridot_gphySetPortSpeed
(
    IN MSD_LPORT  port,
    IN PERIDOT_MSD_PHY_SPEED speed
);
extern MSD_STATUS Peridot_gphyPortPowerDown
(
    IN MSD_LPORT  port,
    IN MSD_BOOL   state
);
extern MSD_STATUS Peridot_gphySetPortDuplexMode
(
    IN MSD_LPORT  port,
    IN MSD_BOOL   dMode
);
extern MSD_STATUS Peridot_gphySetPortAutoMode
(
    IN MSD_LPORT  port,
    IN PERIDOT_MSD_PHY_AUTO_MODE mode
);
extern MSD_STATUS Peridot_gphySetEEE
(
    IN MSD_LPORT portNum,
    IN MSD_BOOL en
);
extern MSD_STATUS Peridot_gphySetFlowControlEnable
(
    IN MSD_LPORT portNum,
    IN MSD_BOOL en
);
extern MSD_STATUS Peridot_gphySetIntEnable
(
IN MSD_LPORT portNum,
IN MSD_U16 en
);
extern MSD_STATUS Peridot_gphyGetIntEnable
(
IN MSD_LPORT portNum,
OUT MSD_U16 *en
);
extern MSD_STATUS Peridot_gphyGetIntStatus
(
IN MSD_LPORT portNum,
OUT MSD_U16 *stat
);
#endif /* APPLICATIONS_MARVELL_88E6390_PHYCTRL_H_ */
