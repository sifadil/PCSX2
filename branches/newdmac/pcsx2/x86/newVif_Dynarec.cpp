/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2010  PCSX2 Dev Team
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

// newVif Dynarec - Dynamically Recompiles Vif 'unpack' Packets
// authors: cottonvibes(@gmail.com)
//			Jake.Stine (@gmail.com)

#include "PrecompiledHeader.h"
#include "newVif_UnpackSSE.h"

void dVifReset(int idx) {
	//memzero(nVif);

	nVif[idx].idx			= idx;

	// If the VIF cache is greater than 12mb, then it's due for a complete reset back
	// down to a reasonable starting point of 4mb.
	if( nVif[idx].vifCache && (nVif[idx].vifCache->getAllocSize() > _1mb*12) )
		safe_delete(nVif[idx].vifCache);

	if( !nVif[idx].vifCache )
		nVif[idx].vifCache = new BlockBuffer(_1mb*4);
	else
		nVif[idx].vifCache->clear();

	if( !nVif[idx].vifBlocks )
		nVif[idx].vifBlocks = new HashBucket<_tParams>();
	else
		nVif[idx].vifBlocks->clear();

	nVif[idx].numBlocks =  0;

	nVif[idx].recPtr	=  nVif[idx].vifCache->getBlock();
	nVif[idx].recEnd	= &nVif[idx].recPtr[nVif[idx].vifCache->getAllocSize()-(_1mb/4)]; // .25mb Safe Zone
}

void dVifClose(int idx) {
	nVif[idx].numBlocks = 0;
	safe_delete(nVif[idx].vifCache);
	safe_delete(nVif[idx].vifBlocks);
}

VifUnpackSSE_Dynarec::VifUnpackSSE_Dynarec(const nVifStruct& vif_)
	: v(vif_)
{
	isFill		= (v.Block.cl < v.Block.wl);
	usn			= (v.Block.upkType>>5) & 1;
	doMask		= (v.Block.upkType>>4) & 1;
	doMode		= v.Block.mode & 3;
	vCL			= 0;
}

#define makeMergeMask(x) {									\
	x = ((x&0x40)>>6) | ((x&0x10)>>3) | (x&4) | ((x&1)<<3);	\
}

__fi void VifUnpackSSE_Dynarec::SetMasks(int cS) const {
	VifProcessingUnit&	vpu	= g_vpu[v.idx];
	u32 m0 = v.Block.mask;
	u32 m1 =  m0 & 0xaaaaaaaa;
	u32 m2 =(~m1>>1) &  m0;
	u32 m3 = (m1>>1) & ~m0;
	if((m2&&(doMask||isFill))||doMode) { xMOVAPS(xmmRow, ptr128[&vpu.MaskRow]); }
	if (m3&&(doMask||isFill)) {
		xMOVAPS(xmmCol0, ptr128[&vpu.MaskCol]);
		if ((cS>=2) && (m3&0x0000ff00)) xPSHUF.D(xmmCol1, xmmCol0, _v1);
		if ((cS>=3) && (m3&0x00ff0000)) xPSHUF.D(xmmCol2, xmmCol0, _v2);
		if ((cS>=4) && (m3&0xff000000)) xPSHUF.D(xmmCol3, xmmCol0, _v3);
		if ((cS>=1) && (m3&0x000000ff)) xPSHUF.D(xmmCol0, xmmCol0, _v0);
	}
	//if (doMask||doMode) loadRowCol((nVifStruct&)v);
}

void VifUnpackSSE_Dynarec::doMaskWrite(const xRegisterSSE& regX) const {
	pxAssumeDev(regX.Id <= 1, "Reg Overflow! XMM2 thru XMM6 are reserved for masking.");
	xRegisterSSE t  =  regX == xmm0 ? xmm1 : xmm0; // Get Temp Reg
	int cc =  aMin(vCL, 3);
	u32 m0 = (v.Block.mask >> (cc * 8)) & 0xff;
	u32 m1 =  m0 & 0xaa;
	u32 m2 =(~m1>>1) &  m0;
	u32 m3 = (m1>>1) & ~m0;
	u32 m4 = (m1>>1) &  m0;
	makeMergeMask(m2);
	makeMergeMask(m3);
	makeMergeMask(m4);
	if (doMask&&m4) { xMOVAPS(xmmTemp, ptr[dstIndirect]);			} // Load Write Protect
	if (doMask&&m2) { mergeVectors(regX, xmmRow,						t, m2); } // Merge Row
	if (doMask&&m3) { mergeVectors(regX, xRegisterSSE(xmmCol0.Id+cc),	t, m3); } // Merge Col
	if (doMask&&m4) { mergeVectors(regX, xmmTemp,						t, m4); } // Merge Write Protect
	if (doMode) {
		u32 m5 = (~m1>>1) & ~m0;
		if (!doMask)  m5 = 0xf;
		else		  makeMergeMask(m5);
		if (m5 < 0xf) {
			xPXOR(xmmTemp, xmmTemp);
			mergeVectors(xmmTemp, xmmRow, t, m5);
			xPADD.D(regX, xmmTemp);
			if (doMode==2) mergeVectors(xmmRow, regX, t, m5);
		}
		else if (m5 == 0xf) {
			xPADD.D(regX, xmmRow);
			if (doMode==2) xMOVAPS(xmmRow, regX);
		}
	}
	xMOVAPS(ptr32[dstIndirect], regX);
}

void VifUnpackSSE_Dynarec::writeBackRow() const {
	VifProcessingUnit&	vpu	= g_vpu[v.idx];
	xMOVAPS(ptr128[&vpu.MaskRow], xmmRow);
	DevCon.WriteLn("nVif: writing back row reg! [doMode = 2]");
}

static void ShiftDisplacementWindow( xAddressVoid& addr, const xRegister32& modReg )
{
	// Shifts the displacement factor of a given indirect address, so that the address
	// remains in the optimal 0xf0 range (which allows for byte-form displacements when
	// generating instructions).

	int addImm = 0;
	while( addr.Displacement >= 0x80 )
	{
		addImm += 0xf0;
		addr -= 0xf0;
	}
	if(addImm) xADD(modReg, addImm);
}

void VifUnpackSSE_Dynarec::CompileRoutine(uint vSize) {
	const int  upkNum	 = v.Block.upkType & 0xf;
	const int  cycleSize = isFill ? v.Block.cl : v.Block.wl;
	const int  blockSize = isFill ? v.Block.wl : v.Block.cl;
	const int  skipSize	 = blockSize - cycleSize;

	uint vNum	= v.Block.num ? v.Block.num : 256;
	doMode		= (upkNum == 0xf) ? 0 : doMode;		// V4_5 unpacks have no mode feature.

	// Value passed determines # of col regs we need to load
	SetMasks(isFill ? blockSize : cycleSize);

	while (vNum) {

		ShiftDisplacementWindow( srcIndirect, edx );
		ShiftDisplacementWindow( dstIndirect, ecx );

		if (vCL < cycleSize) {
			xUnpack(upkNum);
			xMovDest();

			dstIndirect += 16;
			srcIndirect += vSize;

			if( IsUnmaskedOp() ) {
				++destReg;
				++workReg;
			}

			vNum--;
			if (++vCL == blockSize) vCL = 0;
		}
		else if (isFill) {
			//DevCon.WriteLn("filling mode!");
			VifUnpackSSE_Dynarec fill( VifUnpackSSE_Dynarec::FillingWrite( *this ) );
			fill.xUnpack(upkNum);
			fill.xMovDest();

			dstIndirect += 16;
			vNum--;
			if (++vCL == blockSize) vCL = 0;
		}
		else {
			dstIndirect += (16 * skipSize);
			vCL = 0;
		}
	}

	if (doMode==2) writeBackRow();
	xRET();
}

_vifT static __fi u128* dVifsetVUptr(uint cl, uint wl, bool isFill) {
	VifProcessingUnit&	vpu			= g_vpu[idx];
	VIFregisters&		regs		= GetVifXregs;
	const VURegs&		VU			= vuRegs[idx];
	const uint			vuMemLimit	= idx ? 0x400 : 0x100;

	u128* startmem	= (u128*)VU.Mem + (vpu.vu_target_addr & (vuMemLimit-1));
	u128* endmem	= (u128*)VU.Mem + vuMemLimit;
	uint length		= regs.num;

	if (!isFill) {
		// Accounting for skipping mode: Subtract the last skip cycle, since the skipped part of the run
		// shouldn't count as wrapped data.  Otherwise, a trailing skip can cause the emu to drop back
		// to the interpreter. -- Refraction (test with MGS3)

		uint skipSize  = (cl - wl);
		uint blocks    = regs.num / wl;
		length += (blocks-1) * skipSize;
	}

	if ( (startmem+length) <= endmem ) {
		return startmem;
	}
	//Console.WriteLn("nVif%x - VU Mem Ptr Overflow; falling back to interpreter. Start = %x End = %x num = %x, wl = %x, cl = %x", v.idx, vif.tag.addr, vif.tag.addr + (_vBlock.num * 16), _vBlock.num, wl, cl);
	return NULL; // Fall Back to Interpreters which have wrap-around logic
}

// [TODO] :  Finish implementing support for VIF's growable recBlocks buffer.  Currently
//    it clears the buffer only.
static __fi void dVifRecLimit(int idx) {
	if (nVif[idx].recPtr > nVif[idx].recEnd) {
		DevCon.WriteLn("nVif Rec - Out of Rec Cache! [%x > %x]", nVif[idx].recPtr, nVif[idx].recEnd);
		nVif[idx].vifBlocks->clear();
		nVif[idx].recPtr = nVif[idx].vifCache->getBlock();
		nVif[idx].recEnd = &nVif[idx].recPtr[nVif[idx].vifCache->getAllocSize()-(_1mb/4)]; // .25mb Safe Zone
	}
}

_vifT static __fi bool dVifExecuteUnpack(const u8* data, bool isFill, uint vSize)
{
	const nVifStruct&	v		= nVif[idx];
	VIFregisters&		regs	= GetVifXregs;

	if (nVifBlock* b = v.vifBlocks->find(&v.Block)) {
		if (u128* dest = dVifsetVUptr<idx>(regs.cycle.cl, regs.cycle.wl, isFill)) {
			//DevCon.WriteLn("Running Recompiled Block!");
			((nVifrecCall)b->startPtr)((uptr)dest, (uptr)data);
		}
		else {
			//DevCon.WriteLn("Running Interpreter Block");
			VifUnpackLoopTable[idx][!!regs.mode][isFill](vSize, data);	
		}
		return true;
	}
	return false;
}

template< uint idx, bool doMask, uint upkType >
__fi void dVifUnpack(const u8* data, bool isFill, uint vSize) {

	nVifStruct&			v		= nVif[idx];
	VifProcessingUnit&	vpu		= g_vpu[idx];
	VIFregisters&		regs	= GetVifXregs;

	v.Block.upkType = upkType | (doMask << 4) | (regs.code.USN << 5);	// combines VN, VL, doMask, USN into one u8.
	v.Block.num		= (u8&)regs.num;
	v.Block.mode	= (u8&)regs.mode;
	v.Block.cl		= regs.cycle.cl;
	v.Block.wl		= regs.cycle.wl;

	// Zero out the mask parameter if it's unused -- games leave random junk
	// values here which cause false recblock cache misses.
	v.Block.mask	= doMask ? regs.mask : 0;

	//DevCon.WriteLn("nVif%d: Recompiled Block! [%d]", idx, nVif[idx].numBlocks++);
	//DevCon.WriteLn(L"[num=% 3d][upkType=0x%02x][scl=%d][cl=%d][wl=%d][mode=%d][m=%d][mask=%s]",
	//	_vBlock.num, _vBlock.upkType, _vBlock.scl, _vBlock.cl, _vBlock.wl, _vBlock.mode,
	//	doMask >> 4, doMask ? wxsFormat( L"0x%08x", _vBlock.mask ).c_str() : L"ignored"
	//);

	if (dVifExecuteUnpack<idx>(data, isFill, vSize)) return;

	xSetPtr(v.recPtr);
	v.Block.startPtr = (uptr)xGetAlignedCallTarget();
	v.vifBlocks->add(v.Block);
	VifUnpackSSE_Dynarec( v ).CompileRoutine(vSize);
	nVif[idx].recPtr = xGetPtr();

	// [TODO] : Ideally we should test recompile buffer limits prior to each instruction,
	//   which would be safer and more memory efficient than using an 0.25 meg recEnd marker.
	dVifRecLimit(idx);

	// Run the block we just compiled.  Various conditions may force us to still use
	// the interpreter unpacker though, so a recursive call is the safest way here...
	dVifExecuteUnpack<idx>(data, isFill, vSize);
}

#define INST_TMPL_VifUnpack(idx, mask) \
	template void dVifUnpack<idx,mask,0>(const u8* data, bool isFill, uint vSize); \
	template void dVifUnpack<idx,mask,1>(const u8* data, bool isFill, uint vSize); \
	template void dVifUnpack<idx,mask,2>(const u8* data, bool isFill, uint vSize); \
	 \
	template void dVifUnpack<idx,mask,4>(const u8* data, bool isFill, uint vSize); \
	template void dVifUnpack<idx,mask,5>(const u8* data, bool isFill, uint vSize); \
	template void dVifUnpack<idx,mask,6>(const u8* data, bool isFill, uint vSize); \
	 \
	template void dVifUnpack<idx,mask,8>(const u8* data, bool isFill, uint vSize); \
	template void dVifUnpack<idx,mask,9>(const u8* data, bool isFill, uint vSize); \
	template void dVifUnpack<idx,mask,10>(const u8* data, bool isFill, uint vSize); \
	 \
	template void dVifUnpack<idx,mask,12>(const u8* data, bool isFill, uint vSize); \
	template void dVifUnpack<idx,mask,13>(const u8* data, bool isFill, uint vSize); \
	template void dVifUnpack<idx,mask,14>(const u8* data, bool isFill, uint vSize); \
	template void dVifUnpack<idx,mask,15>(const u8* data, bool isFill, uint vSize);

INST_TMPL_VifUnpack(0,false);
INST_TMPL_VifUnpack(0,true);

INST_TMPL_VifUnpack(1,false);
INST_TMPL_VifUnpack(1,true);
