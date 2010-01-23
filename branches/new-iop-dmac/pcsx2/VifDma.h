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
#ifndef __VIFDMA_H__
#define __VIFDMA_H__

struct vifCode {
   u32 addr;
   u32 size;
   u32 cmd;
   u16 wl;
   u16 cl;
};

// NOTE, if debugging vif stalls, use sega classics, spyro, gt4, and taito
struct vifStruct {
	vifCode tag;
	int cmd;
	int irq;
	int cl;
	int qwcalign;
	u8 usn;
	
	bool done;
	bool vifstalled;
	bool stallontag;
	
	u8 irqoffset; // 32bit offset where next vif code is
	u32 savedtag; // need this for backwards compat with save states
	u32 vifpacketsize;
	u8 inprogress;
	u8 dmamode;
};

extern vifStruct vif0, vif1;
extern u8 schedulepath3msk;
static const int VifCycleVoodoo = 4;

extern void vif0Init();
extern void  vif0Interrupt();
extern void vif0Write32(u32 mem, u32 value);
extern void vif0Reset();

extern void  vif1Interrupt();
extern void vif1Init();
extern void Vif1MskPath3();
extern void vif1Write32(u32 mem, u32 value);
extern void vif1Reset();

__forceinline static int _limit(int a, int max)
{
	return ((a > max) ? max : a);
}

#endif
