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

#define GET_DMA_DATA_PTR(offset) (((s16*)thiscore.AdmaTempBuffer)+offset)
//#define GET_DMA_DATA_PTR(offset) (GetMemPtr(0x2000 + (core<<10) + offset))

void __fastcall ReadInput( uint core, StereoOut32& PData ) 
{
	V_Core& thiscore( Cores[core] );

	if((thiscore.AutoDMACtrl&(core+1))==(core+1))
	{
		s32 tl,tr;

		if((core==1)&&((PlayMode&8)==8))
		{
			thiscore.AdmaReadPos&=~1;

			// CDDA mode
			// Source audio data is 32 bits.
			// We don't yet have the capability to handle this high res input data
			// so we just downgrade it to 16 bits for now.

			s32 *pl=(s32*)GET_DMA_DATA_PTR(thiscore.AdmaReadPos);
			s32 *pr=(s32*)GET_DMA_DATA_PTR(thiscore.AdmaReadPos+0x200);
			PData.Left = *pl;
			PData.Right = *pr;

			PData.Left >>= 2; //give 30 bit data (SndOut downsamples the rest of the way)
			PData.Right >>= 2;

			thiscore.AdmaReadPos+=2;
			if((thiscore.AdmaReadPos==0x100)||(thiscore.AdmaReadPos>=0x200)) {
				thiscore.AdmaInProgress=0;
				if(thiscore.AdmaDataLeft>=0x200)
				{
					thiscore.AdmaFree+=2;
					if(thiscore.AdmaFree>2)
					{
						thiscore.AdmaFree=2;
						thiscore.AdmaReadPos=thiscore.AdmaWritePos;
					}

					thiscore.AdmaInProgress=1;

					thiscore.TSA=(core<<10)+thiscore.AdmaReadPos;

					if (thiscore.AdmaDataLeft<0x200) 
					{
						FileLog("[%10d] AutoDMA%c block end.\n",Cycles, (core==0)?'4':'7');

						if( IsDevBuild )
						{
							if(thiscore.AdmaDataLeft>0)
							{
								if(MsgAutoDMA()) ConLog("WARNING: adma buffer didn't finish with a whole block!!\n");
							}
						}
						thiscore.AdmaDataLeft=0;
					}
				}
				thiscore.AdmaReadPos&=0x1ff;
			}

		}
		else if((core==0)&&((PlayMode&4)==4))
		{
			thiscore.AdmaReadPos&=~1;

			s32 *pl=(s32*)GET_DMA_DATA_PTR(thiscore.AdmaReadPos);
			s32 *pr=(s32*)GET_DMA_DATA_PTR(thiscore.AdmaReadPos+0x200);
			PData.Left  = *pl;
			PData.Right = *pr;

			thiscore.AdmaReadPos+=2;
			if(thiscore.AdmaReadPos>=0x200) {
				thiscore.AdmaInProgress=0;
				if(thiscore.AdmaDataLeft>=0x200)
				{
					thiscore.AdmaFree+=2;
					if(thiscore.AdmaFree>2)
					{
						thiscore.AdmaFree=2;
						thiscore.AdmaReadPos=thiscore.AdmaWritePos;
					}

					thiscore.AdmaInProgress=1;

					thiscore.TSA=(core<<10)+thiscore.AdmaReadPos;

					if (thiscore.AdmaDataLeft<0x200) 
					{
						FileLog("[%10d] Spdif AutoDMA%c block end.\n",Cycles, (core==0)?'4':'7');

						if( IsDevBuild )
						{
							if(thiscore.AdmaDataLeft>0)
							{
								if(MsgAutoDMA()) ConLog("WARNING: adma buffer didn't finish with a whole block!!\n");
							}
						}
						thiscore.AdmaDataLeft=0;
					}
				}
				thiscore.AdmaReadPos&=0x1ff;
			}

		}
		else
		{
			if((core==1)&&((PlayMode&2)!=0))
			{
				tl=0;
				tr=0;
			}
			else
			{
				tl = (s32)*GET_DMA_DATA_PTR(thiscore.AdmaReadPos);
				tr = (s32)*GET_DMA_DATA_PTR(thiscore.AdmaReadPos+0x200);
			}

			PData.Left  = tl;
			PData.Right = tr;

			thiscore.AdmaReadPos++;
			if((thiscore.AdmaReadPos==0x100)||(thiscore.AdmaReadPos>=0x200)) {
				thiscore.AdmaInProgress=0;
				if(thiscore.AdmaDataLeft>=0x200)
				{
					thiscore.AdmaFree++;
					if(thiscore.AdmaFree>2)
					{
						thiscore.AdmaFree=2;
						thiscore.AdmaReadPos=thiscore.AdmaWritePos;
					}

					thiscore.AdmaInProgress=1;

					thiscore.TSA=(core<<10)+thiscore.AdmaReadPos;

					if (thiscore.AdmaDataLeft<0x200) 
					{
						thiscore.AutoDMACtrl |= ~3;

						if( IsDevBuild )
						{
							FileLog("[%10d] AutoDMA%c block end.\n",Cycles, (core==0)?'4':'7');
							if(thiscore.AdmaDataLeft>0)
							{
								if(MsgAutoDMA()) ConLog("WARNING: adma buffer didn't finish with a whole block!!\n");
							}
						}

						thiscore.AdmaDataLeft = 0;
					}
				}
				thiscore.AdmaReadPos&=0x1ff;
			}
		}
	}
	else
	{
		PData.Left  = 0;
		PData.Right = 0;
	}
}
