/*  ZeroGS KOSMOS
 *  Copyright (C) 2005-2006 zerorog@gmail.com
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

#include "GS.h"
#include "Mem.h"
#include "NewRegs.h"
#include "PS2Etypes.h"

#include "zerogs.h"
#include "targets.h"

#ifdef USE_OLD_REGS
#include "Regs.h"
#else

const u32 g_primmult[8] = { 1, 2, 2, 3, 3, 3, 2, 0xff };
const u32 g_primsub[8] = { 1, 2, 1, 3, 1, 1, 2, 0 };

#ifdef _MSC_VER
#pragma warning(disable:4244)
#endif

GIFRegHandler g_GIFPackedRegHandlers[16];
GIFRegHandler g_GIFRegHandlers[256];
GIFRegHandler g_GIFTempRegHandlers[16] = {0};

// values for keeping track of changes
u32 s_uTex1Data[2][2] = {{0, }};
u32 s_uClampData[2] = {0, };

u32 results[65535] = {0, };

// return true if triangle SHOULD be painted.
// My brain hurts. --arcum42
inline bool NoHighlights(int i)
{
//	This is hack-code, I still in search of correct reason, why some triangles should not be drawn.

	int resultA = prim->iip + ((prim->tme) << 1) + ((prim->fge) << 2) + ((prim->abe) << 3) + ((prim->aa1) << 4) + ((prim->fst) << 5) + ((prim->ctxt) << 6) + ((prim->fix) << 7) +
				  ((ZeroGS::vb[i].zbuf.psm) << 8);

	const pixTest curtest = ZeroGS::vb[i].test;
	int result = curtest.ate + ((curtest.atst) << 1) + ((curtest.afail) << 4) + ((curtest.date) << 6) + ((curtest.datm) << 7) + ((curtest.zte) << 8) + ((curtest.ztst) << 9);
	if ((resultA == 0x3a2a || resultA == 0x312a) && (result == 0x302 || result == 0x700) && (ZeroGS::vb[i].zbuf.zmsk)) return false; // Silent Hill:SM and Front Mission 5, result != 0x300
	if (((resultA == 0x3100) || (resultA == 0x3108)) && ((result == 0x54c) || (result == 0x50c)) && (ZeroGS::vb[i].zbuf.zmsk)) return false; // Okage
	if ((resultA == 0x310a) && (result == 0x0)) return false; // Radiata Stories
	if (resultA == 0x3a6a && (result == 0x300 || result == 0x500) && ZeroGS::vb[i].zbuf.zmsk) return false; // Okami, result != 0x30d

//	Old code
	return (!(conf.settings().xenosaga_spec) || !ZeroGS::vb[i].zbuf.zmsk || prim->iip) ;
}

void __fastcall GIFPackedRegHandlerNull(u32* data)
{
	FUNCLOG
	ZZLog::Debug_Log("Unexpected packed reg handler %8.8lx_%8.8lx %x.", data[0], data[1], data[2]);
}

// All these just call their non-packed equivalent.
void __fastcall GIFPackedRegHandlerPRIM(u32* data) { GIFRegHandlerPRIM(data); }
void __fastcall GIFPackedRegHandlerTEX0_1(u32* data) { GIFRegHandlerTEX0_1(data); }
void __fastcall GIFPackedRegHandlerTEX0_2(u32* data) { GIFRegHandlerTEX0_2(data); }
void __fastcall GIFPackedRegHandlerCLAMP_1(u32* data) { GIFRegHandlerCLAMP_1(data); }
void __fastcall GIFPackedRegHandlerCLAMP_2(u32* data) { GIFRegHandlerCLAMP_2(data); }
void __fastcall GIFPackedRegHandlerXYZF3(u32* data) { GIFRegHandlerXYZF3(data); }
void __fastcall GIFPackedRegHandlerXYZ3(u32* data) { GIFRegHandlerXYZ3(data); }

void __fastcall GIFPackedRegHandlerRGBA(u32* data)
{
	FUNCLOG
	
	GIFPackedRGBA* r = (GIFPackedRGBA*)(data);
	gs.rgba = (r->R | (r->G <<  8) | (r->B << 16) | (r->A << 24));
	gs.vertexregs.rgba = gs.rgba;
	gs.vertexregs.q = gs.q;
}

void __fastcall GIFPackedRegHandlerSTQ(u32* data)
{
	FUNCLOG
	GIFPackedSTQ* r = (GIFPackedSTQ*)(data);
	gs.vertexregs.s = r->S;
	gs.vertexregs.t = r->T;
	gs.q = r->Q;
}

void __fastcall GIFPackedRegHandlerUV(u32* data)
{
	FUNCLOG
	GIFPackedUV* r = (GIFPackedUV*)(data);
	gs.vertexregs.u = r->U;
	gs.vertexregs.v = r->V;
}

void __forceinline KICK_VERTEX2()
{
	FUNCLOG

	if (++gs.primC >= (int)g_primmult[prim->prim])
	{
		if (NoHighlights(prim->ctxt)) (*ZeroGS::drawfn[prim->prim])();

		gs.primC -= g_primsub[prim->prim];
	}
}

void __forceinline KICK_VERTEX3()
{
	FUNCLOG

	if (++gs.primC >= (int)g_primmult[prim->prim])
	{
		gs.primC -= g_primsub[prim->prim];

		if (prim->prim == 5)
		{
			/* tri fans need special processing */
			if (gs.nTriFanVert == gs.primIndex)
				gs.primIndex = (gs.primIndex + 1) % ArraySize(gs.gsvertex);
		}
	}
}

void __fastcall GIFPackedRegHandlerXYZF2(u32* data)
{
	FUNCLOG
	GIFPackedXYZF2* r = (GIFPackedXYZF2*)(data);
	gs.vertexregs.x = r->X;
	gs.vertexregs.y = r->Y;
	gs.vertexregs.z = r->Z;
	gs.vertexregs.f = r->F;
	
	gs.gsvertex[gs.primIndex] = gs.vertexregs;
	gs.primIndex = (gs.primIndex + 1) % ArraySize(gs.gsvertex);

	// Fix Vertexes up later.
	if (data[3] & 0x8000)
	{
		KICK_VERTEX3();
	}
	else
	{
		KICK_VERTEX2();
	}
}

void __fastcall GIFPackedRegHandlerXYZ2(u32* data)
{
	FUNCLOG
	GIFPackedXYZ2* r = (GIFPackedXYZ2*)(data);
	gs.vertexregs.x = r->X;
	gs.vertexregs.y = r->Y;
	gs.vertexregs.z = r->Z;
	gs.gsvertex[gs.primIndex] = gs.vertexregs;
	gs.primIndex = (gs.primIndex + 1) % ArraySize(gs.gsvertex);

	// Fix Vertexes up later.
	if (data[3] & 0x8000)
	{
		KICK_VERTEX3();
	}
	else
	{
		KICK_VERTEX2();
	}
}

void __fastcall GIFPackedRegHandlerFOG(u32* data)
{
	FUNCLOG
	GIFPackedFOG* r = (GIFPackedFOG*)(data);
	gs.vertexregs.f = r->F;
}

void __fastcall GIFPackedRegHandlerA_D(u32* data)
{
	FUNCLOG
	GIFPackedA_D* r = (GIFPackedA_D*)(data);
	
	g_GIFRegHandlers[r->ADDR](data);
}

void __fastcall GIFPackedRegHandlerNOP(u32* data)
{
	FUNCLOG
}

void tex0Write(int i, u32 *data)
{
	FUNCLOG
	u32 psm = ZZOglGet_psm_TexBitsFix(data[0]);

	if (m_Blocks[psm].bpp == 0)
	{
		// kh and others
		return;
	}

	ZeroGS::vb[i].uNextTex0Data[0] = data[0];

	ZeroGS::vb[i].uNextTex0Data[1] = data[1];
	ZeroGS::vb[i].bNeedTexCheck = 1;

	// don't update unless necessary

	if (PSMT_ISCLUT(psm))
	{
		if (ZeroGS::CheckChangeInClut(data[1], psm))
		{
			// loading clut, so flush whole texture
			ZeroGS::vb[i].FlushTexData();
		}

		// check if csa is the same!! (ffx bisaid island, grass)
		else if ((data[1] & 0x1f780000) != (ZeroGS::vb[i].uCurTex0Data[1] & 0x1f780000))
		{
			ZeroGS::Flush(i); // flush any previous entries
		}
	}
}

void tex2Write(int i, u32 *data)
{
	FUNCLOG
	tex0Info& tex0 = ZeroGS::vb[i].tex0;

	if (ZeroGS::vb[i].bNeedTexCheck) ZeroGS::vb[i].FlushTexData();

	u32 psm = ZZOglGet_psm_TexBitsFix(data[0]);

	u32* s_uTex0Data = ZeroGS::vb[i].uCurTex0Data;

	// don't update unless necessary
//	if( ZZOglGet_psm_TexBitsFix(*s_uTex0Data) == ZZOglGet_psm_TexBitsFix(data[0]) ) { // psm is the same
	if (ZZOglAllExceptClutIsSame(s_uTex0Data, data))
	{
		if (!PSMT_ISCLUT(psm)) return;

		// have to write the CLUT again if changed
		if (ZZOglClutMinusCLDunchanged(s_uTex0Data, data))
		{
			tex0.cld = ZZOglGet_cld_TexBits(data[1]);

			if (tex0.cld != 0)
			{
				ZeroGS::texClutWrite(i);
				// invalidate to make sure target didn't change!
				ZeroGS::vb[i].bVarsTexSync = false;
			}

			return;
		}
	}

	ZeroGS::Flush(i);

	ZeroGS::vb[i].bVarsTexSync = false;
	ZeroGS::vb[i].bTexConstsSync = false;

	s_uTex0Data[0] = (s_uTex0Data[0] & ~0x03f00000) | (psm << 20);
	s_uTex0Data[1] = (s_uTex0Data[1] & 0x1f) | (data[1] & ~0x1f);

	tex0.psm = ZZOglGet_psm_TexBitsFix(data[0]);

	if (PSMT_ISCLUT(tex0.psm)) ZeroGS::CluttingForFlushedTex(&tex0, data[1], i);
}

__forceinline void frameWrite(int i, u32 *data)
{
	FUNCLOG
	frameInfo& gsfb = ZeroGS::vb[i].gsfb;

	if ((gsfb.fbp == ZZOglGet_fbp_FrameBitsMult(data[0])) &&
			(gsfb.fbw == ZZOglGet_fbw_FrameBitsMult(data[0])) &&
			(gsfb.psm == ZZOglGet_psm_FrameBits(data[0])) &&
			(gsfb.fbm == ZZOglGet_fbm_FrameBits(data[0])))
	{
		return;
	}

	ZeroGS::FlushBoth();

	gsfb.fbp = ZZOglGet_fbp_FrameBitsMult(data[0]);
	gsfb.fbw = ZZOglGet_fbw_FrameBitsMult(data[0]);
	gsfb.psm = ZZOglGet_psm_FrameBits(data[0]);
	gsfb.fbm = ZZOglGet_fbm_FrameBitsFix(data[0], data[1]);
	gsfb.fbh = ZZOglGet_fbh_FrameBitsCalc(data[0]);
//	gsfb.fbhCalc = gsfb.fbh;

	ZeroGS::vb[i].bNeedFrameCheck = 1;
}

__forceinline void testWrite(int i, u32 *data)
{
	FUNCLOG
	pixTest* test = &ZeroGS::vb[i].test;

	if ((*(u32*)test & 0x0007ffff) == (data[0] & 0x0007ffff)) return;

	ZeroGS::Flush(i);

	*(u32*)test = data[0];

//  test.ate   = (data[0]	  ) & 0x1;
//  test.atst  = (data[0] >>  1) & 0x7;
//  test.aref  = (data[0] >>  4) & 0xff;
//  test.afail = (data[0] >> 12) & 0x3;
//  test.date  = (data[0] >> 14) & 0x1;
//  test.datm  = (data[0] >> 15) & 0x1;
//  test.zte   = (data[0] >> 16) & 0x1;
//  test.ztst  = (data[0] >> 17) & 0x3;
}

void clampWrite(int i, u32 *data)
{
	FUNCLOG
	clampInfo& clamp = ZeroGS::vb[i].clamp;

	if ((s_uClampData[i] != data[0]) || (((clamp.minv >> 8) | (clamp.maxv << 2)) != (data[1]&0x0fff)))
	{
		ZeroGS::Flush(i);
		s_uClampData[i] = data[0];

		clamp.wms  = (data[0]) & 0x3;
		clamp.wmt  = (data[0] >>  2) & 0x3;
		clamp.minu = (data[0] >>  4) & 0x3ff;
		clamp.maxu = (data[0] >> 14) & 0x3ff;
		clamp.minv = ((data[0] >> 24) & 0xff) | ((data[1] & 0x3) << 8);
		clamp.maxv = (data[1] >> 2) & 0x3ff;

		ZeroGS::vb[i].bTexConstsSync = false;
	}
}

void __fastcall GIFRegHandlerNull(u32* data)
{
	FUNCLOG
#ifdef _DEBUG

	if ((((uptr)&data[2])&0xffff) == 0) return;

	// 0x7f happens on a lot of games
	if (data[2] != 0x7f && (data[0] || data[1]))
	{
		ZZLog::Debug_Log("Unexpected reg handler %x %x %x.", data[0], data[1], data[2]);
	}

#endif
}

void __fastcall GIFRegHandlerPRIM(u32 *data)
{
	FUNCLOG

	//if (data[0] & ~0x3ff)
	//{
		//ZZLog::Warn_Log("Warning: unknown bits in prim %8.8lx_%8.8lx", data[1], data[0]);
	//}

	// Come back to this one...
	gs.nTriFanVert = gs.primIndex;

	gs.primC = 0;
	prim->prim = (data[0]) & 0x7;
	gs._prim[0].prim = (data[0]) & 0x7;
	gs._prim[1].prim = (data[0]) & 0x7;
	gs._prim[1]._val = (data[0] >> 3) & 0xff;

	ZeroGS::Prim();
}

void __fastcall GIFRegHandlerRGBAQ(u32* data)
{
	FUNCLOG
	GIFRegRGBAQ* r = (GIFRegRGBAQ*)(data);
	gs.rgba = (r->R | (r->G <<  8) | (r->B << 16) | (r->A << 24));
	gs.vertexregs.rgba = gs.rgba;
	gs.vertexregs.q = r->Q;
}

void __fastcall GIFRegHandlerST(u32* data)
{
	FUNCLOG
	GIFRegST* r = (GIFRegST*)(data);
	gs.vertexregs.s = r->S;
	gs.vertexregs.t = r->T;
}

void __fastcall GIFRegHandlerUV(u32* data)
{
	FUNCLOG
	GIFRegUV* r = (GIFRegUV*)(data);
	gs.vertexregs.u = r->U;
	gs.vertexregs.v = r->V;
}

void __fastcall GIFRegHandlerXYZF2(u32* data)
{
	FUNCLOG
	GIFRegXYZF* r = (GIFRegXYZF*)(data);
	gs.vertexregs.x = r->X;
	gs.vertexregs.y = r->Y;
	gs.vertexregs.z = r->Z;
	gs.vertexregs.f = r->F;
	gs.gsvertex[gs.primIndex] = gs.vertexregs;
	gs.primIndex = (gs.primIndex + 1) % ARRAY_SIZE(gs.gsvertex);

	KICK_VERTEX2();
}

void __fastcall GIFRegHandlerXYZ2(u32* data)
{
	FUNCLOG
	GIFRegXYZ* r = (GIFRegXYZ*)(data);
	gs.vertexregs.x = r->X;
	gs.vertexregs.y = r->Y;
	gs.vertexregs.z = r->Z;
	gs.gsvertex[gs.primIndex] = gs.vertexregs;
	gs.primIndex = (gs.primIndex + 1) % ARRAY_SIZE(gs.gsvertex);

	KICK_VERTEX2();
}

void __fastcall GIFRegHandlerTEX0_1(u32* data)
{
	FUNCLOG

	// Worry about this later.
	if (!NoHighlights(0)) return;

	tex0Write(0, data);
}

void __fastcall GIFRegHandlerTEX0_2(u32* data)
{
	FUNCLOG

	// Worry about this later.
	if (!NoHighlights(1)) return;

	tex0Write(1, data);
}

void __fastcall GIFRegHandlerCLAMP_1(u32* data)
{
	FUNCLOG

	// Worry about this later.
	if (!NoHighlights(0)) return;

	clampWrite(0, data);
}

void __fastcall GIFRegHandlerCLAMP_2(u32* data)
{
	FUNCLOG

	// Worry about this later.
	if (!NoHighlights(1)) return;

	clampWrite(1, data);
}

void __fastcall GIFRegHandlerFOG(u32* data)
{
	FUNCLOG
	GIFRegFOG* r = (GIFRegFOG*)(data);
	gs.vertexregs.f = r->F;
}

void __fastcall GIFRegHandlerXYZF3(u32* data)
{
	FUNCLOG
	GIFRegXYZF* r = (GIFRegXYZF*)(data);
	gs.vertexregs.x = r->X;
	gs.vertexregs.y = r->Y;
	gs.vertexregs.z = r->Z;
	gs.vertexregs.f = r->F;
	gs.gsvertex[gs.primIndex] = gs.vertexregs;
	gs.primIndex = (gs.primIndex + 1) % ARRAY_SIZE(gs.gsvertex);

	KICK_VERTEX3();
}

void __fastcall GIFRegHandlerXYZ3(u32* data)
{
	FUNCLOG
	GIFRegXYZ* r = (GIFRegXYZ*)(data);
	gs.vertexregs.x = r->X;
	gs.vertexregs.y = r->Y;
	gs.vertexregs.z = r->Z;
	gs.gsvertex[gs.primIndex] = gs.vertexregs;
	gs.primIndex = (gs.primIndex + 1) % ARRAY_SIZE(gs.gsvertex);

	KICK_VERTEX3();
}

void __fastcall GIFRegHandlerNOP(u32* data)
{
	FUNCLOG
}

void tex1Write(int i, u32* data)
{
	FUNCLOG
	tex1Info& tex1 = ZeroGS::vb[i].tex1;

	if (conf.bilinear == 1 && (tex1.mmag != ((data[0] >>  5) & 0x1) || tex1.mmin != ((data[0] >>  6) & 0x7)))
	{
		ZeroGS::Flush(i);
		ZeroGS::vb[i].bVarsTexSync = false;
	}

	tex1.lcm  = (data[0]) & 0x1;

	tex1.mxl  = (data[0] >>  2) & 0x7;
	tex1.mmag = (data[0] >>  5) & 0x1;
	tex1.mmin = (data[0] >>  6) & 0x7;
	tex1.mtba = (data[0] >>  9) & 0x1;
	tex1.l	= (data[0] >> 19) & 0x3;
	tex1.k	= (data[1] >> 4) & 0xff;
}

void __fastcall GIFRegHandlerTEX1_1(u32* data)
{
	FUNCLOG

	// Worry about this later.
	if (!NoHighlights(0)) return;

	tex1Write(0, data);
}

void __fastcall GIFRegHandlerTEX1_2(u32* data)
{
	FUNCLOG

	// Worry about this later.
	if (!NoHighlights(1)) return;

	tex1Write(1, data);
}

void __fastcall GIFRegHandlerTEX2_1(u32* data)
{
	FUNCLOG
	tex2Write(0, data);
}

void __fastcall GIFRegHandlerTEX2_2(u32* data)
{
	FUNCLOG
	tex2Write(1, data);
}

void __fastcall GIFRegHandlerXYOFFSET_1(u32* data)
{
	FUNCLOG
	GIFRegXYOFFSET* r = (GIFRegXYOFFSET*)(data);
	ZeroGS::vb[0].offset.x = r->OFX;
	ZeroGS::vb[0].offset.y = r->OFY;
}

void __fastcall GIFRegHandlerXYOFFSET_2(u32* data)
{
	FUNCLOG
	GIFRegXYOFFSET* r = (GIFRegXYOFFSET*)(data);
	ZeroGS::vb[1].offset.x = r->OFX;
	ZeroGS::vb[1].offset.y = r->OFY;
}

void __fastcall GIFRegHandlerPRMODECONT(u32* data)
{
	FUNCLOG
	GIFRegPRMODECONT* r = (GIFRegPRMODECONT*)(data);
	gs.prac = r->AC;
	prim = &gs._prim[gs.prac];

	ZeroGS::Prim();
}

void __fastcall GIFRegHandlerPRMODE(u32* data)
{
	FUNCLOG
	//GIFRegPRMODE* r = (GIFRegPRMODE*)(data);
	// Re-examine all code dealing with PRIMs in a bit.
	gs._prim[0]._val = (data[0] >> 3) & 0xff;

	if (gs.prac == 0) ZeroGS::Prim();
}

void __fastcall GIFRegHandlerTEXCLUT(u32* data)
{
	FUNCLOG
	GIFRegTEXCLUT* r = (GIFRegTEXCLUT*)(data);

	if (ZeroGS::vb[0].bNeedTexCheck) ZeroGS::vb[0].FlushTexData();
	if (ZeroGS::vb[1].bNeedTexCheck) ZeroGS::vb[1].FlushTexData();
	
	// Multipliers? Fixme.
	gs.clut.cbw = r->CBW  * 64;
	gs.clut.cou = r->COU  * 16;
	gs.clut.cov = r->COV;
}

void __fastcall GIFRegHandlerSCANMSK(u32* data)
{
	FUNCLOG
	GIFRegSCANMSK* r = (GIFRegSCANMSK*)(data);
//  ZeroGS::FlushBoth();
//  ZeroGS::ResolveC(&ZeroGS::vb[0]);
//  ZeroGS::ResolveZ(&ZeroGS::vb[0]);
	// GSdx flushes here, like this:
	/*if(r->SCANMSK != gs.smask)
	{
		//Flush
	}*/
	
	gs.smask = r->MSK;
}

void __fastcall GIFRegHandlerMIPTBP1_1(u32* data)
{
	FUNCLOG
	GIFRegMIPTBP1* r = (GIFRegMIPTBP1*)(data);
	// Similar here.
	/*if(PRIM->CTXT == i && r->MIPTBP1 != m_env.CTXT[i].MIPTBP1)
	{
		Flush();
	}*/
	
	miptbpInfo& miptbp0 = ZeroGS::vb[0].miptbp0;
	miptbp0.tbp[0] = r->TBP1;
	miptbp0.tbw[0] = r->TBW1;
	miptbp0.tbp[1] = r->TBP2;
	miptbp0.tbw[1] = r->TBW2;
	miptbp0.tbp[2] = r->TBP3;
	miptbp0.tbw[2] = r->TBW3;
}

void __fastcall GIFRegHandlerMIPTBP1_2(u32* data)
{
	FUNCLOG
	GIFRegMIPTBP1* r = (GIFRegMIPTBP1*)(data);
	// And here.
	
	miptbpInfo& miptbp0 = ZeroGS::vb[1].miptbp0;
	miptbp0.tbp[0] = r->TBP1;
	miptbp0.tbw[0] = r->TBW1;
	miptbp0.tbp[1] = r->TBP2;
	miptbp0.tbw[1] = r->TBW2;
	miptbp0.tbp[2] = r->TBP3;
	miptbp0.tbw[2] = r->TBW3;
}

void __fastcall GIFRegHandlerMIPTBP2_1(u32* data)
{
	FUNCLOG
	GIFRegMIPTBP2* r = (GIFRegMIPTBP2*)(data);
	// Yep.
	
	miptbpInfo& miptbp1 = ZeroGS::vb[0].miptbp1;
	miptbp1.tbp[0] = r->TBP4;
	miptbp1.tbw[0] = r->TBW4;
	miptbp1.tbp[1] = r->TBP5;
	miptbp1.tbw[1] = r->TBW5;
	miptbp1.tbp[2] = r->TBP6;
	miptbp1.tbw[2] = r->TBW6;
}

void __fastcall GIFRegHandlerMIPTBP2_2(u32* data)
{
	FUNCLOG
	GIFRegMIPTBP2* r = (GIFRegMIPTBP2*)(data);
	// ...
	
	miptbpInfo& miptbp1 = ZeroGS::vb[1].miptbp1;
	miptbp1.tbp[0] = r->TBP4;
	miptbp1.tbw[0] = r->TBW4;
	miptbp1.tbp[1] = r->TBP5;
	miptbp1.tbw[1] = r->TBW5;
	miptbp1.tbp[2] = r->TBP6;
	miptbp1.tbw[2] = r->TBW6;
}

void __fastcall GIFRegHandlerTEXA(u32* data)
{
	FUNCLOG
	GIFRegTEXA* r = (GIFRegTEXA*)(data);
	texaInfo newinfo;
	/*if(r->TEXA != m_env.TEXA)
	{
		Flush();
	}*/
	
	newinfo.aem = r->AEM;
	newinfo.ta[0] = r->TA0;
	newinfo.ta[1] = r->TA1;

	if (*(u32*)&newinfo != *(u32*)&gs.texa)
	{
		ZeroGS::FlushBoth();
		
		*(u32*)&gs.texa = *(u32*) & newinfo;
		
		gs.texa.fta[0] = newinfo.ta[0] / 255.0f;
		gs.texa.fta[1] = newinfo.ta[1] / 255.0f;

		ZeroGS::vb[0].bTexConstsSync = false;
		ZeroGS::vb[1].bTexConstsSync = false;
	}
}

void __fastcall GIFRegHandlerFOGCOL(u32* data)
{
	FUNCLOG
	//GIFRegFOGCOL* r = (GIFRegFOGCOL*)(data);
	
	// Yeah, it even flushes here.
	// I'll worry about changing this later.
	ZeroGS::SetFogColor(data[0]&0xffffff);
}

void __fastcall GIFRegHandlerTEXFLUSH(u32* data)
{
	FUNCLOG
	// GSdx doesn't even do anything here.
	ZeroGS::SetTexFlush();
}

void __fastcall GIFRegHandlerSCISSOR_1(u32* data)
{
	FUNCLOG
	GIFRegSCISSOR* r = (GIFRegSCISSOR*)(data);
	Rect2& scissor = ZeroGS::vb[0].scissor;

	Rect2 newscissor;

	// << 3?
	newscissor.x0 = r->SCAX0 << 3;
	newscissor.x1 = r->SCAX1 << 3;
	newscissor.y0 = r->SCAY0 << 3;
	newscissor.y1 = r->SCAY1 << 3;

	if (newscissor.x1 != scissor.x1 || newscissor.y1 != scissor.y1 ||
			newscissor.x0 != scissor.x0 || newscissor.y0 != scissor.y0)
	{
		ZeroGS::Flush(0);
		scissor = newscissor;
		ZeroGS::vb[0].bNeedFrameCheck = 1;
	}
	
	//Hmm...
	/*
	if(PRIM->CTXT == i && r->SCISSOR != m_env.CTXT[i].SCISSOR)
	{
		Flush();
	}

	m_env.CTXT[i].SCISSOR = (GSVector4i)r->SCISSOR;

	m_env.CTXT[i].UpdateScissor();*/
}

void __fastcall GIFRegHandlerSCISSOR_2(u32* data)
{
	FUNCLOG
	GIFRegSCISSOR* r = (GIFRegSCISSOR*)(data);
	Rect2& scissor = ZeroGS::vb[1].scissor;

	Rect2 newscissor;
	
	// << 3?
	newscissor.x0 = r->SCAX0 << 3;
	newscissor.x1 = r->SCAX1 << 3;
	newscissor.y0 = r->SCAY0 << 3;
	newscissor.y1 = r->SCAY1 << 3;

	if (newscissor.x1 != scissor.x1 || newscissor.y1 != scissor.y1 ||
			newscissor.x0 != scissor.x0 || newscissor.y0 != scissor.y0)
	{
		ZeroGS::Flush(1);
		scissor = newscissor;

		// flush everything
		ZeroGS::vb[1].bNeedFrameCheck = 1;
	}
}

void __fastcall GIFRegHandlerALPHA_1(u32* data)
{
	FUNCLOG
	// Another tangled one. Mess with this later.
	alphaInfo newalpha;
	newalpha.abcd = *(u8*)data;
	newalpha.fix = *(u8*)(data + 1);

	if (*(u16*)&newalpha != *(u16*)&ZeroGS::vb[0].alpha)
	{
		ZeroGS::Flush(0);

		if (newalpha.a == 3) newalpha.a = 0;
		if (newalpha.b == 3) newalpha.b = 0;
		if (newalpha.c == 3) newalpha.c = 0;
		if (newalpha.d == 3) newalpha.d = 0;

		*(u16*)&ZeroGS::vb[0].alpha = *(u16*) & newalpha;
	}
}

void __fastcall GIFRegHandlerALPHA_2(u32* data)
{
	FUNCLOG
	// ...
	alphaInfo newalpha;
	newalpha.abcd = *(u8*)data;
	newalpha.fix = *(u8*)(data + 1);

	if (*(u16*)&newalpha != *(u16*)&ZeroGS::vb[1].alpha)
	{
		ZeroGS::Flush(1);

		if (newalpha.a == 3) newalpha.a = 0;
		if (newalpha.b == 3) newalpha.b = 0;
		if (newalpha.c == 3) newalpha.c = 0;
		if (newalpha.d == 3) newalpha.d = 0;

		*(u16*)&ZeroGS::vb[1].alpha = *(u16*) & newalpha;
	}
}

void __fastcall GIFRegHandlerDIMX(u32* data)
{
	FUNCLOG
	// Not even handled? Fixme.
	/*
	bool update = false;

	if(r->DIMX != m_env.DIMX)
	{
		Flush();

		update = true;
	}

	m_env.DIMX = (GSVector4i)r->DIMX;

	if(update)
	{
		m_env.UpdateDIMX();
	}*/
}

void __fastcall GIFRegHandlerDTHE(u32* data)
{
	FUNCLOG
	GIFRegDTHE* r = (GIFRegDTHE*)(data);
	// Flush me.
	gs.dthe = r->DTHE;
}

void __fastcall GIFRegHandlerCOLCLAMP(u32* data)
{
	FUNCLOG
	GIFRegCOLCLAMP* r = (GIFRegCOLCLAMP*)(data);
	gs.colclamp = r->CLAMP;
#ifdef DISABLE_COLCLAMP
	gs.colclamp = 1;
#endif
}

void __fastcall GIFRegHandlerTEST_1(u32* data)
{
	FUNCLOG
	testWrite(0, data);
}

void __fastcall GIFRegHandlerTEST_2(u32* data)
{
	FUNCLOG
	testWrite(1, data);
}

void __fastcall GIFRegHandlerPABE(u32* data)
{
	FUNCLOG
	GIFRegPABE* r = (GIFRegPABE*)(data);
	//ZeroGS::SetAlphaChanged(0, GPUREG_PABE);
	//ZeroGS::SetAlphaChanged(1, GPUREG_PABE);
	ZeroGS::FlushBoth();

	gs.pabe = r->PABE;
}

void __fastcall GIFRegHandlerFBA_1(u32* data)
{
	FUNCLOG
	GIFRegFBA* r = (GIFRegFBA*)(data);
	
	ZeroGS::FlushBoth();
	ZeroGS::vb[0].fba.fba = r->FBA;
}

void __fastcall GIFRegHandlerFBA_2(u32* data)
{
	FUNCLOG
	GIFRegFBA* r = (GIFRegFBA*)(data);
	
	ZeroGS::FlushBoth();
	
	ZeroGS::vb[1].fba.fba = r->FBA;
}

void __fastcall GIFRegHandlerFRAME_1(u32* data)
{
	FUNCLOG
	frameWrite(0, data);
}

void __fastcall GIFRegHandlerFRAME_2(u32* data)
{
	FUNCLOG
	frameWrite(1, data);
}

void __fastcall GIFRegHandlerZBUF_1(u32* data)
{
	FUNCLOG
	// I'll wait a bit on this one.
	//GIFRegZBUF* r = (GIFRegZBUF*)(data);
	
	zbufInfo& zbuf = ZeroGS::vb[0].zbuf;
	int psm = (0x30 | ((data[0] >> 24) & 0xf));

	if (zbuf.zbp == (data[0] & 0x1ff) * 32 &&
			zbuf.psm == psm &&
			zbuf.zmsk == (data[1] & 0x1))
	{
		return;
	}

	// error detection
	if (m_Blocks[psm].bpp == 0) return;

	ZeroGS::FlushBoth();

	zbuf.zbp = (data[0] & 0x1ff) * 32;
	zbuf.psm = 0x30 | ((data[0] >> 24) & 0xf);
	zbuf.zmsk = data[1] & 0x1;

	ZeroGS::vb[0].zprimmask = 0xffffffff;

	if (zbuf.psm > 0x31) ZeroGS::vb[0].zprimmask = 0xffff;

	ZeroGS::vb[0].bNeedZCheck = 1;
}

void __fastcall GIFRegHandlerZBUF_2(u32* data)
{
	FUNCLOG
	// And this.
	//GIFRegZBUF* r = (GIFRegZBUF*)(data);
	zbufInfo& zbuf = ZeroGS::vb[1].zbuf;

	int psm = (0x30 | ((data[0] >> 24) & 0xf));

	if (zbuf.zbp == (data[0] & 0x1ff) * 32 &&
			zbuf.psm == psm &&
			zbuf.zmsk == (data[1] & 0x1))
	{
		return;
	}

	// error detection
	if (m_Blocks[psm].bpp == 0) return;

	ZeroGS::FlushBoth();

	zbuf.zbp = (data[0] & 0x1ff) * 32;

	zbuf.psm = 0x30 | ((data[0] >> 24) & 0xf);

	zbuf.zmsk = data[1] & 0x1;

	ZeroGS::vb[1].bNeedZCheck = 1;
	ZeroGS::vb[1].zprimmask = 0xffffffff;

	if (zbuf.psm > 0x31) ZeroGS::vb[1].zprimmask = 0xffff;
}

void __fastcall GIFRegHandlerBITBLTBUF(u32* data)
{
	FUNCLOG
	GIFRegBITBLTBUF* r = (GIFRegBITBLTBUF*)(data);
	// Wonder why the multiplier?
	gs.srcbufnew.bp  = r->SBP;   // * 64;
	gs.srcbufnew.bw  = r->SBW * 64;
	gs.srcbufnew.psm = r->SPSM;
	gs.dstbufnew.bp  = r->DBP;   // * 64;
	gs.dstbufnew.bw  = r->DBW * 64;
	gs.dstbufnew.psm = r->DPSM;

	if (gs.dstbufnew.bw == 0) gs.dstbufnew.bw = 64;
	// GSdx does this:
	
	/*if((gs.srcbufnew.bw & 1) && (gs.srcbufnew.psm == PSM_PSMT8 || gs.srcbufnew.psm == PSM_PSMT4))
	{
		gs.srcbufnew.bw &= ~1;
	}

	if((gs.dstbufnew.bw & 1) && (gs.dstbufnew.psm == PSM_PSMT8 || gs.dstbufnew.psm == PSM_PSMT4))
	{
		gs.dstbufnew.bw &= ~1; // namcoXcapcom: 5, 11, refered to as 4, 10 in TEX0.TBW later
	}*/
}

void __fastcall GIFRegHandlerTRXPOS(u32* data)
{
	FUNCLOG
	GIFRegTRXPOS* r = (GIFRegTRXPOS*)(data);
	
	gs.trxposnew.sx  = r->SSAX;
	gs.trxposnew.sy  = r->SSAY;
	gs.trxposnew.dx  = r->DSAX;
	gs.trxposnew.dy  = r->DSAY;
	
	//Fixme. DIRY & DIRX together?
	gs.trxposnew.dir = (data[1] >> 27) & 0x3;
}

void __fastcall GIFRegHandlerTRXREG(u32* data)
{
	FUNCLOG
	GIFRegTRXREG* r = (GIFRegTRXREG*)(data);
	gs.imageWtemp = r->RRW;
	gs.imageHtemp = r->RRH;
}

void __fastcall GIFRegHandlerTRXDIR(u32* data)
{
	FUNCLOG
	// Oh dear...
	
	// terminate any previous transfers

	switch (gs.imageTransfer)
	{
		case 0: // host->loc
			gs.imageTransfer = -1;
			break;

		case 1: // loc->host
			ZeroGS::TerminateLocalHost();
			break;
	}

	gs.srcbuf = gs.srcbufnew;

	gs.dstbuf = gs.dstbufnew;
	gs.trxpos = gs.trxposnew;
	gs.imageTransfer = data[0] & 0x3;
	gs.imageWnew = gs.imageWtemp;
	gs.imageHnew = gs.imageHtemp;

	if (gs.imageWnew > 0 && gs.imageHnew > 0)
	{
		switch (gs.imageTransfer)
		{
			case 0: // host->loc
				ZeroGS::InitTransferHostLocal();
				break;

			case 1: // loc->host
				ZeroGS::InitTransferLocalHost();
				break;

			case 2:
				ZeroGS::TransferLocalLocal();
				break;

			case 3:
				gs.imageTransfer = -1;
				break;

			default:
				assert(0);
		}
	}
	else
	{
#if defined(ZEROGS_DEVBUILD)
		ZZLog::Warn_Log("Dummy transfer.");
#endif
		gs.imageTransfer = -1;
	}
}

void __fastcall GIFRegHandlerHWREG(u32* data)
{
	FUNCLOG

	if (gs.imageTransfer == 0)
	{
		ZeroGS::TransferHostLocal(data, 2);
	}
	else
	{
#if defined(ZEROGS_DEVBUILD)
		ZZLog::Error_Log("ZeroGS: HWREG!? %8.8x_%8.8x", data[0], data[1]);
		//assert(0);
#endif
	}
}

extern int g_GSMultiThreaded;

void __fastcall GIFRegHandlerSIGNAL(u32* data)
{
	FUNCLOG

	if (!g_GSMultiThreaded)
	{
		SIGLBLID->SIGID = (SIGLBLID->SIGID & ~data[1]) | (data[0] & data[1]);

//	  if (gs.CSRw & 0x1) CSR->SIGNAL = 1;
//	  if (!IMR->SIGMSK && GSirq)
//		  GSirq();

		if (gs.CSRw & 0x1)
		{
			CSR->SIGNAL = 1;
			//gs.CSRw &= ~1;
		}

		if (!IMR->SIGMSK && GSirq) GSirq();
	}
}

void __fastcall GIFRegHandlerFINISH(u32* data)
{
	FUNCLOG

	if (!g_GSMultiThreaded)
	{
		if (gs.CSRw & 0x2) CSR->FINISH = 1;

		if (!IMR->FINISHMSK && GSirq) GSirq();

//	  if( gs.CSRw & 2 ) {
//		  //gs.CSRw &= ~2;
//		  //CSR->FINISH = 0;
//
//
//	  }
//	  CSR->FINISH = 1;
//
//	  if( !IMR->FINISHMSK && GSirq )
//		  GSirq();
	}
}

void __fastcall GIFRegHandlerLABEL(u32* data)
{
	FUNCLOG

	if (!g_GSMultiThreaded)
	{
		SIGLBLID->LBLID = (SIGLBLID->LBLID & ~data[1]) | (data[0] & data[1]);
	}
}


void SetMultithreaded()
{
	// Some older versions of PCSX2 didn't properly set the irq callback to NULL
	// in multithreaded mode (possibly because ZeroGS itself would assert in such
	// cases), and didn't bind them to a dummy callback either.  PCSX2 handles all
	// IRQs internally when multithreaded anyway -- so let's ignore them here:

	if (g_GSMultiThreaded)
	{
		g_GIFRegHandlers[GIF_A_D_REG_SIGNAL] = &GIFRegHandlerNull;
		g_GIFRegHandlers[GIF_A_D_REG_FINISH] = &GIFRegHandlerNull;
		g_GIFRegHandlers[GIF_A_D_REG_LABEL] = &GIFRegHandlerNull;
	}
	else
	{
		g_GIFRegHandlers[GIF_A_D_REG_SIGNAL] = &GIFRegHandlerSIGNAL;
		g_GIFRegHandlers[GIF_A_D_REG_FINISH] = &GIFRegHandlerFINISH;
		g_GIFRegHandlers[GIF_A_D_REG_LABEL] = &GIFRegHandlerLABEL;
	}
}

void ResetRegs()
{
	for (int i = 0; i < 16; i++)
	{
		g_GIFPackedRegHandlers[i] = &GIFPackedRegHandlerNull;
	}

	g_GIFPackedRegHandlers[GIF_REG_PRIM] = &GIFPackedRegHandlerPRIM;
	g_GIFPackedRegHandlers[GIF_REG_RGBA] = &GIFPackedRegHandlerRGBA;
	g_GIFPackedRegHandlers[GIF_REG_STQ] = &GIFPackedRegHandlerSTQ;
	g_GIFPackedRegHandlers[GIF_REG_UV] = &GIFPackedRegHandlerUV;
	g_GIFPackedRegHandlers[GIF_REG_XYZF2] = &GIFPackedRegHandlerXYZF2;
	g_GIFPackedRegHandlers[GIF_REG_XYZ2] = &GIFPackedRegHandlerXYZ2;
	g_GIFPackedRegHandlers[GIF_REG_TEX0_1] = &GIFPackedRegHandlerTEX0_1;
	g_GIFPackedRegHandlers[GIF_REG_TEX0_2] = &GIFPackedRegHandlerTEX0_2;
	g_GIFPackedRegHandlers[GIF_REG_CLAMP_1] = &GIFPackedRegHandlerCLAMP_1;
	g_GIFPackedRegHandlers[GIF_REG_CLAMP_2] = &GIFPackedRegHandlerCLAMP_2;
	g_GIFPackedRegHandlers[GIF_REG_FOG] = &GIFPackedRegHandlerFOG;
	g_GIFPackedRegHandlers[GIF_REG_XYZF3] = &GIFPackedRegHandlerXYZF3;
	g_GIFPackedRegHandlers[GIF_REG_XYZ3] = &GIFPackedRegHandlerXYZ3;
	g_GIFPackedRegHandlers[GIF_REG_A_D] = &GIFPackedRegHandlerA_D;
	g_GIFPackedRegHandlers[GIF_REG_NOP] = &GIFPackedRegHandlerNOP;
	
	for (int i = 0; i < 256; i++)
	{
		g_GIFRegHandlers[i] = &GIFPackedRegHandlerNull;
	}
	
	g_GIFRegHandlers[GIF_A_D_REG_PRIM] = &GIFRegHandlerPRIM;
	g_GIFRegHandlers[GIF_A_D_REG_RGBAQ] = &GIFRegHandlerRGBAQ;
	g_GIFRegHandlers[GIF_A_D_REG_ST] = &GIFRegHandlerST;
	g_GIFRegHandlers[GIF_A_D_REG_UV] = &GIFRegHandlerUV;
	g_GIFRegHandlers[GIF_A_D_REG_XYZF2] = &GIFRegHandlerXYZF2;
	g_GIFRegHandlers[GIF_A_D_REG_XYZ2] = &GIFRegHandlerXYZ2;
	g_GIFRegHandlers[GIF_A_D_REG_TEX0_1] = &GIFRegHandlerTEX0_1;
	g_GIFRegHandlers[GIF_A_D_REG_TEX0_2] = &GIFRegHandlerTEX0_2;
	g_GIFRegHandlers[GIF_A_D_REG_CLAMP_1] = &GIFRegHandlerCLAMP_1;
	g_GIFRegHandlers[GIF_A_D_REG_CLAMP_2] = &GIFRegHandlerCLAMP_2;
	g_GIFRegHandlers[GIF_A_D_REG_FOG] = &GIFRegHandlerFOG;
	g_GIFRegHandlers[GIF_A_D_REG_XYZF3] = &GIFRegHandlerXYZF3;
	g_GIFRegHandlers[GIF_A_D_REG_XYZ3] = &GIFRegHandlerXYZ3;
	g_GIFRegHandlers[GIF_A_D_REG_NOP] = &GIFRegHandlerNOP;
	g_GIFRegHandlers[GIF_A_D_REG_TEX1_1] = &GIFRegHandlerTEX1_1;
	g_GIFRegHandlers[GIF_A_D_REG_TEX1_2] = &GIFRegHandlerTEX1_2;
	g_GIFRegHandlers[GIF_A_D_REG_TEX2_1] = &GIFRegHandlerTEX2_1;
	g_GIFRegHandlers[GIF_A_D_REG_TEX2_2] = &GIFRegHandlerTEX2_2;
	g_GIFRegHandlers[GIF_A_D_REG_XYOFFSET_1] = &GIFRegHandlerXYOFFSET_1;
	g_GIFRegHandlers[GIF_A_D_REG_XYOFFSET_2] = &GIFRegHandlerXYOFFSET_2;
	g_GIFRegHandlers[GIF_A_D_REG_PRMODECONT] = &GIFRegHandlerPRMODECONT;
	g_GIFRegHandlers[GIF_A_D_REG_PRMODE] = &GIFRegHandlerPRMODE;
	g_GIFRegHandlers[GIF_A_D_REG_TEXCLUT] = &GIFRegHandlerTEXCLUT;
	g_GIFRegHandlers[GIF_A_D_REG_SCANMSK] = &GIFRegHandlerSCANMSK;
	g_GIFRegHandlers[GIF_A_D_REG_MIPTBP1_1] = &GIFRegHandlerMIPTBP1_1;
	g_GIFRegHandlers[GIF_A_D_REG_MIPTBP1_2] = &GIFRegHandlerMIPTBP1_2;
	g_GIFRegHandlers[GIF_A_D_REG_MIPTBP2_1] = &GIFRegHandlerMIPTBP2_1;
	g_GIFRegHandlers[GIF_A_D_REG_MIPTBP2_2] = &GIFRegHandlerMIPTBP2_2;
	g_GIFRegHandlers[GIF_A_D_REG_TEXA] = &GIFRegHandlerTEXA;
	g_GIFRegHandlers[GIF_A_D_REG_FOGCOL] = &GIFRegHandlerFOGCOL;
	g_GIFRegHandlers[GIF_A_D_REG_TEXFLUSH] = &GIFRegHandlerTEXFLUSH;
	g_GIFRegHandlers[GIF_A_D_REG_SCISSOR_1] = &GIFRegHandlerSCISSOR_1;
	g_GIFRegHandlers[GIF_A_D_REG_SCISSOR_2] = &GIFRegHandlerSCISSOR_2;
	g_GIFRegHandlers[GIF_A_D_REG_ALPHA_1] = &GIFRegHandlerALPHA_1;
	g_GIFRegHandlers[GIF_A_D_REG_ALPHA_2] = &GIFRegHandlerALPHA_2;
	g_GIFRegHandlers[GIF_A_D_REG_DIMX] = &GIFRegHandlerDIMX;
	g_GIFRegHandlers[GIF_A_D_REG_DTHE] = &GIFRegHandlerDTHE;
	g_GIFRegHandlers[GIF_A_D_REG_COLCLAMP] = &GIFRegHandlerCOLCLAMP;
	g_GIFRegHandlers[GIF_A_D_REG_TEST_1] = &GIFRegHandlerTEST_1;
	g_GIFRegHandlers[GIF_A_D_REG_TEST_2] = &GIFRegHandlerTEST_2;
	g_GIFRegHandlers[GIF_A_D_REG_PABE] = &GIFRegHandlerPABE;
	g_GIFRegHandlers[GIF_A_D_REG_FBA_1] = &GIFRegHandlerFBA_1;
	g_GIFRegHandlers[GIF_A_D_REG_FBA_2] = &GIFRegHandlerFBA_2;
	g_GIFRegHandlers[GIF_A_D_REG_FRAME_1] = &GIFRegHandlerFRAME_1;
	g_GIFRegHandlers[GIF_A_D_REG_FRAME_2] = &GIFRegHandlerFRAME_2;
	g_GIFRegHandlers[GIF_A_D_REG_ZBUF_1] = &GIFRegHandlerZBUF_1;
	g_GIFRegHandlers[GIF_A_D_REG_ZBUF_2] = &GIFRegHandlerZBUF_2;
	g_GIFRegHandlers[GIF_A_D_REG_BITBLTBUF] = &GIFRegHandlerBITBLTBUF;
	g_GIFRegHandlers[GIF_A_D_REG_TRXPOS] = &GIFRegHandlerTRXPOS;
	g_GIFRegHandlers[GIF_A_D_REG_TRXREG] = &GIFRegHandlerTRXREG;
	g_GIFRegHandlers[GIF_A_D_REG_TRXDIR] = &GIFRegHandlerTRXDIR;
	g_GIFRegHandlers[GIF_A_D_REG_HWREG] = &GIFRegHandlerHWREG;
	SetMultithreaded();
}

void WriteTempRegs()
{
	memcpy(g_GIFTempRegHandlers, g_GIFPackedRegHandlers, sizeof(g_GIFTempRegHandlers));
}

void SetFrameSkip(bool skip)
{
	if (skip)
	{
		g_GIFPackedRegHandlers[GIF_REG_PRIM] = &GIFPackedRegHandlerNOP;
		g_GIFPackedRegHandlers[GIF_REG_RGBA] = &GIFPackedRegHandlerNOP;
		g_GIFPackedRegHandlers[GIF_REG_STQ] = &GIFPackedRegHandlerNOP;
		g_GIFPackedRegHandlers[GIF_REG_UV] = &GIFPackedRegHandlerNOP;
		g_GIFPackedRegHandlers[GIF_REG_XYZF2] = &GIFPackedRegHandlerNOP;
		g_GIFPackedRegHandlers[GIF_REG_XYZ2] = &GIFPackedRegHandlerNOP;
		g_GIFPackedRegHandlers[GIF_REG_CLAMP_1] = &GIFPackedRegHandlerNOP;
		g_GIFPackedRegHandlers[GIF_REG_CLAMP_2] = &GIFPackedRegHandlerNOP;
		g_GIFPackedRegHandlers[GIF_REG_FOG] = &GIFPackedRegHandlerNOP;
		g_GIFPackedRegHandlers[GIF_REG_XYZF3] = &GIFPackedRegHandlerNOP;
		g_GIFPackedRegHandlers[GIF_REG_XYZ3] = &GIFPackedRegHandlerNOP;

		g_GIFRegHandlers[GIF_A_D_REG_PRIM] = &GIFRegHandlerNOP;
		g_GIFRegHandlers[GIF_A_D_REG_RGBAQ] = &GIFRegHandlerNOP;
		g_GIFRegHandlers[GIF_A_D_REG_ST] = &GIFRegHandlerNOP;
		g_GIFRegHandlers[GIF_A_D_REG_UV] = &GIFRegHandlerNOP;
		g_GIFRegHandlers[GIF_A_D_REG_XYZF2] = &GIFRegHandlerNOP;
		g_GIFRegHandlers[GIF_A_D_REG_XYZ2] = &GIFRegHandlerNOP;
		g_GIFRegHandlers[GIF_A_D_REG_XYZF3] = &GIFRegHandlerNOP;
		g_GIFRegHandlers[GIF_A_D_REG_XYZ3] = &GIFRegHandlerNOP;
		g_GIFRegHandlers[GIF_A_D_REG_PRMODECONT] = &GIFRegHandlerNOP;
		g_GIFRegHandlers[GIF_A_D_REG_PRMODE] = &GIFRegHandlerNOP;
	}
	else
	{
		g_GIFPackedRegHandlers[GIF_REG_PRIM] = &GIFPackedRegHandlerPRIM;
		g_GIFPackedRegHandlers[GIF_REG_RGBA] = &GIFPackedRegHandlerRGBA;
		g_GIFPackedRegHandlers[GIF_REG_STQ] = &GIFPackedRegHandlerSTQ;
		g_GIFPackedRegHandlers[GIF_REG_UV] = &GIFPackedRegHandlerUV;
		g_GIFPackedRegHandlers[GIF_REG_XYZF2] = &GIFPackedRegHandlerXYZF2;
		g_GIFPackedRegHandlers[GIF_REG_XYZ2] = &GIFPackedRegHandlerXYZ2;
		g_GIFPackedRegHandlers[GIF_REG_CLAMP_1] = &GIFPackedRegHandlerCLAMP_1;
		g_GIFPackedRegHandlers[GIF_REG_CLAMP_2] = &GIFPackedRegHandlerCLAMP_2;
		g_GIFPackedRegHandlers[GIF_REG_FOG] = &GIFPackedRegHandlerFOG;
		g_GIFPackedRegHandlers[GIF_REG_XYZF3] = &GIFPackedRegHandlerXYZF3;
		g_GIFPackedRegHandlers[GIF_REG_XYZ3] = &GIFPackedRegHandlerXYZ3;

		g_GIFRegHandlers[GIF_A_D_REG_PRIM] = &GIFRegHandlerPRIM;
		g_GIFRegHandlers[GIF_A_D_REG_RGBAQ] = &GIFRegHandlerRGBAQ;
		g_GIFRegHandlers[GIF_A_D_REG_ST] = &GIFRegHandlerST;
		g_GIFRegHandlers[GIF_A_D_REG_UV] = &GIFRegHandlerUV;
		g_GIFRegHandlers[GIF_A_D_REG_XYZF2] = &GIFRegHandlerXYZF2;
		g_GIFRegHandlers[GIF_A_D_REG_XYZ2] = &GIFRegHandlerXYZ2;
		g_GIFRegHandlers[GIF_A_D_REG_XYZF3] = &GIFRegHandlerXYZF3;
		g_GIFRegHandlers[GIF_A_D_REG_XYZ3] = &GIFRegHandlerXYZ3;
		g_GIFRegHandlers[GIF_A_D_REG_PRMODECONT] = &GIFRegHandlerPRMODECONT;
		g_GIFRegHandlers[GIF_A_D_REG_PRMODE] = &GIFRegHandlerPRMODE;
	}
}
#endif
