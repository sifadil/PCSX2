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

// Note: This header is meant to be included from within the x86Emitter::Internal namespace.
// Instructions implemented in this header are as follows -->>

enum G1Type
{
	G1Type_ADD=0,
	G1Type_OR,
	G1Type_ADC,
	G1Type_SBB,
	G1Type_AND,
	G1Type_SUB,
	G1Type_XOR,
	G1Type_CMP
};

// -------------------------------------------------------------------
template< G1Type InstType, typename ImmType >
class Group1Impl : public ImplementationHelper< ImmType >
{
public: 
	Group1Impl() {}		// because GCC doesn't like static classes

	static __emitinline void Emit( const iRegister<ImmType>& to, const iRegister<ImmType>& from ) 
	{
		ImplementationHelper<ImmType>::prefix16();
		iWrite<u8>( (ImplementationHelper<ImmType>::Is8BitOperand() ? 0 : 1) | (InstType<<3) ); 
		ModRM_Direct( from.Id, to.Id );
	}

	static __emitinline void Emit( const ModSibBase& sibdest, const iRegister<ImmType>& from ) 
	{
		ImplementationHelper<ImmType>::prefix16();
		iWrite<u8>( (ImplementationHelper<ImmType>::Is8BitOperand() ? 0 : 1) | (InstType<<3) ); 
		EmitSibMagic( from.Id, sibdest );
	}

	static __emitinline void Emit( const iRegister<ImmType>& to, const ModSibBase& sibsrc ) 
	{
		ImplementationHelper<ImmType>::prefix16();
		iWrite<u8>( (ImplementationHelper<ImmType>::Is8BitOperand() ? 2 : 3) | (InstType<<3) );
		EmitSibMagic( to.Id, sibsrc );
	}

	static __emitinline void Emit( void* dest, const iRegister<ImmType>& from ) 
	{
		ImplementationHelper<ImmType>::prefix16();
		iWrite<u8>( (ImplementationHelper<ImmType>::Is8BitOperand() ? 0 : 1) | (InstType<<3) ); 
		iWriteDisp( from.Id, dest );
	}

	static __emitinline void Emit( const iRegister<ImmType>& to, const void* src ) 
	{
		ImplementationHelper<ImmType>::prefix16();
		iWrite<u8>( (ImplementationHelper<ImmType>::Is8BitOperand() ? 2 : 3) | (InstType<<3) );
		iWriteDisp( to.Id, src );
	}

	static __emitinline void Emit( const iRegister<ImmType>& to, int imm ) 
	{
		ImplementationHelper<ImmType>::prefix16();
		if( !ImplementationHelper<ImmType>::Is8BitOperand() && is_s8( imm ) )
		{
			iWrite<u8>( 0x83 );
			ModRM_Direct( InstType, to.Id );
			iWrite<s8>( imm );
		}
		else
		{
			if( to.IsAccumulator() )
				iWrite<u8>( (ImplementationHelper<ImmType>::Is8BitOperand() ? 4 : 5) | (InstType<<3) );
			else
			{
				iWrite<u8>( ImplementationHelper<ImmType>::Is8BitOperand() ? 0x80 : 0x81 );
				ModRM_Direct( InstType, to.Id );
			}
			iWrite<ImmType>( imm );
		}
	}

	static __emitinline void Emit( const ModSibStrict<ImmType>& sibdest, int imm ) 
	{
		if( ImplementationHelper<ImmType>::Is8BitOperand() )
		{
			iWrite<u8>( 0x80 );
			EmitSibMagic( InstType, sibdest );
			iWrite<ImmType>( imm );
		}
		else
		{		
			ImplementationHelper<ImmType>::prefix16();
			iWrite<u8>( is_s8( imm ) ? 0x83 : 0x81 );
			EmitSibMagic( InstType, sibdest );
			if( is_s8( imm ) )
				iWrite<s8>( imm );
			else
				iWrite<ImmType>( imm );
		}
	}
};


// -------------------------------------------------------------------
//
template< G1Type InstType >
class Group1ImplAll
{
public:
	template< typename T >
	__forceinline void operator()( const iRegister<T>& to,	const iRegister<T>& from ) const	{ Group1Impl<InstType,T>::Emit( to, from ); }
	template< typename T >
	__forceinline void operator()( const iRegister<T>& to,	const void* src ) const				{ Group1Impl<InstType,T>::Emit( to, src ); }
	template< typename T >
	__forceinline void operator()( void* dest,				const iRegister<T>& from ) const	{ Group1Impl<InstType,T>::Emit( dest, from ); }
	template< typename T >
	__noinline void operator()( const ModSibBase& sibdest,	const iRegister<T>& from ) const	{ Group1Impl<InstType,T>::Emit( sibdest, from ); }
	template< typename T >
	__noinline void operator()( const iRegister<T>& to,		const ModSibBase& sibsrc ) const	{ Group1Impl<InstType,T>::Emit( to, sibsrc ); }

	// Note on Imm forms : use int as the source operand since it's "reasonably inert" from a compiler
	// perspective.  (using uint tends to make the compiler try and fail to match signed immediates with
	// one of the other overloads).
	
	template< typename T >
	__noinline void operator()( const ModSibStrict<T>& sibdest, int imm ) const	{ Group1Impl<InstType,T>::Emit( sibdest, imm ); }
	template< typename T >
	void operator()( const iRegister<T>& to, int imm ) const					{ Group1Impl<InstType,T>::Emit( to, imm ); }


	Group1ImplAll() {}		// Why does GCC need these?
};

