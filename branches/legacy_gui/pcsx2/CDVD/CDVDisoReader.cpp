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
 
/*
 *  Original code from libcdvd by Hiryu & Sjeep (C) 2002
 *  Modified by Florin for PCSX2 emu
 *  Fixed CdRead by linuzappz
 */
 
#include "PrecompiledHeader.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include "CDVDisoReader.h"

static char isoFileName[g_MaxPath];
static u8 *pbuffer;
static u8 cdbuffer[2352] = {0};
static isoFile *iso = NULL;

static int psize, cdtype;

void CALLBACK ISOclose()
{
	isoClose(iso);
	iso = NULL;
}

s32 CALLBACK ISOopen(const char* pTitle)
{
	ISOclose();		// just in case

	if ((pTitle != NULL) && (strlen(pTitle) > 0))
		strcpy(isoFileName, pTitle);

	iso = isoOpen(isoFileName);
	if (iso == NULL)
	{
		Console::Error("Error loading %s\n", params isoFileName);
		return -1;
	}

	switch (iso->type)
	{
		case ISOTYPE_DVD:
			cdtype = CDVD_TYPE_PS2DVD;
			break;
		case ISOTYPE_AUDIO:
			cdtype = CDVD_TYPE_CDDA;
			break;
		default:
			cdtype = CDVD_TYPE_PS2CD;
			break;
	}

	return 0;
}

s32 CALLBACK ISOreadSubQ(u32 lsn, cdvdSubQ* subq)
{
	// fake it
	u8 min, sec, frm;
	subq->ctrl		= 4;
	subq->mode		= 1;
	subq->trackNum	= itob(1);
	subq->trackIndex = itob(1);

	lba_to_msf(lsn, &min, &sec, &frm);
	subq->trackM	= itob(min);
	subq->trackS	= itob(sec);
	subq->trackF	= itob(frm);

	subq->pad		= 0;

	lba_to_msf(lsn + (2*75), &min, &sec, &frm);
	subq->discM		= itob(min);
	subq->discS		= itob(sec);
	subq->discF		= itob(frm);
	return 0;
}

s32 CALLBACK ISOgetTN(cdvdTN *Buffer)
{
	Buffer->strack = 1;
	Buffer->etrack = 1;

	return 0;
}

s32 CALLBACK ISOgetTD(u8 Track, cdvdTD *Buffer)
{
	if (Track == 0)
	{
		Buffer->lsn = iso->blocks;
	}
	else
	{
		Buffer->type = CDVD_MODE1_TRACK;
		Buffer->lsn = 0;
	}

	return 0;
}

static s32 layer1start = -1;

static void FindLayer1Start()
{
	if ((layer1start == -1) && iso->blocks >= 2295104)
	{
		// search for it
		int off = iso->blockofs;
		u8 tempbuffer[2352];

		Console::Status("CDVD ISO: searching for layer1...");
		//tempbuffer = (u8*)malloc(CD_FRAMESIZE_RAW);
		for (layer1start = (iso->blocks / 2 - 0x10) & ~0xf; layer1start < 0x200010; layer1start += 16)
		{
			isoReadBlock(iso, tempbuffer+off, layer1start);
			// CD001
			if ((tempbuffer[off+1] == 0x43) &&
				(tempbuffer[off+2] == 0x44) &&
				(tempbuffer[off+3] == 0x30) &&
				(tempbuffer[off+4] == 0x30) &&
				(tempbuffer[off+5] == 0x31)
				)
				break;
		}
		//free(tempbuffer);

		if(layer1start == 0x200010)
		{
			Console::Status("Couldn't find second layer on dual layer... ignoring\n");
			layer1start=-2;
		}

		if(layer1start>=0) Console::Status("found at 0x%8.8x\n", params layer1start);
	}
}

s32 CALLBACK ISOgetDualInfo(s32* dualType, u32* _layer1start)
{
	FindLayer1Start();

	if(layer1start<0)
	{
		*dualType=0;
		*_layer1start = iso->blocks;
	}
	else
	{
		*dualType = 1;
		*_layer1start = layer1start;
	}
	return 1;
}

s32 CALLBACK ISOgetDiskType()
{
	return cdtype;
}

s32 CALLBACK ISOgetTOC(void* toc)
{
	u8 type = ISOgetDiskType();
	u8* tocBuff = (u8*)toc;

	//CDVD_LOG("CDVDgetTOC\n");

	if (type == CDVD_TYPE_DVDV || type == CDVD_TYPE_PS2DVD)
	{
		// get dvd structure format
		// scsi command 0x43
		memset(tocBuff, 0, 2048);

		FindLayer1Start();

		if (layer1start < 0)
		{
			// fake it
			tocBuff[ 0] = 0x04;
			tocBuff[ 1] = 0x02;
			tocBuff[ 2] = 0xF2;
			tocBuff[ 3] = 0x00;
			tocBuff[ 4] = 0x86;
			tocBuff[ 5] = 0x72;

			tocBuff[16] = 0x00;
			tocBuff[17] = 0x03;
			tocBuff[18] = 0x00;
			tocBuff[19] = 0x00;
			return 0;
		}
		else
		{
			// dual sided
			tocBuff[ 0] = 0x24;
			tocBuff[ 1] = 0x02;
			tocBuff[ 2] = 0xF2;
			tocBuff[ 3] = 0x00;
			tocBuff[ 4] = 0x41;
			tocBuff[ 5] = 0x95;

			tocBuff[14] = 0x60; // dual sided, ptp

			tocBuff[16] = 0x00;
			tocBuff[17] = 0x03;
			tocBuff[18] = 0x00;
			tocBuff[19] = 0x00;

			s32 l1s = layer1start + 0x30000 - 1;
			tocBuff[20] = (l1s >> 24);
			tocBuff[21] = (l1s >> 16) & 0xff;
			tocBuff[22] = (l1s >> 8) & 0xff;
			tocBuff[23] = (l1s >> 0) & 0xff;
		}
	}
	else if ((type == CDVD_TYPE_CDDA) || (type == CDVD_TYPE_PS2CDDA) ||
		(type == CDVD_TYPE_PS2CD) || (type == CDVD_TYPE_PSCDDA) || (type == CDVD_TYPE_PSCD))
	{
		// cd toc
		// (could be replaced by 1 command that reads the full toc)
		u8 min, sec, frm;
		s32 i, err;
		cdvdTN diskInfo;
		cdvdTD trackInfo;
		memset(tocBuff, 0, 1024);
		if (ISOgetTN(&diskInfo) == -1)
		{
			diskInfo.etrack = 0;
			diskInfo.strack = 1;
		}
		if (ISOgetTD(0, &trackInfo) == -1) trackInfo.lsn = 0;

		tocBuff[0] = 0x41;
		tocBuff[1] = 0x00;

		//Number of FirstTrack
		tocBuff[2] = 0xA0;
		tocBuff[7] = itob(diskInfo.strack);

		//Number of LastTrack
		tocBuff[12] = 0xA1;
		tocBuff[17] = itob(diskInfo.etrack);

		//DiskLength
		lba_to_msf(trackInfo.lsn, &min, &sec, &frm);
		tocBuff[22] = 0xA2;
		tocBuff[27] = itob(min);
		tocBuff[28] = itob(sec);

		for (i = diskInfo.strack; i <= diskInfo.etrack; i++)
		{
			err = ISOgetTD(i, &trackInfo);
			lba_to_msf(trackInfo.lsn, &min, &sec, &frm);
			tocBuff[i*10+30] = trackInfo.type;
			tocBuff[i*10+32] = err == -1 ? 0 : itob(i);	  //number
			tocBuff[i*10+37] = itob(min);
			tocBuff[i*10+38] = itob(sec);
			tocBuff[i*10+39] = itob(frm);
		}
	}
	else
		return -1;

	return 0;
}

s32 CALLBACK ISOreadSector(u8* tempbuffer, u32 lsn, int mode)
{
	int _lsn = lsn;

	if (_lsn < 0) lsn = iso->blocks + _lsn;
	if (lsn > iso->blocks) return -1;

	if(mode == CDVD_MODE_2352)
	{
		isoReadBlock(iso, tempbuffer, lsn);
		return 0;
	}

	isoReadBlock(iso, cdbuffer, lsn);

	pbuffer = cdbuffer;
	
	switch (mode)
	{
	case CDVD_MODE_2352:
		psize = 2352;
		break;
	case CDVD_MODE_2340:
		pbuffer += 12;
		psize = 2340;
		break;
	case CDVD_MODE_2328:
		pbuffer += 24;
		psize = 2328;
		break;
	case CDVD_MODE_2048:
		pbuffer += 24;
		psize = 2048;
		break;
	}

	// version 3 blockdumps have no pbuffer header, so lets reset back to the
	// original pointer. :)
	if( iso->flags & ISOFLAGS_BLOCKDUMP_V3 )
		pbuffer = cdbuffer;

	memcpy_fast(tempbuffer,pbuffer,psize);

	return 0;
}

s32 CALLBACK ISOreadTrack(u32 lsn, int mode)
{
	int _lsn = lsn;

	if (_lsn < 0) lsn = iso->blocks + _lsn;
	if (lsn > iso->blocks) return -1;

	isoReadBlock(iso, cdbuffer, lsn);
	pbuffer = cdbuffer;

	switch (mode)
	{
	case CDVD_MODE_2352:
		psize = 2352;
		break;
	case CDVD_MODE_2340:
		pbuffer += 12;
		psize = 2340;
		break;
	case CDVD_MODE_2328:
		pbuffer += 24;
		psize = 2328;
		break;
	case CDVD_MODE_2048:
		pbuffer += 24;
		psize = 2048;
		break;
	}

	// version 3 blockdumps have no pbuffer header, so lets reset back to the
	// original pointer. :)
	if( iso->flags & ISOFLAGS_BLOCKDUMP_V3 )
		pbuffer = cdbuffer;

	return 0;
}

s32 CALLBACK ISOgetBuffer2(u8* buffer)
{
	memcpy_fast(buffer,pbuffer,psize);
	return 0;
}

u8* CALLBACK ISOgetBuffer()
{
	return pbuffer;
}

s32 CALLBACK ISOgetTrayStatus()
{
	return CDVD_TRAY_CLOSE;
}

s32 CALLBACK ISOctrlTrayOpen()
{
	return 0;
}
s32 CALLBACK ISOctrlTrayClose()
{
	return 0;
}

s32 CALLBACK ISOdummyS32()
{
	return 0;
}

void CALLBACK ISOnewDiskCB(void(CALLBACK*)())
{
}

string ISOgetUniqueFilename()
{
	return Path::GetFilenameWithoutExt( isoFileName );
}

CDVD_API CDVDapi_Iso =
{
	ISOclose,

	ISOopen,
	ISOreadTrack,
	ISOgetBuffer, // emu shouldn't use this one.
	ISOreadSubQ,
	ISOgetTN,
	ISOgetTD,
	ISOgetTOC,
	ISOgetDiskType,
	ISOdummyS32,	// trayStatus
	ISOdummyS32,	// trayOpen
	ISOdummyS32,	// trayClose

	ISOnewDiskCB,

	ISOreadSector,
	ISOgetBuffer2,
	ISOgetDualInfo,

	ISOgetUniqueFilename
};