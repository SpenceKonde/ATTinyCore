#ifndef LN_OPC_INCLUDED
#define LN_OPC_INCLUDED
/*****************************************************************************
 *                                                                           *
 *      Copyright (C) 2001 Ron W. Auld                                       *
 *      Copyright (C) 2004 Alex Shepherd                                                                      *
 *                                                                           *
 *      Portions Copyright (C) Digitrax Inc.                                 * 
 *                                                                           *
 *                                                                           *
 *  This library is free software; you can redistribute it and/or            *
 *  modify it under the terms of the GNU Lesser General Public               *
 *  License as published by the Free Software Foundation; either             *
 *  version 2.1 of the License, or (at your option) any later version.       *
 *                                                                           *
 *  This library is distributed in the hope that it will be useful,          *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        *
 *  Lesser General Public License for more details.                          *
 *                                                                           *
 *  You should have received a copy of the GNU Lesser General Public         *
 *  License along with this library; if not, write to the Free Software      *
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA*
 *                                                                           *
 *****************************************************************************
 *
 *  IMPORTANT:
 *
 *  Some of the message formats used in this code are Copyright Digitrax, Inc.
 *  and are used with permission as part of the EmbeddedLocoNet project. That
 *  permission does not extend to uses in other software products. If you wish
 *  to use this code, algorithm or these message formats outside of
 *  EmbeddedLocoNet, please contact Digitrax Inc, for specific permission.
 *
 *  Note: The sale any LocoNet device hardware (including bare PCB's) that
 *  uses this or any other LocoNet software, requires testing and certification
 *  by Digitrax Inc. and will be subject to a licensing agreement.
 *
 *  Please contact Digitrax Inc. for details.
 *
 *****************************************************************************
 *  NOTE                                                                     *
 *                                                                           *
 *  This file was first released under the GPL license but Ron Auld has      *
 *  given his permission for this file to be included in the EmbeddedLoconet *
 *  project under the LGPL license                                           *
 *                                                                           *
 *****************************************************************************
 *                                                                           *
 *  Some of this code is based on work done by John Kabat and I thank him    *
 *  for the use of that code and his contributions to the understanding      *
 *  and use of the Digitrax LocoNet.                                         *
 *                                                                           *
 *****************************************************************************
 *
 * File Name:     loconet.h
 * Module:        Generic Loconet (r) message definitions.
 * Language:
 * Contributors:  Ron W. Auld (RwA), John Kabat
 * Remarks:
 *
 *    This file contains the definitions and structures pertinant to the
 *    Loconet (r) message protocol.
 *
 * Version Control:
 * $Log: loconet.h,v $
 * Plocher: changed byte to uint8_t
 *
 * Revision 1.14  2007/07/02 07:40:37  sbormann71
 * analog IO for servo test
 *
 * Revision 1.13  2007/02/21 21:08:06  sbormann71
 * added Uhlenbrock basic packet format to lnMsg union
 *
 * Revision 1.12  2006/11/05 12:03:29  tkurz
 * OPC_SE added
 *
 * Revision 1.11  2006/06/22 23:02:32  sbormann71
 * fixed OPC_BUSY
 *
 * Revision 1.10  2006/06/21 22:27:53  sbormann71
 * added GTRK_POWER
 *
 * Revision 1.9  2004/03/08 09:15:34  kiwi64ajs
 * minor bug fixes found in testing in the Emulator Test App
 *
 * Revision 1.8  2004/02/19 04:04:16  kiwi64ajs
 * moved sv message definition from sv.h to loconet.h
 * added special Digitrax copyright notice to loconet.h
 * added message union to loconet.h
 *
 * Revision 1.7  2003/11/03 20:27:19  kiwi64ajs
 * added #ifndef for multiple inclusion
 *
 * Revision 1.6  2003/01/02 12:05:48  kiwi64ajs
 * moved byte definition out to common_defs.h
 *
 * Revision 1.5  2002/12/28 11:08:07  kiwi64ajs
 * change #ifndef test for byte and word being already defined
 *
 * Revision 1.4  2002/12/14 21:08:05  kiwi64ajs
 * minor changes to be compatible with BORLANDC
 * changed all char array index vars to byte and modified code to handle invalid case ox 0xFF not 0x00
 *
 * Revision 1.3  2002/12/03 10:49:23  kiwi64ajs
 * Added locoDataMsg and pragma pack
 *
 * Revision 1.2  2002/09/23 09:49:01  kiwi64ajs
 * Added comment about license change from GPL to LGPL
 *
 * Revision 1.1  2002/09/23 09:22:19  kiwi64ajs
 * Added generic loconet directory for non-avr specific loconet files
 * Added Ron Auld's comprehensive loconet structures definition
 *
 * Revision 1.2  2001/05/31 17:40:05  rauld
 * Added options and code needed for llnmon to use a Locobuffer interface instead
 * of an MS-100.
 *
 * Revision 1.1.1.1  2001/01/16 12:55:59  rauld
 * Initial import into CVS
 *
 *
 */

#include <stdint.h>

#if defined (__cplusplus)
	extern "C" {
#endif

/* various bit masks */
#define DIRF_DIR          0x20  /* direction bit    */
#define DIRF_F0           0x10  /* Function 0 bit   */
#define DIRF_F4           0x08  /* Function 1 bit   */
#define DIRF_F3           0x04  /* Function 2 bit   */
#define DIRF_F2           0x02  /* Function 3 bit   */
#define DIRF_F1           0x01  /* Function 4 bit   */
#define SND_F8            0x08  /* Sound 4/Function 8 bit */
#define SND_F7            0x04  /* Sound 3/Function 7 bit */
#define SND_F6            0x02  /* Sound 2/Function 6 bit */
#define SND_F5            0x01  /* Sound 1/Function 5 bit */

#define OPC_SW_ACK_CLOSED 0x20  /* command switch closed/open bit   */
#define OPC_SW_ACK_OUTPUT 0x10  /* command switch output on/off bit */

#define OPC_INPUT_REP_CB  0x40  /* control bit, reserved otherwise      */
#define OPC_INPUT_REP_SW  0x20  /* input is switch input, aux otherwise */
#define OPC_INPUT_REP_HI  0x10  /* input is HI, LO otherwise            */

#define OPC_SW_REP_SW     0x20  /* switch input, aux input otherwise    */
#define OPC_SW_REP_HI     0x10  /* input is HI, LO otherwise            */
#define OPC_SW_REP_CLOSED 0x20  /* 'Closed' line is ON, OFF otherwise   */
#define OPC_SW_REP_THROWN 0x10  /* 'Thrown' line is ON, OFF otherwise   */
#define OPC_SW_REP_INPUTS 0x40  /* sensor inputs, outputs otherwise     */

#define OPC_SW_REQ_DIR    0x20  /* switch direction - closed/thrown     */
#define OPC_SW_REQ_OUT    0x10  /* output On/Off                        */

#define OPC_LOCO_SPD_ESTOP 0x01 /* emergency stop command               */
#define OPC_SE             0xE4 /* Opcode Security Element              */
#define OPC_ANALOGIO       0xE5 /* Analog IO                            */


/* Slot Status byte definitions and macros */
/***********************************************************************************
*   D7-SL_SPURGE    ; 1=SLOT purge en,                                             *
*                   ; ALSO adrSEL (INTERNAL use only) (not seen on NET!)           *
*                                                                                  *
*   D6-SL_CONUP     ; CONDN/CONUP: bit encoding-Control double linked Consist List *
*                   ;    11=LOGICAL MID CONSIST , Linked up AND down               *
*                   ;    10=LOGICAL CONSIST TOP, Only linked downwards             *
*                   ;    01=LOGICAL CONSIST SUB-MEMBER, Only linked upwards        *
*                   ;    00=FREE locomotive, no CONSIST indirection/linking        *
*                   ; ALLOWS "CONSISTS of CONSISTS". Uplinked means that           *
*                   ; Slot SPD number is now SLOT adr of SPD/DIR and STATUS        *
*                   ; of consist. i.e. is ;an Indirect pointer. This Slot          *
*                   ; has same BUSY/ACTIVE bits as TOP of Consist. TOP is          *
*                   ; loco with SPD/DIR for whole consist. (top of list).          *
*                   ; BUSY/ACTIVE: bit encoding for SLOT activity                  *
*                                                                                  *
*   D5-SL_BUSY      ; 11=IN_USE loco adr in SLOT -REFRESHED                        *
*                                                                                  *
*   D4-SL_ACTIVE    ; 10=IDLE loco adr in SLOT -NOT refreshed                      *
*                   ; 01=COMMON loco adr IN SLOT -refreshed                        *
*                   ; 00=FREE SLOT, no valid DATA -not refreshed                   *
*                                                                                  *
*   D3-SL_CONDN     ; shows other SLOT Consist linked INTO this slot, see SL_CONUP *
*                                                                                  *
*   D2-SL_SPDEX     ; 3 BITS for Decoder TYPE encoding for this SLOT               *
*                                                                                  *
*   D1-SL_SPD14     ; 011=send 128 speed mode packets                              *
*                                                                                  *
*   D0-SL_SPD28     ; 010=14 step MODE                                             *
*                   ; 001=28 step. Generate Trinary packets for this               *
*                   ;              Mobile ADR                                      *
*                   ; 000=28 step. 3 BYTE PKT regular mode                         *
*                   ; 111=128 Step decoder, Allow Advanced DCC consisting          *
*                   ; 100=28 Step decoder ,Allow Advanced DCC consisting           *
***********************************************************************************/

#define STAT1_SL_SPURGE   0x80  /* internal use only, not seen on net */
#define STAT1_SL_CONUP    0x40  /* consist status                     */
#define STAT1_SL_BUSY     0x20  /* used with STAT1_SL_ACTIVE,         */
#define STAT1_SL_ACTIVE   0x10  /*                                    */
#define STAT1_SL_CONDN    0x08  /*                                    */
#define STAT1_SL_SPDEX    0x04  /*                                    */
#define STAT1_SL_SPD14    0x02  /*                                    */
#define STAT1_SL_SPD28    0x01  /*                                    */
#define STAT2_SL_SUPPRESS 0x01  /* 1 = Adv. Consisting supressed      */
#define STAT2_SL_NOT_ID   0x04  /* 1 = ID1/ID2 is not ID usage        */
#define STAT2_SL_NOTENCOD 0x08  /* 1 = ID1/ID2 is not encoded alias   */
#define STAT2_ALIAS_MASK  (STAT2_SL_NOTENCOD | STAT2_SL_NOT_ID)
#define STAT2_ID_IS_ALIAS STAT2_SL_NOT_ID

/* mask and values for consist determination */
#define CONSIST_MASK      (STAT1_SL_CONDN | STAT1_SL_CONUP)
#define CONSIST_MID       (STAT1_SL_CONDN | STAT1_SL_CONUP)
#define CONSIST_TOP       (STAT1_SL_CONDN)
#define CONSIST_SUB       (STAT1_SL_CONUP)
#define CONSIST_NO        (0)
#define CONSIST_STAT(s) (  ((s & CONSIST_MASK) == CONSIST_MID) ? "Mid-Consisted" : \
                         ( ((s & CONSIST_MASK) == CONSIST_TOP) ? "Consist TOP" : \
                          (((s & CONSIST_MASK) == CONSIST_SUB) ? "Sub-Consisted" : \
                           "Not Consisted")))

#define CONSISTED(s) (((s & CONSIST_MASK) == CONSIST_MID) || ((s & CONSIST_MASK) == CONSIST_SUB))

/* mask and values for locomotive use determination */
#define LOCOSTAT_MASK     (STAT1_SL_BUSY  | STAT1_SL_ACTIVE)
#define LOCO_IN_USE       (STAT1_SL_BUSY  | STAT1_SL_ACTIVE)
#define LOCO_IDLE         (STAT1_SL_BUSY)
#define LOCO_COMMON       (STAT1_SL_ACTIVE)
#define LOCO_FREE         (0)
#define LOCO_STAT(s)    (  ((s & LOCOSTAT_MASK) == LOCO_IN_USE) ? "In-Use" : \
                         ( ((s & LOCOSTAT_MASK) == LOCO_IDLE)   ? "Idle" : \
                          (((s & LOCOSTAT_MASK) == LOCO_COMMON) ? "Common" : \
                           "Free")))

/* mask and values for decoder type encoding for this slot */
#define DEC_MODE_MASK     (STAT1_SL_SPDEX | STAT1_SL_SPD14 | STAT1_SL_SPD28)
/* Advanced consisting allowed for the next two */
#define DEC_MODE_128A     (STAT1_SL_SPDEX | STAT1_SL_SPD14 | STAT1_SL_SPD28)
#define DEC_MODE_28A      (STAT1_SL_SPDEX                                  )
/* normal modes */
#define DEC_MODE_128      (STAT1_SL_SPD14 | STAT1_SL_SPD28)
#define DEC_MODE_14       (STAT1_SL_SPD14)
#define DEC_MODE_28TRI    (STAT1_SL_SPD28)
#define DEC_MODE_28       (0)
#define DEC_MODE(s)    (    ((s & DEC_MODE_MASK) == DEC_MODE_128A)  ? "128 (Allow Adv. consisting)" : \
                        (   ((s & DEC_MODE_MASK) == DEC_MODE_28A)   ? "28 (Allow Adv. consisting)" : \
                         (  ((s & DEC_MODE_MASK) == DEC_MODE_128)   ? "128" : \
                          ( ((s & DEC_MODE_MASK) == DEC_MODE_14)    ? "14" : \
                           (((s & DEC_MODE_MASK) == DEC_MODE_28TRI) ? "28 (Motorola)" : "28")))))

/* values for track status encoding for this slot */
#define GTRK_PROG_BUSY    0x08      /* 1 = programming track in this master is Busy         */
#define GTRK_MLOK1        0x04      /* 0 = Master is DT200, 1=Master implements LocoNet 1.1 */
#define GTRK_IDLE         0x02      /* 0 = Track paused, B'cast EMERG STOP, 1 = Power On    */
#define GTRK_POWER        0x01

#define FC_SLOT           0x7b      /* Fast clock is in this slot                           */
#define PRG_SLOT          0x7c      /* This slot communicates with the programming track    */

/* values and macros to decode programming messages */
#define PCMD_RW           0x40      /* 1 = write, 0 = read                                  */
#define PCMD_BYTE_MODE    0x20      /* 1 = byte operation, 0 = bit operation (if possible)  */
#define PCMD_TY1          0x10      /* TY1 Programming type select bit                      */
#define PCMD_TY0          0x08      /* TY0 Programming type select bit                      */
#define PCMD_OPS_MODE     0x04      /* 1 = Ops mode, 0 = Service Mode                       */
#define PCMD_RSVRD1       0x02      /* reserved                                             */
#define PCMD_RSVRD0       0x01      /* reserved                                             */

/* programming mode mask */
#define PCMD_MODE_MASK      (PCMD_BYTE_MODE | PCMD_OPS_MODE | PCMD_TY1 | PCMD_TY0)

/*
 *  programming modes
 */
/* Paged mode  byte R/W on Service Track */
#define PAGED_ON_SRVC_TRK       (PCMD_BYTE_MODE)

/* Direct mode byte R/W on Service Track */
#define DIR_BYTE_ON_SRVC_TRK    (PCMD_BYTE_MODE | PCMD_TY0)

/* Direct mode bit  R/W on Service Track */
#define DIR_BIT_ON_SRVC_TRK     (PCMD_TY0)

/* Physical Register byte R/W on Service Track */
#define REG_BYTE_RW_ON_SRVC_TRK (PCMD_TY1)

/* Service Track Reserved function */
#define SRVC_TRK_RESERVED       (PCMD_TY1 | PCMD_TY0)

/* Ops mode byte program - no feedback */
#define OPS_BYTE_NO_FEEDBACK    (PCMD_BYTE_MODE | PCMD_OPS_MODE)

/* Ops mode byte program - feedback */
#define OPS_BYTE_FEEDBACK       (OPS_BYTE_NO_FEEDBACK | PCMD_TY0)

/* Ops mode bit program - no feedback */
#define OPS_BIT_NO_FEEDBACK     (PCMD_OPS_MODE)

/* Ops mode bit program - feedback */
#define OPS_BIT_FEEDBACK        (OPS_BIT_NO_FEEDBACK | PCMD_TY0)

/* Programmer Status error flags */
#define PSTAT_USER_ABORTED  0x08    /* User aborted this command */
#define PSTAT_READ_FAIL     0x04    /* Failed to detect Read Compare Acknowledge from decoder */
#define PSTAT_WRITE_FAIL    0x02    /* No Write acknowledge from decoder                      */
#define PSTAT_NO_DECODER    0x01    /* Service mode programming track empty                   */

/* bit masks for CVH */
#define CVH_CV8_CV9         0x30    /* mask for CV# bits 8 and 9    */
#define CVH_CV7             0x01    /* mask for CV# bit 7           */
#define CVH_D7              0x02    /* MSbit for data value         */

/* build data byte from programmer message */
#define PROG_DATA(ptr)      (((ptr->cvh & CVH_D7) << 6) | (ptr->data7 & 0x7f))

/* build CV # from programmer message */
#define PROG_CV_NUM(ptr)    (((((ptr->cvh & CVH_CV8_CV9) >> 3) | (ptr->cvh & CVH_CV7)) * 128)   \
                            + (ptr->cvl & 0x7f))

#ifdef __BORLANDC__
#pragma pack( push, 1 )
#endif

/* Locomotive Address Message */
typedef struct locoadr_t {
    uint8_t command;
    uint8_t adr_hi;        /* ms seven bits of loco address (D6-D0)                */
    uint8_t adr_lo;        /* ls seven bits of loco address (D6-D0)                */
    uint8_t chksum;        /* exclusive-or checksum for the message                */
} locoAdrMsg;

/* Switch with/without Acknowledge */
typedef struct switchack_t {
    uint8_t command;
    uint8_t sw1;           /* ls seven bits of switch address (D6-D0)              */
    uint8_t sw2;           /* ms four  bits of switch address (D3-D0)              */
                        /* and control bits                                     */
    uint8_t chksum;        /* exclusive-or checksum for the message                */
} switchAckMsg, switchReqMsg;

/* Slot data request */
typedef struct slotreq_t {
    uint8_t command;
    uint8_t slot;          /* slot number for this request                         */
    uint8_t pad;           /* should be zero                                       */
    uint8_t chksum;        /* exclusive-or checksum for the message                */
} slotReqMsg;

/* Move/Link Slot Message */
typedef struct slotmove_t {
    uint8_t command;
    uint8_t src;           /* source slot number for the move/link                 */
    uint8_t dest;          /* destination slot for the move/link                   */
    uint8_t chksum;        /* exclusive-or checksum for the message                */
} slotMoveMsg, slotLinkMsg;

/* Consist Function Message */
typedef struct consistfunc_t {
    uint8_t command;
    uint8_t slot;          /* slot number for this request                         */
    uint8_t dirf;          /* direction and light function bits                    */
    uint8_t chksum;        /* exclusive-or checksum for the message                */
} consistFuncMsg;

/* Write slot status message */
typedef struct slotstat_t {
    uint8_t command;
    uint8_t slot;          /* slot number for this request                         */
    uint8_t stat;          /* status to be written                                 */
    uint8_t chksum;        /* exclusive-or checksum for the message                */
} slotStatusMsg;

/* Long ACK message */
typedef struct longack_t{
    uint8_t command;
    uint8_t opcode;        /* op-code of message getting the response (msb=0)      */
    uint8_t ack1;          /* response code                                        */
    uint8_t chksum;        /* exclusive-or checksum for the message                */
} longAckMsg;

/* Sensor input report */
typedef struct inputrep_t {
    uint8_t command;
    uint8_t in1;           /* first  byte of report                                */
    uint8_t in2;           /* second byte of report                                */
    uint8_t chksum;        /* exclusive-or checksum for the message                */
} inputRepMsg;

/* Turnout sensor state report */
typedef struct swrep_t {
    uint8_t command;
    uint8_t sn1;           /* first  byte of report                                */
    uint8_t sn2;           /* second byte of report                                */
    uint8_t chksum;        /* exclusive-or checksum for the message                */
} swRepMsg;

/* Request Switch function */
typedef struct swreq_t {
    uint8_t command;
    uint8_t sw1;           /* first  byte of request                               */
    uint8_t sw2;           /* second byte of request                               */
    uint8_t chksum;        /* exclusive-or checksum for the message                */
} swReqMsg;

/* Set slot sound functions */
typedef struct locodata_t {
    uint8_t command;
    uint8_t slot;          /* slot number for this request                         */
    uint8_t data;          /* data for request                                     */
    uint8_t chksum;        /* exclusive-or checksum for the message                */
} locoDataMsg;

/* Set slot sound functions */
typedef struct locosnd_t {
    uint8_t command;
    uint8_t slot;          /* slot number for this request                         */
    uint8_t snd;           /* sound/function request                               */
    uint8_t chksum;        /* exclusive-or checksum for the message                */
} locoSndMsg;

/* Set slot direction and F0-F4 functions */
typedef struct locodirf_t {
    uint8_t command;
    uint8_t slot;          /* slot number for this request                         */
    uint8_t dirf;          /* direction & function request                         */
    uint8_t chksum;        /* exclusive-or checksum for the message                */
} locoDirfMsg;

/* Set slot speed functions */
typedef struct locospd_t {
    uint8_t command;
    uint8_t slot;          /* slot number for this request                         */
    uint8_t spd;           /* speed request                                        */
    uint8_t chksum;        /* exclusive-or checksum for the message                */
} locoSpdMsg;

/* Read/Write Slot data messages */
typedef struct rwslotdata_t {
    uint8_t command;
    uint8_t mesg_size;     /* ummmmm, size of the message in bytes?                */
    uint8_t slot;          /* slot number for this request                         */
    uint8_t stat;          /* slot status                                          */
    uint8_t adr;           /* loco address                                         */
    uint8_t spd;           /* command speed                                        */
    uint8_t dirf;          /* direction and F0-F4 bits                             */
    uint8_t trk;           /* track status                                         */
    uint8_t ss2;           /* slot status 2 (tells how to use ID1/ID2 & ADV Consist*/
    uint8_t adr2;          /* loco address high                                    */
    uint8_t snd;           /* Sound 1-4 / F5-F8                                    */
    uint8_t id1;           /* ls 7 bits of ID code                                 */
    uint8_t id2;           /* ms 7 bits of ID code                                 */
    uint8_t chksum;        /* exclusive-or checksum for the message                */
} rwSlotDataMsg;

/* Fast Clock Message */
typedef struct fastclock_t {
    uint8_t command;
    uint8_t mesg_size;     /* ummmmm, size of the message in bytes?                    */
    uint8_t slot;          /* slot number for this request                             */
    uint8_t clk_rate;      /* 0 = Freeze clock, 1 = normal, 10 = 10:1 etc. Max is 0x7f */
    uint8_t frac_minsl;    /* fractional minutes. not for external use.                */
    uint8_t frac_minsh;
    uint8_t mins_60;       /* 256 - minutes   */
    uint8_t track_stat;    /* track status    */
    uint8_t hours_24;      /* 256 - hours     */
    uint8_t days;          /* clock rollovers */
    uint8_t clk_cntrl;     /* bit 6 = 1; data is valid clock info */
                        /*  "  "   0; ignore this reply        */
    uint8_t id1;           /* id1/id2 is device id of last device to set the clock */
    uint8_t id2;           /*  "   "  = zero shows not set has happened            */
    uint8_t chksum;        /* exclusive-or checksum for the message                */
} fastClockMsg;

/* Programmer Task Message (used in Start and Final Reply, both )*/
typedef struct progtask_t {
    uint8_t command;
    uint8_t mesg_size;     /* ummmmm, size of the message in bytes?                    */
    uint8_t slot;          /* slot number for this request - slot 124 is programmer    */
    uint8_t pcmd;          /* programmer command                                       */
    uint8_t pstat;         /* programmer status error flags in reply message           */
    uint8_t hopsa;         /* Ops mode - 7 high address bits of loco to program        */
    uint8_t lopsa;         /* Ops mode - 7 low  address bits of loco to program        */
    uint8_t trk;           /* track status. Note: bit 3 shows if prog track is busy    */
    uint8_t cvh;           /* hi 3 bits of CV# and msb of data7                        */
    uint8_t cvl;           /* lo 7 bits of CV#                                         */
    uint8_t data7;         /* 7 bits of data to program, msb is in cvh above           */
    uint8_t pad2;
    uint8_t pad3;
    uint8_t chksum;        /* exclusive-or checksum for the message                */
} progTaskMsg;

/* peer-peer transfer message */
typedef struct peerxfer_t {
    uint8_t command;
    uint8_t mesg_size;     /* ummmmm, size of the message in bytes?                */
    uint8_t src;           /* source of transfer                                   */
    uint8_t dst_l;         /* ls 7 bits of destination                             */
    uint8_t dst_h;         /* ms 7 bits of destination                             */
    uint8_t pxct1;
    uint8_t d1;            /* data byte 1                                          */
    uint8_t d2;            /* data byte 2                                          */
    uint8_t d3;            /* data byte 3                                          */
    uint8_t d4;            /* data byte 4                                          */
    uint8_t pxct2;
    uint8_t d5;            /* data byte 5                                          */
    uint8_t d6;            /* data byte 6                                          */
    uint8_t d7;            /* data byte 7                                          */
    uint8_t d8;            /* data byte 8                                          */
    uint8_t chksum;        /* exclusive-or checksum for the message                */
} peerXferMsg;

/* send packet immediate message */
typedef struct sendpkt_t {
    uint8_t command;
    uint8_t mesg_size;     /* ummmmm, size of the message in bytes?                */
    uint8_t val7f;         /* fixed value of 0x7f                                  */
    uint8_t reps;          /* repeat count                                         */
    uint8_t dhi;           /* high bits of data bytes                              */
    uint8_t im1;
    uint8_t im2;
    uint8_t im3;
    uint8_t im4;
    uint8_t im5;
    uint8_t chksum;        /* exclusive-or checksum for the message                */
} sendPktMsg;

/* sv message */
typedef struct sv_t {
    uint8_t command;   /* LocoNet Op Code */
    uint8_t mesg_size; /* size of the message in bytes */
    uint8_t src;       /* 8 bit source address */
    uint8_t sv_cmd;    /* SV Command */
    uint8_t sv_type;   /* SV Type = 0x02 for this format */
    uint8_t svx1;      /* <0,0,0,1,D3,D2,D1,D0 of next 4 bytes */
    uint8_t dst_lo;    /* low  8 bits of 16 bit destination address */
    uint8_t dst_hi;    /* high 8 bits of 16 bit destination address */
    uint8_t sv_addl;   /* low  8 bits of 16 bit SV Offset */
    uint8_t sv_addh;   /* low  8 bits of 16 bit SV Offset */
    uint8_t svx2;      /* <0,0,0,1,D3,D2,D1,D0 of next 4 bytes */
    uint8_t d1;        /* SV data byte 1 */
    uint8_t d2;        /* SV data byte 2 */
    uint8_t d3;        /* SV data byte 3 */
    uint8_t d4;        /* SV data byte 4 */
    uint8_t chksum;    /* exclusive-or checksum for the message */
} svMsg;

	// Message structure to determine the size of a message
typedef struct {
    uint8_t command;   /* LocoNet Op Code */
    uint8_t mesg_size; /* size of the message in bytes */
} szMsg;

typedef struct se_t {
    uint8_t command;
    uint8_t mesg_size;     /* ummmmm, size of the message in bytes?                */
    uint8_t addr_h;           /* ms 7 bits of destination                                   */
    uint8_t addr_l;         /* ls 7 bits of destination                             */
    uint8_t cmd;         /* command                             */
    uint8_t se1;		/* data Byte 1 */
    uint8_t se2;            /* data byte 2                                          */
    uint8_t se3;            /* data byte 3                                          */
    uint8_t chksum;        /* exclusive-or checksum for the message                */
} seMsg;

typedef struct
{
	uint8_t command;   // OPC_PEER_XFER for replies, OPC_IMM_PACKET for commands
	uint8_t mesg_size; // 15 bytes
	uint8_t SRC;       // source
	uint8_t DSTL;      // destination, low byte
	uint8_t DSTH;      // destination, high byte
	uint8_t ReqId;     // Request ID, distinguishes commands
	uint8_t PXCT1;     // MSBs of following data
	uint8_t D[7];
} UhlenbrockMsg;

typedef struct
{
	uint8_t command;    // OPC_ANALOGIO
	uint8_t mesg_size;  // 8 bytes
	uint8_t analog_cmd; // <0,WR,aa-aaaa>  a=1: analog read/write
	uint8_t analog_1;   // address low
	uint8_t analog_2;   // address high
	uint8_t analog_3;   // scale/polarity bits (coding????)
	uint8_t analog_4;   // ls 7 bits analog value (ms bits where???)
} AnalogIoMsg;


typedef union {
		locoAdrMsg		la ;
		switchAckMsg		sa ;
		slotReqMsg		sr ;
		slotMoveMsg		sm ;
		consistFuncMsg	        cf ;
		slotStatusMsg 	        ss ;
		longAckMsg	 	lack ;
		inputRepMsg		ir ;
		swRepMsg		srp ;
		swReqMsg		srq ;
		locoDataMsg		ld ;
		locoSndMsg		ls ;
		locoDirfMsg		ldf ;
		locoSpdMsg		lsp ;
		rwSlotDataMsg		sd ;
		fastClockMsg		fc ;
		progTaskMsg		pt ;
		peerXferMsg		px ;
		sendPktMsg		sp ;
		svMsg			sv ;
		szMsg			sz ;
		seMsg			se ;
		UhlenbrockMsg           ub;
		AnalogIoMsg             anio;
		uint8_t			data[16] ;
} lnMsg ;

#ifdef __BORLANDC__
#pragma pack( pop )
#endif

/* loconet opcodes */
#define OPC_BUSY          0x81
#define OPC_GPOFF         0x82
#define OPC_GPON          0x83
#define OPC_IDLE          0x85
#define OPC_LOCO_SPD      0xa0
#define OPC_LOCO_DIRF     0xa1
#define OPC_LOCO_SND      0xa2
#define OPC_SW_REQ        0xb0
#define OPC_SW_REP        0xb1
#define OPC_INPUT_REP     0xb2
#define OPC_UNKNOWN       0xb3
#define OPC_LONG_ACK      0xb4
#define OPC_SLOT_STAT1    0xb5
#define OPC_CONSIST_FUNC  0xb6
#define OPC_UNLINK_SLOTS  0xb8
#define OPC_LINK_SLOTS    0xb9
#define OPC_MOVE_SLOTS    0xba
#define OPC_RQ_SL_DATA    0xbb
#define OPC_SW_STATE      0xbc
#define OPC_SW_ACK        0xbd
#define OPC_LOCO_ADR      0xbf
#define OPC_PEER_XFER     0xe5
#define OPC_SL_RD_DATA    0xe7
#define OPC_IMM_PACKET    0xed
#define OPC_IMM_PACKET_2  0xee
#define OPC_WR_SL_DATA    0xef
#define OPC_MASK          0x7f  /* mask for acknowledge opcodes */

#if defined (__cplusplus)
}
#endif

#endif
