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

#pragma once

#include "x86emitter/tools.h"

class IniInterface;

enum PluginsEnum_t
{
	PluginId_GS = 0,
	PluginId_PAD,
	PluginId_SPU2,
	PluginId_CDVD,
	PluginId_USB,
	PluginId_FW,
	PluginId_DEV9,
	PluginId_Count,

	// Memorycard plugin support is preliminary, and is only hacked/hardcoded in at this
	// time.  So it's placed afer PluginId_Count so that it doesn't show up in the conf
	// screens or other plugin tables.

	PluginId_Mcd
};

//------------ DEFAULT sseMXCSR VALUES ---------------
#define DEFAULT_sseMXCSR	0xffc0 //FPU rounding > DaZ, FtZ, "chop"
#define DEFAULT_sseVUMXCSR	0xffc0 //VU  rounding > DaZ, FtZ, "chop"

// --------------------------------------------------------------------------------------
//  TraceFiltersEE
// --------------------------------------------------------------------------------------
struct TraceFiltersEE
{
	BITFIELD32()
	bool
		m_EnableAll		:1,		// Master Enable switch (if false, no logs at all)

		m_Bios			:1,		// SYSCALL, RPCs, etc.
		m_Memory		:1,		// memory accesses (*all* loads and stores)
		m_Cache			:1,		// Data Cache (Unimplemented)
		m_SysCtrl		:1,		// TLB logs, PERF logs, Debug register logs

		m_VIFunpack		:1,
		m_GIFtag		:1,

		m_EnableDisasm	:1,
		m_R5900			:1,		// instructions, branches, and exception vectors (includes COP0)
		m_COP0			:1,		// System Control instructions
		m_COP1			:1,		// EE FPU instructions
		m_COP2			:1,		// VU0 macro mode instructions and pipeline states
		m_VU0micro		:1,
		m_VU1micro		:1,

		m_EnableHardware:1,
		m_KnownHw		:1,		// Known and handled HW Registers
		m_UnknownHw		:1,		// Unknown/Unhandled Hardware Registers
		m_DMA			:1,		// DMA Log filter, to allow for filtering HW separate of the verbose Known Registers.

		m_EnableEvents	:1,		// Enables logging of event-driven activity -- counters, DMAs, etc.
		m_Counters		:1,		// EE's counters!
		m_VIF			:1,
		m_GIF			:1,
		m_IPU			:1,
		m_SPR			:1;		// Scratchpad Ram DMA
	BITFIELD_END

	bool Enabled() const		{ return m_EnableAll; }
	bool DisasmEnabled() const	{ return m_EnableAll && m_EnableDisasm; }
	bool HwEnabled() const		{ return m_EnableAll && m_EnableHardware; }
	bool EventsEnabled() const	{ return m_EnableAll && m_EnableEvents; }

	bool Bios() const			{ return m_EnableAll && m_Bios; }
	bool Memory() const			{ return m_EnableAll && m_Memory; }
	bool Cache() const			{ return m_EnableAll && m_Cache; }
	bool SysCtrl() const		{ return m_EnableAll && m_SysCtrl; }

	bool GIFtag() const			{ return m_EnableAll && m_GIFtag; }
	bool VIFunpack() const		{ return m_EnableAll && m_VIFunpack; }

	bool Counters() const		{ return EventsEnabled() && m_Counters; }
	bool VIF() const			{ return EventsEnabled() && m_VIF; }
	bool GIF() const			{ return EventsEnabled() && m_GIF; }
	bool IPU() const			{ return EventsEnabled() && m_IPU; }
	bool SPR() const			{ return EventsEnabled() && m_SPR; }

	bool R5900() const			{ return DisasmEnabled() && m_R5900; }
	bool COP0() const			{ return DisasmEnabled() && m_COP0; }
	bool COP1() const			{ return DisasmEnabled() && m_COP1; }
	bool COP2() const			{ return DisasmEnabled() && m_COP2; }
	bool VU0micro() const		{ return DisasmEnabled() && m_VU0micro; }
	bool VU1micro() const		{ return DisasmEnabled() && m_VU1micro; }

	bool KnownHw() const		{ return HwEnabled() && m_KnownHw; }
	bool UnknownHw() const		{ return HwEnabled() && m_UnknownHw; }
	bool DMA() const			{ return HwEnabled() && m_DMA; }

	TraceFiltersEE()
	{
		bitset = 0;
	}

	bool operator ==( const TraceFiltersEE& right ) const
	{
		return OpEqu( bitset );
	}

	bool operator !=( const TraceFiltersEE& right ) const
	{
		return !this->operator ==( right );
	}
};

// --------------------------------------------------------------------------------------
//  TraceFiltersIOP
// --------------------------------------------------------------------------------------
struct TraceFiltersIOP
{
	BITFIELD32()
	bool
		m_EnableAll		:1,		// Master Enable switch (if false, no logs at all)
		m_Bios			:1,		// SYSCALL, RPCs, etc.
		m_Memory		:1,		// memory accesses (*all* loads and stores)

		m_EnableDisasm	:1,
		m_R3000A		:1,		// instructions. branches, and exception vectors
		m_COP2			:1,		// PS1 Triangle helper (disabled in PS2 mode)

		m_EnableHardware:1,
		m_KnownHw		:1,		// Known and handled HW Registers
		m_UnknownHw		:1,		// Unknown/Unhandled Hardware Registers
		m_DMA			:1,		// DMA Log filter, to allow for filtering HW separate of the verbose Known Registers.

		m_EnableEvents	:1,		// Enables logging of event-driven activity -- counters, DMAs, etc.
		m_Counters		:1,		// IOP's counters!
		m_Memcards		:1,
		m_PAD			:1,
		m_SPU2			:1,
		m_USB			:1,
		m_FW			:1,
		m_CDVD			:1,		// CDROM or CDVD activity (depends on game media type)

		m_GPU			:1;		// PS1's GPU (currently unimplemented and uncategorized)
	BITFIELD_END

	bool Enabled() const		{ return m_EnableAll; }
	bool DisasmEnabled() const	{ return m_EnableAll && m_EnableDisasm; }
	bool HwEnabled() const		{ return m_EnableAll && m_EnableHardware; }
	bool EventsEnabled() const	{ return m_EnableAll && m_EnableEvents; }

	bool Bios() const			{ return m_EnableAll && m_Bios; }
	bool Memory() const			{ return m_EnableAll && m_Memory; }

	bool Counters() const		{ return EventsEnabled() && m_Counters; }
	bool Memcards() const		{ return EventsEnabled() && m_Memcards; }
	bool PAD() const			{ return EventsEnabled() && m_PAD; }
	bool SPU2() const			{ return EventsEnabled() && m_SPU2; }
	bool USB() const			{ return EventsEnabled() && m_USB; }
	bool FW() const				{ return EventsEnabled() && m_FW; }
	bool CDVD() const			{ return EventsEnabled() && m_CDVD; }

	bool R3000A() const			{ return DisasmEnabled() && m_R3000A; }
	bool COP2() const			{ return DisasmEnabled() && m_COP2; }

	bool KnownHw() const		{ return HwEnabled() && m_KnownHw; }
	bool UnknownHw() const		{ return HwEnabled() && m_UnknownHw; }
	bool DMA() const			{ return HwEnabled() && m_DMA; }

	TraceFiltersIOP()
	{
		bitset = 0;
	}

	bool operator ==( const TraceFiltersIOP& right ) const
	{
		return OpEqu( bitset );
	}

	bool operator !=( const TraceFiltersIOP& right ) const
	{
		return !this->operator ==( right );
	}
};

// --------------------------------------------------------------------------------------
//  TraceLogFilters
// --------------------------------------------------------------------------------------
struct TraceLogFilters
{
	// Enabled - global toggle for high volume logging.  This is effectively the equivalent to
	// (EE.Enabled() || IOP.Enabled() || SIF) -- it's cached so that we can use the macros
	// below to inline the conditional check.  This is desirable because these logs are
	// *very* high volume, and debug builds get noticably slower if they have to invoke
	// methods/accessors to test the log enable bits.  Debug builds are slow enough already,
	// so I prefer this to help keep them usable.
	bool	Enabled;

	bool	SIF;			// SIF DMA Activity (both EE and IOP sides)

	TraceFiltersEE	EE;
	TraceFiltersIOP	IOP;

	TraceLogFilters()
	{
		Enabled	= false;
		SIF		= false;
	}

	void LoadSave( IniInterface& ini );

	bool operator ==( const TraceLogFilters& right ) const
	{
		return OpEqu( Enabled ) && OpEqu( SIF ) && OpEqu( EE ) && OpEqu( IOP );
	}

	bool operator !=( const TraceLogFilters& right ) const
	{
		return !this->operator ==( right );
	}
};

struct ConsoleLogFilters
{
	BITFIELD32()
		bool
			ELF			:1,
			Deci2		:1,
			StdoutEE	:1,
			StdoutIOP	:1;
	BITFIELD_END;

	ConsoleLogFilters();
	void LoadSave( IniInterface& ini );

	bool operator ==( const ConsoleLogFilters& right ) const
	{
		return OpEqu( bitset );
	}

	bool operator !=( const ConsoleLogFilters& right ) const
	{
		return !this->operator ==( right );
	}
};

// --------------------------------------------------------------------------------------
//  Pcsx2Config class
// --------------------------------------------------------------------------------------
// This is intended to be a public class library between the core emulator and GUI only.
// It is *not* meant to be shared data between core emulation and plugins, due to issues
// with version incompatibilities if the structure formats are changed.
//
// When GUI code performs modifications of this class, it must be done with strict thread
// safety, since the emu runs on a separate thread.  Additionally many components of the
// class require special emu-side resets or state save/recovery to be applied.  Please
// use the provided functions to lock the emulation into a safe state and then apply
// chances on the necessary scope (see Core_Pause, Core_ApplySettings, and Core_Resume).
//
struct Pcsx2Config
{
	struct ProfilerOptions
	{
		BITFIELD32()
			bool
				Enabled:1,			// universal toggle for the profiler.
				RecBlocks_EE:1,		// Enables per-block profiling for the EE recompiler [unimplemented]
				RecBlocks_IOP:1,	// Enables per-block profiling for the IOP recompiler [unimplemented]
				RecBlocks_VU0:1,	// Enables per-block profiling for the VU0 recompiler [unimplemented]
				RecBlocks_VU1:1;	// Enables per-block profiling for the VU1 recompiler [unimplemented]
		BITFIELD_END

		// Default is Disabled, with all recs enabled underneath.
		ProfilerOptions() : bitset( 0xfffffffe ) {}
		void LoadSave( IniInterface& conf );

		bool operator ==( const ProfilerOptions& right ) const
		{
			return OpEqu( bitset );
		}

		bool operator !=( const ProfilerOptions& right ) const
		{
			return !OpEqu( bitset );
		}
	};

	// ------------------------------------------------------------------------
	struct RecompilerOptions
	{
		BITFIELD32()
			bool
				EnableEE		:1,
				EnableIOP		:1,
				EnableVU0		:1,
				EnableVU1		:1;

			bool
				UseMicroVU0		:1,
				UseMicroVU1		:1;

			bool
				vuOverflow		:1,
				vuExtraOverflow	:1,
				vuSignOverflow	:1,
				vuUnderflow		:1;

			bool
				fpuOverflow		:1,
				fpuExtraOverflow:1,
				fpuFullMode		:1;

			bool
				StackFrameChecks:1;
		BITFIELD_END

		RecompilerOptions();
		void ApplySanityCheck();

		void LoadSave( IniInterface& conf );

		bool operator ==( const RecompilerOptions& right ) const
		{
			return OpEqu( bitset );
		}

		bool operator !=( const RecompilerOptions& right ) const
		{
			return !OpEqu( bitset );
		}

	} Recompiler;

	// ------------------------------------------------------------------------
	struct CpuOptions
	{
		RecompilerOptions Recompiler;

		SSE_MXCSR sseMXCSR;
		SSE_MXCSR sseVUMXCSR;

		CpuOptions();
		void LoadSave( IniInterface& conf );
		void ApplySanityCheck();

		bool operator ==( const CpuOptions& right ) const
		{
			return OpEqu( sseMXCSR ) && OpEqu( sseVUMXCSR ) && OpEqu( Recompiler );
		}

		bool operator !=( const CpuOptions& right ) const
		{
			return !this->operator ==( right );
		}
	};

	// ------------------------------------------------------------------------
	struct GSOptions
	{
		// forces the MTGS to execute tags/tasks in fully blocking/synchronous
		// style.  Useful for debugging potential bugs in the MTGS pipeline.
		bool	SynchronousMTGS;
		bool	DisableOutput;

		bool	FrameLimitEnable;
		bool	FrameSkipEnable;
		bool	VsyncEnable;

		// The region mode controls the default Maximum/Minimum FPS settings and also
		// regulates the vsync rates (which in turn control the IOP's SPU2 tick sync and ensure
		// proper audio playback speed).
		int		DefaultRegionMode;	// 0=NTSC and 1=PAL

		int		FramesToDraw;	// number of consecutive frames (fields) to render
		int		FramesToSkip;	// number of consecutive frames (fields) to skip

		Fixed100	LimitScalar;
		Fixed100	FramerateNTSC;
		Fixed100	FrameratePAL;

		GSOptions();
		void LoadSave( IniInterface& conf );

		bool operator ==( const GSOptions& right ) const
		{
			return
				OpEqu( SynchronousMTGS )		&&
				OpEqu( DisableOutput )			&&
				OpEqu( FrameSkipEnable )		&&
				OpEqu( FrameLimitEnable )		&&
				OpEqu( VsyncEnable )			&&

				OpEqu( LimitScalar )			&&
				OpEqu( FramerateNTSC )			&&
				OpEqu( FrameratePAL )			&&

				OpEqu( DefaultRegionMode )		&&
				OpEqu( FramesToDraw )			&&
				OpEqu( FramesToSkip );
		}

		bool operator !=( const GSOptions& right ) const
		{
			return !this->operator ==( right );
		}

	};

	// ------------------------------------------------------------------------
	// NOTE: The GUI's GameFixes panel is dependent on the order of bits in this structure.
	struct GamefixOptions
	{
		BITFIELD32()
			bool
				VuAddSubHack	:1,		// Tri-ace games, they use an encryption algorithm that requires VU ADDI opcode to be bit-accurate.
				VuClipFlagHack	:1,		// Persona games, maybe others. It's to do with the VU clip flag (again).
				FpuCompareHack	:1,		// Digimon Rumble Arena 2, fixes spinning/hanging on intro-menu.
				FpuMulHack		:1,		// Tales of Destiny hangs.
				FpuNegDivHack	:1,		// Gundam games messed up camera-view.
				XgKickHack		:1,		// Erementar Gerad, adds more delay to VU XGkick instructions. Corrects the color of some graphics, but breaks Tri-ace games and others.
				IPUWaitHack     :1;
		BITFIELD_END

		// all gamefixes are disabled by default.
		GamefixOptions() : bitset( 0 ) {}
		void LoadSave( IniInterface& conf );

		bool operator ==( const GamefixOptions& right ) const
		{
			return OpEqu( bitset );
		}

		bool operator !=( const GamefixOptions& right ) const
		{
			return !OpEqu( bitset );
		}
	};

	// ------------------------------------------------------------------------
	struct SpeedhackOptions
	{
		BITFIELD32()
			bool
				IopCycleRate_X2	:1,		// enables the x2 multiplier of the IOP cyclerate
				IntcStat		:1,		// tells Pcsx2 to fast-forward through intc_stat waits.
				WaitLoop		:1,		// enables constant loop detection and fast-forwarding
				vuFlagHack		:1,		// microVU specific flag hack; Can cause Infinite loops, SPS, etc...
				vuMinMax		:1;		// microVU specific MinMax hack; Can cause SPS, Black Screens,  etc...
		BITFIELD_END

		u8	EECycleRate;		// EE cycle rate selector (1.0, 1.5, 2.0)
		u8	VUCycleSteal;		// VU Cycle Stealer factor (0, 1, 2, or 3)

		SpeedhackOptions();
		void LoadSave( IniInterface& conf );

		bool operator ==( const SpeedhackOptions& right ) const
		{
			return OpEqu( bitset ) && OpEqu( EECycleRate ) && OpEqu( VUCycleSteal );
		}

		bool operator !=( const SpeedhackOptions& right ) const
		{
			return !this->operator ==( right );
		}
	};

	BITFIELD32()
		bool
			CdvdVerboseReads	:1,		// enables cdvd read activity verbosely dumped to the console
			CdvdDumpBlocks		:1,		// enables cdvd block dumping
			EnablePatches		:1,		// enables patch detection and application

		// when enabled performs bios stub execution, skipping full sony bios + splash screens
			SkipBiosSplash		:1,

		// enables simulated ejection of memory cards when loading savestates
			McdEnableEjection	:1,

			MultitapPort0_Enabled:1,
			MultitapPort1_Enabled:1,

			ConsoleToStdio:1;
	BITFIELD_END

	CpuOptions			Cpu;
	GSOptions			GS;
	SpeedhackOptions	Speedhacks;
	GamefixOptions		Gamefixes;
	ProfilerOptions		Profiler;

	ConsoleLogFilters	Log;
	TraceLogFilters		Trace;

	wxFileName			BiosFilename;

	Pcsx2Config();
	void LoadSave( IniInterface& ini );

	void Load( const wxString& srcfile );
	void Load( const wxInputStream& srcstream );
	void Save( const wxString& dstfile );
	void Save( const wxOutputStream& deststream );

	bool MultitapEnabled( uint port ) const;

	bool operator ==( const Pcsx2Config& right ) const
	{
		return
			OpEqu( bitset )		&&
			OpEqu( Cpu )		&&
			OpEqu( GS )		&&
			OpEqu( Speedhacks )	&&
			OpEqu( Gamefixes )	&&
			OpEqu( Profiler )	&&
			OpEqu( Log )		&&
			OpEqu( Trace )		&&
			OpEqu( BiosFilename );
	}

	bool operator !=( const Pcsx2Config& right ) const
	{
		return !this->operator ==( right );
	}
};

extern const Pcsx2Config EmuConfig;

Pcsx2Config::GSOptions&	SetGSConfig();
ConsoleLogFilters&			SetConsoleConfig();
TraceLogFilters&			SetTraceConfig();


/////////////////////////////////////////////////////////////////////////////////////////
// Helper Macros for Reading Emu Configurations.
//

// ------------ CPU / Recompiler Options ---------------

#define CHECK_MICROVU0				(EmuConfig.Cpu.Recompiler.UseMicroVU0)
#define CHECK_MICROVU1				(EmuConfig.Cpu.Recompiler.UseMicroVU1)
#define CHECK_EEREC					(EmuConfig.Cpu.Recompiler.EnableEE && GetSysCoreAlloc().IsRecAvailable_EE())
#define CHECK_IOPREC				(EmuConfig.Cpu.Recompiler.EnableIOP && GetSysCoreAlloc().IsRecAvailable_IOP())

//------------ SPECIAL GAME FIXES!!! ---------------
#define NUM_OF_GAME_FIXES 7

#define CHECK_VUADDSUBHACK			(EmuConfig.Gamefixes.VuAddSubHack) // Special Fix for Tri-ace games, they use an encryption algorithm that requires VU addi opcode to be bit-accurate.
#define CHECK_FPUCOMPAREHACK		(EmuConfig.Gamefixes.FpuCompareHack) // Special Fix for Digimon Rumble Arena 2, fixes spinning/hanging on intro-menu.
#define CHECK_VUCLIPFLAGHACK		(EmuConfig.Gamefixes.VuClipFlagHack) // Special Fix for Persona games, maybe others. It's to do with the VU clip flag (again).
#define CHECK_FPUMULHACK			(EmuConfig.Gamefixes.FpuMulHack) // Special Fix for Tales of Destiny hangs.
#define CHECK_FPUNEGDIVHACK			(EmuConfig.Gamefixes.FpuNegDivHack) // Special Fix for Gundam games messed up camera-view.
#define CHECK_XGKICKHACK			(EmuConfig.Gamefixes.XgKickHack) // Special Fix for Erementar Gerad, adds more delay to VU XGkick instructions. Corrects the color of some graphics.
#define CHECK_IPUWAITHACK			(EmuConfig.Gamefixes.IPUWaitHack) // Special Fix for Erementar Gerad, adds more delay to VU XGkick instructions. Corrects the color of some graphics.

//------------ Advanced Options!!! ---------------
#define CHECK_VU_OVERFLOW			(EmuConfig.Cpu.Recompiler.vuOverflow)
#define CHECK_VU_EXTRA_OVERFLOW		(EmuConfig.Cpu.Recompiler.vuExtraOverflow) // If enabled, Operands are clamped before being used in the VU recs
#define CHECK_VU_SIGN_OVERFLOW		(EmuConfig.Cpu.Recompiler.vuSignOverflow)
#define CHECK_VU_UNDERFLOW			(EmuConfig.Cpu.Recompiler.vuUnderflow)
#define CHECK_VU_EXTRA_FLAGS		0	// Always disabled now // Sets correct flags in the sVU recs

#define CHECK_FPU_OVERFLOW			(EmuConfig.Cpu.Recompiler.fpuOverflow)
#define CHECK_FPU_EXTRA_OVERFLOW	(EmuConfig.Cpu.Recompiler.fpuExtraOverflow) // If enabled, Operands are checked for infinities before being used in the FPU recs
#define CHECK_FPU_EXTRA_FLAGS		1	// Always enabled now // Sets D/I flags on FPU instructions
#define CHECK_FPU_FULL				(EmuConfig.Cpu.Recompiler.fpuFullMode)

//------------ EE Recompiler defines - Comment to disable a recompiler ---------------

#define SHIFT_RECOMPILE		// Speed majorly reduced if disabled
#define BRANCH_RECOMPILE	// Speed extremely reduced if disabled - more then shift

// Disabling all the recompilers in this block is interesting, as it still runs at a reasonable rate.
// It also adds a few glitches. Really reminds me of the old Linux 64-bit version. --arcum42
#define ARITHMETICIMM_RECOMPILE
#define ARITHMETIC_RECOMPILE
#define MULTDIV_RECOMPILE
#define JUMP_RECOMPILE
#define LOADSTORE_RECOMPILE
#define MOVE_RECOMPILE
#define MMI_RECOMPILE
#define MMI0_RECOMPILE
#define MMI1_RECOMPILE
#define MMI2_RECOMPILE
#define MMI3_RECOMPILE
#define FPU_RECOMPILE
#define CP0_RECOMPILE
#define CP2_RECOMPILE

// You can't recompile ARITHMETICIMM without ARITHMETIC.
#ifndef ARITHMETIC_RECOMPILE
#undef ARITHMETICIMM_RECOMPILE
#endif

#define EE_CONST_PROP 1 // rec2 - enables constant propagation (faster)

// Change to 1 if working on getting PS1 emulation working.
// This disables the exception normally caused by trying to load PS1
// games. Note: currently PS1 games will error out even without this
// commented, so this is for development purposes only.
#define ENABLE_LOADING_PS1_GAMES 0

// Change to 1 to enable SIF wakeup hack:
#define IOP_ENABLE_SIF_HACK 0

// Change to 1 to cause all logs to be written to the console. (Very slow)
#define LOG_TO_CONSOLE 0
