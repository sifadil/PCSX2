/*  Pcsx2 - Pc Ps2 Emulator
 *  Copyright (C) 2002-2003  Pcsx2 Team
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __VUFLAGS_H__
#define __VUFLAGS_H__

#include "VU.h"

void vuUpdateDI(VURegs * VU);
u32  VU_MACx_UPDATE(VURegs * VU, float x);
u32  VU_MACy_UPDATE(VURegs * VU, float y);
u32  VU_MACz_UPDATE(VURegs * VU, float z);
u32  VU_MACw_UPDATE(VURegs * VU, float w);
void VU_MACx_CLEAR(VURegs * VU);
void VU_MACy_CLEAR(VURegs * VU);
void VU_MACz_CLEAR(VURegs * VU);
void VU_MACw_CLEAR(VURegs * VU);
void VU_STAT_UPDATE(VURegs * VU);


#endif


