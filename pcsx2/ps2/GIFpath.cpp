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

#include "PrecompiledHeader.h"
#include "Common.h"
#include "GS.h"
#include "Gif.h"
#include "Vif.h"
#include "NewDmac.h"

#include <xmmintrin.h>

// --------------------------------------------------------------------------------------
//  GIFpath -- the GIFtag Parser
// --------------------------------------------------------------------------------------

enum GIF_REG
{
	GIF_REG_PRIM	= 0x00,
	GIF_REG_RGBA	= 0x01,
	GIF_REG_STQ		= 0x02,
	GIF_REG_UV		= 0x03,
	GIF_REG_XYZF2	= 0x04,
	GIF_REG_XYZ2	= 0x05,
	GIF_REG_TEX0_1	= 0x06,
	GIF_REG_TEX0_2	= 0x07,
	GIF_REG_CLAMP_1	= 0x08,
	GIF_REG_CLAMP_2	= 0x09,
	GIF_REG_FOG		= 0x0a,
	GIF_REG_XYZF3	= 0x0c,
	GIF_REG_XYZ3	= 0x0d,
	GIF_REG_A_D		= 0x0e,
	GIF_REG_NOP		= 0x0f,
};

typedef void (__fastcall *GIFRegHandler)(const u32* data);

__aligned16 GIFPath		g_gifpath;

// --------------------------------------------------------------------------------------
//  SIGNAL / FINISH / LABEL
// --------------------------------------------------------------------------------------

bool SIGNAL_IMR_Pending = false;
u32 SIGNAL_Data_Pending[2];

// SIGNAL : This register is a double-throw.  If the SIGNAL bit in CSR is clear, set the CSR
//   and raise a gsIrq.  If CSR is already *set*, then do not raise a gsIrq, and ignore all
//   subsequent drawing operations and writes to general purpose registers to the GS. (note:
//   I'm pretty sure this includes direct GS and GSreg accesses, as well as those coming
//   through the GIFpath -- but that behavior isn't confirmed yet).  Privileged writes are
//   still active.
//
//   Ignorance continues until the SIGNAL bit in CSR is manually cleared by the EE.  And here's
//   the tricky part: the interrupt from the second SIGNAL is still pending, and should be
//   raised once the EE has reset the *IMR* mask for SIGNAL -- meaning setting the bit to 1
//   (disabled/masked) and then back to 0 (enabled/unmasked).  Until the *IMR* is cleared, the
//   SIGNAL is still in the second throw stage, and will freeze the GS upon being written.
//
static void __fastcall RegHandlerSIGNAL(const u32* data)
{
	// HACK:
	// Soul Calibur 3 seems to be doing SIGNALs on PATH2 and PATH3 simultaneously, and isn't
	// too happy with the results (dies on bootup).  It properly clears the SIGNAL interrupt
	// but seems to get stuck on a VBLANK OVERLAP loop.  Fixing SIGNAL so that it properly
	// stalls the GIF might fix it.  Investigating the game's internals more deeply may also
	// be revealing. --air

	if (CSRreg.SIGNAL)
	{
		// Time to ignore all subsequent drawing operations.
		if (!SIGNAL_IMR_Pending)
		{
			//DevCon.WriteLn( Color_StrongOrange, "GS SIGNAL double throw encountered!" );
			SIGNAL_IMR_Pending	= true;
			SIGNAL_Data_Pending[0]	= data[0];
			SIGNAL_Data_Pending[1]	= data[1];

			throw Exception::SignalDoubleIMR();
		}
	}
	else
	{
		GIF_LOG("GS SIGNAL data=%x_%x IMR=%x CSRr=%x",data[0], data[1], GSIMR, GSCSRr);
		GSSIGLBLID.SIGID = (GSSIGLBLID.SIGID&~data[1])|(data[0]&data[1]);

		if (!(GSIMR&0x100))
			gsIrq();

		CSRreg.SIGNAL = true;
	}
}

// FINISH : Enables end-of-draw signaling.  When FINISH is written it tells the GIF to
//   raise a gsIrq and set the FINISH bit of CSR when the *current drawing operation* is
//   finished.  Translation: Only after all three logical GIFpaths are in EOP status.
//
//   This feature can be used for both reversing the GS transfer mode (downloading post-
//   processing effects to the EE), and more importantly for *DMA synch* between the
//   three logical GIFpaths.
//
static void __fastcall RegHandlerFINISH(const u32* data)
{
	GifTagLog("GIFpath FINISH data=%x_%x CSRr=%x", data[0], data[1], GSCSRr);

	// The FINISH bit is set here, and then it will be cleared when all three
	// logical GIFpaths finish their packets (EOPs) At that time (found below
	// in the GIFpath_Parser), IMR is tested and a gsIrq() raised if needed.

	CSRreg.FINISH = true;
}

static void __fastcall RegHandlerLABEL(const u32* data)
{
	GifTagLog( "GIFpath LABEL" );
	GSSIGLBLID.LBLID = (GSSIGLBLID.LBLID&~data[1])|(data[0]&data[1]);
}

static void __fastcall RegHandlerUNMAPPED(const u32* data)
{
	const int regidx = ((u8*)data)[8];

	// Known "unknowns":
	//  It's possible that anything above 0x63 should just be silently ignored, but in the
	//  offhand chance not, I'm documenting known cases of unknown register use here.
	//
	//  0x7F -->
	//   the bios likes to write to 0x7f using an EOP giftag with NLOOP set to 4.
	//   Not sure what it's trying to accomplish exactly.  Ignoring seems to work fine,
	//   and is probably the intended behavior (it's likely meant to be a NOP).
	//
	//  0xEE -->
	//   .hack Infection [PAL confirmed, NTSC unknown] uses 0xee when you zoom the camera.
	//   The use hasn't been researched yet so parameters are unknown.  Everything seems
	//   to work fine as usual -- The 0xEE address in common programming terms is typically
	//   left over uninitialized data, and this might be a case of that, which is to be
	//   silently ignored.
	//
	//  Guitar Hero 3+ : Massive spamming when using superVU (along with several VIF errors)
	//  Using microVU avoids the GIFtag errors, so probably just one of sVU's hacks conflicting
	//  with one of VIF's hacks, and causing corrupted packet data.

	if( regidx != 0x7f && regidx != 0xee )
		DbgCon.Warning( "Ignoring Unmapped GIFtag Register, Index = %02x", regidx );
}

#define INSERT_UNMAPPED_4	RegHandlerUNMAPPED, RegHandlerUNMAPPED, RegHandlerUNMAPPED, RegHandlerUNMAPPED,
#define INSERT_UNMAPPED_16	INSERT_UNMAPPED_4 INSERT_UNMAPPED_4 INSERT_UNMAPPED_4 INSERT_UNMAPPED_4
#define INSERT_UNMAPPED_64	INSERT_UNMAPPED_16 INSERT_UNMAPPED_16 INSERT_UNMAPPED_16 INSERT_UNMAPPED_16

// handlers for 0x60->0x100
static __aligned16 const GIFRegHandler s_gsHandlers[0x100-0x60] =		
{
	RegHandlerSIGNAL, RegHandlerFINISH, RegHandlerLABEL, RegHandlerUNMAPPED,

	// Rest are mapped to Unmapped
	INSERT_UNMAPPED_4  INSERT_UNMAPPED_4  INSERT_UNMAPPED_4
	INSERT_UNMAPPED_64 INSERT_UNMAPPED_64 INSERT_UNMAPPED_16
};

// --------------------------------------------------------------------------------------
//  GIFTAG  (implementations)
// --------------------------------------------------------------------------------------
wxString GIFTAG::ToString() const
{
	static const char* GifTagModeLabel[] =
	{
		"Packed", "RegList", "Image", "Image2"
	};

	FastFormatUnicode result;
	result.Write("NLOOP=0x%04X, PRE=%u, PRIM=0x%03X, MODE=%s",
		NLOOP, PRE, PRIM, GifTagModeLabel[FLG]);

	return result;
}

wxString GIFTAG::DumpRegsToString() const
{
	static const char* PackedModeRegsLabel[] =
	{
		"PRIM",		"RGBA",		"STQ",		"UV",
		"XYZF2",	"XYZ2",		"TEX0_1",	"TEX0_2",
		"CLAMP_1",	"CLAMP_2",	"FOG",		"Unknown",
		"XYZF3",	"XYZ3",		"A_D",		"NOP"
	};

	u32 tempreg		= REGS[0];
	uint numregs	= ((NREG-1)&0xf) + 1;

	FastFormatUnicode result;
	result.Write("NREG=0x%02X (", NREG);

	for (u32 i = 0; i < numregs; i++) {
		if (i == 8) tempreg = REGS[1];
		if (i > 0) result.Write(" ");
		result.Write(PackedModeRegsLabel[tempreg & 0xf]);
		tempreg >>= 4;
	}

	result.Write(")");
	return result;
}


// --------------------------------------------------------------------------------------
//  GIFPath Method Implementations
// --------------------------------------------------------------------------------------

GIFPath::GIFPath() : tag()
{
	Reset();
}

// Warning!  This function must always be accompanied by a reset sent to the GS plugin.  If
// the GS plugin is not reset, undesired results may occur if the GS plugin was midst PATH
// processing (brief video corruption, possibly crashes).
__fi void GIFPath::Reset()
{
	memzero(*this);
	const_cast<GIFTAG&>(tag).EOP = 1;
}

__fi bool GIFPath::StepReg()
{
	if (++curreg >= numregs) {
		curreg = 0;
		if (--nloop == 0) {
			return false;
		}
	}
	return true;
}

__fi u8 GIFPath::GetReg() { return regs[curreg]; }

// Unpack the registers - registers are stored as a sequence of 4 bit values in the
// upper 64 bits of the GIFTAG.  That sucks for us when handling partialized GIF packets
// coming in from paths 2 and 3, so we unpack them into an 8 bit array here.
//
__fi void GIFPath::PrepPackedRegs()
{
	// Only unpack registers if we're starting a new pack.  Otherwise the unpacked
	// array should have already been initialized by a previous partial transfer.

	if (curreg != 0) return;
	DetectE = 0;
	u32 tempreg = tag.REGS[0];
	numregs		= ((tag.NREG-1)&0xf) + 1;

	for (u32 i = 0; i < numregs; i++) {
		if (i == 8) tempreg = tag.REGS[1];
		regs[i] = tempreg & 0xf;
		if(regs[i] == 0xe) DetectE++;
		tempreg >>= 4;
	}
}

__fi void GIFPath::SetTag(const void* mem)
{
	static const bool Aligned = true;
	_mm_store_ps( (float*)&tag, Aligned ? _mm_load_ps((const float*)mem) : _mm_loadu_ps((const float*)mem) );

	nloop	= tag.NLOOP;
	curreg	= 0;
}

void SaveStateBase::gifPathFreeze()
{
	FreezeTag( "GIFpath" );
	Freeze( g_gifpath );
}

static __fi void gsHandler(const u8* pMem)
{
	const int reg = pMem[8];

	#if 0
	
	// Not needed anymore.  The GS plugin API/spec has been updated so that reverse FIFO transfers
	// from GS let the VIF know when its run out of data to drain.
	
	if (reg == 0x50)
	{
		vif1.BITBLTBUF._u64 = *(u64*)pMem;
	}
	else if (reg == 0x52)
	{
		vif1.TRXREG._u64 = *(u64*)pMem;
	}
	else if (reg == 0x53)
	{
		// local -> host
		if ((pMem[0] & 3) == 1)
		{
			// Onimusha does TRXREG without BLTDIVIDE first, so we "assume" 32bit for this equation, probably isnt important.
			// ^ WTF, seriously? This is really important (pseudonym)
			u8 bpp = 32;

			switch(vif1.BITBLTBUF.SPSM & 7)
			{
				case 0: bpp = 32; break;
				case 1:	bpp = 24; break;
				case 2: bpp = 16; break;
				case 3:	bpp = 8; break;

				// 4 is 4 bit but this is forbidden

				default:
					pxAssumeDev( false, pxsFmt("Illegal format for GS upload: SPSM=%u", vif1.BITBLTBUF.SPSM) );
				break;
			}

			VIF_LOG("GS Download %dx%d SPSM=%x bpp=%d", vif1.TRXREG.RRW, vif1.TRXREG.RRH, vif1.BITBLTBUF.SPSM, bpp);

			// qwords, rounded down; any extra bits are lost
			// games must take care to ensure transfer rectangles are exact multiples of a qword
			vif1.GSLastDownloadSize = (vif1.TRXREG.RRW * vif1.TRXREG.RRH * bpp) / 128;
			//DevCon.Warning("GS download in progress");
			gifRegs.stat.OPH = true;
		}
	}
	#endif
	if (reg >= 0x60)
	{
		// Question: What happens if an app writes to uncharted register space on real PS2
		// hardware (handler 0x63 and higher)?  Probably a silent ignorance, but not tested
		// so just guessing... --air

		s_gsHandlers[reg-0x60]((const u32*)pMem);
	}
}

#define aMin(x, y) std::min(x, y)

// Parameters:
//  destSize - Size of the destination buffer.  Copies will copy from destBase -> destBase+destSize and
//    then wrap back around.  If the destSize is 0, wrapping is disabled (the method call is regarded as
//    a regular unwrapped copy).
__ri void MemCopy_WrappedDest( const u128* src, u128* destBase, uint& destStart, uint destSize, uint len )
{
	uint endpos = destStart + len;
	if ((destSize == 0) || (endpos < destSize))
	{
		memcpy_qwc(&destBase[destStart], src, len );
		destStart += len;
	}
	else
	{
		uint firstcopylen = destSize - destStart;
		memcpy_qwc(&destBase[destStart], src, firstcopylen );

		destStart = endpos % destSize;
		memcpy_qwc(destBase, src+firstcopylen, destStart );
	}
}

__ri void MemCopy_WrappedSrc( const u128* srcBase, uint& srcStart, uint srcSize, u128* dest, uint len )
{
	uint endpos = srcStart + len;
	if ((srcSize==0) || (endpos < srcSize))
	{
		memcpy_qwc(dest, &srcBase[srcStart], len );
		srcStart += len;
	}
	else
	{
		uint firstcopylen = srcSize - srcStart;
		memcpy_qwc(dest, &srcBase[srcStart], firstcopylen );

		srcStart = endpos % srcSize;
		memcpy_qwc(dest+firstcopylen, srcBase, srcStart );
	}
}

#define copyTag() do {						\
	_mm_store_ps( (float*)&RingBuffer.m_Ring[ringpos], Aligned ? _mm_load_ps((float*)pMem128) : _mm_loadu_ps((float*)pMem128)); \
	/*_mm_stream_ps( (float*)&RingBuffer.m_Ring[ringpos], Aligned ? _mm_load_ps((float*)pMem128) : _mm_loadu_ps((float*)pMem128));*/ \
	++pMem128; --size;						\
	ringpos = (ringpos+1)&RingBufferMask;	\
} while(false)

__ri void GIFPath::TransferPackedRegs( const u128*& pMem128, uint& size, uint& ringpos )
{
	static const bool Aligned = true;

	PrepPackedRegs();
	GifTagLog("\tPacked Mode, %ls", tag.DumpRegsToString().c_str());
	
	if(DetectE > 0)
	{
		do {
			if (GetReg() == 0xe) {
				gsHandler((u8*)pMem128);
			}
			copyTag();
		} while(StepReg() && size > 0);
	}
	else
	{
		//DevCon.WriteLn(Color_Orange, "No E detected on Path%d: nloop=%x, numregs=%x, curreg=%x, size=%x", gifRegs.stat.APATH, nloop, numregs, curreg, size);

		// Note: curreg is *usually* zero here, but can be non-zero if a previous fragment was
		// handled via this optimized copy code below.

		const u32 listlen = (nloop * numregs) - curreg;	// the total length of this packed register list (in QWC)
		u32 len;

		if(size < listlen)
		{
			len = size;

			// We need to calculate both the number of full iterations of regs copied (nloops),
			// and any remaining registers not copied by this fragment.  A div/mod pair should
			// hopefully be optimized by the compiler into a single x86 div. :)

			const int nloops_copied		= len / numregs;
			const int regs_not_copied	= len % numregs;

			// Make sure to add regs_not_copied to curreg, to handle cases of multiple partial fragments.
			// (example: 3 fragments each of only 2 regs, then curreg should be 0, 2, and then 4 after
			//  each call to GIFPath_Parse; with no change to NLOOP).  Because of this we also need to
			//  check for cases where curreg wraps past an nloop.

			nloop -= nloops_copied;
			curreg += regs_not_copied;
			if(curreg >= numregs)
			{
				--nloop;
				curreg -= numregs;
			}
		}
		else 
		{
			len = listlen;
			curreg = 0;
			nloop = 0;
		}

		MemCopy_WrappedDest( pMem128, RingBuffer.m_Ring, ringpos, RingBufferSize, len );
		pMem128 += len;
		size -= len;
	}
}

// Parameters:
//   baseMem - pointer to the base of the memory buffer.  If the transfer wraps past the specified
//      memSize, it restarts bask here.
//
//   fragment_size - size of incoming data stream, in qwc (simd128).  If this parameter is 0, then
//      the GIF continues to process until the first EOP condition is met.
//
//   startPos - start position within the buffer.
//   memSize - size of the source memory buffer pointed to by baseMem.  If 0, no wrapping logic is performed.
//
// Returns:
//   Amount of data processed.  Actual processed amount may be less than provided size, depending
//   on GS stalls (caused by SIGNAL or EOP, etc).
//
int GIFPath::CopyTag(const u128* baseMem, uint fragment_size, uint startPos, uint memSize)
{
	// Cannot transfer while a second IMR is pending (!)
	if (SIGNAL_IMR_Pending)
	{
		GifTagLog("GIFTAG IGNORED due to SIGNAL/IMR condition.");
		return 0;
	}

	static const bool Aligned = true;

	uint& ringpos = GetMTGS().m_packet_writepos;
	const uint original_ringpos = ringpos;

	const u128* pMem128 = baseMem + startPos;
	uint size;
	uint processed = 0;

	if (memSize)
	{
		uint firstlen = memSize - startPos;
		if (!fragment_size)
			size = firstlen;
		else
			size = std::min(firstlen, fragment_size);

		GifTagLog("GIFTAG BEGIN fragment_size=0x%03X (wrapping @ wrapSize=0x%06X, startPos=0x%04X)",
			fragment_size, memSize, startPos
		);
	}
	else
	{
		pxAssume(fragment_size);
		size = fragment_size;

		GifTagLog("GIFTAG BEGIN fragment_size=0x%03X", fragment_size);
	}

	uint startSize = size;

	try {
		while (size > 0) {
			if (!nloop) {

				SetTag((u8*)pMem128);
				copyTag();
				
				GifTagLog("\tSetTag  %ls", tag.ToString().c_str());
			}
			else
			{
				switch(tag.FLG) {
					case GIF_FLG_PACKED:
						TransferPackedRegs(pMem128, size, ringpos);
					break;

					case GIF_FLG_REGLIST:
					{
						GifTagLog("\tReglist Mode, NREG=0x%02X", tag.NREG);

						// In reglist mode, the GIF packs 2 registers into each QWC.  The nloop however
						// can be an odd number, in which case the upper half of the final QWC is ignored (skipped).

						numregs	= ((tag.NREG-1)&0xf) + 1;
						const u32 total_reglen = (nloop * numregs) - curreg;	// total 'expected length' of this packed register list (in registers)
						const u32 total_listlen = (total_reglen+1) / 2;			// total 'expected length' of the register list, in QWC!  (+1 so to round it up)

						u32 len;

						if(size < total_listlen)
						{
							len = size;
							const u32 reglen = len * 2;

							const int nloops_copied		= reglen / numregs;
							const int regs_not_copied	= reglen % numregs;

							curreg += regs_not_copied;
							nloop -= nloops_copied;

							if(curreg >= numregs)
							{
								--nloop;
								curreg -= numregs;
							}
						}
						else 
						{
							len = total_listlen;
							curreg = 0;
							nloop = 0;
						}

						MemCopy_WrappedDest( pMem128, RingBuffer.m_Ring, ringpos, RingBufferSize, len );
						pMem128 += len;
						size -= len;
					}
					break;

					case GIF_FLG_IMAGE:
					case GIF_FLG_IMAGE2:
					{
						int len = aMin(size, nloop);
						GifTagLog("\tImage mode, len=0x%04X", len);

						MemCopy_WrappedDest( pMem128, RingBuffer.m_Ring, ringpos, RingBufferSize, len );

						pMem128 += len;
						size -= len;
						nloop -= len;
					}
					break;
				}
			}

			if ((!UseDmaBurstHack || !fragment_size) && tag.EOP && !nloop)
			{
				// If the DMA burst hack is not in use then we need to break transfer after each burst
				// of GIFtag data.  This is because another path's transfer could be pending with a higher
				// priority than the current path.  For example, PATH1 has the ability to overtake (stall)
				// a PATH2/DIRECT transfer at any EOP/NLOOP=0 boundary.
				
				// If fragment_size is zero it means to transfer until the first EOP and finish (XGKICK/PATH1)

				// Clear the GIF path transfer status.  The caller will check queues and reassign new
				// transfers as needed.

				gifRegs.stat.OPH = 0;
				gifRegs.stat.APATH = GIF_APATH_IDLE;

				break;
			}

			if (memSize && (size == 0))
			{
				processed += startSize;

				if (!fragment_size)
				{
					// waiting for EOP -- fragment_size is ignored.  We still need to check for and detect
					// cases where the transfer has wrapped all the way around the buffer (infinite loop)
					// and force-break the transfer and resume it after some amount of time has passed.
					// (the BIOS does this, by using an XGKICK on VU memory and then writes an EOP to VU
					// memory later on).

					if (startSize < memSize)
					{
						GifTagLog("\tVU1 Buffer Wrap!");
						size		= memSize - startSize;
						memSize		= size;
						startSize	= size;
						pMem128		= baseMem;
					}
					else
					{
						DevCon.Warning("GIF PATH1 infinite transfer detected!");
						//GIF_DelayArbitration(0x200);
						//CPU_ScheduleEvent(GIF_EVENT);

						nloop	= 0;
						const_cast<GIFTAG&>(tag).EOP = 1;

						// Don't send the packet to the GS -- its incomplete and might cause the GS plugin
						// to get confused and die. >_<

						ringpos = original_ringpos;
						gifRegs.SetActivePath( GIF_APATH_IDLE );

						break;
					}
				}
				else
				{
					size		= fragment_size - processed;
					startSize	= size;
					pMem128		= baseMem;

					if (size) GifTagLog("\tMFIFO/SPRAM buffer Wrap!");
				}
			}
		}
	}
	catch( Exception::SignalDoubleIMR& )
	{
		// IMR occurred twice; GIF transfer stalls indefinitely until the EE writes
		// the proper sequence of CSR/IMR regs to restart it.

		GifTagLog("\tSIGNAL/IMR detected, parsing aborted");
		StepReg();		// advances past the SIGNAL that caused the exception.
	}

	GifTagLog("GIFTAG END, processed=0x%04X", processed + (startSize - size));
	return processed + (startSize - size);
}
