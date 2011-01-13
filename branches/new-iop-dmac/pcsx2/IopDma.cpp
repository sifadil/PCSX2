/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2009  PCSX2 Dev Team
 * 
 *  PCSX2 is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with PCSX2.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include "PrecompiledHeader.h"
#include "IopCommon.h"

#include "Sif.h"

using namespace R3000A;

// Dma0/1   in Mdec.c
// Dma3     in CdRom.c
// Dma8     in PsxSpd.c
// Dma11/12 in PsxSio2.c

bool iopsifbusy[2] = { false, false };
extern bool eesifbusy[2];

#ifndef ENABLE_NEW_IOPDMA_SPU2
static void __fastcall psxDmaGeneric(u32 madr, u32 bcr, u32 chcr, u32 spuCore, _SPU2writeDMA4Mem spu2WriteFunc, _SPU2readDMA4Mem spu2ReadFunc)
{
	const char dmaNum = spuCore ? '7' : '4';

	/*if (chcr & 0x400) DevCon.Status("SPU 2 DMA %c linked list chain mode! chcr = %x madr = %x bcr = %x\n", dmaNum, chcr, madr, bcr);
	if (chcr & 0x40000000) DevCon.Warning("SPU 2 DMA %c Unusual bit set on 'to' direction chcr = %x madr = %x bcr = %x\n", dmaNum, chcr, madr, bcr);
	if ((chcr & 0x1) == 0) DevCon.Status("SPU 2 DMA %c loading from spu2 memory chcr = %x madr = %x bcr = %x\n", dmaNum, chcr, madr, bcr);*/

	const int size = (bcr >> 16) * (bcr & 0xFFFF);

	// Update the spu2 to the current cycle before initiating the DMA

	if (SPU2async)
	{
		SPU2async(psxRegs.cycle - psxCounters[6].sCycleT);
		//Console.Status("cycles sent to SPU2 %x\n", psxRegs.cycle - psxCounters[6].sCycleT);

		psxCounters[6].sCycleT = psxRegs.cycle;
		psxCounters[6].CycleT = size * 3;

		psxNextCounter -= (psxRegs.cycle - psxNextsCounter);
		psxNextsCounter = psxRegs.cycle;
		if (psxCounters[6].CycleT < psxNextCounter)
			psxNextCounter = psxCounters[6].CycleT;

		if((g_psxNextBranchCycle - psxNextsCounter) > (u32)psxNextCounter)
		{
			//DevCon.Warning("SPU2async Setting new counter branch, old %x new %x ((%x - %x = %x) > %x delta)", g_psxNextBranchCycle, psxNextsCounter + psxNextCounter, g_psxNextBranchCycle, psxNextsCounter, (g_psxNextBranchCycle - psxNextsCounter), psxNextCounter);
			g_psxNextBranchCycle = psxNextsCounter + psxNextCounter;
		}
	}
    
	switch (chcr)
	{
		case 0x01000201: //cpu to spu2 transfer
			PSXDMA_LOG("*** DMA %c - mem2spu *** %x addr = %x size = %x", dmaNum, chcr, madr, bcr);
			spu2WriteFunc((u16 *)iopPhysMem(madr), size*2);
			break;

		case 0x01000200: //spu2 to cpu transfer
			PSXDMA_LOG("*** DMA %c - spu2mem *** %x addr = %x size = %x", dmaNum, chcr, madr, bcr);
			spu2ReadFunc((u16 *)iopPhysMem(madr), size*2);
			psxCpu->Clear(spuCore ? HW_DMA7_MADR : HW_DMA4_MADR, size);
			break;

		default:
			Console.Error("*** DMA %c - SPU unknown *** %x addr = %x size = %x", dmaNum, chcr, madr, bcr);
			break;
	}
}
#endif

void psxDma2(u32 madr, u32 bcr, u32 chcr)		// GPU
{
	HW_DMA2_CHCR &= ~0x01000000;
	psxDmaInterrupt(2);
}

/*  psxDma3 is in CdRom.cpp */

#ifndef ENABLE_NEW_IOPDMA_SPU2
void psxDma4(u32 madr, u32 bcr, u32 chcr)		// SPU2's Core 0
{
	psxDmaGeneric(madr, bcr, chcr, 0, SPU2writeDMA4Mem, SPU2readDMA4Mem);
}

int psxDma4Interrupt()
{
	HW_DMA4_CHCR &= ~0x01000000;
	psxDmaInterrupt(4);
	iopIntcIrq(9);
	return 1;
}
#endif

void psxDma6(u32 madr, u32 bcr, u32 chcr)
{
	u32 *mem = (u32 *)iopPhysMem(madr);

	PSXDMA_LOG("*** DMA 6 - OT *** %lx addr = %lx size = %lx", chcr, madr, bcr);

	if (chcr == 0x11000002)
	{
		while (bcr--)
		{
			*mem-- = (madr - 4) & 0xffffff;
			madr -= 4;
		}
		mem++;
		*mem = 0xffffff;
	}
	else
	{
		// Unknown option
		PSXDMA_LOG("*** DMA 6 - OT unknown *** %lx addr = %lx size = %lx", chcr, madr, bcr);
	}
	HW_DMA6_CHCR &= ~0x01000000;
	psxDmaInterrupt(6);
}

#ifndef ENABLE_NEW_IOPDMA_SPU2
void psxDma7(u32 madr, u32 bcr, u32 chcr)		// SPU2's Core 1
{
	psxDmaGeneric(madr, bcr, chcr, 1, SPU2writeDMA7Mem, SPU2readDMA7Mem);
}

int psxDma7Interrupt()
{
	HW_DMA7_CHCR &= ~0x01000000;
	psxDmaInterrupt2(0);
	return 1;

}
#endif

void psxDma8(u32 madr, u32 bcr, u32 chcr)
{
	const int size = (bcr >> 16) * (bcr & 0xFFFF) * 8;

	switch (chcr & 0x01000201)
	{
		case 0x01000201: //cpu to dev9 transfer
			PSXDMA_LOG("*** DMA 8 - DEV9 mem2dev9 *** %lx addr = %lx size = %lx", chcr, madr, bcr);
			DEV9writeDMA8Mem((u32*)iopPhysMem(madr), size);
			break;

		case 0x01000200: //dev9 to cpu transfer
			PSXDMA_LOG("*** DMA 8 - DEV9 dev9mem *** %lx addr = %lx size = %lx", chcr, madr, bcr);
			DEV9readDMA8Mem((u32*)iopPhysMem(madr), size);
			break;

		default:
			PSXDMA_LOG("*** DMA 8 - DEV9 unknown *** %lx addr = %lx size = %lx", chcr, madr, bcr);
			break;
	}
	HW_DMA8_CHCR &= ~0x01000000;
	psxDmaInterrupt2(1);
}

void psxDma9(u32 madr, u32 bcr, u32 chcr)
{
	SIF_LOG("IOP: dmaSIF0 chcr = %lx, madr = %lx, bcr = %lx, tadr = %lx",	chcr, madr, bcr, HW_DMA9_TADR);

	iopsifbusy[0] = true;
	psHu32(SBUS_F240) |= 0x2000;

	if (eesifbusy[0])
	{
		SIF0Dma();
		psHu32(SBUS_F240) &= ~0x20;
		psHu32(SBUS_F240) &= ~0x2000;
	}
}

void psxDma10(u32 madr, u32 bcr, u32 chcr)
{
	SIF_LOG("IOP: dmaSIF1 chcr = %lx, madr = %lx, bcr = %lx",	chcr, madr, bcr);

	iopsifbusy[1] = true;
	psHu32(SBUS_F240) |= 0x4000;

	if (eesifbusy[1])
	{
        XMMRegisters::Freeze();
		SIF1Dma();
		psHu32(SBUS_F240) &= ~0x40;
		psHu32(SBUS_F240) &= ~0x100;
		psHu32(SBUS_F240) &= ~0x4000;
        XMMRegisters::Thaw();
	}
}

/* psxDma11 & psxDma 12 are in IopSio2,cpp, along with the appropriate interrupt functions. */

void dev9Interrupt()
{
	if ((dev9Handler != NULL) && (dev9Handler() != 1)) return;

	iopIntcIrq(13);
	hwIntcIrq(INTC_SBUS);
}

void dev9Irq(int cycles)
{
	PSX_INT(IopEvt_DEV9, cycles);
}

void usbInterrupt()
{
	if (usbHandler != NULL && (usbHandler() != 1)) return;

	iopIntcIrq(22);
	hwIntcIrq(INTC_SBUS);
}

void usbIrq(int cycles)
{
	PSX_INT(IopEvt_USB, cycles);
}

void fwIrq()
{
	iopIntcIrq(24);
	hwIntcIrq(INTC_SBUS);
}

#ifndef ENABLE_NEW_IOPDMA_SPU2
void spu2DMA4Irq()
{
	SPU2interruptDMA4();
	HW_DMA4_CHCR &= ~0x01000000;
	psxDmaInterrupt(4);
}

void spu2DMA7Irq()
{
	SPU2interruptDMA7();
	HW_DMA7_CHCR &= ~0x01000000;
	psxDmaInterrupt2(0);
}
#endif

void spu2Irq()
{
	iopIntcIrq(9);
	hwIntcIrq(INTC_SBUS);
}

void iopIntcIrq(uint irqType)
{
	psxHu32(0x1070) |= 1 << irqType;
	iopTestIntc();
}

//////////////////////////////////////////////////////////////////////////////////////////////
//
// Gigaherz's "Improved DMA Handling" Engine WIP...
//

#ifdef ENABLE_NEW_IOPDMA

s32  spu2DmaRead(s32 channel, u32* data, u32 bytesLeft, u32* bytesProcessed)
{
#ifdef ENABLE_NEW_IOPDMA_SPU2
	return SPU2dmaRead(channel,data,bytesLeft,bytesProcessed);
#else
	return 0;
#endif
}

s32  spu2DmaWrite(s32 channel, u32* data, u32 bytesLeft, u32* bytesProcessed)
{
#ifdef ENABLE_NEW_IOPDMA_SPU2
	return SPU2dmaWrite(channel,data,bytesLeft,bytesProcessed);
#else
	return 0;
#endif
}

void spu2DmaInterrupt(s32 channel)
{
#ifdef ENABLE_NEW_IOPDMA_SPU2
	SPU2dmaInterrupt(channel);
#endif
}

//typedef s32(* DmaHandler)(s32 channel, u32* data, u32 bytesLeft, u32* bytesProcessed);
//typedef void (* DmaIHandler)(s32 channel);

extern s32 sio2DmaRead(s32 channel, u32* data, u32 bytesLeft, u32* bytesProcessed);
extern s32 sio2DmaWrite(s32 channel, u32* data, u32 bytesLeft, u32* bytesProcessed);
extern void sio2DmaInterrupt(s32 channel);

s32 errDmaWrite(s32 channel, u32* data, u32 bytesLeft, u32* bytesProcessed);
s32 errDmaRead(s32 channel, u32* data, u32 bytesLeft, u32* bytesProcessed);

DmaStatusInfo  IopChannels[DMA_CHANNEL_MAX]; // I dont' knwo how many there are, 10?

DmaHandlerInfo IopDmaHandlers[DMA_CHANNEL_MAX] =
{
	// First DMAC, same as PS1
	{"Ps1 Mdec",       0}, //0
	{"Ps1 Mdec",       0}, //1
	{"Ps1 Gpu",        0}, //2
	{"CDVD",           cdvdDmaRead, errDmaWrite,  cdvdDmaInterrupt}, //3:  CDVD
	{"SPU2 Core0",     spu2DmaRead, spu2DmaWrite, spu2DmaInterrupt}, //4:  Spu Core0
	{"?",              0}, //5
	{"OT",             0}, //6: OT?

	// Second DMAC, new in PS2 IOP
	{"SPU2 Core1",     spu2DmaRead, spu2DmaWrite, spu2DmaInterrupt}, //7:  Spu Core1
	{"Dev9",		   0},//dev9DmaRead, dev9DmaWrite, dev9DmaInterrupt}, //8:  Dev9
	{"Sif0",           0},//sif0DmaRead, sif0DmaWrite, sif0DmaInterrupt}, //9:  SIF0
	{"Sif1",           0},//sif1DmaRead, sif1DmaWrite, sif1DmaInterrupt}, //10: SIF1
	{"Sio2 (writes)",  errDmaRead, sio2DmaWrite, sio2DmaInterrupt}, //11: Sio2
	{"Sio2 (reads)",   sio2DmaRead, errDmaWrite, sio2DmaInterrupt}, //12: Sio2
	{"?",              0}, //13
	// if each dmac has 7 channels, the list would end here, but i made it 16 cos I'm not sure :p
	{"?",              0}, //14
	{"?",              0}, //15
};

// Prototypes. To be implemented later (or in other parts of the emulator)
void SetDmaUpdateTarget(u32 delay)
{
	psxCounters[8].CycleT = delay;
	if (delay < psxNextCounter)
		psxNextCounter = delay;
}

void RaiseDmaIrq(u32 channel)
{
	if(channel<7)
		psxDmaInterrupt(channel);
	else
		psxDmaInterrupt2(channel-7);
}

void IopDmaStart(int channel, u32 chcr, u32 madr, u32 bcr)
{
	// I dont' really understand this, but it's used above. Is this BYTES OR WHAT?
	int size = 4* (bcr >> 16) * (bcr & 0xFFFF);

	IopChannels[channel].Control = chcr | DMA_CTRL_ACTIVE;
	IopChannels[channel].MemAddr = madr;
	IopChannels[channel].ByteCount = size;
	IopChannels[channel].Target=0;

	//SetDmaUpdateTarget(1);
	{
		const s32 difference = psxRegs.cycle - psxCounters[8].sCycleT;

		psxCounters[8].sCycleT = psxRegs.cycle;
		psxCounters[8].CycleT = psxCounters[8].rate;
		IopDmaUpdate(difference);

		s32 c = psxCounters[8].CycleT;
		if (c < psxNextCounter) psxNextCounter = c;
	}
}

void IopDmaUpdate(u32 elapsed)
{
	s32 MinDelay=0;
	
	do {
		MinDelay = 0x7FFFFFFF;

		for (int i = 0;i < DMA_CHANNEL_MAX;i++)
		{
			DmaStatusInfo *ch = IopChannels + i;

			if (ch->Control&DMA_CTRL_ACTIVE)
			{
				ch->Target -= elapsed;
				if (ch->Target <= 0)
				{
					if (ch->ByteCount <= 0)
					{
						ch->Target = 0x7fffffff;

						ch->Control &= ~DMA_CTRL_ACTIVE;
						RaiseDmaIrq(i);
						IopDmaHandlers[i].Interrupt(i);
					}
					else
					{
						DmaHandler handler = (ch->Control & DMA_CTRL_DIRECTION) ? IopDmaHandlers[i].Write : IopDmaHandlers[i].Read;

						u32 BCount = 0;
						s32 Target = (handler) ? handler(i, (u32*)iopPhysMem(ch->MemAddr), ch->ByteCount, &BCount) : 0;

						if(BCount>0)
						{
							psxCpu->Clear(ch->MemAddr, BCount/4);
						}

						int TTarget = 100;
						if (Target < 0)
						{
							// TODO: ... What to do if the handler gives an error code? :P
						}
						else if (BCount > 0)
						{
							ch->MemAddr   += BCount;
							ch->ByteCount -= BCount;

							TTarget = BCount/4; // / ch->Width;
						}

						if (Target != 0) TTarget = Target;

						ch->Target += TTarget;
						
						TTarget = ch->Target;
						if(TTarget < 0)
							TTarget = 0;

						if (TTarget<MinDelay)
							MinDelay = TTarget;
					}
				}
				else
				{
					int TTarget = ch->Target;
					if (TTarget<MinDelay)
						MinDelay = TTarget;
				}
			}
		}
		elapsed=0;
	}
	while(MinDelay <= 0);

	if(MinDelay<0x7FFFFFFF)
		SetDmaUpdateTarget(MinDelay);
	else
		SetDmaUpdateTarget(10000);
}

s32 errDmaRead(s32 channel, u32* data, u32 bytesLeft, u32* bytesProcessed)
{
	PSXDMA_LOG("ERROR: Tried to read using DMA %d (%s). Ignoring.", 0, channel, IopDmaHandlers[channel].Name);

	*bytesProcessed = bytesLeft;
	return 0;
}

s32 errDmaWrite(s32 channel, u32* data, u32 bytesLeft, u32* bytesProcessed)
{
	PSXDMA_LOG("ERROR: Tried to write using DMA %d (%s). Ignoring.", 0, channel, IopDmaHandlers[channel].Name);

	*bytesProcessed = bytesLeft;
	return 0;
}


#endif