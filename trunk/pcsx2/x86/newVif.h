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

#pragma once

#ifdef newVif
#include "x86emitter/x86emitter.h"
using namespace x86Emitter;
extern void mVUmergeRegs(int dest, int src, int xyzw, bool modXYZW = 0);
extern void _nVifUnpack(int idx, u8 *data, u32 size);

struct instBlock { u8 data[16*64]; };
static __pagealigned instBlock nVifUpk[2][2][4][3][16]; // [USN][Masking][curCycle][CyclesToWrite-1][Unpack Type]
static __aligned16 u32 nVifMask[3][4][4] = {0};			// [MaskNumber][CycleNumber][Vector]
typedef u32 (__fastcall *nVifCall)(void*, void*);
#define nVifUnpackF(dest, src,   usn, doMask, curCycle, cycles, unpackType) {				\
	(((nVifCall)((void*)&nVifUpk[usn][doMask][curCycle][cycles][unpackType]))(dest, src));	\
}

#define	_v0 0
#define	_v1 0x55
#define	_v2 0xaa
#define	_v3 0xff
#define aMax(x, y) (((x) > (y) ? (x) : (y)))
#define aMin(x, y) (((x) < (y) ? (x) : (y)))
#define _f __forceinline

#define xShiftR(regX, n) {			\
	if (usn) { xPSRL.D(regX, n); }	\
	else	 { xPSRA.D(regX, n); }	\
}

u32 nVifT[16] = { 
	4, // S-32
	2, // S-16
	1, // S-8
	0, // ----
	8, // V2-32
	4, // V2-16
	2, // V2-8
	0, // ----
	12,// V3-32
	6, // V3-16
	3, // V3-8
	0, // ----
	16,// V4-32
	8, // V4-16
	4, // V4-8
	2, // V4-5
};

#include "newVif_BlockBuffer.h"
#include "newVif_OldUnpack.inl"
#include "newVif_UnpackGen.inl"
#include "newVif_Unpack.inl"

#endif
