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
 
#ifndef __VIFDMA_INTERNAL_H__
#define __VIFDMA_INTERNAL_H__

// Generic constants
static const unsigned int VIF0intc = 4;
static const unsigned int VIF1intc = 5;

typedef void (__fastcall *UNPACKFUNCTYPE)(u32 *dest, u32 *data, int size);
typedef int (*UNPACKPARTFUNCTYPESSE)(u32 *dest, u32 *data, int size);

struct VIFUnpackFuncTable
{
	UNPACKFUNCTYPE       funcU;
	UNPACKFUNCTYPE       funcS;

	u32 bsize; // currently unused
	u32 dsize; // byte size of one channel
	u32 gsize; // size of data in bytes used for each write cycle
	u32 qsize; // used for unpack parts, num of vectors that
	// will be decompressed from data for 1 cycle
};

extern int g_vifCycles;
extern u8 s_maskwrite[256];
extern const VIFUnpackFuncTable VIFfuncTable[16];
extern __aligned16 u32 g_vif0Masks[64], g_vif1Masks[64];
extern u32 g_vif0HasMask3[4], g_vif1HasMask3[4];

template<const u32 VIFdmanum> void ProcessMemSkip(u32 size, u32 unpackType);
template<const u32 VIFdmanum> u32 VIFalign(u32 *data, vifCode *v, u32 size);
template<const u32 VIFdmanum> void VIFunpack(u32 *data, vifCode *v, u32 size);
template <const u32 VIFdmanum> void vuExecMicro(u32 addr);
extern __forceinline void vif0FLUSH();
extern __forceinline void vif1FLUSH();

__forceinline static u32 vif_size(u8 num)
{
    if (num == 0) 
        return 0x1000;
    else if (num == 1)
        return 0x4000;
    
    return 0;
}

#endif
