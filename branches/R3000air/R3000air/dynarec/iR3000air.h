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

#include "ix86/ix86.h"
#include <map>

#include "iR3000airRegMapping.h"

using namespace x86Emitter;

#define ptrT xAddressIndexer<T>()

#define IMPL_GetInterface() \
	static void GetInterface( InstructionEmitterAPI& api ) \
	{ \
		api.RegMapInfo = RegMapInfo; \
		api.Emit = Emit; \
	}
	
// Implements the GetInterface function with standard templating!  (used mostly by MemoryOps)
#define IMPL_GetInterfaceTee( mess ) \
	template< mess T > \
	static void GetInterface( InstructionEmitterAPI& api ) \
	{ \
		api.RegMapInfo = RegMapInfo; \
		api.Emit = Emit<T>; \
	}

// Some helpers for placebos (unimplemented ops)	
#define IMPL_RecInstAPI( name ) \
	void InstAPI::name() \
	{ \
		API.ConstNone	= rec##name##_ConstNone::GetInterface; \
		API.ConstRt		= rec##name##_ConstNone::GetInterface; \
		API.ConstRs		= rec##name##_ConstNone::GetInterface; \
	}

// Some helpers for placebos (unimplemented ops)	
#define IMPL_RecPlacebo( name ) \
	namespace rec##name##_ConstNone \
	{ \
		static void RegMapInfo( IntermediateRepresentation& info ) {	} \
		static void Emit( const IntermediateRepresentation& info ) { } \
		IMPL_GetInterface() \
	} \
	IMPL_RecInstAPI( name );


//////////////////////////////////////////////////////////////////////////////////////////
// xAnyOperand - Can be direct, indirect, or immediate/const!
//
//
template< typename OperandType >
struct xAnyOperand
{
	u32 Imm;
	bool IsConst;
	xRegister<OperandType> RegDirect;
	ModSibStrict<OperandType> MemIndirect;

	xAnyOperand() :
		Imm( 0 ),
		IsConst( false ),
		RegDirect(),
		MemIndirect( 0 )
	{
	}

	xAnyOperand( const xRegister<OperandType>& direct ) :
		Imm( 0 ),
		IsConst( false ),
		RegDirect( direct ),
		MemIndirect( 0 )
	{
	}

	xAnyOperand( const ModSibStrict<OperandType>& indirect ) :
		Imm( 0 ),
		IsConst( false ),
		RegDirect(),
		MemIndirect( indirect )
	{
	}
};

namespace R3000A {

using namespace x86Emitter;
class IntermediateRepresentation;

extern void __fastcall DivStallUpdater( uint cycleAcc, int newstall );

//////////////////////////////////////////////////////////////////////////////////////////
// iopRecState - contains data representing the current known state of the emu during the
// process of block recompilation.  Information in this struct
//
struct iopRecState
{
	int BlockCycleAccum;
	int DivCycleAccum;
	u32 pc;				// pc for the currently recompiling/emitting instruction
	u8* x86blockptr;	// address of the current block being generated (pointer to the recBlock array)

	int GetScaledBlockCycles() const
	{
		return BlockCycleAccum * 1;
	}

	int GetScaledDivCycles() const
	{
		return DivCycleAccum * 1;
	}
	
	__releaseinline void DivCycleInc()
	{
		if( DivCycleAccum < 0x7f )		// cap it at 0x7f (anything over 35 is ignored anyway)
			DivCycleAccum++;
	}
	
	__releaseinline void IncCycleAccum()
	{
		BlockCycleAccum++;
		DivCycleInc();
	}
};


// ------------------------------------------------------------------------
//
struct InstructionEmitterAPI
{
	// Optimizations - Optional implementation -
	// Allows a recompiled function to specify optimization hints to the recompiler, and also
	// to force certain behaviors if they are needed for correct instruction generations (such
	// as forcing Rs or Rt to a register, for example).
	//
	// Remarks:
	//   If your instruction uses a GPR (like Rs), but the recompiled version clobbers all
	//   registers early on, then leave the GPR unmapped and load it when needed using
	//   info.MoveRsTo( reg ).  The recompiler will flush the reg to memory and then re-
	//   load it when requested.
	//
	void (*RegMapInfo)( IntermediateRepresentation& info );
	
	// Emits code for an instruction.
	// The instruction should assume that the register mappings specified in MapRegisters.inmaps
	// are assigned on function entry.  Results should be written back to GPRs using functions
	// info.MoveToRt(), info.MoveToRd(), or info.MoveToHiLo() -- and need not match the opti-
	// mization hints given in MapRegister.outmaps.
	void (*Emit)( const IntermediateRepresentation& info );
};


// ------------------------------------------------------------------------
//
struct InstructionRecAPI
{
	// fully non-const emitter.
	void (*ConstNone)( InstructionEmitterAPI& api );

	// const status on Rt, Rs is non-const
	void (*ConstRt)( InstructionEmitterAPI& api );

	// const status on Rs, Rt is non-const
	void (*ConstRs)( InstructionEmitterAPI& api );

	// fully const (provided for Memory Write ops only, all others should optimize away)
	void (*ConstRsRt)( InstructionEmitterAPI& api );
	
	static void Error_ConstNone( InstructionEmitterAPI& api );
	static void Error_ConstRsRt( InstructionEmitterAPI& api );
	static void Error_ConstRs( InstructionEmitterAPI& api );
	static void Error_ConstRt( InstructionEmitterAPI& api );

	static void _const_error();
	
	// Clears the contents of the structure to the default values, which in this case
	// map to handlers that throw exceptions (LogicError)
	void Reset()
	{
		ConstNone	= Error_ConstNone;
		ConstRt		= Error_ConstRt;
		ConstRs		= Error_ConstRs;
		ConstRsRt	= Error_ConstRsRt;
	}
};

// ------------------------------------------------------------------------
//
class InstructionRecMess : public InstructionConstOpt
{
public:
	InstructionRecAPI API;

public:
	InstructionRecMess() : InstructionConstOpt( Opcode( 0 ) ) {}

	InstructionRecMess( const InstructionConstOpt& src ) :
		InstructionConstOpt( src )
	{
		// Default constructors should do everything we need.
	}
	
	void GetRecInfo();

public:
	INSTRUCTION_API()
};

// ------------------------------------------------------------------------
// Implementation note: I've separated the Gpr const info into two arrays to have an easier
// time of bit-packing the IsConst array.
//
struct InstConstInfoEx
{
	InstructionConstOpt	inst;
	u32					ConstVal[34];
	u8					m_IsConstBits[5];		// 5 bytes to contain 34 bits.

	bool IsConst( int gpridx ) const
	{
		return !!( m_IsConstBits[gpridx/8] & (1<<(gpridx&7)) );
	}
};

// ------------------------------------------------------------------------
// Temporary register from slot zero (32-bit variety)
#define tmp0reg info.TempReg(0)
#define tmp1reg info.TempReg(1)		// temp 32-bit register from slot 1
#define tmp2reg info.TempReg(2)		// temp 32-bit register from slot 2
#define tmp3reg info.TempReg(3)		// temp 32-bit register from slot 3

// Temporary register from slot zero (8-bit variety)
// Note: this is the *lo* register, such as AL, BL, etc.
#define tmp0reg8 info.TempReg8(0)
#define tmp1reg8 info.TempReg8(1)		// temp 8-bit register from slot 1
#define tmp2reg8 info.TempReg8(2)		// temp 8-bit register from slot 2
#define tmp3reg8 info.TempReg8(3)		// temp 8-bit register from slot 3

#define RegRs info.SrcField(RF_Rs)
#define RegRt info.SrcField(RF_Rt)

#define DestRegRd info.DestField(RF_Rd)
#define DestRegRt info.DestField(RF_Rt)

//////////////////////////////////////////////////////////////////////////////////////////
// Contains information about each instruction of the original MIPS code in a sort of
// "halfway house" status -- with partial x86 register mapping.
//
class IntermediateRepresentation
{
public:
	// Mapping State of the GPRs upon entry to the recompiled instruction code generator.
	// The recompiler will do necessary "busywork" needed to move registers from the
	// dest mapping of the previous instruction to the src mapping of this instruction.
	// source operand can either be register or memory.
	xDirectOrIndirect32 Src[34];

	// Mapping state of the GPRs upon exit of the recompiled instruction code generator.
	// The recompiler will do necessary "busywork" needed to move registers from the
	// dest mapping here to the src mapping of the next instruction.
	// Dest operand can either be register or memory.
	xDirectOrIndirect32 Dest[34];

	// Strict indicator for each gpr's source mapping.  When 'true' secondary and
	// tertiary passes through the recompiler will not unmap these entries under any
	// circumstance.  Anything left 'false' can be unmapped and left as an indirect.
	bool RequiredSrcMapping[34];
	
	xTempRegsArray32<bool> xForceFlush;			// optional forced flush of volatile x86 registers

	InstructionRecMess Inst;		// raw instruction information.
	InstructionEmitterAPI Emitface;
	RegisterMappingOptions RegOpts;
	s32 ixImm;
	
	// Set true by the recompiler if the CommunativeSources flag is true and the recompiler
	// found it advantageous to swap them.  Typically code needn't check this flag, however
	// it can be useful for instructions that use conditionals (xCMP, etc) and need to swap
	// the conditional accordingly.
	bool IsSwappedSources;
	const InstConstInfoEx& m_constinfoex;

protected:
	xRegister32 m_TempReg[4];
	xRegister8 m_TempReg8[4];

public:
	IntermediateRepresentation() :
		m_constinfoex( (InstConstInfoEx&)*(InstConstInfoEx*)0 )		// One of the more evil hacks you'll ever see?
	{}

	IntermediateRepresentation( const xAddressReg& idxreg );
	IntermediateRepresentation( const InstConstInfoEx& src );
	
	const xDirectOrIndirect32& SrcField( RegField_t field ) const
	{
		int gpr = Inst.ReadsField( field );
		if( IsDevBuild && (gpr == -1) )
			throw Exception::LogicError( "IOPrec Logic Error: Instruction attempted to read an invalid field." );

		return Src[gpr];
	}

	const xDirectOrIndirect32& DestField( RegField_t field ) const
	{
		int gpr = Inst.WritesField( field );
		if( IsDevBuild && (gpr == -1) )
			throw Exception::LogicError( "IOPrec Logic Error: Instruction attempted to write to an invalid field." );

		return Dest[gpr];
	}

	const xRegister32& TempReg( uint tempslot ) const
	{
		jASSUME( tempslot < 4 );
		jASSUME( !m_TempReg[tempslot].IsEmpty() );	// make sure register was allocated properly!
		return m_TempReg[tempslot];
	}

	const xRegister8& TempReg8( uint tempslot ) const
	{
		jASSUME( tempslot < 4 );
		jASSUME( !m_TempReg8[tempslot].IsEmpty() );	// make sure register was allocated properly!
		return m_TempReg8[tempslot];
	}

	// --------------------------------------------------------
	//            Register Mapping / Allocation API
	// --------------------------------------------------------
	void SetFlushingState();
	
	//void		DynRegs_UnmapForcedIndirects( const RegMapInfo_Dynamic& dyno );
	void		DynRegs_AssignTempReg( int tempslot, const xRegister32& reg );

	void StrictRegs_UnmapClobbers( const RegMapInfo_Strict& stro );

	void MapTemporaryRegs();
	void UnmapTemporaryRegs();

	void UnmapReg( xDirectOrIndirect32 maparray[34], const xRegister32& reg );
	bool IsMappedReg( const xDirectOrIndirect32 maparray[34], const xRegister32& reg ) const;
	xRegister32 FindFreeTempReg( xDirectOrIndirect32 maparray[34] ) const;

public:
	s32 GetImm() const { return ixImm; }

	bool SignExtendsResult() const { return Inst.SignExtendResult(); }

	bool IsConstRs() const { return Inst.IsConstRs(); }
	bool IsConstRt() const { return Inst.IsConstRt(); }

	int GetConstRs() const
	{
		jASSUME( Inst.IsConstRs() );
		return Inst.ConstVal_Rs;
	}

	int GetConstRt() const
	{
		jASSUME( Inst.IsConstRt() );
		return Inst.ConstVal_Rt;
	}

	// ------------------------------------------------------------------------
	void MoveRsTo( const xRegister32& destreg ) const 
	{
		if( IsConstRs() )
			xMOV( destreg, Inst.ConstVal_Rs );
		else
			xMOV( destreg, SrcField( RF_Rs ) );
	}

	void MoveRtTo( const xRegister32& destreg ) const 
	{
		if( IsConstRt() )
			xMOV( destreg, Inst.ConstVal_Rt );
		else
			xMOV( destreg, SrcField( RF_Rt ) );	
	}

	// ------------------------------------------------------------------------
	template< typename T >
	void SignExtendedMove( const xRegister32& dest, const ModSibStrict<T>& src ) const
	{
		if( Inst.SignExtendResult() )
			xMOVSX( dest, src );
		else
			xMOVZX( dest, src );
	}

	template<>
	void SignExtendedMove<u32>( const xRegister32& dest, const ModSibStrict<u32>& src ) const
	{
		xMOV( dest, src );
	}

	template< typename T >
	void SignExtendEax() const
	{
		if( SignExtendsResult() )
		{
			if( sizeof(T) == 1 )
				xMOVSX( eax, al );
			else
				xCWDE();
		}
		else
		{
			if( sizeof(T) == 1 )
				xMOVZX( eax, al );
			else
				xMOVZX( eax, ax );
		}
	}
	
	// Do nothing for 32-bit sign extension.
	template<> void SignExtendEax<u32>() const { }

	// Important!  You should generally use this instead of the expanded Dest, as you
	// must be careful to check for instances of the zero register.
	void MoveToRt( const xDirectOrIndirect32& src ) const
	{
		if( Inst._Rt_ == 0 ) return;
		xMOV( DestField(RF_Rt), src );
	}

	void MoveToRd( const xDirectOrIndirect32& src ) const
	{
		if( Inst._Rd_ == 0 ) return;
		xMOV( DestField(RF_Rd), src );
	}
	
	// ------------------------------------------------------------------------
	void MoveToHiLo( const xRegister32& hireg, const xRegister32& loreg ) const
	{
		xMOV( DestField(RF_Hi), hireg );
		xMOV( DestField(RF_Lo), loreg );
	}

	// loads lo with a register, and loads hi with zero.
	void MoveToHiLo( const xRegister32& loreg ) const
	{
		xMOV( DestField(RF_Hi), 0 );
		xMOV( DestField(RF_Lo), loreg );
	}
};



//////////////////////////////////////////////////////////////////////////////////////////
//
class IntermediateBlock
{
};

//////////////////////////////////////////////////////////////////////////////////////////
// xJumpLink - houses the address of the jump instruction and the comparison type.  The
// address is the full instruction (not just the displacement portion), and comparison type
// is also provided so that we can write the entire jump using either j8 or j32 as needed.
//
struct xJumpLink
{
	u8* xPtr;					// address of the jump to be written
	JccComparisonType ccType;	// type of comparison to be used when writing jumps
	
	xJumpLink() {}
	
	xJumpLink( u8* x86ptr, JccComparisonType cctype ) :
		xPtr( x86ptr ),
		ccType( cctype )
	{
	}
	
	void SetTarget( const void* target ) const;

	void SetTarget( uptr target ) const
	{
		SetTarget( (void*)target );
	}
};

static const int MaxCyclesPerBlock = 64;

//////////////////////////////////////////////////////////////////////////////////////////
// recBlockItemTemp - Temporary workspace buffer used to reduce the number of heap allocations
// required during block recompilation.
//
struct recBlockItemTemp
{
	InstConstInfoEx	icex[MaxCyclesPerBlock];
	int instlen;

	//u32				ramcopy[MaxCyclesPerBlock];
	//int ramlen;
};

//////////////////////////////////////////////////////////////////////////////////////////
//
struct recBlockItem : public NoncopyableObject
{
	uint x86len;	// length of the recompiled block
	uint clears;	// number of times this block has been cleared and recompiled
	
	// Intermediate language allocation.  If size is non-zero, then we're on our second pass
	// and the IL should be recompiled into x86 code for direct execution.
	SafeArray<InstConstInfoEx> IL;

	// A list of all block links dependent on this block.  If this block moves, then all links
	// in this list need to have their x86 jump instructions rewritten.
	SafeList<xJumpLink> DependentLinks;

	// This member contains a copy of the code originally recompiled, for use with
	// MMX/XMM optimized validation of blocks (and subsequent clearing if the block
	// in memory does not match the validation copy recorded when recompilation was
	// performed).
	SafeArray<u32> ValidationCopy;

	recBlockItem() :
		x86len( 0 ),
		clears( 0 ),
		IL( "recBlockItem::IL" ),
		DependentLinks( 4, "recBlockItem::DependentLinks" ),
		ValidationCopy( "recBlockItem::ValidationCopy" )
	{
		IL.ChunkSize = 32;
		DependentLinks.ChunkSize = 8;
		ValidationCopy.ChunkSize = 32;
	}

	void Assign( const recBlockItemTemp& src );
};


//////////////////////////////////////////////////////////////////////////////////////////


extern recBlockItemTemp m_blockspace;
extern iopRecState m_RecState;

extern void recIR_Block();

}
