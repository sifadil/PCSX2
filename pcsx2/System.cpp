/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2009  PCSX2 Dev Team
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

#include "HostGui.h"

#include "Common.h"
#include "VUmicro.h"
#include "iR5900.h"
#include "R3000A.h"
#include "IopMem.h"
#include "sVU_zerorec.h"		// for SuperVUReset

#include "R5900Exceptions.h"
#include "CDVD/CDVD.h"
#include "System/PageFaultSource.h"

#include "Utilities/EventSource.inl"
EventSource_ImplementType( PageFaultInfo );

SrcType_PageFault Source_PageFault;

#if _MSC_VER
#	include "svnrev.h"
#endif

const Pcsx2Config EmuConfig;

// disable all session overrides by default...
SessionOverrideFlags	g_Session = {false};

// This function should be called once during program execution.
void SysDetect()
{
	Console.Notice("PCSX2 %d.%d.%d.r%d %s - compiled on " __DATE__, PCSX2_VersionHi, PCSX2_VersionMid, PCSX2_VersionLo,
		SVN_REV, SVN_MODS ? "(modded)" : ""
	);

	Console.Notice("Savestate version: %x", g_SaveVersion);

	cpudetectInit();

	Console.SetColor( Color_Black );

	Console.WriteLn( "x86Init:" );
	Console.WriteLn( wxsFormat(
		L"\tCPU vendor name  =  %s\n"
		L"\tFamilyID         =  %x\n"
		L"\tx86Family        =  %s\n"
		L"\tCPU speed        =  %d.%03d ghz\n"
		L"\tCores            =  %d physical [%d logical]\n"
		L"\tx86PType         =  %s\n"
		L"\tx86Flags         =  %8.8x %8.8x\n"
		L"\tx86EFlags        =  %8.8x\n",
			wxString::FromAscii( x86caps.VendorName ).c_str(), x86caps.StepID,
			wxString::FromAscii( x86caps.FamilyName ).Trim().Trim(false).c_str(),
			x86caps.Speed / 1000, x86caps.Speed%1000,
			x86caps.PhysicalCores, x86caps.LogicalCores,
			wxString::FromAscii( x86caps.TypeName ).c_str(),
			x86caps.Flags, x86caps.Flags2,
			x86caps.EFlags
	) );

	wxArrayString features[2];	// 2 lines, for readability!

	if( x86caps.hasMultimediaExtensions )			features[0].Add( L"MMX" );
	if( x86caps.hasStreamingSIMDExtensions )		features[0].Add( L"SSE" );
	if( x86caps.hasStreamingSIMD2Extensions )		features[0].Add( L"SSE2" );
	if( x86caps.hasStreamingSIMD3Extensions )		features[0].Add( L"SSE3" );
	if( x86caps.hasSupplementalStreamingSIMD3Extensions ) features[0].Add( L"SSSE3" );
	if( x86caps.hasStreamingSIMD4Extensions )		features[0].Add( L"SSE4.1" );
	if( x86caps.hasStreamingSIMD4Extensions2 )		features[0].Add( L"SSE4.2" );

	if( x86caps.hasMultimediaExtensionsExt )		features[1].Add( L"MMX2  " );
	if( x86caps.has3DNOWInstructionExtensions )		features[1].Add( L"3DNOW " );
	if( x86caps.has3DNOWInstructionExtensionsExt )	features[1].Add( L"3DNOW2" );
	if( x86caps.hasStreamingSIMD4ExtensionsA )		features[1].Add( L"SSE4a " );

	wxString result[2];
	JoinString( result[0], features[0], L".. " );
	JoinString( result[1], features[1], L".. " );

	Console.WriteLn( L"Features Detected:\n\t" + result[0] + (result[1].IsEmpty() ? L"" : (L"\n\t" + result[1])) + L"\n" );

	//if ( x86caps.VendorName[0] == 'A' ) //AMD cpu

	Console.ClearColor();
}

// returns the translated error message for the Virtual Machine failing to allocate!
static wxString GetMemoryErrorVM()
{
	return pxE( ".Popup Error:EmuCore::MemoryForVM",
		L"PCSX2 is unable to allocate memory needed for the PS2 virtual machine. "
		L"Close out some memory hogging background tasks and try again."
	);
}

SysCoreAllocations::SysCoreAllocations()
{
	InstallSignalHandler();

	Console.Status( "Initializing PS2 virtual machine..." );

	RecSuccess_EE		= false;
	RecSuccess_IOP		= false;
	RecSuccess_VU0		= false;
	RecSuccess_VU1		= false;

	try
	{
		vtlb_Core_Alloc();
		memAlloc();
		psxMemAlloc();
		vuMicroMemAlloc();
	}
	// ----------------------------------------------------------------------------
	catch( Exception::OutOfMemory& ex )
	{
		wxString newmsg( ex.UserMsg() + L"\n\n" + GetMemoryErrorVM() );
		ex.UserMsg() = newmsg;
		CleanupMess();
		throw;
	}
	catch( std::bad_alloc& ex )
	{
		CleanupMess();

		// re-throw std::bad_alloc as something more friendly.

		throw Exception::OutOfMemory(
			wxsFormat(			// Diagnostic (english)
				L"std::bad_alloc caught while trying to allocate memory for the PS2 Virtual Machine.\n"
				L"Error Details: " + fromUTF8( ex.what() )
			),

			GetMemoryErrorVM()	// translated
		);
	}

	Console.Status( "Allocating memory for recompilers..." );

	try
	{
		recCpu.Allocate();
		RecSuccess_EE = true;
	}
	catch( Exception::BaseException& ex )
	{
		Console.Error( L"EE Recompiler Allocation Failed:\n" + ex.FormatDiagnosticMessage() );
		recCpu.Shutdown();
	}

	try
	{
		psxRec.Allocate();
		RecSuccess_IOP = true;
	}
	catch( Exception::BaseException& ex )
	{
		Console.Error( L"IOP Recompiler Allocation Failed:\n" + ex.FormatDiagnosticMessage() );
		psxRec.Shutdown();
	}

	// hmm! : VU0 and VU1 pre-allocations should do sVU and mVU separately?  Sounds complicated. :(

	try
	{
		VU0micro::recAlloc();
		RecSuccess_VU0 = true;
	}
	catch( Exception::BaseException& ex )
	{
		Console.Error( L"VU0 Recompiler Allocation Failed:\n" + ex.FormatDiagnosticMessage() );
		VU0micro::recShutdown();
	}

	try
	{
		VU1micro::recAlloc();
		RecSuccess_VU1 = true;
	}
	catch( Exception::BaseException& ex )
	{
		Console.Error( L"VU1 Recompiler Allocation Failed:\n" + ex.FormatDiagnosticMessage() );
		VU1micro::recShutdown();
	}

	// If both VUrecs failed, then make sure the SuperVU is totally closed out, because it
	// actually initializes everything once and then shares it between both VU recs.
	if( !RecSuccess_VU0 && !RecSuccess_VU1 )
		SuperVUDestroy( -1 );
}

void SysCoreAllocations::CleanupMess() throw()
{
	try
	{
		// Special SuperVU "complete" terminator.
		SuperVUDestroy( -1 );

		VU1micro::recShutdown();
		VU0micro::recShutdown();

		psxRec.Shutdown();
		recCpu.Shutdown();

		vuMicroMemShutdown();
		psxMemShutdown();
		memShutdown();
		vtlb_Core_Shutdown();
	}
	DESTRUCTOR_CATCHALL
}

SysCoreAllocations::~SysCoreAllocations() throw()
{
	CleanupMess();
}

bool SysCoreAllocations::HadSomeFailures( const Pcsx2Config::RecompilerOptions& recOpts ) const
{
	return	(recOpts.EnableEE && !RecSuccess_EE) ||
			(recOpts.EnableIOP && !RecSuccess_IOP) ||
			(recOpts.EnableVU0 && !RecSuccess_VU0) ||
			(recOpts.EnableVU1 && !RecSuccess_VU1);
}

// Resets all PS2 cpu execution caches, which does not affect that actual PS2 state/condition.
// This can be called at any time outside the context of a Cpu->Execute() block without
// bad things happening (recompilers will slow down for a brief moment since rec code blocks
// are dumped).
// Use this method to reset the recs when important global pointers like the MTGS are re-assigned.
void SysClearExecutionCache()
{
	Cpu		= CHECK_EEREC ? &recCpu : &intCpu;
	psxCpu	= CHECK_IOPREC ? &psxRec : &psxInt;

	Cpu->Reset();
	psxCpu->Reset();

	vuMicroCpuReset();
}

// Maps a block of memory for use as a recompiled code buffer, and ensures that the
// allocation is below a certain memory address (specified in "bounds" parameter).
// The allocated block has code execution privileges.
// Returns NULL on allocation failure.
u8 *SysMmapEx(uptr base, u32 size, uptr bounds, const char *caller)
{
	u8 *Mem = (u8*)HostSys::Mmap( base, size );

	if( (Mem == NULL) || (bounds != 0 && (((uptr)Mem + size) > bounds)) )
	{
		DevCon.Notice( "First try failed allocating %s at address 0x%x", caller, base );

		// memory allocation *must* have the top bit clear, so let's try again
		// with NULL (let the OS pick something for us).

		SafeSysMunmap( Mem, size );

		Mem = (u8*)HostSys::Mmap( NULL, size );
		if( bounds != 0 && (((uptr)Mem + size) > bounds) )
		{
			DevCon.Error( "Fatal Error:\n\tSecond try failed allocating %s, block ptr 0x%x does not meet required criteria.", caller, Mem );
			SafeSysMunmap( Mem, size );

			// returns NULL, caller should throw an exception.
		}
	}
	return Mem;
}
