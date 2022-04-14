#ifndef __Marvell_88E6390_msdPortRmon_H__
#define __Marvell_88E6390_msdPortRmon_H__

#include "Marvell_88E6390_type.h"
#include "Marvell_88E6390_HwAccess.h"
#include "Marvell_88E6390_regs.h"
#include "kubot_debug.h"

/*
* typedef: enum MSD_HISTOGRAM_MODE
*
* Description: Enumeration of the histogram counters mode.
*
* Enumerations:
*   MSD_COUNT_RX_ONLY - In this mode, Rx Histogram Counters are counted.
*   MSD_COUNT_TX_ONLY - In this mode, Tx Histogram Counters are counted.
*   MSD_COUNT_RX_TX   - In this mode, Rx and Tx Histogram Counters are counted.
*/
typedef enum
{
	MSD_COUNT_RX_ONLY = 1,
	MSD_COUNT_TX_ONLY,
	MSD_COUNT_RX_TX
} MSD_HISTOGRAM_MODE;
/****************************************************************************/
/* Exported MIBS Types			 			                                   */
/****************************************************************************/
/*
 *  typedef: struct MSD_STATS_COUNTERS
 *
 *  Description: MIB counter
 *     
 */
/*
*  typedef: struct MSD_STATS_COUNTERS
*
*  Description: MIB counter
*
*  Fields:
*      MSD_STATS_InGoodOctetsLo -
*						The lower 32-bits of the 64-bit InGoodOctets counter
*	   MSD_STATS_InGoodOctetsHi -
*                       The upper 32-bits of the 64-bit InGoodOctets counter
*      MSD_STATS_InBadOctets -
*                       The sum of lengths of all bad Ethrnet frames received
*      MSD_STATS_OutFCSErr -
*                       The number of frames transmitted with an invalid FCS
*      MSD_STATS_InUnicasts -
*                       The number of good frames received that have a Unicast destination MAC address
*      MSD_STATS_Deferred -
*                       The total number of successfully transmitted frames that experienced no collisions but are delayed
*      MSD_STATS_InBroadcasts -
*                       The number of good frames received that have a Broadcast destination MAC address
*      MSD_STATS_InMulticasts -
*                       The number of good frames received that have a Multicast destination MAC address
*      MSD_STATS_64Octets -
*						Total frames received with a length of exactly 64 octets, including those with errors
*      MSD_STATS_127Octets -
*						Total frames received with a length of exactly 65 and 127 octets, including those with errors
*      MSD_STATS_255Octets -
*						Total frames received with a length of exactly 128 and 255 octets, including those with errors
*      MSD_STATS_511Octets -
*						Total frames received with a length of exactly 256 and 511 octets, including those with errors
*      MSD_STATS_1023Octets -
*						Total frames received with a length of exactly 1024 and MaxSize octets, including those with errors
*      MSD_STATS_MaxOctets -
*						The number of good frames received that have a Unicast destination MAC address
*      MSD_STATS_OutOctetsLo -
*						The lower 32-bits of the 64-bit OutOctets counter
*      MSD_STATS_OutOctetsHi -
*						The upper 32-bits of the 64-bit OutOctets counter
*      MSD_STATS_OutUnicasts -
*						The number of frames sent that have a Unicast destination MAC address
*      MSD_STATS_Excessive -
*						The number frames dropped in the transmit MAC.
*      MSD_STATS_OutMulticasts -
*						The number of good frames sent that have a Multicast destination MAC address
*      MSD_STATS_OutBroadcasts -
*						The number of good frames sent that have a Broadcast destination MAC address
*      MSD_STATS_Single -
*						The total number of successfully transmitted framnes that experienced exactly one collision
*      MSD_STATS_OutPause -
*						The number of Flow Control frames sent
*      MSD_STATS_InPause -
*						The number of Good frames received that have a Pause destination MAC address
*      MSD_STATS_Multiple -
*						The total number of successfully transmitted frames that experienced more than one collision.
*      MSD_STATS_InUndersize -
*						Total frames received with a length of less than 64 octets but with a valid FCS
*      MSD_STATS_InFragments -
*						Total frames received with a length of less than 64 octets and an invalid FCS
*      MSD_STATS_InOversize -
*						Total frames received with a length of less than MaxSize octets but with a valid FCS
*      MSD_STATS_InJabber -
*						Total frames received with a length of less than MaxSize octets but with an invalid FCS
*      MSD_STATS_InRxErr -
*						Total frames received with an RxErr signal from the PHY
*      MSD_STATS_InFCSErr -
*						Total frames received with a CRC error not counted in InFragments, InJabber or InRxErr.
*      MSD_STATS_Collisions -
*						The number of collision events seen by the MAC not including those counted in
*						Single, Multiple, Excessive, or Late
*      MSD_STATS_Late -
*						The number of times a collision is detected later than 52 bits-times into the
*						transmission of a frame
*      MSD_STATS_InDiscards -
*						The number of good, non-filtered, frames that are received but can¡¯t be forwarded
*						due to a lack of buffer memory
*      MSD_STATS_InFiltered -
*						The number of good frames that were not forwarded due to policy filtering rules
*						such as but not limited to: 802.1Q Mode, Tagging mode, SA fitering e.t.c
*      MSD_STATS_InAccepted -
*						The number of good frames that are not policy filtered nor discarded due to an error
*						and made it throuth the Ingress amd is presented to the Queue Controller
*	   MSD_STATS_InBadAccepted -
*						The number of frames with a CRC error that is not filtered nor discarded
*      MSD_STATS_InGoodAvbClassA -
*						The number of good AVB frames received that have a Priority Code Point for
*						Class A that are not Undersize nor Oversize and are not discarded or fitered
*      MSD_STATS_InGoodAvbClassB -
*						The number of good AVB frames received that have a Priority Code Point for
*						Class B that are not Undersize nor Oversize and are not discarded or fitered
*      MSD_STATS_InBadAvbClassA -
*						The number of bad AVB frames received that have a Priority Code Point for
*						Class A that are not Undersize nor Oversize
*	   MSD_STATS_InBadAvbClassB -
*						The number of bad AVB frames received that have a Priority Code Point for
*						Class B that are not Undersize nor Oversize
*      MSD_STATS_TCAMCounter0 -
*						The number of good frames received that have a TCAM Hit on a TCAM Entry that
*						has its IncTcamCtr bit set to a one and its FlowID[7:6]=0 and are not discarded or filtered
*      MSD_STATS_TCAMCounter1 -
*						The number of good frames received that have a TCAM Hit on a TCAM Entry that
*						has its IncTcamCtr bit set to a one and its FlowID[7:6]=1 and are not discarded or filtered
*      MSD_STATS_TCAMCounter2 -
*						The number of good frames received that have a TCAM Hit on a TCAM Entry that
*						has its IncTcamCtr bit set to a one and its FlowID[7:6]=2 and are not discarded or filtered
*	   MSD_STATS_TCAMCounter3 -
*						The number of good frames received that have a TCAM Hit on a TCAM Entry that
*						has its IncTcamCtr bit set to a one and its FlowID[7:6]=3 and are not discarded or filtered
*      MSD_STATS_InDroppedAvbA -
*						The number of good AVB frames received that have a Priority Code Point for Class A that are not
*						Undersize nor Oversize and are not discarded or filtered but were not kept by the switch due to a lack of AVB buffers
*      MSD_STATS_InDroppedAvbB -
*						The number of good AVB frames received that have a Priority Code Point for Class B that are not
*						Undersize nor Oversize and are not discarded or filtered but were not kept by the switch due to a lack of AVB buffers
*      MSD_STATS_InDaUnknown -
*						The number of good frames received that did not have a Destination Address ¡®hit¡¯ from the ATU
*						and are not discarded or filtered
*      MSD_STATS_InMGMT -
*						The number of good frames received that are considered to be Management frames and are not discared
*						size is legal and its CRC is good or it was firced good by register
*	   MSD_STATS_OutQueue0 -
*						The number of frames that egress this port from Queue0
*      MSD_STATS_OutQueue1 -
*						The number of frames that egress this port from Queue1
*      MSD_STATS_OutQueue2 -
*						The number of frames that egress this port from Queue2
*      MSD_STATS_OutQueue3 -
*						The number of frames that egress this port from Queue3
*      MSD_STATS_OutQueue4 -
*						The number of frames that egress this port from Queue4
*      MSD_STATS_OutQueue5 -
*						The number of frames that egress this port from Queue5
*      MSD_STATS_OutQueue6 -
*						The number of frames that egress this port from Queue6
*      MSD_STATS_OutQueue7 -
*						The number of frames that egress this port from Queue7
*      MSD_STATS_OutCutThrough -
*						The number of frames that egress this port from the Cut Through path
*      MSD_STATS_InBadQbv -
*						The number of good, non-filtered, frames that are received but can¡¯t be forwarded 
*						due to them arriving at the wrong time per the Qbv ingress policier
*      MSD_STATS_OutOctetsA -
*						The sum of lengths of all Ethernet frames sent from the AVB Class A Queue not including frames
*						that are considered Management by ingress
*      MSD_STATS_OutOctetsB -
*						The sum of lengths of all Ethernet frames sent from the AVB Class B Queue not including frames
*						that are considered Management by ingress
*      MSD_STATS_OutYel -
*						The number of Yellow frames that egressed this port
*      MSD_STATS_OutDroppedYel -
*						The number of Yellow frames not counted in InDiscards that are ¡®head dropped¡¯ from an egress port¡¯s
*						queues and the number of Yellow frames¡¯s ¡®tail dropped¡¯ from an egress port¡¯s queues due to Queue
*						Controller¡¯s queue limits
*      MSD_STATS_OutDiscards -
*						The number of Green frames not counted in Indiscards that are ¡®head dropped¡¯ from an egress port¡¯s
*						queues and the number of Green frames¡¯s ¡®tail dropped¡¯ from an egress port¡¯s queues due to Queue
*						Controller¡¯s queue limits
*      MSD_STATS_OutMGMT -
*						The number of frames transmitted that were considered to be Management frames
*/
#define PERIDOT_MSD_TYPE_BANK 0x400
typedef enum
{
	/* Bank 0 */
    Peridot_STATS_InGoodOctetsLo = 0,
    Peridot_STATS_InGoodOctetsHi,
    Peridot_STATS_InBadOctets,
    
    Peridot_STATS_OutFCSErr,
    Peridot_STATS_InUnicasts,
    Peridot_STATS_Deferred,             /* offset 5 */
    Peridot_STATS_InBroadcasts,
    Peridot_STATS_InMulticasts,
    Peridot_STATS_64Octets,
    Peridot_STATS_127Octets,
    Peridot_STATS_255Octets,            /* offset 10 */
    Peridot_STATS_511Octets,
    Peridot_STATS_1023Octets,
    Peridot_STATS_MaxOctets,
    Peridot_STATS_OutOctetsLo,
    Peridot_STATS_OutOctetsHi,
    Peridot_STATS_OutUnicasts,          /* offset 16 */
    Peridot_STATS_Excessive,
    Peridot_STATS_OutMulticasts,
    Peridot_STATS_OutBroadcasts,
    Peridot_STATS_Single,
    Peridot_STATS_OutPause,
    Peridot_STATS_InPause,
    Peridot_STATS_Multiple,
    Peridot_STATS_InUndersize,          /* offset 24 */
    Peridot_STATS_InFragments,
    Peridot_STATS_InOversize,
    Peridot_STATS_InJabber,
    Peridot_STATS_InRxErr,
    Peridot_STATS_InFCSErr,
    Peridot_STATS_Collisions,
    Peridot_STATS_Late,                 /* offset 31 */
	/* Bank 1 */
    Peridot_STATS_InDiscards      = PERIDOT_MSD_TYPE_BANK+0x00,
    Peridot_STATS_InFiltered      = PERIDOT_MSD_TYPE_BANK+0x01,
    Peridot_STATS_InAccepted      = PERIDOT_MSD_TYPE_BANK+0x02,
    Peridot_STATS_InBadAccepted   = PERIDOT_MSD_TYPE_BANK+0x03,
    Peridot_STATS_InGoodAvbClassA = PERIDOT_MSD_TYPE_BANK+0x04,
    Peridot_STATS_InGoodAvbClassB = PERIDOT_MSD_TYPE_BANK+0x05,
    Peridot_STATS_InBadAvbClassA  = PERIDOT_MSD_TYPE_BANK+0x06,
    Peridot_STATS_InBadAvbClassB  = PERIDOT_MSD_TYPE_BANK+0x07,
    Peridot_STATS_TCAMCounter0    = PERIDOT_MSD_TYPE_BANK+0x08,
    Peridot_STATS_TCAMCounter1    = PERIDOT_MSD_TYPE_BANK+0x09,
    Peridot_STATS_TCAMCounter2    = PERIDOT_MSD_TYPE_BANK+0x0a,
    Peridot_STATS_TCAMCounter3    = PERIDOT_MSD_TYPE_BANK+0x0b,
    Peridot_STATS_InDroppedAvbA   = PERIDOT_MSD_TYPE_BANK+0x0c,
    Peridot_STATS_InDroppedAvbB   = PERIDOT_MSD_TYPE_BANK+0x0d, 
    Peridot_STATS_InDaUnknown     = PERIDOT_MSD_TYPE_BANK+0x0e,
    Peridot_STATS_InMGMT          = PERIDOT_MSD_TYPE_BANK+0x0f,
    Peridot_STATS_OutQueue0       = PERIDOT_MSD_TYPE_BANK+0x10,
    Peridot_STATS_OutQueue1       = PERIDOT_MSD_TYPE_BANK+0x11,
    Peridot_STATS_OutQueue2       = PERIDOT_MSD_TYPE_BANK+0x12,
    Peridot_STATS_OutQueue3       = PERIDOT_MSD_TYPE_BANK+0x13,
    Peridot_STATS_OutQueue4       = PERIDOT_MSD_TYPE_BANK+0x14,
    Peridot_STATS_OutQueue5       = PERIDOT_MSD_TYPE_BANK+0x15,
    Peridot_STATS_OutQueue6       = PERIDOT_MSD_TYPE_BANK+0x16,
    Peridot_STATS_OutQueue7       = PERIDOT_MSD_TYPE_BANK+0x17,
    Peridot_STATS_OutCutThrough   = PERIDOT_MSD_TYPE_BANK+0x18,
    Peridot_STATS_OutOctetsA      = PERIDOT_MSD_TYPE_BANK+0x1a,
    Peridot_STATS_OutOctetsB      = PERIDOT_MSD_TYPE_BANK+0x1b,
    Peridot_STATS_OutYel          = PERIDOT_MSD_TYPE_BANK+0x1c,
    Peridot_STATS_OutDroppedYel   = PERIDOT_MSD_TYPE_BANK+0x1d, 
    Peridot_STATS_OutDiscards     = PERIDOT_MSD_TYPE_BANK+0x1e, 
    Peridot_STATS_OutMGMT         = PERIDOT_MSD_TYPE_BANK+0x1f

} PERIDOT_MSD_STATS_COUNTERS;

/*
 *  typedef: struct PERIDOT_MSD_STATS_COUNTER_SET
 *
 *  Description: MIB Counter Set
 *     
 */
/*
*  typedef: struct MSD_STATS_COUNTER_SET
*
*  Description: MIB counter
*
*  Fields:
*      InGoodOctetsLo -
*						The lower 32-bits of the 64-bit InGoodOctets counter
*	   InGoodOctetsHi -
*                       The upper 32-bits of the 64-bit InGoodOctets counter
*      InBadOctets -
*                       The sum of lengths of all bad Ethrnet frames received
*      OutFCSErr -
*                       The number of frames transmitted with an invalid FCS
*      InUnicasts -
*                       The number of good frames received that have a Unicast destination MAC address
*      Deferred -
*                       The total number of successfully transmitted frames that experienced no collisions but are delayed
*      InBroadcasts -
*                       The number of good frames received that have a Broadcast destination MAC address
*      InMulticasts -
*                       The number of good frames received that have a Multicast destination MAC address
*      Octets64 -
*						Total frames received with a length of exactly 64 octets, including those with errors
*      Octets127 -
*						Total frames received with a length of exactly 65 and 127 octets, including those with errors
*      Octets255 -
*						Total frames received with a length of exactly 128 and 255 octets, including those with errors
*      Octets511 -
*						Total frames received with a length of exactly 256 and 511 octets, including those with errors
*      Octets1023 -
*						Total frames received with a length of exactly 1024 and MaxSize octets, including those with errors
*      OctetsMax -
*						The number of good frames received that have a Unicast destination MAC address
*      OutOctetsLo -
*						The lower 32-bits of the 64-bit OutOctets counter
*      OutOctetsHi -
*						The upper 32-bits of the 64-bit OutOctets counter
*      OutUnicasts -
*						The number of frames sent that have a Unicast destination MAC address
*      Excessive -
*						The number frames dropped in the transmit MAC.
*      OutMulticasts -
*						The number of good frames sent that have a Multicast destination MAC address
*      OutBroadcasts -
*						The number of good frames sent that have a Broadcast destination MAC address
*      Single -
*						The total number of successfully transmitted framnes that experienced exactly one collision
*      OutPause -
*						The number of Flow Control frames sent
*      InPause -
*						The number of Good frames received that have a Pause destination MAC address
*      Multiple -
*						The total number of successfully transmitted frames that experienced more than one collision.
*      InUndersize -
*						Total frames received with a length of less than 64 octets but with a valid FCS
*      InFragments -
*						Total frames received with a length of less than 64 octets and an invalid FCS
*      InOversize -
*						Total frames received with a length of less than MaxSize octets but with a valid FCS
*      InJabber -
*						Total frames received with a length of less than MaxSize octets but with an invalid FCS
*      InRxErr -
*						Total frames received with an RxErr signal from the PHY
*      InFCSErr -
*						Total frames received with a CRC error not counted in InFragments, InJabber or InRxErr.
*      Collisions -
*						The number of collision events seen by the MAC not including those counted in
*						Single, Multiple, Excessive, or Late
*      Late -
*						The number of times a collision is detected later than 52 bits-times into the
*						transmission of a frame
*      InDiscards -
*						The number of good, non-filtered, frames that are received but can¡¯t be forwarded
*						due to a lack of buffer memory
*      InFiltered -
*						The number of good frames that were not forwarded due to policy filtering rules
*						such as but not limited to: 802.1Q Mode, Tagging mode, SA fitering e.t.c
*      InAccepted -
*						The number of good frames that are not policy filtered nor discarded due to an error
*						and made it throuth the Ingress amd is presented to the Queue Controller
*	   InBadAccepted -
*						The number of frames with a CRC error that is not filtered nor discarded
*      InGoodAvbClassA -
*						The number of good AVB frames received that have a Priority Code Point for
*						Class A that are not Undersize nor Oversize and are not discarded or fitered
*      InGoodAvbClassB -
*						The number of good AVB frames received that have a Priority Code Point for
*						Class B that are not Undersize nor Oversize and are not discarded or fitered
*      InBadAvbClassA -
*						The number of bad AVB frames received that have a Priority Code Point for
*						Class A that are not Undersize nor Oversize
*	   InBadAvbClassB -
*						The number of bad AVB frames received that have a Priority Code Point for
*						Class B that are not Undersize nor Oversize
*      TCAMCounter0 -
*						The number of good frames received that have a TCAM Hit on a TCAM Entry that
*						has its IncTcamCtr bit set to a one and its FlowID[7:6]=0 and are not discarded or filtered
*      TCAMCounter1 -
*						The number of good frames received that have a TCAM Hit on a TCAM Entry that
*						has its IncTcamCtr bit set to a one and its FlowID[7:6]=1 and are not discarded or filtered
*      TCAMCounter2 -
*						The number of good frames received that have a TCAM Hit on a TCAM Entry that
*						has its IncTcamCtr bit set to a one and its FlowID[7:6]=2 and are not discarded or filtered
*	   TCAMCounter3 -
*						The number of good frames received that have a TCAM Hit on a TCAM Entry that
*						has its IncTcamCtr bit set to a one and its FlowID[7:6]=3 and are not discarded or filtered
*      InDroppedAvbA -
*						The number of good AVB frames received that have a Priority Code Point for Class A that are not
*						Undersize nor Oversize and are not discarded or filtered but were not kept by the switch due to a lack of AVB buffers
*      InDroppedAvbB -
*						The number of good AVB frames received that have a Priority Code Point for Class B that are not
*						Undersize nor Oversize and are not discarded or filtered but were not kept by the switch due to a lack of AVB buffers
*      InDaUnknown -
*						The number of good frames received that did not have a Destination Address ¡®hit¡¯ from the ATU
*						and are not discarded or filtered
*      InMGMT -
*						The number of good frames received that are considered to be Management frames and are not discared
*						size is legal and its CRC is good or it was firced good by register
*	   OutQueue0 -
*						The number of frames that egress this port from Queue0
*      OutQueue1 -
*						The number of frames that egress this port from Queue1
*      OutQueue2 -
*						The number of frames that egress this port from Queue2
*      OutQueue3 -
*						The number of frames that egress this port from Queue3
*      OutQueue4 -
*						The number of frames that egress this port from Queue4
*      OutQueue5 -
*						The number of frames that egress this port from Queue5
*      OutQueue6 -
*						The number of frames that egress this port from Queue6
*      OutQueue7 -
*						The number of frames that egress this port from Queue7
*      OutCutThrough -
*						The number of frames that egress this port from the Cut Through path
*      InBadQbv -
*						The number of good, non-filtered, frames that are received but can¡¯t be forwarded 
*						due to them arriving at the wrong time per the Qbv ingress policier
*      OutOctetsA -
*						The sum of lengths of all Ethernet frames sent from the AVB Class A Queue not including frames
*						that are considered Management by ingress
*      OutOctetsB -
*						The sum of lengths of all Ethernet frames sent from the AVB Class B Queue not including frames
*						that are considered Management by ingress
*      OutYel -
*						The number of Yellow frames that egressed this port
*      OutDroppedYel -
*						The number of Yellow frames not counted in InDiscards that are ¡®head dropped¡¯ from an egress port¡¯s
*						queues and the number of Yellow frames¡¯s ¡®tail dropped¡¯ from an egress port¡¯s queues due to Queue
*						Controller¡¯s queue limits
*      OutDiscards -
*						The number of Green frames not counted in Indiscards that are ¡®head dropped¡¯ from an egress port¡¯s
*						queues and the number of Green frames¡¯s ¡®tail dropped¡¯ from an egress port¡¯s queues due to Queue
*						Controller¡¯s queue limits
*      OutMGMT -
*						The number of frames transmitted that were considered to be Management frames
*/
typedef struct _PERIDOT_MSD_STATS_COUNTER_SET
{
	/* Bank 0 */
    MSD_U32    InGoodOctetsLo;     /* offset 0 */
    MSD_U32    InGoodOctetsHi;     /* offset 1 */
    MSD_U32    InBadOctets;        /* offset 2 */
    MSD_U32    OutFCSErr;          /* offset 3 */
    MSD_U32    InUnicasts;         /* offset 4 */
    MSD_U32    Deferred;           /* offset 5 */
    MSD_U32    InBroadcasts;       /* offset 6 */
    MSD_U32    InMulticasts;       /* offset 7 */
    /* 
        Histogram Counters : Rx Only, Tx Only, or both Rx and Tx 
        (refer to Histogram Mode) 
    */
    MSD_U32    Octets64;         /* 64 Octets, offset 8 */
    MSD_U32    Octets127;        /* 65 to 127 Octets, offset 9 */
    MSD_U32    Octets255;        /* 128 to 255 Octets, offset 10 */
    MSD_U32    Octets511;        /* 256 to 511 Octets, offset 11 */
    MSD_U32    Octets1023;       /* 512 to 1023 Octets, offset 12 */
    MSD_U32    OctetsMax;        /* 1024 to Max Octets, offset 13 */
    MSD_U32    OutOctetsLo;      /* offset 14 */
    MSD_U32    OutOctetsHi;      /* offset 15 */
    MSD_U32    OutUnicasts;      /* offset 16 */
    MSD_U32    Excessive;        /* offset 17 */
    MSD_U32    OutMulticasts;    /* offset 18 */
    MSD_U32    OutBroadcasts;    /* offset 19 */
    MSD_U32    Single;           /* offset 20 */

    MSD_U32    OutPause;         /* offset 21 */
    MSD_U32    InPause;          /* offset 22 */
    MSD_U32    Multiple;         /* offset 23 */
    MSD_U32    InUndersize;        /* offset 24 */
    MSD_U32    InFragments;        /* offset 25 */
    MSD_U32    InOversize;         /* offset 26 */
    MSD_U32    InJabber;           /* offset 27 */
    MSD_U32    InRxErr;          /* offset 28 */
    MSD_U32    InFCSErr;         /* offset 29 */
    MSD_U32    Collisions;       /* offset 30 */
    MSD_U32    Late;             /* offset 31 */
	/* Bank 1 */
    MSD_U32    InDiscards;       /* offset 0x00 */
    MSD_U32    InFiltered;       /* offset 0x01 */
    MSD_U32    InAccepted;       /* offset 0x02 */
    MSD_U32    InBadAccepted;    /* offset 0x03 */
    MSD_U32    InGoodAvbClassA;  /* offset 0x04 */
    MSD_U32    InGoodAvbClassB;  /* offset 0x05 */
    MSD_U32    InBadAvbClassA ;  /* offset 0x06 */
    MSD_U32    InBadAvbClassB ;  /* offset 0x07 */
    MSD_U32    TCAMCounter0;     /* offset 0x08 */
    MSD_U32    TCAMCounter1;     /* offset 0x09 */
    MSD_U32    TCAMCounter2;     /* offset 0x0a */
    MSD_U32    TCAMCounter3;     /* offset 0x0b */
    MSD_U32    InDroppedAvbA;    /* offset 0x0c */
    MSD_U32    InDroppedAvbB;    /* offset 0x0d */
    MSD_U32    InDaUnknown ;     /* offset 0x0e */
    MSD_U32    InMGMT;           /* offset 0x0f */
    MSD_U32    OutQueue0;        /* offset 0x10 */
    MSD_U32    OutQueue1;        /* offset 0x11 */
    MSD_U32    OutQueue2;        /* offset 0x12 */
    MSD_U32    OutQueue3;        /* offset 0x13 */
    MSD_U32    OutQueue4;        /* offset 0x14 */
    MSD_U32    OutQueue5;        /* offset 0x15 */
    MSD_U32    OutQueue6;        /* offset 0x16 */
    MSD_U32    OutQueue7;        /* offset 0x17 */
    MSD_U32    OutCutThrough;    /* offset 0x18 */
    MSD_U32    reserved_19 ;     /* offset 0x19 */
    MSD_U32    OutOctetsA;       /* offset 0x1a */
    MSD_U32    OutOctetsB;       /* offset 0x1b */
    MSD_U32    OutYel;           /* offset 0x1c */
    MSD_U32    OutDroppedYel;    /* offset 0x1d */
    MSD_U32    OutDiscards;      /* offset 0x1e */
    MSD_U32    OutMGMT;          /* offset 0x1f */

}PERIDOT_MSD_STATS_COUNTER_SET;

/*
 * typedef: enum MSD_HISTOGRAM_MODE
 *
 * Description: Enumeration of the histogram counters mode.
 *
 * Enumerations:
 *   MSD_COUNT_RX_ONLY - In this mode, Rx Histogram Counters are counted.
 *   MSD_COUNT_TX_ONLY - In this mode, Tx Histogram Counters are counted.
 *   MSD_COUNT_RX_TX   - In this mode, Rx and Tx Histogram Counters are counted.
 */
typedef enum
{
	PERIDOT_MSD_COUNT_RX_ONLY = 1,
	PERIDOT_MSD_COUNT_TX_ONLY,
	PERIDOT_MSD_COUNT_RX_TX
} PERIDOT_MSD_HISTOGRAM_MODE;

extern MSD_STATUS Peridot_gstatsFlushAll(void);
MSD_STATUS Peridot_gstatsFlushPort
(
    IN MSD_LPORT    port
);
MSD_STATUS Peridot_gstatsGetPortCounter
(
    IN  MSD_LPORT         port,
	IN  PERIDOT_MSD_STATS_COUNTERS    counter,
    OUT MSD_U32            *statsData
);
MSD_STATUS Peridot_gstatsGetPortAllCounters
(
    IN  MSD_LPORT        port,
	OUT PERIDOT_MSD_STATS_COUNTER_SET    *statsCounterSet
);
MSD_STATUS Peridot_gstatsGetHistogramMode
(
	OUT PERIDOT_MSD_HISTOGRAM_MODE    *mode
);
MSD_STATUS Peridot_gstatsSetHistogramMode
(
	IN PERIDOT_MSD_HISTOGRAM_MODE        mode
);
#endif /*__Marvell_88E6390_msdPortRmon_H__*/
