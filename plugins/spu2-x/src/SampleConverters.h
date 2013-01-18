/* SPU2-X, A plugin for Emulating the Sound Processing Unit of the Playstation 2
 * Developed and maintained by the Pcsx2 Development Team.
 *
 * Original portions from SPU2ghz are (c) 2008 by David Quintana [gigaherz]
 *
 * SPU2-X is free software: you can redistribute it and/or modify it under the terms
 * of the GNU Lesser General Public License as published by the Free Software Found-
 * ation, either version 3 of the License, or (at your option) any later version.
 *
 * SPU2-X is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with SPU2-X.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

struct Stereo51Out16DplII;
struct Stereo51Out32DplII;

struct Stereo51Out16Dpl; // similar to DplII but without rear balancing
struct Stereo51Out32Dpl;

extern void ResetDplIIDecoder();
extern void ProcessDplIISample16( const StereoOut32& src, Stereo51Out16DplII * s);
extern void ProcessDplIISample32( const StereoOut32& src, Stereo51Out32DplII * s);
extern void ProcessDplSample16( const StereoOut32& src, Stereo51Out16Dpl * s);
extern void ProcessDplSample32( const StereoOut32& src, Stereo51Out32Dpl * s);

struct StereoOut16
{
	s16 Left;
	s16 Right;

	StereoOut16() :
		Left( 0 ),
		Right( 0 )
	{
	}

	StereoOut16( const StereoOut32& src ) :
		Left( (s16)src.Left ),
		Right( (s16)src.Right )
	{
	}

	StereoOut16( s16 left, s16 right ) :
		Left( left ),
		Right( right )
	{
	}

	StereoOut32 UpSample() const;

	void ResampleFrom( const StereoOut32& src )
	{
		// Use StereoOut32's built in conversion
		*this = src.DownSample();
	}
};

struct StereoOutFloat
{
	float Left;
	float Right;

	StereoOutFloat() :
		Left( 0 ),
		Right( 0 )
	{
	}

	explicit StereoOutFloat( const StereoOut32& src ) :
		Left( src.Left / 2147483647.0f ),
		Right( src.Right / 2147483647.0f )
	{
	}

	explicit StereoOutFloat( s32 left, s32 right ) :
		Left( left / 2147483647.0f ),
		Right( right / 2147483647.0f )
	{
	}

	StereoOutFloat( float left, float right ) :
		Left( left ),
		Right( right )
	{
	}
};

struct Stereo21Out16
{
	s16 Left;
	s16 Right;
	s16 LFE;

	void ResampleFrom( const StereoOut32& src )
	{
		Left = src.Left >> SndOutVolumeShift;
		Right = src.Right >> SndOutVolumeShift;
		LFE = (src.Left + src.Right) >> (SndOutVolumeShift + 1);
	}
};

struct Stereo40Out16
{
	s16 Left;
	s16 Right;
	s16 LeftBack;
	s16 RightBack;

	void ResampleFrom( const StereoOut32& src )
	{
		Left = src.Left >> SndOutVolumeShift;
		Right = src.Right >> SndOutVolumeShift;
		LeftBack = src.Left >> SndOutVolumeShift;
		RightBack = src.Right >> SndOutVolumeShift;
	}
};

struct Stereo40Out32
{
	s32 Left;
	s32 Right;
	s32 LeftBack;
	s32 RightBack;

	void ResampleFrom( const StereoOut32& src )
	{
		Left = src.Left << SndOutVolumeShift32;
		Right = src.Right << SndOutVolumeShift32;
		LeftBack = src.Left << SndOutVolumeShift32;
		RightBack = src.Right << SndOutVolumeShift32;
	}
};

struct Stereo41Out16
{
	s16 Left;
	s16 Right;
	s16 LFE;
	s16 LeftBack;
	s16 RightBack;

	void ResampleFrom( const StereoOut32& src )
	{
		Left = src.Left >> SndOutVolumeShift;
		Right = src.Right >> SndOutVolumeShift;
		LFE = (src.Left + src.Right) >> (SndOutVolumeShift + 1);
		LeftBack = src.Left >> SndOutVolumeShift;
		RightBack = src.Right >> SndOutVolumeShift;
	}
};

struct Stereo51Out16
{
	s16 Left;
	s16 Right;
	s16 Center;
	s16 LFE;
	s16 LeftBack;
	s16 RightBack;

	// Implementation Note: Center and Subwoofer/LFE -->
	// This method is simple and sounds nice.  It relies on the speaker/soundcard
	// systems do to their own low pass / crossover.  Manual lowpass is wasted effort
	// and can't match solid state results anyway.

	void ResampleFrom( const StereoOut32& src )
	{
		Left = src.Left >> SndOutVolumeShift;
		Right = src.Right >> SndOutVolumeShift;
		Center = (src.Left + src.Right) >> (SndOutVolumeShift + 1);
		LFE = Center;
		LeftBack = src.Left >> SndOutVolumeShift;
		RightBack = src.Right >> SndOutVolumeShift;
	}
};

struct Stereo51Out16DplII
{
	s16 Left;
	s16 Right;
	s16 Center;
	s16 LFE;
	s16 LeftBack;
	s16 RightBack;

	void ResampleFrom( const StereoOut32& src )
	{
		ProcessDplIISample16(src, this);
	}
};

struct Stereo51Out32DplII
{
	s32 Left;
	s32 Right;
	s32 Center;
	s32 LFE;
	s32 LeftBack;
	s32 RightBack;

	void ResampleFrom( const StereoOut32& src )
	{
		ProcessDplIISample32(src, this);
	}
};

struct Stereo51Out16Dpl
{
	s16 Left;
	s16 Right;
	s16 Center;
	s16 LFE;
	s16 LeftBack;
	s16 RightBack;

	void ResampleFrom( const StereoOut32& src )
	{
		ProcessDplSample16(src, this);
	}
};

struct Stereo51Out32Dpl
{
	s32 Left;
	s32 Right;
	s32 Center;
	s32 LFE;
	s32 LeftBack;
	s32 RightBack;

	void ResampleFrom( const StereoOut32& src )
	{
		ProcessDplSample32(src, this);
	}
};

struct Stereo71Out16
{
	s16 Left;
	s16 Right;
	s16 Center;
	s16 LFE;
	s16 LeftBack;
	s16 RightBack;
	s16 LeftSide;
	s16 RightSide;

	void ResampleFrom( const StereoOut32& src )
	{
		Left = src.Left >> SndOutVolumeShift;
		Right = src.Right >> SndOutVolumeShift;
		Center = (src.Left + src.Right) >> (SndOutVolumeShift + 1);
		LFE = Center;
		LeftBack = src.Left >> SndOutVolumeShift;
		RightBack = src.Right >> SndOutVolumeShift;

		LeftSide = src.Left >> (SndOutVolumeShift+1);
		RightSide = src.Right >> (SndOutVolumeShift+1);
	}
};

struct Stereo71Out32
{
	s32 Left;
	s32 Right;
	s32 Center;
	s32 LFE;
	s32 LeftBack;
	s32 RightBack;
	s32 LeftSide;
	s32 RightSide;

	void ResampleFrom( const StereoOut32& src )
	{
		Left = src.Left << SndOutVolumeShift32;
		Right = src.Right << SndOutVolumeShift32;
		Center = (src.Left + src.Right) << (SndOutVolumeShift32 - 1);
		LFE = Center;
		LeftBack = src.Left << SndOutVolumeShift32;
		RightBack = src.Right << SndOutVolumeShift32;

		LeftSide = src.Left << (SndOutVolumeShift32 - 1);
		RightSide = src.Right << (SndOutVolumeShift32 - 1);
	}
};

struct Stereo20Out32
{
	s32 Left;
	s32 Right;
	
	void ResampleFrom( const StereoOut32& src )
	{
		Left = src.Left << SndOutVolumeShift32;
		Right = src.Right << SndOutVolumeShift32;
	}
};

struct Stereo21Out32
{
	s32 Left;
	s32 Right;
	s32 LFE;
	
	void ResampleFrom( const StereoOut32& src )
	{
		Left = src.Left << SndOutVolumeShift32;
		Right = src.Right << SndOutVolumeShift32;
		LFE = (src.Left + src.Right) << (SndOutVolumeShift32 - 1);
	}
};

struct Stereo41Out32
{
	s32 Left;
	s32 Right;
	s32 LFE;
	s32 LeftBack;
	s32 RightBack;
	
	void ResampleFrom( const StereoOut32& src )
	{
		Left = src.Left << SndOutVolumeShift32;
		Right = src.Right << SndOutVolumeShift32;
		LFE = (src.Left + src.Right) << (SndOutVolumeShift32 - 1);

		LeftBack = src.Left << SndOutVolumeShift32;
		RightBack = src.Right << SndOutVolumeShift32;
	}
};

struct Stereo51Out32
{
	s32 Left;
	s32 Right;
	s32 Center;
	s32 LFE;
	s32 LeftBack;
	s32 RightBack;

	void ResampleFrom( const StereoOut32& src )
	{
		Left = src.Left << SndOutVolumeShift32;
		Right = src.Right << SndOutVolumeShift32;
		Center = (src.Left + src.Right) << (SndOutVolumeShift32 - 1);
		LFE = Center;
		LeftBack = src.Left << SndOutVolumeShift32;
		RightBack = src.Right << SndOutVolumeShift32;
	}
};