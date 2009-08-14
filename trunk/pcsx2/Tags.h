/*  Pcsx2 - Pc Ps2 Emulator
 *  Copyright (C) 2002-2009  Pcsx2 Team
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

// This is meant to be a collection of generic functions dealing with tags.
// I kept seeing the same code over and over with different structure names
// and the same members, and figured it'd be a good spot to use templates...

enum TransferMode
{
	NORMAL_MODE = 0,
	CHAIN_MODE,
	INTERLEAVE_MODE,
	UNDEFINED_MODE
};

// Transfer a tag. 
template <class T>
static __forceinline bool TransferTag(const char *s, T tag, u32* &ptag)
{
	if (ptag == NULL)  					 // Is ptag empty?
	{
		Console::Error("%s BUSERR", params s);
		tag->chcr = (tag->chcr & 0xFFFF) | ((*ptag) & 0xFFFF0000);	// Transfer upper part of tag to CHCR bits 31-15
		psHu32(DMAC_STAT) |= DMAC_STAT_BEIS;						// Set BEIS (BUSERR) in DMAC_STAT register
		return false;
	}
	else
	{
		tag->chcr = (tag->chcr & 0xFFFF) | ((*ptag) & 0xFFFF0000);	//Transfer upper part of tag to CHCR bits 31-15
		tag->qwc  = (u16)ptag[0];									//QWC set to lower 16bits of the tag
		return true;
	}
}

enum pce_values
{
	PCE_NOTHING = 0, 
	PCE_RESERVED,
	PCE_DISABLED,
	PCE_ENABLED
};

enum tag_id
{
	TAG_REFE = 0,
	TAG_CNT,
	TAG_NEXT,
	TAG_REF,
	TAG_REFS,
	TAG_CALL,
	TAG_RET,
	TAG_END
};

namespace ChainTags
{
	// Untested
	static __forceinline u16 QWC(u32 *tag)
	{
		return (tag[0] & 0xffff);
	}
	
	// Untested
	static __forceinline pce_values PCE(u32 *tag)
	{
		u8 temp = 0;
		if (tag[0] & (1 << 22)) temp |= (1 << 0);
		if (tag[0] & (1 << 23)) temp |= (1 << 1);
		return (pce_values)temp;
	}
	
	static __forceinline tag_id Id(u32* &tag)
	{
		u8 temp = 0;
		if (tag[0] & (1 << 28)) temp |= (1 << 0);
		if (tag[0] & (1 << 29)) temp |= (1 << 1);
		if (tag[0] & (1 << 30)) temp |= (1 << 2);
		return (tag_id)temp;
	}
	
	static __forceinline bool IRQ(u32 *tag)
	{
		return (tag[0] & 0x8000000);
	}
}

enum chcr_flags
{
	CHCR_DIR = 0x0,
	CHCR_MOD1 = 0x4,
	CHCR_MOD2 = 0x8,
	CHCR_ASP1 = 0x10,
	CHCR_ASP2 = 0x20,
	CHCR_TTE = 0x40,
	CHCR_TIE = 0x80,
	CHCR_STR = 0x100
};

namespace CHCR
{
	// Query the flags in the channel control register.
	template <class T>
	static __forceinline bool STR(T tag) { return (tag->chcr & CHCR_STR); }
	
	template <class T>
	static __forceinline bool TIE(T tag) { return (tag->chcr & CHCR_TIE); }
	
	template <class T>
	static __forceinline bool TTE(T tag) { return (tag->chcr & CHCR_TTE); }
	
	template <class T>
	static __forceinline u8 DIR(T tag) { return (tag->chcr & CHCR_DIR); }
	
	template <class T>
	static __forceinline TransferMode MOD(T tag)
	{
		u8 temp = 0;
		if (tag->chcr & CHCR_MOD1) temp |= (1 << 0);
		if (tag->chcr & CHCR_MOD2) temp |= (1 << 1);
		return (TransferMode)temp;
	}
	
	template <class T>
	static __forceinline u8 ASP(T tag)
	{
		u8 temp = 0;
		if (tag->chcr & CHCR_ASP1) temp |= (1 << 0);
		if (tag->chcr & CHCR_ASP2) temp |= (1 << 1);
		return temp;
	}

	// Set the individual flags. Untested.
	template <class T>
	static __forceinline void setSTR(T tag) { tag->chcr &= ~CHCR_STR; }
	
	template <class T>
	static __forceinline void setTIE(T tag) { tag->chcr &= ~CHCR_TIE; }
	
	template <class T>
	static __forceinline void setTTE(T tag) { tag->chcr &= ~CHCR_TTE; }
	
	template <class T>
	static __forceinline void setDIR(T tag) { tag->chcr &= ~CHCR_DIR; }
	
	template <class T>
	static __forceinline void setMOD(T tag, TransferMode mode)
	{
		if (mode & (1 << 0))
			tag->chcr |= CHCR_MOD1; 
		else
			tag->chcr &= CHCR_MOD1; 
			
		if (mode & (1 << 1)) 
			tag->chcr |= CHCR_MOD2;
		else
			tag->chcr &= CHCR_MOD2;
	}
	
	template <class T>
	static __forceinline void ASP(T tag, u8 num)
	{
		if (num & (1 << 0))
			tag->chcr |= CHCR_ASP1; 
		else
			tag->chcr &= CHCR_ASP2; 
			
		if (num & (1 << 1)) 
			tag->chcr |= CHCR_ASP1;
		else
			tag->chcr &= CHCR_ASP2;
	}
	
	// Clear them. Untested.
	template <class T>
	static __forceinline void clearSTR(T tag) { tag->chcr |= CHCR_STR; }
	
	template <class T>
	static __forceinline void clearTIE(T tag) { tag->chcr |= CHCR_TIE; }
	
	template <class T>
	static __forceinline void clearTTE(T tag) { tag->chcr |= CHCR_TTE; }
	
	template <class T>
	static __forceinline void clearDIR(T tag) { tag->chcr |= CHCR_DIR; }
	
	// Print information about a chcr tag.
	template <class T>
	static __forceinline void Print(const char*  s, T tag)
	{
		u8 num_addr = ASP(tag);
		TransferMode mode = MOD(tag);
		
		Console::Write("%s chcr %s mem: ", params s, (DIR(tag)) ? "from" : "to");
		
		if (mode == NORMAL_MODE)
			Console::Write(" normal mode; ");
		else if (mode == CHAIN_MODE)
			Console::Write(" chain mode; ");
		else if (mode == INTERLEAVE_MODE)
			Console::Write(" interleave mode; ");
		else
			Console::Write(" ?? mode; ");
		
		if (num_addr != 0) Console::Write("ASP = %d;", params num_addr);
		if (TTE(tag)) Console::Write("TTE;");
		if (TIE(tag)) Console::Write("TIE;");
		if (STR(tag)) Console::Write(" (DMA started)."); else Console::Write(" (DMA stopped).");
		Console::WriteLn("");
	}
}