/*  Pcsx2 - Pc Ps2 Emulator
 *  Copyright (C) 2009  Pcsx2 Team
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

//------------------------------------------------------------------
// Micro VU - Pass 1 Functions
//------------------------------------------------------------------

//------------------------------------------------------------------
// FMAC1 - Normal FMAC Opcodes
//------------------------------------------------------------------

#define aReg(x) mVUregs.VF[x]
#define bReg(x, y) mVUregsTemp.VFreg[y] = x; mVUregsTemp.VF[y]
#define aMax(x, y) ((x > y) ? x : y)

#define analyzeReg1(reg) {									\
	if (reg) {												\
		if (_X) { mVUstall = aMax(mVUstall, aReg(reg).x); }	\
		if (_Y) { mVUstall = aMax(mVUstall, aReg(reg).y); }	\
		if (_Z) { mVUstall = aMax(mVUstall, aReg(reg).z); }	\
		if (_W) { mVUstall = aMax(mVUstall, aReg(reg).w); } \
	}														\
}

#define analyzeReg2(reg, isLowOp) {				\
	if (reg) {									\
		if (_X) { bReg(reg, isLowOp).x = 4; }	\
		if (_Y) { bReg(reg, isLowOp).y = 4; }	\
		if (_Z) { bReg(reg, isLowOp).z = 4; }	\
		if (_W) { bReg(reg, isLowOp).w = 4; }	\
	}											\
}

#define analyzeReg1b(reg) {						\
	if (reg) {									\
		analyzeReg1(reg);						\
		if (mVUregsTemp.VFreg[0] == reg) {		\
			if ((mVUregsTemp.VF[0].x && _X)		\
			||  (mVUregsTemp.VF[0].y && _Y)		\
			||  (mVUregsTemp.VF[0].z && _Z)		\
			||  (mVUregsTemp.VF[0].w && _W))	\
			{ mVUinfo.swapOps = 1; }			\
		}										\
	}											\
}

microVUt(void) mVUanalyzeFMAC1(mV, int Fd, int Fs, int Ft) {
	mVUup.doFlags = 1;
	sFLAG.doSticky = 1;
	analyzeReg1(Fs);
	analyzeReg1(Ft);
	analyzeReg2(Fd, 0);
}

//------------------------------------------------------------------
// FMAC2 - ABS/FTOI/ITOF Opcodes
//------------------------------------------------------------------

microVUt(void) mVUanalyzeFMAC2(mV, int Fs, int Ft) {
	analyzeReg1(Fs);
	analyzeReg2(Ft, 0);
}

//------------------------------------------------------------------
// FMAC3 - BC(xyzw) FMAC Opcodes
//------------------------------------------------------------------

#define analyzeReg3(reg) {											\
	if (reg) {														\
		if (_bc_x)		{ mVUstall = aMax(mVUstall, aReg(reg).x); } \
		else if (_bc_y) { mVUstall = aMax(mVUstall, aReg(reg).y); }	\
		else if (_bc_z) { mVUstall = aMax(mVUstall, aReg(reg).z); }	\
		else			{ mVUstall = aMax(mVUstall, aReg(reg).w); } \
	}																\
}

microVUt(void) mVUanalyzeFMAC3(mV, int Fd, int Fs, int Ft) {
	mVUup.doFlags = 1;
	sFLAG.doSticky = 1;
	analyzeReg1(Fs);
	analyzeReg3(Ft);
	analyzeReg2(Fd, 0);
}

//------------------------------------------------------------------
// FMAC4 - Clip FMAC Opcode
//------------------------------------------------------------------

#define analyzeReg4(reg) {								 \
	if (reg) { mVUstall = aMax(mVUstall, aReg(reg).w); } \
}

microVUt(void) mVUanalyzeFMAC4(mV, int Fs, int Ft) {
	cFLAG.doFlag = 1;
	analyzeReg1(Fs);
	analyzeReg4(Ft);
}

//------------------------------------------------------------------
// IALU - IALU Opcodes
//------------------------------------------------------------------

#define analyzeVIreg1(reg)			{ if (reg) { mVUstall = aMax(mVUstall, mVUregs.VI[reg]); } }
#define analyzeVIreg2(reg, aCycles)	{ if (reg) { mVUregsTemp.VIreg = reg; mVUregsTemp.VI = aCycles; mVUlow.writesVI = 1; mVU->VIbackup[0] = reg; } }
#define analyzeVIreg3(reg, aCycles)	{ if (reg) { mVUregsTemp.VIreg = reg; mVUregsTemp.VI = aCycles; } }

microVUt(void) mVUanalyzeIALU1(mV, int Id, int Is, int It) {
	if (!Id) { mVUlow.isNOP = 1; }
	analyzeVIreg1(Is);
	analyzeVIreg1(It);
	analyzeVIreg2(Id, 1);
}

microVUt(void) mVUanalyzeIALU2(mV, int Is, int It) {
	if (!It) { mVUlow.isNOP = 1; }
	analyzeVIreg1(Is);
	analyzeVIreg2(It, 1);
}

//------------------------------------------------------------------
// MR32 - MR32 Opcode
//------------------------------------------------------------------

// Flips xyzw stalls to yzwx
#define analyzeReg6(reg) {									\
	if (reg) {												\
		if (_X) { mVUstall = aMax(mVUstall, aReg(reg).y); }	\
		if (_Y) { mVUstall = aMax(mVUstall, aReg(reg).z); }	\
		if (_Z) { mVUstall = aMax(mVUstall, aReg(reg).w); }	\
		if (_W) { mVUstall = aMax(mVUstall, aReg(reg).x); } \
		if (mVUregsTemp.VFreg[0] == reg) {					\
			if ((mVUregsTemp.VF[0].y && _X)					\
			||  (mVUregsTemp.VF[0].z && _Y)					\
			||  (mVUregsTemp.VF[0].w && _Z)					\
			||  (mVUregsTemp.VF[0].x && _W))				\
			{ mVUinfo.swapOps = 1; }						\
		}													\
	}														\
}

microVUt(void) mVUanalyzeMR32(mV, int Fs, int Ft) {
	if (!Ft) { mVUlow.isNOP = 1; }
	analyzeReg6(Fs);
	analyzeReg2(Ft, 1);
}

//------------------------------------------------------------------
// FDIV - DIV/SQRT/RSQRT Opcodes
//------------------------------------------------------------------

#define analyzeReg5(reg, fxf) {										\
	if (reg) {														\
		switch (fxf) {												\
			case 0: mVUstall = aMax(mVUstall, aReg(reg).x); break;	\
			case 1: mVUstall = aMax(mVUstall, aReg(reg).y); break;	\
			case 2: mVUstall = aMax(mVUstall, aReg(reg).z); break;	\
			case 3: mVUstall = aMax(mVUstall, aReg(reg).w); break;	\
		}															\
		if (mVUregsTemp.VFreg[0] == reg) {							\
			if ((mVUregsTemp.VF[0].x && (fxf == 0))					\
			||  (mVUregsTemp.VF[0].y && (fxf == 1))					\
			||  (mVUregsTemp.VF[0].z && (fxf == 2))					\
			||  (mVUregsTemp.VF[0].w && (fxf == 3)))				\
			{ mVUinfo.swapOps = 1; }								\
		}															\
	}																\
}

#define analyzeQreg(x) { mVUregsTemp.q = x; mVUstall = aMax(mVUstall, mVUregs.q); }
#define analyzePreg(x) { mVUregsTemp.p = x; mVUstall = aMax(mVUstall, ((mVUregs.p) ? (mVUregs.p - 1) : 0)); }

microVUt(void) mVUanalyzeFDIV(mV, int Fs, int Fsf, int Ft, int Ftf, u8 xCycles) {
	mVUprint("microVU: DIV Opcode");
	analyzeReg5(Fs, Fsf);
	analyzeReg5(Ft, Ftf);
	analyzeQreg(xCycles);
}

//------------------------------------------------------------------
// EFU - EFU Opcodes
//------------------------------------------------------------------

microVUt(void) mVUanalyzeEFU1(mV, int Fs, int Fsf, u8 xCycles) {
	mVUprint("microVU: EFU Opcode");
	analyzeReg5(Fs, Fsf);
	analyzePreg(xCycles);
}

microVUt(void) mVUanalyzeEFU2(mV, int Fs, u8 xCycles) {
	mVUprint("microVU: EFU Opcode");
	analyzeReg1b(Fs);
	analyzePreg(xCycles);
}

//------------------------------------------------------------------
// MFP - MFP Opcode
//------------------------------------------------------------------

microVUt(void) mVUanalyzeMFP(mV, int Ft) {
	if (!Ft) { mVUlow.isNOP = 1; }
	analyzeReg2(Ft, 1);
}

//------------------------------------------------------------------
// MOVE - MOVE Opcode
//------------------------------------------------------------------

microVUt(void) mVUanalyzeMOVE(mV, int Fs, int Ft) {
	if (!Ft || (Ft == Fs)) { mVUlow.isNOP = 1; }
	analyzeReg1b(Fs);
	analyzeReg2(Ft, 1);
}


//------------------------------------------------------------------
// LQx - LQ/LQD/LQI Opcodes
//------------------------------------------------------------------

microVUt(void) mVUanalyzeLQ(mV, int Ft, int Is, bool writeIs) {
	analyzeVIreg1(Is);
	analyzeReg2(Ft, 1);
	if (!Ft)	 { if (writeIs && Is) { mVUlow.noWriteVF = 1; } else { mVUlow.isNOP = 1; } }
	if (writeIs) { analyzeVIreg2(Is, 1); }
}

//------------------------------------------------------------------
// SQx - SQ/SQD/SQI Opcodes
//------------------------------------------------------------------

microVUt(void) mVUanalyzeSQ(mV, int Fs, int It, bool writeIt) {
	analyzeReg1b(Fs);
	analyzeVIreg1(It);
	if (writeIt) { analyzeVIreg2(It, 1); }
}

//------------------------------------------------------------------
// R*** - R Reg Opcodes
//------------------------------------------------------------------

#define analyzeRreg() { mVUregsTemp.r = 1; }

microVUt(void) mVUanalyzeR1(mV, int Fs, int Fsf) {
	analyzeReg5(Fs, Fsf);
	analyzeRreg();
}

microVUt(void) mVUanalyzeR2(mV, int Ft, bool canBeNOP) {
	if (!Ft) { if (canBeNOP) { mVUlow.isNOP = 1; } else { mVUlow.noWriteVF = 1; } }
	analyzeReg2(Ft, 1);
	analyzeRreg();
}

//------------------------------------------------------------------
// Sflag - Status Flag Opcodes
//------------------------------------------------------------------

#define setFlagInst(xDoFlag) {										\
	int curPC = iPC;												\
	for (int i = mVUcount, j = 0; i > 0; i--, j++) {				\
		incPC2(-2);													\
		if (mVUup.doFlags) { xDoFlag = 1; if (j >= 3) { break; } }	\
	}																\
	iPC = curPC;													\
}

microVUt(void) mVUanalyzeSflag(mV, int It) {
	if (!It) { mVUlow.isNOP = 1; }
	else {
		mVUinfo.swapOps = 1;
		mVUsFlagHack = 0; // Don't Optimize Out Status Flags for this block
		if (mVUcount < 4)	{ mVUpBlock->pState.needExactMatch |= 0xf /*<< mVUcount*/; }
		if (mVUcount >= 1)	{ incPC2(-2); mVUlow.useSflag = 1; incPC2(2); }
		// Note: useSflag is used for status flag optimizations when a FSSET instruction is called.
		// Do to stalls, it can only be set one instruction prior to the status flag read instruction
		// if we were guaranteed no-stalls were to happen, it could be set 4 instruction prior.
		setFlagInst(sFLAG.doFlag);
	}
	analyzeVIreg3(It, 1);
}

microVUt(void) mVUanalyzeFSSET(mV) {
	mVUinfo.swapOps = 1;
	mVUlow.isFSSET  = 1;
	sFLAG.doSticky  = 0;
}

//------------------------------------------------------------------
// Mflag - Mac Flag Opcodes
//------------------------------------------------------------------

microVUt(void) mVUanalyzeMflag(mV, int Is, int It) {
	if (!It) { mVUlow.isNOP = 1; }
	else { // Need set _doMac for 4 previous Ops (need to do all 4 because stalls could change the result needed)
		mVUinfo.swapOps = 1;
		if (mVUcount < 4) { mVUpBlock->pState.needExactMatch |= 0xf << (/*mVUcount +*/ 4); }
		setFlagInst(mFLAG.doFlag);
	}
	analyzeVIreg1(Is);
	analyzeVIreg3(It, 1);
}

//------------------------------------------------------------------
// Cflag - Clip Flag Opcodes
//------------------------------------------------------------------

microVUt(void) mVUanalyzeCflag(mV, int It) {
	mVUinfo.swapOps = 1;
	if (mVUcount < 4) { mVUpBlock->pState.needExactMatch |= 0xf << (/*mVUcount +*/ 8); }
	analyzeVIreg3(It, 1);
}

//------------------------------------------------------------------
// XGkick
//------------------------------------------------------------------

#define analyzeXGkick1()  { mVUstall = aMax(mVUstall, mVUregs.xgkick); }
#define analyzeXGkick2(x) { mVUregsTemp.xgkick = x; }

microVUt(void) mVUanalyzeXGkick(mV, int Fs, int xCycles) {
	analyzeVIreg1(Fs);
	analyzeXGkick1();
	analyzeXGkick2(xCycles);
	// Note: Technically XGKICK should stall on the next instruction,
	// this code stalls on the same instruction. The only case where this
	// will be a problem with, is if you have very-specifically placed
	// FMxxx or FSxxx opcodes checking flags near this instruction AND
	// the XGKICK instruction stalls. No-game should be effected by 
	// this minor difference.
}

//------------------------------------------------------------------
// Branches - Branch Opcodes
//------------------------------------------------------------------

#define analyzeBranchVI(reg, infoVar) {						\
	/* First ensure branch is not first opcode in block */	\
	if (reg && (mVUcount > 0)) { 							\
		incPC2(-2);											\
		/* Check if prev Op modified VI reg */				\
		if (mVUlow.writesVI && (reg == mVU->VIbackup[0])) { \
			mVUlow.backupVI = 1;							\
			incPC2(2);										\
			infoVar = 1;									\
		}													\
		else { incPC2(2); }									\
	}														\
}

microVUt(void) mVUanalyzeBranch1(mV, int Is) {
	if (mVUregs.VI[Is] || mVUstall)	{ analyzeVIreg1(Is); }
	else							{ analyzeBranchVI(Is, mVUlow.memReadIs); }
}

microVUt(void) mVUanalyzeBranch2(mV, int Is, int It) {
	if (mVUregs.VI[Is] || mVUregs.VI[It] || mVUstall) { analyzeVIreg1(Is); analyzeVIreg1(It); }
	else { analyzeBranchVI(Is, mVUlow.memReadIs); analyzeBranchVI(It, mVUlow.memReadIt);}
}
