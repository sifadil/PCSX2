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


#include "Spu2.h"
#include "regtable.h"

void StartVoices(int core, u32 value);
void StopVoices(int core, u32 value);

void InitADSR();

DWORD CALLBACK TimeThread(PVOID /* unused param */);


// [Air]: fixed the hacky part of UpdateTimer with this:
bool resetClock = true;

// Used to make spu2 more robust at loading incompatible saves.
// Disables re-freezing of save state data.
bool disableFreezes = false;

void (* _irqcallback)();
void (* dma4callback)();
void (* dma7callback)();

short *spu2regs;
short *_spu2mem;
s32 uTicks;

u8 callirq;

HANDLE hThreadFunc;
u32	ThreadFuncID;

V_CoreDebug DebugCores[2];
V_Core Cores[2];
V_SPDIF Spdif;

s16 OutPos;
s16 InputPos;
u32 Cycles;

u32* cPtr=NULL;
u32  lClocks=0;

bool hasPtr=false;

int PlayMode;

s16 attrhack[2]={0,0};

HINSTANCE hInstance;

CRITICAL_SECTION threadSync;

bool has_to_call_irq=false;

void SetIrqCall()
{
	has_to_call_irq=true;
}

void SysMessage(const char *fmt, ...) 
{
	va_list list;
	char tmp[512];
	wchar_t wtmp[512];

	va_start(list,fmt);
	sprintf_s(tmp,fmt,list);
	va_end(list);
	swprintf_s(wtmp, _T("%S"), tmp);
	MessageBox(0, wtmp, _T("SPU2-X System Message"), 0);
}

__forceinline s16 * __fastcall GetMemPtr(u32 addr)
{
#ifndef _DEBUG_FAST
	// In case you're wondering, this assert is the reason SPU2-X
	// runs so incrediously slow in Debug mode. :P
	jASSUME( addr < 0x100000 );
#endif
	return (_spu2mem+addr);
}

__forceinline s16 __fastcall spu2M_Read( u32 addr )
{
	return *GetMemPtr( addr & 0xfffff );
}

// writes a signed value to the SPU2 ram
// Invalidates the ADPCM cache in the process.
// Optimization note: don't use __forceinline because the footprint of this
// function is a little too heavy now.  Better to let the compiler decide.
__inline void __fastcall spu2M_Write( u32 addr, s16 value )
{
	// Make sure the cache is invalidated:
	// (note to self : addr address WORDs, not bytes)

	addr &= 0xfffff;
	if( addr >= SPU2_DYN_MEMLINE )
	{
		const int cacheIdx = addr / pcm_WordsPerBlock;
		pcm_cache_data[cacheIdx].Validated = false;

		ConLog( " * SPU2 : PcmCache Block Clear at 0x%x (cacheIdx=0x%x)\n", addr, cacheIdx);
	}
	*GetMemPtr( addr ) = value;
}

// writes an unsigned value to the SPU2 ram
__inline void __fastcall spu2M_Write( u32 addr, u16 value )
{
	spu2M_Write( addr, (s16)value );
}

void V_Core::Reset()
{
	memset( this, 0, sizeof(V_Core) );
 
	const int c = (this == Cores) ? 0 : 1;
 
	Regs.STATX=0;
	Regs.ATTR=0;
	ExtL = 0x7FFFFFFF;
	ExtR = 0x7FFFFFFF;
	InpL = 0x7FFFFFFF;
	InpR = 0x7FFFFFFF;
	FxL  = 0x7FFFFFFF;
	FxR  = 0x7FFFFFFF;
	MasterL.Reg_VOL= 0x3FFF;
	MasterR.Reg_VOL= 0x3FFF;
	MasterL.Value  = 0x7FFFFFFF;
	MasterR.Value  = 0x7FFFFFFF;
	ExtWetR = -1;
	ExtWetL = -1;
	ExtDryR = -1;
	ExtDryL = -1;
	InpWetR = -1;
	InpWetL = -1;
	InpDryR = -1;
	InpDryL = -1;
	SndWetR = -1;
	SndWetL = -1;
	SndDryR = -1;
	SndDryL = -1;
	Regs.MMIX = 0xFFCF;
	Regs.VMIXL = 0xFFFFFF;
	Regs.VMIXR = 0xFFFFFF;
	Regs.VMIXEL = 0xFFFFFF;
	Regs.VMIXER = 0xFFFFFF;
	EffectsStartA= 0xEFFF8 + 0x10000*c;
	EffectsEndA  = 0xEFFFF + 0x10000*c;
	FxEnable=0;
	IRQA=0xFFFF0;
	IRQEnable=1;
 
	for( uint v=0; v<24; ++v )
	{
		Voices[v].VolumeL.Reg_VOL = 0x3FFF;
		Voices[v].VolumeR.Reg_VOL = 0x3FFF;

		Voices[v].VolumeL.Value = 0x7FFFFFFF;
		Voices[v].VolumeR.Value = 0x7FFFFFFF;
		
		Voices[v].ADSR.Value=0;
		Voices[v].ADSR.Phase=0;
		Voices[v].Pitch=0x3FFF;
		Voices[v].DryL = -1;
		Voices[v].DryR = -1;
		Voices[v].WetL = -1;
		Voices[v].WetR = -1;
		Voices[v].NextA=2800;
		Voices[v].StartA=2800;
		Voices[v].LoopStartA=2800;
	}
	DMAICounter=0;
	AdmaInProgress=0;
 
	Regs.STATX=0x80;
 }

void V_Core::UpdateEffectsBufferSize()
{
	EffectsBufferSize = EffectsEndA - EffectsStartA + 1;
}

void V_Voice::Start()
{
	if((Cycles-PlayCycle)>=4)
	{
		if(StartA&7)
		{
			fprintf( stderr, " *** Misaligned StartA %05x!\n",StartA);
			StartA=(StartA+0xFFFF8)+0x8;
		}

		ADSR.Releasing	= false;
		ADSR.Value		= 1;
		ADSR.Phase		= 1;
		PlayCycle		= Cycles;
		SCurrent		= 28;
		LoopMode		= 0;
		LoopFlags		= 0;
		LoopStartA		= StartA;
		NextA			= StartA;
		Prev1			= 0;
		Prev2			= 0;

		PV1 = PV2		= 0;
		PV3 = PV4		= 0;
	}
	else
	{
		printf(" *** KeyOn after less than 4 T disregarded.\n");
	}
}

void V_Voice::Stop()
{
	ADSR.Value = 0;
	ADSR.Phase = 0;
}

static const int TickInterval = 768;
static const int SanityInterval = 4800;

u32 TicksCore = 0;
u32 TicksThread = 0;

void __fastcall TimeUpdate(u32 cClocks)
{
	u32 dClocks = cClocks-lClocks;

	// [Air]: Sanity Check
	//  If for some reason our clock value seems way off base, just mix
	//  out a little bit, skip the rest, and hope the ship "rights" itself later on.

	if( dClocks > TickInterval*SanityInterval )
	{
		ConLog( " * SPU2 > TimeUpdate Sanity Check (Tick Delta: %d) (PS2 Ticks: %d)\n", dClocks/TickInterval, cClocks/TickInterval );
		dClocks = TickInterval*SanityInterval;
		lClocks = cClocks-dClocks;
	}

	//UpdateDebugDialog();

	//Update Mixing Progress
	while(dClocks>=TickInterval)
	{
		if(has_to_call_irq)
		{
			ConLog(" * SPU2: Irq Called (%04x).\n",Spdif.Info);
			has_to_call_irq=false;
			if(_irqcallback) _irqcallback();
		}

		if(Cores[0].InitDelay>0)
		{
			Cores[0].InitDelay--;
			if(Cores[0].InitDelay==0)
			{
				Cores[0].Reset();
			}
		}

		if(Cores[1].InitDelay>0)
		{
			Cores[1].InitDelay--;
			if(Cores[1].InitDelay==0)
			{
				Cores[1].Reset();
			}
		}

		//Update DMA4 interrupt delay counter
		if(Cores[0].DMAICounter>0) 
		{
			Cores[0].DMAICounter-=TickInterval;
			if(Cores[0].DMAICounter<=0)
			{
				Cores[0].MADR=Cores[0].TADR;
				Cores[0].DMAICounter=0;
				if(dma4callback) dma4callback();
			}
			else {
				Cores[0].MADR+=TickInterval<<1;
			}
		}

		//Update DMA7 interrupt delay counter
		if(Cores[1].DMAICounter>0) 
		{
			Cores[1].DMAICounter-=TickInterval;
			if(Cores[1].DMAICounter<=0)
			{
				Cores[1].MADR=Cores[1].TADR;
				Cores[1].DMAICounter=0;
				//ConLog( "* SPU2 > DMA 7 Callback!  %d\n", Cycles );
				if(dma7callback) dma7callback();
			}
			else {
				Cores[1].MADR+=TickInterval<<1;
			}
		}

		dClocks-=TickInterval;
		lClocks+=TickInterval;
		Cycles++;

		Mix();
	}
}

static u16 mask = 0xFFFF;

void UpdateSpdifMode()
{
	int OPM=PlayMode;
	u16 last = 0;

	if(mask&Spdif.Out)
	{
		last = mask & Spdif.Out;
		mask=mask&(~Spdif.Out);
	}

	if(Spdif.Out&0x4) // use 24/32bit PCM data streaming
	{
		PlayMode=8;
		ConLog(" * SPU2: WARNING: Possibly CDDA mode set!\n");
		return;
	}

	if(Spdif.Out&SPDIF_OUT_BYPASS)
	{
		PlayMode=2;
		if(Spdif.Mode&SPDIF_MODE_BYPASS_BITSTREAM)
			PlayMode=4; //bitstream bypass
	}
	else
	{
		PlayMode=0; //normal processing
		if(Spdif.Out&SPDIF_OUT_PCM)
		{
			PlayMode=1;
		}
	}
	if(OPM!=PlayMode)
	{
		ConLog(" * SPU2: Play Mode Set to %s (%d).\n",
			(PlayMode==0) ? "Normal" : ((PlayMode==1) ? "PCM Clone" : ((PlayMode==2) ? "PCM Bypass" : "BitStream Bypass")),PlayMode);
	}
}

// Converts an SPU2 register volume write into a 32 bit SPU2-X volume.  The value is extended
// properly into the lower 16 bits of the value to provide a full spectrum of volumes.
static s32 GetVol32( u16 src )
{
	return (((s32)src) << 16 ) | ((src<<1) & 0xffff);
}

void SPU_ps1_write(u32 mem, u16 value) 
{
	bool show=true;

	u32 reg = mem&0xffff;

	if((reg>=0x1c00)&&(reg<0x1d80))
	{
		//voice values
		u8 voice = ((reg-0x1c00)>>4);
		u8 vval = reg&0xf;
		switch(vval)
		{
			case 0: //VOLL (Volume L)
				Cores[0].Voices[voice].VolumeL.Mode = 0;
				Cores[0].Voices[voice].VolumeL.Value = GetVol32( value<<1 );
				Cores[0].Voices[voice].VolumeL.Reg_VOL = value;
			break;

			case 1: //VOLR (Volume R)
				Cores[0].Voices[voice].VolumeR.Mode = 0;
				Cores[0].Voices[voice].VolumeR.Value = GetVol32( value<<1 );
				Cores[0].Voices[voice].VolumeR.Reg_VOL = value;
			break;
			
			case 2:	Cores[0].Voices[voice].Pitch = value; break;
			case 3:	Cores[0].Voices[voice].StartA = (u32)value<<8; break;

			case 4: // ADSR1 (Envelope)
				Cores[0].Voices[voice].ADSR.AttackMode = (value & 0x8000)>>15;
				Cores[0].Voices[voice].ADSR.AttackRate = (value & 0x7F00)>>8;
				Cores[0].Voices[voice].ADSR.DecayRate = (value & 0xF0)>>4;
				Cores[0].Voices[voice].ADSR.SustainLevel = (value & 0xF);
				Cores[0].Voices[voice].ADSR.Reg_ADSR1 = value;
			break;

			case 5: // ADSR2 (Envelope)
				Cores[0].Voices[voice].ADSR.SustainMode = (value & 0xE000)>>13;
				Cores[0].Voices[voice].ADSR.SustainRate = (value & 0x1FC0)>>6;
				Cores[0].Voices[voice].ADSR.ReleaseMode = (value & 0x20)>>5;
				Cores[0].Voices[voice].ADSR.ReleaseRate = (value & 0x1F);
				Cores[0].Voices[voice].ADSR.Reg_ADSR2 = value;
			break;
			
			case 6:
				Cores[0].Voices[voice].ADSR.Value = ((s32)value<<16) | value;
				ConLog( "* SPU2: Mysterious ADSR Volume Set to 0x%x", value );
			break;

			case 7:	Cores[0].Voices[voice].LoopStartA = (u32)value <<8;	break;

			jNO_DEFAULT;
		}
	}

	else switch(reg)
	{
		case 0x1d80://         Mainvolume left
			Cores[0].MasterL.Mode = 0;
			Cores[0].MasterL.Value = GetVol32( value );
			break;
		case 0x1d82://         Mainvolume right
			Cores[0].MasterL.Mode = 0;
			Cores[0].MasterR.Value = GetVol32( value );
			break;
		case 0x1d84://         Reverberation depth left
			Cores[0].FxL = GetVol32( value );
			break;
		case 0x1d86://         Reverberation depth right
			Cores[0].FxR = GetVol32( value );
			break;

		case 0x1d88://         Voice ON  (0-15)
			SPU2_FastWrite(REG_S_KON,value);
			break;
		case 0x1d8a://         Voice ON  (16-23)
			SPU2_FastWrite(REG_S_KON+2,value);
			break;

		case 0x1d8c://         Voice OFF (0-15)
			SPU2_FastWrite(REG_S_KOFF,value);
			break;
		case 0x1d8e://         Voice OFF (16-23)
			SPU2_FastWrite(REG_S_KOFF+2,value);
			break;

		case 0x1d90://         Channel FM (pitch lfo) mode (0-15)
			SPU2_FastWrite(REG_S_PMON,value);
			break;
		case 0x1d92://         Channel FM (pitch lfo) mode (16-23)
			SPU2_FastWrite(REG_S_PMON+2,value);
			break;


		case 0x1d94://         Channel Noise mode (0-15)
			SPU2_FastWrite(REG_S_NON,value);
			break;
		case 0x1d96://         Channel Noise mode (16-23)
			SPU2_FastWrite(REG_S_NON+2,value);
			break;

		case 0x1d98://         Channel Reverb mode (0-15)
			SPU2_FastWrite(REG_S_VMIXEL,value);
			SPU2_FastWrite(REG_S_VMIXER,value);
			break;
		case 0x1d9a://         Channel Reverb mode (16-23)
			SPU2_FastWrite(REG_S_VMIXEL+2,value);
			SPU2_FastWrite(REG_S_VMIXER+2,value);
			break;
		case 0x1d9c://         Channel Reverb mode (0-15)
			SPU2_FastWrite(REG_S_VMIXL,value);
			SPU2_FastWrite(REG_S_VMIXR,value);
			break;
		case 0x1d9e://         Channel Reverb mode (16-23)
			SPU2_FastWrite(REG_S_VMIXL+2,value);
			SPU2_FastWrite(REG_S_VMIXR+2,value);
			break;

		case 0x1da2://         Reverb work area start
			{
				u32 val=(u32)value <<8;

				SPU2_FastWrite(REG_A_ESA,  val&0xFFFF);
				SPU2_FastWrite(REG_A_ESA+2,val>>16);
			}
			break;
		case 0x1da4:
			Cores[0].IRQA=(u32)value<<8;
			break;
		case 0x1da6:
			Cores[0].TSA=(u32)value<<8;
			break;

		case 0x1daa:
			SPU2_FastWrite(REG_C_ATTR,value);
			break;
		case 0x1dae:
			SPU2_FastWrite(REG_P_STATX,value);
			break;
		case 0x1da8:// Spu Write to Memory
			DmaWrite(0,value);
			show=false;
			break;
	}

	if(show) FileLog("[%10d] (!) SPU write mem %08x value %04x\n",Cycles,mem,value);

	spu2Ru16(mem)=value;
}

u16 SPU_ps1_read(u32 mem) 
{
	bool show=true;
	u16 value = spu2Ru16(mem);

	u32 reg = mem&0xffff;

	if((reg>=0x1c00)&&(reg<0x1d80))
	{
		//voice values
		u8 voice = ((reg-0x1c00)>>4);
		u8 vval = reg&0xf;
		switch(vval)
		{
			case 0: //VOLL (Volume L)
				//value=Cores[0].Voices[voice].VolumeL.Mode;
				//value=Cores[0].Voices[voice].VolumeL.Value;
				value=Cores[0].Voices[voice].VolumeL.Reg_VOL;	break;
			case 1: //VOLR (Volume R)
				//value=Cores[0].Voices[voice].VolumeR.Mode;
				//value=Cores[0].Voices[voice].VolumeR.Value;
				value=Cores[0].Voices[voice].VolumeR.Reg_VOL;	break;
			case 2:	value=Cores[0].Voices[voice].Pitch;			break;
			case 3:	value=Cores[0].Voices[voice].StartA;	break;
			case 4: value=Cores[0].Voices[voice].ADSR.Reg_ADSR1;	break;
			case 5: value=Cores[0].Voices[voice].ADSR.Reg_ADSR2;	break;
			case 6:	value=Cores[0].Voices[voice].ADSR.Value >> 16;	break;
			case 7:	value=Cores[0].Voices[voice].LoopStartA;	break;

			jNO_DEFAULT;
		}
	}
	else switch(reg)
	{
		case 0x1d80: value = Cores[0].MasterL.Value>>16; break;
		case 0x1d82: value = Cores[0].MasterR.Value>>16; break;
		case 0x1d84: value = Cores[0].FxL>>16;           break;
		case 0x1d86: value = Cores[0].FxR>>16;           break;

		case 0x1d88: value = 0; break;
		case 0x1d8a: value = 0; break;
		case 0x1d8c: value = 0; break;
		case 0x1d8e: value = 0; break;

		case 0x1d90: value = Cores[0].Regs.PMON&0xFFFF;   break;
		case 0x1d92: value = Cores[0].Regs.PMON>>16;      break;

		case 0x1d94: value = Cores[0].Regs.NON&0xFFFF;    break;
		case 0x1d96: value = Cores[0].Regs.NON>>16;       break;

		case 0x1d98: value = Cores[0].Regs.VMIXEL&0xFFFF; break;
		case 0x1d9a: value = Cores[0].Regs.VMIXEL>>16;    break;
		case 0x1d9c: value = Cores[0].Regs.VMIXL&0xFFFF;  break;
		case 0x1d9e: value = Cores[0].Regs.VMIXL>>16;     break;

		case 0x1da2:
			value = Cores[0].EffectsStartA>>3;
			Cores[0].UpdateEffectsBufferSize();
		break;
		case 0x1da4: value = Cores[0].IRQA>>3;            break;
		case 0x1da6: value = Cores[0].TSA>>3;             break;

		case 0x1daa:
			value = SPU2read(REG_C_ATTR);
			break;
		case 0x1dae:
			value = 0; //SPU2read(REG_P_STATX)<<3;
			break;
		case 0x1da8:
			value = DmaRead(0);
			show=false;
			break;
	}

	if(show) FileLog("[%10d] (!) SPU read mem %08x value %04x\n",Cycles,mem,value);
	return value;
}

static u32 SetLoWord( u32 var, u16 writeval )
{
	return (var & 0xFFFF0000) | writeval;
}


static u32 SetHiWord( u32 var, u16 writeval )
{
	return (var & 0x0000FFFF) | (writeval<<16);
}

__forceinline void SPU2_FastWrite( u32 rmem, u16 value )
{
	u32 vx=0, vc=0, core=0, omem, mem;
	omem=mem=rmem & 0x7FF; //FFFF;
	if (mem & 0x400) { omem^=0x400; core=1; }

	SPU2writeLog(mem,value);

	if (omem < 0x0180)	// Voice Params
	{ 
		const u32 voice = (omem & 0x1F0) >> 4;
		const u32 param = (omem & 0xF) >> 1;
		V_Voice& thisvoice = Cores[core].Voices[voice];

		switch (param) 
		{ 
			case 0: //VOLL (Volume L)
			case 1: //VOLR (Volume R)
			{
				V_Volume& thisvol = (param==0) ? thisvoice.VolumeL : thisvoice.VolumeR;
				if (value & 0x8000)		// +Lin/-Lin/+Exp/-Exp
				{
					thisvol.Mode = (value & 0xF000)>>12;
					thisvol.Increment = (value & 0x3F);
				}
				else
				{
					// Constant Volume mode (no slides or envelopes)
					// Volumes range from 0x3fff to 0x7fff, with 0x4000 serving as
					// the "sign" bit, so a simple bitwise extension will do the trick:

					thisvol.Value = GetVol32( value<<1 );
					thisvol.Mode = 0;
					thisvol.Increment = 0;
				}
				thisvol.Reg_VOL = value;
			}
			break;

			case 2:	thisvoice.Pitch=value;			break;
			case 3: // ADSR1 (Envelope)
				thisvoice.ADSR.AttackMode = (value & 0x8000)>>15;
				thisvoice.ADSR.AttackRate = (value & 0x7F00)>>8;
				thisvoice.ADSR.DecayRate = (value & 0xF0)>>4;
				thisvoice.ADSR.SustainLevel = (value & 0xF);
				thisvoice.ADSR.Reg_ADSR1 = value;	break;
			case 4: // ADSR2 (Envelope)
				thisvoice.ADSR.SustainMode = (value & 0xE000)>>13;
				thisvoice.ADSR.SustainRate = (value & 0x1FC0)>>6;
				thisvoice.ADSR.ReleaseMode = (value & 0x20)>>5;
				thisvoice.ADSR.ReleaseRate = (value & 0x1F);
				thisvoice.ADSR.Reg_ADSR2 = value;	break;
			case 5:
				// [Air] : Mysterious ADSR set code.  Too bad none of my games ever use it.
				//      (as usual... )
				thisvoice.ADSR.Value = (value << 16) | value;
				ConLog( "* SPU2: Mysterious ADSR Volume Set to 0x%x", value );
			break;
			
			case 6:	thisvoice.VolumeL.Value = GetVol32( value ); break;
			case 7:	thisvoice.VolumeR.Value = GetVol32( value ); break;

			jNO_DEFAULT;
		}
	}
	else if ((omem >= 0x01C0) && (omem < 0x02DE))
	{
		const u32 voice   = ((omem-0x01C0) / 12);
		const u32 address = ((omem-0x01C0) % 12) >> 1;
		V_Voice& thisvoice = Cores[core].Voices[voice];

		switch (address)
		{
			case 0:
				thisvoice.StartA = ((value & 0x0F) << 16) | (thisvoice.StartA & 0xFFF8); 
				if( IsDevBuild )
					DebugCores[core].Voices[voice].lastSetStartA = thisvoice.StartA; 
			break;
			
			case 1:
				thisvoice.StartA = (thisvoice.StartA & 0x0F0000) | (value & 0xFFF8); 
				if( IsDevBuild )
					DebugCores[core].Voices[voice].lastSetStartA = thisvoice.StartA; 
			break;
			
			case 2:	
				thisvoice.LoopStartA = ((value & 0x0F) << 16) | (thisvoice.LoopStartA & 0xFFF8);
				thisvoice.LoopMode = 3;
			break;
			
			case 3:
				thisvoice.LoopStartA = (thisvoice.LoopStartA & 0x0F0000) | (value & 0xFFF8);
				thisvoice.LoopMode = 3;
			break;

			case 4:
				thisvoice.NextA = ((value & 0x0F) << 16) | (thisvoice.NextA & 0xFFF8);
			break;
			
			case 5:
				thisvoice.NextA = (thisvoice.NextA & 0x0F0000) | (value & 0xFFF8);
			break;
		}
	}
	else if((mem>=0x07C0) && (mem<0x07CE)) 
	{
		*(regtable[mem>>1]) = value;
		UpdateSpdifMode();
	}
	else
	{
		switch(omem)
		{
			case REG_C_ATTR:
			{
				int irqe = Cores[core].IRQEnable;
				int bit0 = Cores[core].AttrBit0;
				int bit4 = Cores[core].AttrBit4;

				if( ((value>>15)&1) && (!Cores[core].CoreEnabled) && (Cores[core].InitDelay==0) ) // on init/reset
				{
					if(hasPtr)
					{
						Cores[core].InitDelay=1;
						Cores[core].Regs.STATX=0;	
					}
					else
					{
						Cores[core].Reset();
					}
				}

				Cores[core].AttrBit0   =(value>> 0) & 0x01; //1 bit
				Cores[core].DMABits	   =(value>> 1) & 0x07; //3 bits
				Cores[core].AttrBit4   =(value>> 4) & 0x01; //1 bit
				Cores[core].AttrBit5   =(value>> 5) & 0x01; //1 bit
				Cores[core].IRQEnable  =(value>> 6) & 0x01; //1 bit
				Cores[core].FxEnable   =(value>> 7) & 0x01; //1 bit
				Cores[core].NoiseClk   =(value>> 8) & 0x3f; //6 bits
				//Cores[core].Mute	   =(value>>14) & 0x01; //1 bit
				Cores[core].Mute=0;
				Cores[core].CoreEnabled=(value>>15) & 0x01; //1 bit
				Cores[core].Regs.ATTR  =value&0x7fff;

				if(value&0x000E)
				{
					ConLog(" * SPU2: Core %d ATTR unknown bits SET! value=%04x\n",core,value);
				}

				if(Cores[core].AttrBit0!=bit0)
				{
					ConLog(" * SPU2: ATTR bit 0 set to %d\n",Cores[core].AttrBit0);
				}
				if(Cores[core].IRQEnable!=irqe)
				{
					ConLog(" * SPU2: IRQ %s\n",((Cores[core].IRQEnable==0)?"disabled":"enabled"));
					if(!Cores[core].IRQEnable)
						Spdif.Info=0;
				}

			}
			break;

			case REG_S_PMON:
				vx=2; for (vc=1;vc<16;vc++) { Cores[core].Voices[vc].Modulated=(s8)((value & vx)/vx); vx<<=1; }
				Cores[core].Regs.PMON = SetLoWord( Cores[core].Regs.PMON, value );
			break;

			case (REG_S_PMON + 2):
				vx=1; for (vc=16;vc<24;vc++) { Cores[core].Voices[vc].Modulated=(s8)((value & vx)/vx); vx<<=1; }
				Cores[core].Regs.PMON = SetHiWord( Cores[core].Regs.PMON, value );
			break;

			case REG_S_NON:
				vx=1; for (vc=0;vc<16;vc++) { Cores[core].Voices[vc].Noise=(s8)((value & vx)/vx); vx<<=1; }
				Cores[core].Regs.NON = SetLoWord( Cores[core].Regs.NON, value );
			break;

			case (REG_S_NON + 2):
				vx=1; for (vc=16;vc<24;vc++) { Cores[core].Voices[vc].Noise=(s8)((value & vx)/vx); vx<<=1; }
				Cores[core].Regs.NON = SetHiWord( Cores[core].Regs.NON, value );
			break;

// Games like to repeatedly write these regs over and over with the same value, hence
// the shortcut that skips the bitloop if the values are equal.
#define vx_SetSomeBits( reg_out, mask_out, hiword ) \
{ \
	const uint start_bit	= hiword ? 16 : 0; \
	const uint end_bit		= hiword ? 24 : 16; \
	const u32 result		= hiword ? SetHiWord( Cores[core].Regs.reg_out, value ) : SetLoWord( Cores[core].Regs.reg_out, value ); \
	if( result == Cores[core].Regs.reg_out ) return; \
 \
	Cores[core].Regs.reg_out = result; \
	for (uint vc=start_bit, vx=1; vc<end_bit; vc++, vx<<=1) \
		Cores[core].Voices[vc].mask_out = (value & vx) ? -1 : 0; \
}

			case REG_S_VMIXL:
				vx_SetSomeBits( VMIXL, DryL, false );
			break;

			case (REG_S_VMIXL + 2):
				vx_SetSomeBits( VMIXL, DryL, true );
			break;

			case REG_S_VMIXEL:
				vx_SetSomeBits( VMIXEL, WetL, false );
			break;

			case (REG_S_VMIXEL + 2):
				vx_SetSomeBits( VMIXEL, WetL, true );
			break;

			case REG_S_VMIXR:
				vx_SetSomeBits( VMIXR, DryR, false );
			break;

			case (REG_S_VMIXR + 2):
				vx_SetSomeBits( VMIXR, DryR, true );
			break;

			case REG_S_VMIXER:
				vx_SetSomeBits( VMIXER, WetR, false );
			break;

			case (REG_S_VMIXER + 2):
				vx_SetSomeBits( VMIXER, WetR, true );
			break;

			case REG_P_MMIX:
	
				// Each MMIX gate is assigned either 0 or 0xffffffff depending on the status
				// of the MMIX bits.  I use -1 below as a shorthand for 0xffffffff. :)
			
				vx = value;
				if (core == 0) vx&=0xFF0;
				Cores[core].ExtWetR = (vx & 0x001) ? -1 : 0;
				Cores[core].ExtWetL = (vx & 0x002) ? -1 : 0;
				Cores[core].ExtDryR = (vx & 0x004) ? -1 : 0;
				Cores[core].ExtDryL = (vx & 0x008) ? -1 : 0;
				Cores[core].InpWetR = (vx & 0x010) ? -1 : 0;
				Cores[core].InpWetL = (vx & 0x020) ? -1 : 0;
				Cores[core].InpDryR = (vx & 0x040) ? -1 : 0;
				Cores[core].InpDryL = (vx & 0x080) ? -1 : 0;
				Cores[core].SndWetR = (vx & 0x100) ? -1 : 0;
				Cores[core].SndWetL = (vx & 0x200) ? -1 : 0;
				Cores[core].SndDryR = (vx & 0x400) ? -1 : 0;
				Cores[core].SndDryL = (vx & 0x800) ? -1 : 0;
				Cores[core].Regs.MMIX = value;
			break;

			case (REG_S_KON + 2):
				StartVoices(core,((u32)value)<<16);
			break;

			case REG_S_KON:
				StartVoices(core,((u32)value));
			break;

			case (REG_S_KOFF + 2):
				StopVoices(core,((u32)value)<<16);
			break;

			case REG_S_KOFF:
				StopVoices(core,((u32)value));
			break;

			case REG_S_ENDX:
				Cores[core].Regs.ENDX&=0x00FF0000;
			break;

			case (REG_S_ENDX + 2):	
				Cores[core].Regs.ENDX&=0xFFFF;
			break;

			// Reverb Start and End Address Writes!
			//  * Yes, these are backwards from all the volumes -- the hiword comes FIRST (wtf!)
			//  * End position is a hiword only!  Lowword is always ffff.
			//  * The Reverb buffer position resets on writes to StartA.  It probably resets
			//    on writes to End too.  Docs don't say, but they're for PSX, which couldn't
			//    change the end address anyway.

			case REG_A_ESA:
				Cores[core].EffectsStartA = (Cores[core].EffectsStartA & 0x0000FFFF) | (value<<16);
				Cores[core].ReverbX = 0;
				Cores[core].UpdateEffectsBufferSize();
			break;

			case (REG_A_ESA + 2):
				Cores[core].EffectsStartA = (Cores[core].EffectsStartA & 0xFFFF0000) | value;
				Cores[core].ReverbX = 0;
				Cores[core].UpdateEffectsBufferSize();
			break;

			case REG_A_EEA:
				Cores[core].EffectsEndA = ((u32)value<<16) | 0xFFFF;
				Cores[core].ReverbX = 0;
				Cores[core].UpdateEffectsBufferSize();
			break;
			
			// Master Volume Address Write!
			
			case REG_P_MVOLL:
			case REG_P_MVOLR:
			{
				V_Volume& thisvol = (omem==REG_P_MVOLL) ? Cores[core].MasterL : Cores[core].MasterR;

				if( value & 0x8000 )	// +Lin/-Lin/+Exp/-Exp
				{ 
					thisvol.Mode = (value & 0xE000) / 0x2000;
					thisvol.Increment = (value & 0x7F); // | ((value & 0x800)/0x10);
				}
				else
				{
					// Constant Volume mode (no slides or envelopes)
					// Volumes range from 0x3fff to 0x7fff, with 0x4000 serving as
					// the "sign" bit, so a simple bitwise extension will do the trick:

					thisvol.Value = GetVol32( value<<1 );
					thisvol.Mode = 0;
					thisvol.Increment = 0;
				}
				thisvol.Reg_VOL = value;
			}
			break;

			case REG_P_EVOLL:
				Cores[core].FxL = GetVol32( value );
			break;

			case REG_P_EVOLR:
				Cores[core].FxR = GetVol32( value );
			break;
			
			case REG_P_AVOLL:
				Cores[core].ExtL = GetVol32( value );
			break;

			case REG_P_AVOLR:
				Cores[core].ExtR = GetVol32( value );
			break;
			
			case REG_P_BVOLL:
				Cores[core].InpL = GetVol32( value );
			break;

			case REG_P_BVOLR:
				Cores[core].InpR = GetVol32( value );
			break;

			case REG_S_ADMAS:
				//ConLog(" * SPU2: Core %d AutoDMAControl set to %d (%d)\n",core,value, Cycles);
				Cores[core].AutoDMACtrl=value;

				if(value==0)
				{
					Cores[core].AdmaInProgress=0;
				}
			break;

			default:
				*(regtable[mem>>1]) = value;
			break;
		}
	}
}


void StartVoices(int core, u32 value)
{
	// Optimization: Games like to write zero to the KeyOn reg a lot, so shortcut
	// this loop if value is zero.

	if( value == 0 ) return;

	Cores[core].Regs.ENDX &= ~value;
	
	for( u8 vc=0; vc<24; vc++ )
	{
		if ((value>>vc) & 1)
		{
			Cores[core].Voices[vc].Start();
			Cores[core].Regs.ENDX &= ~( 1 << vc );

			if( IsDevBuild )
			{
				V_Voice& thisvc( Cores[core].Voices[vc] );

				if(MsgKeyOnOff()) ConLog(" * SPU2: KeyOn: C%dV%02d: SSA: %8x; M: %s%s%s%s; H: %02x%02x; P: %04x V: %04x/%04x; ADSR: %04x%04x\n",
					core,vc,thisvc.StartA,
					(thisvc.DryL)?"+":"-",(thisvc.DryR)?"+":"-",
					(thisvc.WetL)?"+":"-",(thisvc.WetR)?"+":"-",
					*(u8*)GetMemPtr(thisvc.StartA),*(u8 *)GetMemPtr((thisvc.StartA)+1),
					thisvc.Pitch,
					thisvc.VolumeL.Value,thisvc.VolumeR.Value,
					thisvc.ADSR.Reg_ADSR1,thisvc.ADSR.Reg_ADSR2);
			}
		}
	}
}

void StopVoices(int core, u32 value)
{
	if( value == 0 ) return;
	for( u8 vc=0; vc<24; vc++ )
	{
		if ((value>>vc) & 1)
		{
			Cores[core].Voices[vc].ADSR.Releasing = true;
			//if(MsgKeyOnOff()) ConLog(" * SPU2: KeyOff: Core %d; Voice %d.\n",core,vc);
		}
	}
}

