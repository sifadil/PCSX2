/* SPU2-X, A plugin for Emulating the Sound Processing Unit of the Playstation 2
 * Developed and maintained by the Pcsx2 Development Team.
 * 
 * Original portions from SPU2ghz are (c) 2008 by David Quintana [gigaherz]
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free 
 * Software Foundation; either version 2.1 of the the License, or (at your
 * option) any later version.
 * 
 * This library is distributed in the hope that it will be useful, but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License along
 * with this library; if not, write to the Free Software Foundation, Inc., 59
 * Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 */

#ifndef DEFS_H_INCLUDED
#define DEFS_H_INCLUDED

struct V_VolumeLR
{
	static V_VolumeLR Max;

	s32 Left;
	s32 Right;

	V_VolumeLR() {}
	V_VolumeLR( s32 both ) :
		Left( both ),
		Right( both )
	{
	}
	
	void DebugDump( FILE* dump, const char* title );
};

struct V_VolumeSlide
{
	// Holds the "original" value of the volume for this voice, prior to slides.
	// (ie, the volume as written to the register)
	
	s16 Reg_VOL;
	s32 Value;
	s8 Increment;
	s8 Mode;
	
public:
	V_VolumeSlide() {}
	V_VolumeSlide( s16 regval, s32 fullvol ) :
		Reg_VOL( regval ),
		Value( fullvol ),
		Increment( 0 ),
		Mode( 0 )
	{
	}

	void Update();
	void RegSet( u16 src );		// used to set the volume from a register source (16 bit signed)
	void DebugDump( FILE* dump, const char* title, const char* nameLR );
	
};

struct V_VolumeSlideLR
{
	static V_VolumeSlideLR Max;

	V_VolumeSlide Left;
	V_VolumeSlide Right;

public:
	V_VolumeSlideLR() {}
		
	V_VolumeSlideLR( s16 regval, s32 bothval ) :
		Left( regval, bothval ),
		Right( regval, bothval )
	{
	}

	void Update()
	{
		Left.Update();
		Right.Update();
	}
	
	void DebugDump( FILE* dump, const char* title );
};


struct V_ADSR
{
	u16 Reg_ADSR1;
	u16 Reg_ADSR2;

	s32 Value;		// Ranges from 0 to 0x7fffffff (signed values are clamped to 0) [Reg_ENVX]
	u8 Phase;
	u8 AttackRate;		// Ar
	u8 AttackMode;		// Am
	u8 DecayRate;		// Dr
	u8 SustainLevel;	// Sl
	u8 SustainRate;		// Sr
	u8 SustainMode;		// Sm
	u8 ReleaseRate;		// Rr
	u8 ReleaseMode;		// Rm

	bool Releasing;		// Ready To Release, triggered by Voice.Stop();

public:
	bool Calculate();
};


struct V_Voice
{
	u32 PlayCycle;		// SPU2 cycle where the Playing started

	V_VolumeSlideLR Volume;

// Envelope
	V_ADSR ADSR;
// Pitch (also Reg_PITCH)
	s16 Pitch; 
// Pitch Modulated by previous voice
	s8 Modulated;
// Source (Wave/Noise)
	s8 Noise;
// Loop Start address (also Reg_LSAH/L)
	u32 LoopStartA; 
// Sound Start address (also Reg_SSAH/L)
	u32 StartA;
// Next Read Data address (also Reg_NAXH/L)
	u32 NextA;
// Voice Decoding State
	s32 Prev1;
	s32 Prev2;

	s8 LoopMode;
	s8 LoopFlags;

// [Air] : Replaced loop flags read from the ADPCM header with
//  a single LoopFlags value (above) -- more cache-friendly.
	//s8 LoopStart;
	//s8 Loop;
	//s8 LoopEnd;

// Sample pointer (19:12 bit fixed point)
	s32 SP;

// Sample pointer for Cubic Interpolation
// Cubic interpolation mixes a sample behind Linear, so that it
// can have sample data to either side of the end points from which
// to extrapolate.  This SP represents that late sample position.
	s32 SPc;

// Previous sample values - used for interpolation
// Inverted order of these members to match the access order in the
//   code (might improve cache hits).
	s32 PV4;
	s32 PV3;
	s32 PV2;
	s32 PV1;

// Last outputted audio value, used for voice modulation.
	s32 OutX;

// SBuffer now points directly to an ADPCM cache entry.
	s16 *SBuffer;

// sample position within the current decoded packet.
	s32 SCurrent;

	void Start();
	void Stop();
};

// ** Begin Debug-only variables section **
// Separated from the V_Voice struct to improve cache performance of
// the Public Release build.
struct V_VoiceDebug
{
	s8 FirstBlock;
	s32 SampleData;
	s32 PeakX;
	s32 displayPeak;
	s32 lastSetStartA;
};

struct V_CoreDebug
{
	V_VoiceDebug Voices[24];
// Last Transfer Size
	u32 lastsize;
};

// Debug tracking information - 24 voices and 2 cores.
extern V_CoreDebug DebugCores[2];

struct V_Reverb
{
	s16 IN_COEF_L;
	s16 IN_COEF_R;

	u32 FB_SRC_A;
	u32 FB_SRC_B;

	s16 FB_ALPHA;
	s16 FB_X;
	
	u32 IIR_SRC_A0;
	u32 IIR_SRC_A1;
	u32 IIR_SRC_B1;
	u32 IIR_SRC_B0;
	u32 IIR_DEST_A0;
	u32 IIR_DEST_A1;
	u32 IIR_DEST_B0;
	u32 IIR_DEST_B1;

	s16 IIR_ALPHA;
	s16 IIR_COEF;

	u32 ACC_SRC_A0;
	u32 ACC_SRC_A1;
	u32 ACC_SRC_B0;
	u32 ACC_SRC_B1;
	u32 ACC_SRC_C0;
	u32 ACC_SRC_C1;
	u32 ACC_SRC_D0;
	u32 ACC_SRC_D1;

	s16 ACC_COEF_A;
	s16 ACC_COEF_B;
	s16 ACC_COEF_C;
	s16 ACC_COEF_D;

	u32 MIX_DEST_A0;
	u32 MIX_DEST_A1;
	u32 MIX_DEST_B0;
	u32 MIX_DEST_B1;
};

struct V_ReverbBuffers
{
	s32 FB_SRC_A0;
	s32 FB_SRC_B0;
	s32 FB_SRC_A1;
	s32 FB_SRC_B1;

	s32 IIR_SRC_A0;
	s32 IIR_SRC_A1;
	s32 IIR_SRC_B1;
	s32 IIR_SRC_B0;
	s32 IIR_DEST_A0;
	s32 IIR_DEST_A1;
	s32 IIR_DEST_B0;
	s32 IIR_DEST_B1;

	s32 ACC_SRC_A0;
	s32 ACC_SRC_A1;
	s32 ACC_SRC_B0;
	s32 ACC_SRC_B1;
	s32 ACC_SRC_C0;
	s32 ACC_SRC_C1;
	s32 ACC_SRC_D0;
	s32 ACC_SRC_D1;

	s32 MIX_DEST_A0;
	s32 MIX_DEST_A1;
	s32 MIX_DEST_B0;
	s32 MIX_DEST_B1;
	
	bool NeedsUpdated;
};

struct V_SPDIF
{
	u16 Out;
	u16 Info;
	u16 Unknown1;
	u16 Mode;
	u16 Media;
	u16 Unknown2;
	u16 Protection;
};

struct V_CoreRegs
{
	u32 PMON;
	u32 NON;
	u32 VMIXL;
	u32 VMIXR;
	u32 VMIXEL;
	u32 VMIXER;
	u16 MMIX;
	u32 ENDX;
	u16 STATX;
	u16 ATTR;
	u16 _1AC;
};

struct V_VoiceGates
{
	s16 DryL;	// 'AND Gate' for Direct Output to Left Channel
	s16 DryR;	// 'AND Gate' for Direct Output for Right Channel
	s16 WetL;	// 'AND Gate' for Effect Output for Left Channel
	s16 WetR;	// 'AND Gate' for Effect Output for Right Channel
};

union V_CoreGates
{
	struct
	{
		u64 lo;
		u64 hi;
	} v128;

	struct  
	{
		s16 InpL;	// Sound Data Input to Direct Output (Left)
		s16 InpR;	// Sound Data Input to Direct Output (Right)
		s16 SndL;	// Voice Data to Direct Output (Left)
		s16 SndR;	// Voice Data to Direct Output (Right)
		s16 ExtL;	// External Input to Direct Output (Left)
		s16 ExtR;	// External Input to Direct Output (Right)
	};
};

struct V_Core
{
	static const uint NumVoices = 24;

	// Voice Gates -- These are SSE-related values, and must always be
	// first to ensure 16 byte alignment

	V_VoiceGates VoiceGates[NumVoices];
	V_CoreGates DryGate;
	V_CoreGates WetGate;

	V_VolumeSlideLR MasterVol;// Master Volume
	V_VolumeLR ExtVol;		// Volume for External Data Input
	V_VolumeLR InpVol;		// Volume for Sound Data Input
	V_VolumeLR FxVol;		// Volume for Output from Effects 

	V_Voice Voices[NumVoices];
	
// Interrupt Address
	u32 IRQA;
// DMA Transfer Start Address
	u32 TSA;  
// DMA Transfer Data Address (Internal...)
	u32 TDA;  

// Interrupt Enable
	s8 IRQEnable;
// DMA related?
	s8 DMABits;
// Effect Enable
	s8 FxEnable;
// Noise Clock
	s8 NoiseClk;
// AutoDMA Status
	u16 AutoDMACtrl;
// Mute
	s8 Mute;

// Reverb
	V_Reverb Revb;
	V_ReverbBuffers RevBuffers;		// buffer pointers for reverb, pre-calculated and pre-clipped.
	u32 EffectsStartA;
	u32 EffectsEndA;
	u32 ReverbX;
	
	// Current size of the effects buffer.  Pre-caculated when the effects start
	// or end position registers are written.  CAN BE NEGATIVE OR ZERO, in which
	// case reverb should be disabled.
	s32 EffectsBufferSize;
	
// Registers
	V_CoreRegs Regs;

	// Last samples to pass through the effects processor.
	// Used because the effects processor works at 24khz and just pulls
	// from this for the odd Ts.
	StereoOut32 LastEffect;

	u8 InitDelay;

	u8 CoreEnabled;

	u8 AttrBit0;
	u8 AttrBit4;
	u8 AttrBit5;

	u32 TADR;

// Input Buffer
	u32 AdmaInProgress;
	u32 AdmaDataLeft;
	u32 AdmaReadPos;
	s32 AdmaWritePos;
	s32 AdmaFree;
	s16 AdmaTempBuffer[0x800];

	u32 ADMAPV;
	StereoOut32 ADMAP;

	void Reset();
	void UpdateEffectsBufferSize();

	V_Core();		// our badass constructor
	s32 EffectsBufferIndexer( s32 offset ) const;
	void UpdateFeedbackBuffersA();
	void UpdateFeedbackBuffersB();
};

extern V_Core Cores[2];
extern V_SPDIF Spdif;

// Output Buffer Writing Position (the same for all data);
extern s16 OutPos;
// Input Buffer Reading Position (the same for all data);
extern s16 InputPos;
// SPU Mixing Cycles ("Ticks mixed" counter)
extern u32 Cycles;

#ifdef __LINUX__

#include <sys/types.h>
#include <sys/timeb.h>

static __forceinline u32 timeGetTime()
{
	struct timeb t;
	ftime(&t);
	return (u32)(t.time*1000+t.millitm);
}
#endif

#endif // DEFS_H_INCLUDED //
