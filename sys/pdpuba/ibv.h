

/*
 * IBS Register bits, LSB first.
 *
 *  0 TCS (R/W) Take Control Synchronously.
 *  1 EOP (R/W) End or Poll
 *  2 REM (R/W) Remote On
 *  3 IBC (R/W) Interface Bus Clear
 *  4 LON (R/W) Listener On
 *  5 TON (R/W) Talker On
 *  6 IE  (R/W) Interrupt Enable
 *  7 ACC (R/W) Accept Data
 *  8 LNR (R) Listener Ready
 *  9 TKR (R) Talker Reader
 * 10 CMD (R) Command Done
 * 11 n/a always zero
 * 12 n/a always zero
 * 13 ER1 (R) Error 1
 * 14 ER2 (R) Error 2
 * 15 SRQ (R/W*) Service Request
 *
 *
 * IBD Register Bits
 *
 * {0:7} DIO (R/W) Data Input/Output
 *  8    DAC (R) Not Data Accepted (NDAC) Inverted
 *  9    DAV (R)
 * 10    RFD (R) Not Ready for Data (NRFD) Inverted
 * 11    SRQ (R) Service Request
 * 12    REN (R) Remote Enable
 * 13    IFC (R) Interface Clear
 * 14    ATN (R) Attention
 * 15    EOI (R) End or Identify
 *
 * (R) = Read Only
 * (R/W) = Read and Write
 * (R/W*) = Read, Write if ER1 is set
 */


struct ibvdevice {
  union {
    short ibs; /* Control status register */
    struct {
      char ibsl; /* low byte for r/w */
      char ibsh; /* high byte for ro (except for b15) */
    } s;
  } un1;
#define ibvcsr un1.ibs
#define ibvcsrl un1.s.ibsl
#define ibvcsrh un1.s.ibsh
  union {
    short ibd; /* Data register */
    struct {
      char ibd_io; /* i/o data (low byte) */
      char ibd_status; /* data bus status (high byte) */
    } s;
  } un2;
#define ibvd un2.ibd
#define ibvio un2.s.ibd_io
#define ibvds un2.s.ibd_status
};


/* Status Bits */
#define IBVS_TCS  0000001 /* (R/W) Take Control Synchronously */
#define IBVS_EOP  0000002 /* (R/W) End or Poll */
#define IBVS_REM  0000004 /* (R/W) Remote On */
#define IBVS_IBC  0000010 /* (R/W) Interface Bus Clear */
#define IBVS_LON  0000020 /* (R/W) Listener On */
#define IBVS_TON  0000040 /* (R/W) Talker On */
#define IBVS_IE   0000100 /* (R/W) Interrupt Enable */
#define IBVS_ACC  0000200 /* (R/W) Accept Data */
#define IBVS_LNR  0000400 /* (R)   Listener Ready */
#define IBVS_TKR  0001000 /* (R)   Talker Ready */
#define IBVS_CMD  0002000 /* (R)   Command Done */
#define IBVS_ER1  0020000 /* (R)   Error 1 */
#define IBVS_ER2  0040000 /* (R)   Error 2 */
#define IBVS_SRQ  0100000 /* (R/W) Service Request */

/* Data Bits */
#define IBVD_DIO  0000377 /* (R/W) I/O Data */
#define IBVD_DAC  0000400 /* (R)   Not Data Accepted (NDAC Interted) */
#define IBVD_DAV  0001000 /* (R)   ??? */
#define IBVD_RFD  0002000 /* (R)   Not Ready for Data (NRFD Inverted) */
#define IBVD_SRQ  0004000 /* (R)   Service Request */
#define IBVD_REN  0010000 /* (R)   Remote Enable */
#define IBVD_IFC  0020000 /* (R)   Interface Clear */
#define IBVD_ATN  0040000 /* (R)   Attention */
#define IBVD_EOI  0100000 /* (R)   End or Identify */

/* Messages */
#define IBV_MSG_LISTEN    040 /* 0 1 L L L L L */
#define IBV_MSG_UNLISTEN  077
#define IBV_MSG_TALK	 0100 /* 1 0 T T T T T */
#define IBV_MSG_UNTALK   0137

