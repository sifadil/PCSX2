/* SPU2-X, A plugin for Emulating the Sound Processing Unit of the Playstation 2
 * Developed and maintained by the Pcsx2 Development Team.
 *
 * Original portions from SPU2ghz are (c) 2008 by David Quintana [gigaherz]
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License along
 * with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include "Global.h"

const float Scale16 = (float)(1LL << 16); // tweak this value to change the overall output volume
const float Scale32 = (float)(1LL << 32); // tweak this value to change the overall output volume

const float GainL  = 0.90f;
const float GainR  = 0.90f;

const float GainC  = 0.80f;

const float GainSL = 0.90f;
const float GainSR = 0.90f;

const float GainLFE= 0.95f;

const float AddCLR = 0.20f;	// Stereo expansion

const int RearDelay = 120; // in Samples
float RearL[RearDelay];
float RearR[RearDelay];
int RP;

static float AccL=0;
static float AccR=0;
static float AccB=0;

extern void ResetDplIIDecoder()
{
	AccL=0;
	AccR=0;
	AccB=0;
	memset(RearL, 0, sizeof(RearL));
	memset(RearR, 0, sizeof(RearR));
	RP = 0;
}

static float inline Lerp(float a, float b, float p)
{
	return a + (b-a) * p;
}

static void Decode(float IL, float IR, float& L, float& R, float& C, float& SUB, float& SL, float& SR)
{
	// Rate at which the new power values affect the previous
	const float AlphaDown = 0.2f;
	const float AlphaUp = 0.5f;
	
	// 1.73+1.22 = 2.94; 2.94 * 0.34 = 0.9996; Close enough.
	// The range for VL/VR is approximately 0..1,
	// But in the cases where VL/VR are > 0.5, Rearness is 0 so it should never overflow.
	const float RearScale = 0.34f;
	const float FrontScale = 0.2f;
	const float SideScale = 0.6f;

	float IC = (IL+IR) * 0.5f;

	// Peak L/R
	float PL = abs(IL);
	float PR = abs(IR);
	float PB = abs(IC);

	float a = (PL > AccL || PR > AccR || PB > AccB) ? AlphaUp : AlphaDown;

	AccL = Lerp(AccL, PL, a); // Crest of the Left channel
	AccR = Lerp(AccR, PR, a); // Crest of the Right channel
	AccB = Lerp(AccR, PB, a); // Crest of the Bias (center) channel
	
	float Volume = std::max(AccL, AccR);
	
	float CoefL = 1;
	float CoefR = 1;
	float CoefC = 0;
	float CoefSL = 0;
	float CoefSR = 0;
	float Balance = 0;
	if(Volume > 0)
	{
		Balance = (AccR-AccL); // -1 .. 1

		float Sideness = SideScale * abs(Balance) / Volume;
		float Centerness = 1 - Sideness; 
		
		float Frontness = FrontScale * AccB / Volume;
		float Rearness = 1 - Frontness;
				
		// Center: PB > 0, PL = PR
		// Left:   PB > 0, PL > 0, PR = 0
		// Right:  PB > 0, PL = 0, PR > 0
		// SC:     PB = 0, PL > 0, PR > 0
		// SL:     PB > 0, PL > PR
		// SR:     PB > 0, PL < PR

		CoefC = Centerness;
		CoefL = Frontness;
		CoefR = Frontness;

		CoefSL = Rearness * RearScale;
		CoefSR = Rearness * RearScale;
	}
		
	// Calculate center channel and LFE
	C = IC * CoefC;
	SUB = (IL-IR) * 0.5f; // no need to lowpass, the speaker amplifier should take care of it

	L = IL - C; // Effective L/R data
	R = IR - C;

	// Equalize the power levels for L/R
	float B = std::min(0.9f,std::max(-0.9f,Balance));

	float VL = L / (1-B); // if B>0, it means R>L, so increase L, else decrease L 
	float VR = R / (1+B); // vice-versa

	float sl = (VR*1.73f - VL*1.22f) * CoefSL;
	float sr = (VR*1.22f - VL*1.73f) * CoefSR;
	
	if(RearDelay > 0)
	{
		SL = RearL[RP];
		SR = RearR[RP];

		RearL[RP] = sl;
		RearR[RP] = sr;
		
		RP = (RP + 1) % RearDelay;
	}
	else
	{
		SL = sl;
		SR = sr;
	}

	// Possible experiment: Play with stereo expension levels on rear

	// Adjust the volume of the front speakers based on what we calculated above
	L *= CoefL;
	R *= CoefR;	
}

void ProcessDplIISample32( const StereoOut32& src, Stereo51Out32DplII * s)
{
	const float InputScale = (float)(1<<(SndOutVolumeShift+16));
	float IL = src.Left  / InputScale;
	float IR = src.Right / InputScale;

	float L, R, C, SUB, SL, SR;
	Decode(IL, IR, L, R, C, SUB, SL, SR);
	
	s32 CX       = (s32)(C   * AddCLR  * Scale32);

	s->Left	     = (s32)(L   * GainL   * Scale32) + CX;
	s->Right     = (s32)(R   * GainR   * Scale32) + CX;
	s->Center    = (s32)(C   * GainC   * Scale32);
	s->LFE       = (s32)(SUB * GainLFE * Scale32);
	s->LeftBack	 = (s32)(SL  * GainSL  * Scale32);
	s->RightBack = (s32)(SR  * GainSR  * Scale32);
}

void ProcessDplIISample16( const StereoOut32& src, Stereo51Out16DplII * s)
{
	const float InputScale = (float)(1<<(SndOutVolumeShift));
	float IL = src.Left  / InputScale;
	float IR = src.Right / InputScale;

	float L, R, C, SUB, SL, SR;
	Decode(IL, IR, L, R, C, SUB, SL, SR);

	s32 CX       = (s32)(C   * AddCLR  * Scale16);

	s->Left	     = (s32)(L   * GainL   * Scale16) + CX;
	s->Right     = (s32)(R   * GainR   * Scale16) + CX;
	s->Center    = (s32)(C   * GainC   * Scale16);
	s->LFE       = (s32)(SUB * GainLFE * Scale16);
	s->LeftBack	 = (s32)(SL  * GainSL  * Scale16);
	s->RightBack = (s32)(SR  * GainSR  * Scale16);
}

void ProcessDplSample32( const StereoOut32& src, Stereo51Out32Dpl * s)
{
	float ValL = src.Left  / (float)(1<<(SndOutVolumeShift+16));
	float ValR = src.Right / (float)(1<<(SndOutVolumeShift+16));

	float C = (ValL+ValR)*0.5f;	//+15.8
	float S = (ValL-ValR)*0.5f;

	float L=ValL-C;			//+15.8
	float R=ValR-C;

	float SUB = C;
	
	s32 CX  = (s32)(C * AddCLR); // +15.16

	s->Left	     = (s32)(L   * GainL  ) + CX; // +15.16 = +31, can grow to +32 if (GainL + AddCLR)>255
	s->Right     = (s32)(R   * GainR  ) + CX;
	s->Center    = (s32)(C   * GainC  );			// +15.16 = +31
	s->LFE       = (s32)(SUB * GainLFE);
	s->LeftBack	 = (s32)(S   * GainSL );
	s->RightBack = (s32)(S   * GainSR );
}

void ProcessDplSample16( const StereoOut32& src, Stereo51Out16Dpl * s)
{
	Stereo51Out32Dpl ss;
	ProcessDplSample32(src, &ss);

	s->Left       = ss.Left >> 16;
	s->Right      = ss.Right >> 16;
	s->Center     = ss.Center >> 16;
	s->LFE        = ss.LFE >> 16;
	s->LeftBack   = ss.LeftBack >> 16;
	s->RightBack  = ss.RightBack >> 16;
}
