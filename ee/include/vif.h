/**
 * @file
 * VIF Addresses and DMA Tags.
 */

#ifndef __VIF_H__
#define __VIF_H__

#include <tamtypes.h>

#define VU0_MEM0_START			0x11000000
#define VU0_MICROMEM0_START	0x11004000

#define VU1_MEM1_START			0x11008000
#define VU1_MICROMEM1_START	0x1100C000

#define VIF0_FIFO				0x10004000
#define VIF1_FIFO				0x10005000

#define VIF0_STAT		*(volatile u32 *)0x10003800
#define VIF0_FBRST		*(volatile u32 *)0x10003810
#define VIF0_ERR		*(volatile u32 *)0x10003820
#define VIF0_MARK		*(volatile u32 *)0x10003830
#define VIF0_CYCLE		*(volatile u32 *)0x10003840
#define VIF0_MODE		*(volatile u32 *)0x10003850
#define VIF0_NUM		*(volatile u32 *)0x10003860
#define VIF0_MASK		*(volatile u32 *)0x10003870
#define VIF0_CODE		*(volatile u32 *)0x10003880
#define VIF0_ITOPS		*(volatile u32 *)0x10003890
#define VIF0_ITOP		*(volatile u32 *)0x100038d0
#define VIF0_TOP		*(volatile u32 *)0x100038e0
#define VIF0_R0			*(volatile u32 *)0x10003900
#define VIF0_R1			*(volatile u32 *)0x10003910
#define VIF0_R2			*(volatile u32 *)0x10003920
#define VIF0_R3			*(volatile u32 *)0x10003930
#define VIF0_C0			*(volatile u32 *)0x10003940
#define VIF0_C1			*(volatile u32 *)0x10003950
#define VIF0_C2			*(volatile u32 *)0x10003960
#define VIF0_C3			*(volatile u32 *)0x10003970

#define VIF1_STAT		*(volatile u32 *)0x10003c00
#define VIF1_FBRST		*(volatile u32 *)0x10003c10
#define VIF1_ERR		*(volatile u32 *)0x10003c20
#define VIF1_MARK		*(volatile u32 *)0x10003c30
#define VIF1_CYCLE		*(volatile u32 *)0x10003c40
#define VIF1_MODE		*(volatile u32 *)0x10003c50
#define VIF1_NUM		*(volatile u32 *)0x10003c60
#define VIF1_MASK		*(volatile u32 *)0x10003c70
#define VIF1_CODE		*(volatile u32 *)0x10003c80
#define VIF1_ITOPS		*(volatile u32 *)0x10003c90
#define VIF1_BASE		*(volatile u32 *)0x10003ca0
#define VIF1_OFST		*(volatile u32 *)0x10003cb0
#define VIF1_TOPS		*(volatile u32 *)0x10003cc0
#define VIF1_ITOP		*(volatile u32 *)0x10003cd0
#define VIF1_TOP		*(volatile u32 *)0x10003ce0
#define VIF1_R0			*(volatile u32 *)0x10003d00
#define VIF1_R1			*(volatile u32 *)0x10003d10
#define VIF1_R2			*(volatile u32 *)0x10003d20
#define VIF1_R3			*(volatile u32 *)0x10003d30
#define VIF1_C0			*(volatile u32 *)0x10003d40
#define VIF1_C1			*(volatile u32 *)0x10003d50
#define VIF1_C2			*(volatile u32 *)0x10003d60
#define VIF1_C3			*(volatile u32 *)0x10003d70

#define VIF_SET_FBRST(RST,FBK,STP,STC) \
	(u32)((RST) & 0x00000001) << 0 | (u32)((FBK) & 0x00000001) << 1 | \
	(u32)((STP) & 0x00000001) << 2 | (u32)((STC) & 0x00000001) << 3

#define VIF_SET_ERR(MII,ME0,ME1) \
	(u32)((MII) & 0x00000001) << 0 | (u32)((ME0) & 0x00000001) << 1 | \
	(u32)((ME1) & 0x00000001) << 2

#define VIF_SET_MARK(MARK) \
	(u32)((MARK) & 0x0000FFFF) << 0

typedef struct {
	u32 vps:2;
	u32 vew:1;
	u32 vgw:1; //vif1
	u32 pad0:2;
	u32 mrk:1;
	u32 dbf:1; //vif1
	u32 vss:1;
	u32 vfs:1;
	u32 vis:1;
	u32 irq:1;
	u32 er0:1;
	u32 er1:1;
	u32 pad1:9;
	u32 fdr:1; //vif1
	u32 fqc:5;
	u32 pad2:3;
} __attribute__((packed)) VIFSTAT;

typedef struct {
	u32 cl:8;
	u32 wl:8;
	u32 pad0:16;
} __attribute__((packed)) VIFCYCLE;

typedef struct {
	u32 mode:2;
	u32 pad0:30;
} __attribute__((packed)) VIFMODE;

typedef struct {
	u32 m0:2;
	u32 m1:2;
	u32 m2:2;
	u32 m3:2;
	u32 m4:2;
	u32 m5:2;
	u32 m6:2;
	u32 m7:2;
	u32 m8:2;
	u32 m9:2;
	u32 m10:2;
	u32 m11:2;
	u32 m12:2;
	u32 m13:2;
	u32 m14:2;
	u32 m15:2;
} __attribute__((packed)) VIFMASK;

typedef struct {
	u32 imdt:16;
	u32 num:8;
	u32 cmd:8;
} __attribute__((packed)) VIFCODE;

typedef struct {
	u32 itops:10;
	u32 pad0:22;
} __attribute__((packed)) VIFITOPS;

typedef struct {
	u32 base:10;
	u32 pad0:22;
} __attribute__((packed)) VIF1BASE; //vif1

typedef struct {
	u32 offset:10;
	u32 pad0:22;
} __attribute__((packed)) VIF1OFST; //vif1

typedef struct {
	u32 tops:10;
	u32 pad0:22;
} __attribute__((packed)) VIF1TOPS; //vif1

typedef struct {
	u32 itop:10;
	u32 pad0:22;
} __attribute__((packed)) VIFITOP;

typedef struct {
	u32 top:10;
	u32 pad0:22;
} __attribute__((packed)) VIFTOP;

typedef struct {
	u32 row;
} VIFR;

typedef struct {
	u32 column;
} VIFC;

// VIF DMA Tags
/** No Operation */
#define VIF_CMD_NOP			0x00
/** Sets CYCLE register */
#define VIF_CMD_STCYCL		0x01
/** Sets OFFSET register (VIF1) */
#define VIF_CMD_OFFSET		0x02
/** Sets BASE register (VIF1) */
#define VIF_CMD_BASE		0x03
/** Sets ITOPS register */
#define VIF_CMD_ITOP		0x04
/** Sets MODE register */
#define VIF_CMD_STMOD		0x05
/** Mask GIF transfer (VIF1) */
#define VIF_CMD_MSKPATH3	0x06
/** Sets Mark register */
#define VIF_CMD_MARK		0x07
/** Wait for end of microprogram */
#define VIF_CMD_FLUSHE		0x10
/** Wait for end of microprogram & Path 1/2 GIF xfer (VIF1) */
#define VIF_CMD_FLUSH		0x11
/** Wait for end of microprogram & all Path GIF xfer (VIF1) */
#define VIF_CMD_FLUSHA		0x13
/** Activate microprogram */
#define VIF_CMD_MSCAL		0x14
/** Execute microrprogram continuously */
#define VIF_CMD_MSCNT		0x17
/** Activate microprogram (VIF1) */
#define VIF_CMD_MSCALF		0x15
/** Sets MASK register */
#define VIF_CMD_STMASK		0x20
/** Sets ROW register */
#define VIF_CMD_STROW		0x30
/** Sets COL register */
#define VIF_CMD_STCOL		0x31
/** Load microprogram */
#define VIF_CMD_MPG			0x4A
/** Transfer data to GIF (VIF1) */
#define VIF_CMD_DIRECT		0x50
/** Transfer data to GIF but stall for Path 3 IMAGE mode (VIF1) */
#define VIF_CMD_DIRECTHL	0x51


#define PACK_VIFTAG(Q,W0,W1,W2,W3) \
	Q->sw[0] = (u32)(W0), \
	Q->sw[1] = (u32)(W1), \
	Q->sw[2] = (u32)(W2), \
	Q->sw[3] = (u32)(W3)

/** Transfers data to the VU Mem */
#define VIF_CMD_UNPACK(M,VN,VL) \
	(u32)((VL) & 0x00000003) << 0 | (u32)((VN) & 0x00000003) << 2 | \
	(u32)((M)  & 0x00000001) << 4 | (u32)((3)  & 0x00000003) << 5

#define STCYCL_IMDT(CL,WL) \
	(u32)((CL) & 0x000000FF) << 0 | (u32)((WL) & 0x000000FF) << 8

#define OFFSET_IMDT(OFFSET) \
	(u32)((OFFSET) & 0x000003FF)

#define BASE_IMDT(BASE) \
	(u32)((BASE) & 0x000003FF)

#define ITOP_IMDT(ADDR) \
	(u32)((ADDR) & 0x000003FF)

#define STMOD_IMDT(MODE) \
	(u32)((MODE) & 0x00000003)

#define MSKPATH3_IMDT(MASK) \
	(u32)((MASK) & 0x00000001) << 15

#define MARK_IMDT(MARK) \
	(u32)((MARK) & 0x0000FFFF)

#define MSCAL_IMDT(EXECADDR) \
	(u32)((EXECADDR) & 0x0000FFFF)

#define MSCALF_IMDT(EXECADDR) \
	(u32)((EXECADDR) & 0x0000FFFF)

#define MPG_IMDT(LOADADDR) \
	(u32)((LOADADDR) & 0x0000FFFF)

#define MPG_NUM(SIZE) \
	(u32)((SIZE) & 0x000000FF) << 16

#define DIRECT_IMDT(SIZE) \
	(u32)((SIZE) & 0x0000FFFF)

#define DIRECTHL_IMDT(EXECADDR) \
	(u32)((EXECADDR) & 0x0000FFFF)

#define UNPACK_IMDT(ADDR,USN,FLG) \
	(u32)((ADDR) & 0x000003FF) << 0  | (u32)((USN) & 0x00000001) << 14 | \
	(u32)((FLG)  & 0x00000001) << 15

#define UNPACK_NUM(SIZE) \
	(u32)((SIZE) & 0x000000FF) << 16

#define VIF_CODE(IMDT,NUM,CMD,IRQ) \
	(u32)((IMDT) & 0x0000FFFF) << 0  | (u32)((NUM)  & 0x000000FF) << 16 | \
	(u32)((CMD)  & 0x000000FF) << 24 | (u32)((IRQ)  & 0x00000001) << 31

#endif /* __VIF_H__ */
