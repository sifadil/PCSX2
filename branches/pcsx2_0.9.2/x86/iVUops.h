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

#ifdef __MSCW32__
#pragma warning(disable:4244)
#endif

#define REC_VUOP(VU, f) { \
	_freeXMMregs(&VU); \
	_freeMMXregs(); \
	SetFPUstate(); \
	MOV32ItoM((u32)&VU.code, (u32)VU.code); \
	CALLFunc((u32)VU##MI_##f); \
}

#define REC_VUOPFLAGS(VU, f) { \
	_freeXMMregs(&VU); \
	_freeMMXregs(); \
	SetFPUstate(); \
	MOV32ItoM((u32)&VU.code, (u32)VU.code); \
	CALLFunc((u32)VU##MI_##f); \
}

#define REC_VUBRANCH(VU, f) { \
	_freeXMMregs(&VU); \
	_freeMMXregs(&VU0); \
	SetFPUstate(); \
	MOV32ItoM((u32)&VU.code, (u32)VU.code); \
	MOV32ItoM((u32)&VU.VI[REG_TPC].UL, (u32)pc); \
	CALLFunc((u32)VU##MI_##f); \
	branch = 1; \
}