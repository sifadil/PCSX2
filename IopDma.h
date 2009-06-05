/*  Pcsx2 - Pc Ps2 Emulator
 *  Copyright (C) 2002-2009  Pcsx2 Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifndef __PSXDMA_H__
#define __PSXDMA_H__

#include "PS2Edefs.h"

//#define ENABLE_NEW_IOPDMA

#ifdef ENABLE_NEW_IOPDMA

typedef s32(* DmaHandler)(s32 channel, u32* data, u32 bytesLeft, u32* bytesProcessed);
typedef void (* DmaIHandler)(s32 channel);

struct DmaHandlerInfo
{
	DmaHandler  Read;
	DmaHandler  Write;
	DmaIHandler Interrupt;
};

struct DmaStatusInfo
{
	u32 Control;
	u32 Width;		// bytes/word, for timing purposes
	u32 MemAddr;
	u32 ByteCount;
	s32 Target;
};

// FIXME: Dummy constants, to be "filled in" with proper values later
#define DMA_CTRL_ACTIVE		0x01000000
#define DMA_CTRL_DIRECTION	0x00000001

#define DMA_CHANNEL_MAX		16 /* ? */

// WARNING: CALLER ****[MUST]**** CALL IopDmaUpdate RIGHT AFTER THIS!
void IopDmaStart(int channel, u32 chcr, u32 madr, u32 bcr);
void IopDmaUpdate(u32 elapsed);

// external dma handlers
extern s32 cdvdDmaRead(s32 channel, u32* data, u32 bytesLeft, u32* bytesProcessed);
extern void cdvdDmaInterrupt(s32 channel);

//#else
#endif

enum IopDmaType
{
	IopDma_Cdvd = 5		// General Cdvd commands (Seek, Standby, Break, etc)
,	IopDma_SIF0 = 9
,	IopDma_SIF1 = 10
,	IopDma_Dma11 = 11
,	IopDma_Dma12 = 12
,	IopDma_SIO = 16
,	IopDma_Cdrom = 17
,	IopDma_CdromRead = 18
,	IopDma_CdvdRead = 19
,	IopDma_DEV9 = 20
,	IopDma_USB = 21
};

//extern void PSX_INT( IopEventId n, s32 ecycle );

extern void psxDma2(u32 madr, u32 bcr, u32 chcr);
extern void psxDma3(u32 madr, u32 bcr, u32 chcr);
extern void psxDma4(u32 madr, u32 bcr, u32 chcr);
extern void psxDma6(u32 madr, u32 bcr, u32 chcr);
extern void psxDma7(u32 madr, u32 bcr, u32 chcr);
extern void psxDma8(u32 madr, u32 bcr, u32 chcr);
extern void psxDma9(u32 madr, u32 bcr, u32 chcr);
extern void psxDma10(u32 madr, u32 bcr, u32 chcr);

extern int  psxDma4Interrupt();
extern int  psxDma7Interrupt();
extern void dev9Interrupt();
extern void dev9Irq(int cycles);
extern void usbInterrupt();
extern void usbIrq(int cycles);
extern void fwIrq();
extern void spu2Irq();

extern void  sif1Interrupt();
extern void  sif0Interrupt();


extern void iopIntcIrq( uint irqType );
extern void iopTestIntc();

extern DEV9handler dev9Handler;
extern USBhandler usbHandler;

#endif /* __PSXDMA_H__ */
