/*  Pcsx2 - Pc Ps2 Emulator
*  Copyright (C) 2009  Pcsx2-Playground Team
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

#pragma once

union regInfo {
	u32 reg;
	struct {
		u8 x;
		u8 y;
		u8 z;
		u8 w;
	};
};

struct microRegInfo {
	regInfo VF[32];
	regInfo Acc;
	u8 VI[32];
	u8 i;
	u8 q;
	u8 p;
	u8 r;
};

template<u32 pSize>
struct microAllocInfo {
	microRegInfo regs;
	u8  branch; // 0 = No Branch, 1 = Branch, 2 = Conditional Branch, 3 = Jump (JALR/JR)
	u32 curPC;  // Current PC
	u32 cycles; // Cycles for current block
	u32 info[pSize];// bit 00 = Lower Instruction is NOP
					// bit 01
					// bit 02
					// bit 03
					// bit 04
					// bit 05 = Write to Q1 or Q2?
					// bit 06 = Read Q1 or Q2?
					// bit 07 = Read/Write to P1 or P2?
					// bit 08 = Update Mac Flags?
					// bit 09 = Update Status Flags?
					// bit 10 = Used with bit 11 to make a 2-bit key for mac flag instance
					// bit 11
					// bit 12 = Used with bit 13 to make a 2-bit key for status flag instance
					// bit 13
					// bit 14 = Used with bit 15 to make a 2-bit key for clip flag instance
					// bit 15
					// bit 16 = Used with bit 17 to make a 2-bit key for mac flag instance
					// bit 17
					// bit 18 = Used with bit 19 to make a 2-bit key for status flag instance
					// bit 19
					// bit 20 = Used with bit 21 to make a 2-bit key for clip flag instance
					// bit 21
					// bit 22 = Read VI(Fs) from backup memory?
					// bit 23 = Read VI(Ft) from backup memory?
};
