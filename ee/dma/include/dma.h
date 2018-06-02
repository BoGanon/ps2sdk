/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2005 Naomi Peori <naomi@peori.ca>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * DMA channel utilities
 */

#ifndef __DMA_H__
#define __DMA_H__

#include <tamtypes.h>

/** DMA Control Register */
#define DMA_REG_CTRL		(volatile u32 *)0x1000E000
/** Interrupt Status Register */
#define DMA_REG_STAT		(volatile u32 *)0x1000E010
/** Priority Control Register */
#define DMA_REG_PCR		(volatile u32 *)0x1000E020
/** Interleave Size Register */
#define DMA_REG_SQWC		(volatile u32 *)0x1000E030
/** Ring Buffer Size Register */
#define DMA_REG_RBSR		(volatile u32 *)0x1000E040
/** Ring Buffer Address Register */
#define DMA_REG_RBOR		(volatile u32 *)0x1000E050
/** Stall Address Register */
#define DMA_REG_STADR		(volatile u32 *)0x1000E060
/** DMA Hold State Register */
#define DMA_REG_ENABLER		(volatile u32 *)0x1000F520
/** DMA Hold Control Register */
#define DMA_REG_ENABLEW		(volatile u32 *)0x1000F590

/** Enable DMA Controller */
#define DMAE_DISABLE 0
#define DMAE_ENABLE  1

/** Enable Cycle Stealing */
#define RELE_OFF 0
#define RELE_ON  1

/** fifo drain? */
#define MFD_OFF  0
#define MFD_RES  1
#define MFD_VIF  2
#define MFD_GIF  3

/** stall source? */
#define STS_UNSPEC 0
#define STS_SIF    1
#define STS_SPR    2
#define STS_IPU    3

/** stall drain? */
#define STD_OFF 0
#define STD_VIF 1
#define STD_GIF 2
#define STD_SIF 3

/** Cycles to release control */
#define RCYC_8   0
#define RCYC_16  1
#define RCYC_32  2
#define RCYC_64  3
#define RCYC_128 4
#define RCYC_256 5

#define DMA_SET_CTRL(DMAE,RELE,MFD,STS,STD,RCYC) \
	(u32)(A   & 0x00000001) <<  0 | (u32)(RELE & 0x00000001) <<  1 | \
	(u32)(MFD & 0x00000003) <<  2 | (u32)(STS  & 0x00000003) <<  4 | \
	(u32)(STD & 0x00000003) <<  6 | (u32)(RCYC & 0x00000007) <<  8

#define DMA_SET_STAT(CIS,SIS,MEIS,BEIS,CIM,SIM,MEIM) \
	(u32)((CIS)  & 0x000003FF) <<  0  | (u32)((SIS)  & 0x00000001) <<  13 | \
	(u32)((MEIS) & 0x00000001) <<  14 | (u32)((BEIS) & 0x00000001) <<  15 | \
	(u32)((CIM)  & 0x000003FF) <<  16 | (u32)((SIM)  & 0x00000001) <<  29 | \
	(u32)((MEIM) & 0x00000001) <<  30

#define DMA_SET_PCR(CPCOND,CDE,PCE) \
	(u32)((CPCOND) & 0x000003FF) <<  0 | (u32)((CDE) & 0x000003FF) << 16 | \
	(u32)((PCE)    & 0x00000001) << 31

#define DMA_SET_SQWC(SQWC,TQWC) \
	(u32)((SQWC) & 0x000000FF) <<  0 | (u32)((TQWC) & 0x000000FF) << 16

#define DMA_SET_RBOR(ADDR)  (u32)((ADDR) & 0x00007FFF)

#define DMA_SET_RBSR(RMSK)  (u32)((RMSK) & 0x00007FFF)

#define DMA_SET_STADR(ADDR) (u32)((ADDR) & 0x00007FFF)

#define DMA_SET_ENABLEW(A)  (u32)((A) & 0x00000001) << 16

#define DMA_SET_ENABLER(A)  (u32)((A) & 0x00000001) << 16

/** Per-dma channel registers */
#define DMA_SET_CHCR(DIR,MODE,ASP,TTE,TIE,STR,TAG) \
	(u32)((DIR) & 0x00000001) <<  0 | (u32)((MODE) & 0x00000003) <<  2 | \
	(u32)((ASP) & 0x00000003) <<  4 | (u32)((TTE ) & 0x00000001) <<  6 | \
	(u32)((TIE) & 0x00000001) <<  7 | (u32)((STR ) & 0x00000001) <<  8 | \
	(u32)((TAG) & 0x0000FFFF) << 16

#define DMA_SET_MADR(ADDR,SPR) \
	(u32)((ADDR) & 0x7FFFFFFF) <<  0 | (u32)((SPR) & 0x00000001) << 31

#define DMA_SET_TADR(ADDR,SPR) \
	(u32)((ADDR) & 0x7FFFFFFF) <<  0 | (u32)((SPR) & 0x00000001) << 31

#define DMA_SET_ASR0(ADDR,SPR) \
	(u32)((ADDR) & 0x7FFFFFFF) <<  0 | (u32)((SPR) & 0x00000001) << 31

#define DMA_SET_ASR1(ADDR,SPR) \
	(u32)((ADDR) & 0x7FFFFFFF) <<  0 | (u32)((SPR) & 0x00000001) << 31

#define DMA_SET_SADR(ADDR) (u32)((ADDR) & 0x00003FFF)

#define DMA_SET_QWC(QWC)   (u32)((QWC)  & 0x0000FFFF)

/** 
 * T   = location of qwords
 * D   = next dmatag to be read
 * QWC = quadword count for dmatag
 */

/** T=QWC D=QWC+1 MADR => STADR */
#define DMA_TAG_CNTS	0x00
/** T=ADDR then END */
#define DMA_TAG_REFE	0x00
/** T=QWC D=QWC+1 */
#define DMA_TAG_CNT	0x01
/** T=QWC D=ADDR */
#define DMA_TAG_NEXT	0x02
/** D=D+1 T=ADDR */
#define DMA_TAG_REF	0x03
/** .. + stall ctrl */
#define DMA_TAG_REFS	0x04
/** T=QWC D=ADDR QWC+1 => ASR0 */
#define DMA_TAG_CALL	0x05
/** T=QWC (ASR0 => D) if !ASR0 then END */
#define DMA_TAG_RET	0x06
/** T=QWC then END */
#define DMA_TAG_END	0x07

#define DMATAG(QWC,PCE,ID,IRQ,ADDR,SPR) \
	(u64)((QWC)  & 0x0000FFFF) <<  0 | (u64)((PCE) & 0x00000003) << 26 | \
	(u64)((ID)   & 0x00000007) << 28 | (u64)((IRQ) & 0x00000001) << 31 | \
	(u64)((ADDR) & 0x7FFFFFFF) << 32 | (u64)((SPR) & 0x00000001) << 63

#define PACK_DMATAG(Q,D0,W2,W3) \
	Q->dw[0] = (u64)(D0), \
	Q->sw[2] = (u32)(W2), \
	Q->sw[3] = (u32)(W3)

/** Insert before qword block, and add dmatag after qword block to continue */
#define DMATAG_CNT(Q,QWC,SPR,W2,W3) \
	Q->dw[0] = DMATAG(QWC,0,DMA_TAG_CNT,0,0,SPR), \
	Q->sw[2] = (u32)(W2), \
	Q->sw[3] = (u32)(W3)

/** Insert before qword block, and add dmatag after qword block to continue */
#define DMATAG_CNTS(Q,QWC,SPR,W2,W3) \
	Q->dw[0] = DMATAG(QWC,0,DMA_TAG_CNTS,0,0,SPR), \
	Q->sw[2] = (u32)(W2), \
	Q->sw[3] = (u32)(W3)

/** Insert before qword block, and add dmatag at addr to continue */
#define DMATAG_NEXT(Q,QWC,ADDR,SPR,W2,W3), \
	Q->dw[0] = DMATAG(QWC,0,DMA_TAG_NEXT,0,ADDR,SPR), \
	Q->sw[2] = (u32)(W2), \
	Q->sw[3] = (u32)(W3)

/** Insert before final qword block, or by itself to end transfer */
#define DMATAG_END(Q,QWC,SPR,W2,W3) \
	Q->dw[0] = DMATAG(QWC,0,DMA_TAG_END,0,0,SPR), \
	Q->sw[2] = (u32)(W2), \
	Q->sw[3] = (u32)(W3)

/** Insert before qword block, and add dmatag at qword after qwc to be saved (up to 2) */
#define DMATAG_CALL(Q,QWC,ADDR,SPR,W2,W3) \
	Q->dw[0] = DMATAG(QWC,0,DMA_TAG_CALL,0,ADDR,SPR), \
	Q->sw[2] = (u32)(W2), \
	Q->sw[3] = (u32)(W3)

/** Insert before qword block, and continues with saved dmatag */
#define DMATAG_RET(Q,QWC,SPR,W2,W3) \
	Q->dw[0] = DMATAG(QWC,0,DMA_TAG_RET,0,0,SPR), \
	Q->sw[2] = (u32)(W2), \
	Q->sw[3] = (u32)(W3)

/** Insert anywhere, qwc at addr is sent, reads next qword to continue */
#define DMATAG_REF(Q,QWC,ADDR,SPR,W2,W3) \
	Q->dw[0] = DMATAG(QWC,0,DMA_TAG_REF,0,ADDR,SPR), \
	Q->sw[2] = (u32)(W2), \
	Q->sw[3] = (u32)(W3)

/** Insert anywhere, qwc at addr is sent, reads next qword to continue */
#define DMATAG_REFS(Q,QWC,ADDR,SPR,W2,W3) \
	Q->dw[0] = DMATAG(QWC,0,DMA_TAG_REFS,0,ADDR,SPR), \
	Q->sw[2] = (u32)(W2), \
	Q->sw[3] = (u32)(W3)

/** Insert anywhere, qwc at addr is sent, then ends transfer */
#define DMATAG_REFE(Q,QWC,ADDR,SPR,W2,W3) \
	Q->dw[0] = DMATAG(qwc, 0, DMA_TAG_REFE, 0, addr, spr), \
	Q->sw[2] = (u32)(W2), \
	Q->sw[3] = (u32)(W3)

#define DMA_CHANNEL_VIF0     0x00
#define DMA_CHANNEL_VIF1     0x01
#define DMA_CHANNEL_GIF      0x02
#define DMA_CHANNEL_fromIPU  0x03
#define DMA_CHANNEL_toIPU    0x04
#define DMA_CHANNEL_fromSIF0 0x05
#define DMA_CHANNEL_toSIF1   0x06
#define DMA_CHANNEL_SIF2     0x07
#define DMA_CHANNEL_fromSPR  0x08
#define DMA_CHANNEL_toSPR    0x09

#define DMA_FLAG_TRANSFERTAG   0x01
#define DMA_FLAG_INTERRUPTSAFE 0x02

#ifdef __cplusplus
extern "C" {
#endif

int dma_reset(void);

/** Initializes the specified dma channel. */
int dma_channel_initialize(int channel, void *handler, int flags);

/** Enables dma fast waits for that channel */
void dma_channel_fast_waits(int channel);

/** Waits until channel is usable based on coprocessor status */
void dma_wait_fast(void);

/** Wait until the specified dma channel is ready. */
int dma_channel_wait(int channel, int timeout);

/** Send a dmachain to the specified dma channel. */
int dma_channel_send_chain(int channel, void *data, int qwc, int flags, int spr);

/** Send a ucab dmachain to the specified dma channel. */
int dma_channel_send_chain_ucab(int channel, void *data, int qwc, int flags);

/** Send data to the specified dma channel. */
int dma_channel_send_normal(int channel, void *data, int qwc, int flags, int spr);

/** Send ucab data to the specified dma channel. */
int dma_channel_send_normal_ucab(int channel, void *data, int qwc, int flags);

/** Receive data from the specified dma channel. */
int dma_channel_receive_normal(int channel, void *data, int data_size, int flags, int spr);

/** Receive data from the specified dma channel. */
int dma_channel_receive_chain(int channel, void *data, int data_size, int flags, int spr);

/** Shut down the specified dma channel. */
int dma_channel_shutdown(int channel, int flags);

#ifdef __cplusplus
}
#endif

#endif /* __DMA_H__ */
