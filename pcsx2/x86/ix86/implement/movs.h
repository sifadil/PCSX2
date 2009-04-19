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

#pragma once

// Header: ix86_impl_movs.h -- covers mov, cmov, movsx/movzx, and SETcc (which shares
// with cmov many similarities).

// Note: This header is meant to be included from within the x86Emitter::Internal namespace.

//////////////////////////////////////////////////////////////////////////////////////////
// MOV instruction Implementation

template< typename ImmType >
class MovImpl
{
protected:
	static const uint OperandSize = sizeof(ImmType);
	static bool Is8BitOperand()	{ return OperandSize == 1; }
	static void prefix16()		{ if( OperandSize == 2 ) iWrite<u8>( 0x66 ); }

public:
	MovImpl() {}
	
	// ------------------------------------------------------------------------
	static __emitinline void Emit( const iRegister<ImmType>& to, const iRegister<ImmType>& from )
	{
		if( to == from ) return;	// ignore redundant MOVs.

		prefix16();
		iWrite<u8>( Is8BitOperand() ? 0x88 : 0x89 );
		ModRM_Direct( from.Id, to.Id );
	}

	// ------------------------------------------------------------------------
	static __emitinline void Emit( const ModSibBase& dest, const iRegister<ImmType>& from )
	{
		prefix16();

		// mov eax has a special from when writing directly to a DISP32 address
		// (sans any register index/base registers).

		if( from.IsAccumulator() && dest.Index.IsEmpty() && dest.Base.IsEmpty() )
		{
			iWrite<u8>( Is8BitOperand() ? 0xa2 : 0xa3 );
			iWrite<u32>( dest.Displacement );
		}
		else
		{
			iWrite<u8>( Is8BitOperand() ? 0x88 : 0x89 );
			EmitSibMagic( from.Id, dest );
		}
	}

	// ------------------------------------------------------------------------
	static __emitinline void Emit( const iRegister<ImmType>& to, const ModSibBase& src )
	{
		prefix16();

		// mov eax has a special from when reading directly from a DISP32 address
		// (sans any register index/base registers).

		if( to.IsAccumulator() && src.Index.IsEmpty() && src.Base.IsEmpty() )
		{
			iWrite<u8>( Is8BitOperand() ? 0xa0 : 0xa1 );
			iWrite<u32>( src.Displacement );
		}
		else
		{
			iWrite<u8>( Is8BitOperand() ? 0x8a : 0x8b );
			EmitSibMagic( to.Id, src );
		}
	}

	// ------------------------------------------------------------------------
	static __emitinline void Emit( void* dest, const iRegister<ImmType>& from )
	{
		prefix16();

		// mov eax has a special from when writing directly to a DISP32 address

		if( from.IsAccumulator() )
		{
			iWrite<u8>( Is8BitOperand() ? 0xa2 : 0xa3 );
			iWrite<s32>( (s32)dest );
		}
		else
		{
			iWrite<u8>( Is8BitOperand() ? 0x88 : 0x89 );
			iWriteDisp( from.Id, dest );
		}
	}

	// ------------------------------------------------------------------------
	static __emitinline void Emit( const iRegister<ImmType>& to, const void* src )
	{
		prefix16();

		// mov eax has a special from when reading directly from a DISP32 address

		if( to.IsAccumulator() )
		{
			iWrite<u8>( Is8BitOperand() ? 0xa0 : 0xa1 );
			iWrite<s32>( (s32)src );
		}
		else
		{
			iWrite<u8>( Is8BitOperand() ? 0x8a : 0x8b );
			iWriteDisp( to.Id, src );
		}
	}

	// ------------------------------------------------------------------------
	static __emitinline void Emit( const iRegister<ImmType>& to, ImmType imm )
	{
		// Note: MOV does not have (reg16/32,imm8) forms.

		prefix16();
		iWrite<u8>( (Is8BitOperand() ? 0xb0 : 0xb8) | to.Id ); 
		iWrite<ImmType>( imm );
	}

	// ------------------------------------------------------------------------
	static __emitinline void Emit( ModSibStrict<ImmType> dest, ImmType imm )
	{
		prefix16();
		iWrite<u8>( Is8BitOperand() ? 0xc6 : 0xc7 );
		EmitSibMagic( 0, dest );
		iWrite<ImmType>( imm );
	}
};

// ------------------------------------------------------------------------
class MovImplAll
{
public:
	template< typename T >
	__forceinline void operator()( const iRegister<T>& to,	const iRegister<T>& from ) const	{ MovImpl<T>::Emit( to, from ); }
	template< typename T >
	__forceinline void operator()( const iRegister<T>& to,	const void* src ) const				{ MovImpl<T>::Emit( to, src ); }
	template< typename T >
	__forceinline void operator()( void* dest,				const iRegister<T>& from ) const	{ MovImpl<T>::Emit( dest, from ); }
	template< typename T >
	__noinline void operator()( const ModSibBase& sibdest,	const iRegister<T>& from ) const	{ MovImpl<T>::Emit( sibdest, from ); }
	template< typename T >
	__noinline void operator()( const iRegister<T>& to,		const ModSibBase& sibsrc ) const	{ MovImpl<T>::Emit( to, sibsrc ); }

	template< typename T >
	__noinline void operator()( const ModSibStrict<T>& sibdest, int imm ) const					{ MovImpl<T>::Emit( sibdest, imm ); }

	// preserve_flags  - set to true to disable optimizations which could alter the state of
	//   the flags (namely replacing mov reg,0 with xor).

	template< typename T >
	__emitinline void operator()( const iRegister<T>& to, int imm, bool preserve_flags=false ) const
	{
		if( !preserve_flags && (imm == 0) )
			iXOR( to, to );
		else
			MovImpl<T>::Emit( to, imm );
	}
	
	MovImplAll() {} // Satisfy GCC's whims.
};


//////////////////////////////////////////////////////////////////////////////////////////
// CMOV !!  [in all of it's disappointing lack-of glory]  .. and ..
// SETcc!!  [more glory, less lack!]
//
// CMOV Disclaimer: Caution!  This instruction can look exciting and cool, until you
// realize that it cannot load immediate values into registers. -_-
//
template< typename ImmType, int InstBaseVal >
class CMovSetImpl
{
protected:
	static const uint OperandSize = sizeof(ImmType);

	static bool Is8BitOperand()	{ return OperandSize == 1; }
	static void prefix16()		{ if( OperandSize == 2 ) iWrite<u8>( 0x66 ); }
	
	static __forceinline void emit_base( JccComparisonType cc )
	{
		jASSUME( cc >= 0 && cc <= 0x0f );
		prefix16();
		write8( 0x0f );
		write8( InstBaseVal | cc );	
	}

public:
	CMovSetImpl() {}

	static __emitinline void Emit( JccComparisonType cc, const iRegister<ImmType>& to, const iRegister<ImmType>& from )
	{
		if( to == from ) return;
		emit_base( cc );
		ModRM_Direct( to.Id, from.Id );
	}

	static __emitinline void Emit( JccComparisonType cc, const iRegister<ImmType>& to, const void* src )
	{
		emit_base( cc );
		iWriteDisp( to.Id, src );
	}

	static __emitinline void Emit( JccComparisonType cc, const iRegister<ImmType>& to, const ModSibBase& sibsrc )
	{
		emit_base( cc );
		EmitSibMagic( to.Id, sibsrc );
	}

	// This form is provided for SETcc only (not available in CMOV)
	static __emitinline void EmitSet( JccComparisonType cc, const iRegister<ImmType>& to )
	{
		emit_base( cc );
		ModRM_Direct( 0, to.Id );
	}

	// This form is provided for SETcc only (not available in CMOV)
	static __emitinline void EmitSet( JccComparisonType cc, const void* src )
	{
		emit_base( cc );
		iWriteDisp( 0, src );
	}

	// This form is provided for SETcc only (not available in CMOV)
	static __emitinline void EmitSet( JccComparisonType cc, const ModSibStrict<ImmType>& sibsrc )
	{
		emit_base( cc );
		EmitSibMagic( 0, sibsrc );
	}
};

// ------------------------------------------------------------------------
// I use explicit method declarations here instead of templates, in order to provide
// *only* 32 and 16 bit register operand forms (8 bit registers are not valid in CMOV).
//
class CMovImplGeneric
{
protected:
	typedef CMovSetImpl<u32, 0x40> m_32;	// 0x40 is the cmov base instruction id
	typedef CMovSetImpl<u16, 0x40> m_16;	// 0x40 is the cmov base instruction id

public:
	__forceinline void operator()( JccComparisonType ccType, const iRegister32& to, const iRegister32& from ) const		{ m_32::Emit( ccType, to, from ); }
	__forceinline void operator()( JccComparisonType ccType, const iRegister32& to, const void* src ) const				{ m_32::Emit( ccType, to, src ); }
	__noinline void operator()( JccComparisonType ccType, const iRegister32& to, const ModSibBase& sibsrc ) const		{ m_32::Emit( ccType, to, sibsrc ); }

	__forceinline void operator()( JccComparisonType ccType, const iRegister16& to, const iRegister16& from ) const		{ m_16::Emit( ccType, to, from ); }
	__forceinline void operator()( JccComparisonType ccType, const iRegister16& to, const void* src ) const				{ m_16::Emit( ccType, to, src ); }
	__noinline void operator()( JccComparisonType ccType, const iRegister16& to, const ModSibBase& sibsrc ) const		{ m_16::Emit( ccType, to, sibsrc ); }

	CMovImplGeneric() {}		// don't ask.
};

// ------------------------------------------------------------------------
template< JccComparisonType ccType >
class CMovImplAll
{
protected:
	typedef CMovSetImpl<u32, 0x40> m_32;
	typedef CMovSetImpl<u16, 0x40> m_16;

public:
	__forceinline void operator()( const iRegister32& to, const iRegister32& from ) const	{ m_32::Emit( ccType, to, from ); }
	__forceinline void operator()( const iRegister32& to, const void* src ) const			{ m_32::Emit( ccType, to, src ); }
	__noinline void operator()( const iRegister32& to, const ModSibBase& sibsrc ) const		{ m_32::Emit( ccType, to, sibsrc ); }

	__forceinline void operator()( const iRegister16& to, const iRegister16& from ) const	{ m_16::Emit( ccType, to, from ); }
	__forceinline void operator()( const iRegister16& to, const void* src ) const			{ m_16::Emit( ccType, to, src ); }
	__noinline void operator()( const iRegister16& to, const ModSibBase& sibsrc ) const		{ m_16::Emit( ccType, to, sibsrc ); }

	CMovImplAll() {}		// don't ask.
};

// ------------------------------------------------------------------------
class SetImplGeneric
{
protected:
	typedef CMovSetImpl<u8, 0x90> Impl;	// 0x90 is the SETcc base instruction id

public:
	__forceinline void operator()( JccComparisonType cc, const iRegister8& to ) const		{ Impl::EmitSet( cc, to ); }
	__forceinline void operator()( JccComparisonType cc, void* dest ) const					{ Impl::EmitSet( cc, dest ); }
	__noinline void operator()( JccComparisonType cc, const ModSibStrict<u8>& dest ) const	{ Impl::EmitSet( cc, dest ); }

	SetImplGeneric() {}		// if you do, ask GCC.
};

// ------------------------------------------------------------------------
template< JccComparisonType ccType >
class SetImplAll
{
protected:
	typedef CMovSetImpl<u8, 0x90> Impl;	// 0x90 is the SETcc base instruction id

public:
	__forceinline void operator()( const iRegister8& to ) const			{ Impl::EmitSet( ccType, to ); }
	__forceinline void operator()( void* dest ) const					{ Impl::EmitSet( ccType, dest ); }
	__noinline void operator()( const ModSibStrict<u8>& dest ) const	{ Impl::EmitSet( ccType, dest ); }

	SetImplAll() {}		// if you do, ask GCC.
};


//////////////////////////////////////////////////////////////////////////////////////////
// Mov with sign/zero extension implementations (movsx / movzx)
//
template< typename DestImmType, typename SrcImmType >
class MovExtendImpl
{
protected:
	static const uint DestOperandSize = sizeof( DestImmType );
	static const uint SrcOperandSize = sizeof( SrcImmType );

	static bool Is8BitOperand()	{ return SrcOperandSize == 1; }
	static void prefix16()		{ if( DestOperandSize == 2 ) iWrite<u8>( 0x66 ); }
	static __forceinline void emit_base( bool SignExtend )
	{
		prefix16();
		iWrite<u8>( 0x0f );
		iWrite<u8>( 0xb6 | (Is8BitOperand() ? 0 : 1) | (SignExtend ? 8 : 0 ) );
	}

public: 
	MovExtendImpl() {}		// For the love of GCC.

	static __emitinline void Emit( const iRegister<DestImmType>& to, const iRegister<SrcImmType>& from, bool SignExtend )
	{
		emit_base( SignExtend );
		ModRM_Direct( to.Id, from.Id );
	}

	static __emitinline void Emit( const iRegister<DestImmType>& to, const ModSibStrict<SrcImmType>& sibsrc, bool SignExtend )
	{
		emit_base( SignExtend );
		EmitSibMagic( to.Id, sibsrc );
	}
};

// ------------------------------------------------------------------------
template< bool SignExtend >
class MovExtendImplAll
{
protected:
	typedef MovExtendImpl<u32, u16> m_16to32;
	typedef MovExtendImpl<u32, u8> m_8to32;
	typedef MovExtendImpl<u16, u8> m_8to16;

public:
	__forceinline void operator()( const iRegister32& to, const iRegister16& from )	const		{ m_16to32::Emit( to, from, SignExtend ); }
	__noinline void operator()( const iRegister32& to, const ModSibStrict<u16>& sibsrc ) const	{ m_16to32::Emit( to, sibsrc, SignExtend ); }

	__forceinline void operator()( const iRegister32& to, const iRegister8& from ) const		{ m_8to32::Emit( to, from, SignExtend ); }
	__noinline void operator()( const iRegister32& to, const ModSibStrict<u8>& sibsrc ) const	{ m_8to32::Emit( to, sibsrc, SignExtend ); }

	__forceinline void operator()( const iRegister16& to, const iRegister8& from ) const		{ m_8to16::Emit( to, from, SignExtend ); }
	__noinline void operator()( const iRegister16& to, const ModSibStrict<u8>& sibsrc ) const	{ m_8to16::Emit( to, sibsrc, SignExtend ); }

	MovExtendImplAll() {}		// don't ask.
};

