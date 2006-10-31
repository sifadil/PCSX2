#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

#include "CDVDiso.h"
#include "Config.h"

char *methods[] = {
	".Z  - compress faster",
	".BZ - compress better",
	NULL
};

char *LibName = "Linuzappz Iso CDVD Driver";

const unsigned char version = PS2E_CDVD_VERSION;
const unsigned char revision = 0;
const unsigned char build = 5;

s32 msf_to_lba(u8 m, u8 s, u8 f) {
	u32 lsn;
	lsn = f;
	lsn+=(s - 2) * 75;
	lsn+= m * 75 * 60;
	return lsn;
}

void lba_to_msf(s32 lba, u8* m, u8* s, u8* f) {
	lba += 150;
	*m = lba / (60*75);
	*s = (lba / 75) % 60;
	*f = lba % 75;
}

#define btoi(b)		((b)/16*10 + (b)%16)		/* BCD to u_char */
#define itob(i)		((i)/10*16 + (i)%10)		/* u_char to BCD */


char* CALLBACK PS2EgetLibName() {
	return LibName;
}

u32 CALLBACK PS2EgetLibType() {
	return PS2E_LT_CDVD;
}

u32 CALLBACK PS2EgetLibVersion2(u32 type) {
	return (version << 16) | (revision << 8) | build;
}

#ifdef CDVD_LOG
void __Log(char *fmt, ...) {
	va_list list;

	va_start(list, fmt);
	vfprintf(cdvdLog, fmt, list);
	va_end(list);
}
#endif


s32 CALLBACK CDVDinit() {
#ifdef CDVD_LOG
	cdvdLog = fopen("logs/cdvdLog.txt", "w");
	if (cdvdLog == NULL) {
		cdvdLog = fopen("cdvdLog.txt", "w");
		if (cdvdLog == NULL) {
			SysMessage("Can't create cdvdLog.txt"); return -1;
		}
	}
	setvbuf(cdvdLog, NULL,  _IONBF, 0);
	CDVD_LOG("CDVDinit\n");
#endif
	memset(cdIso, 0, sizeof(cdIso));
	return 0;
}

void CALLBACK CDVDshutdown() {
#ifdef CDVD_LOG
	fclose(cdvdLog);
#endif
}

s32 CALLBACK CDVDopen() {
	LoadConf();

	if (*IsoFile == 0) {
		char temp[256];

		CfgOpenFile();

		LoadConf();
		strcpy(temp, IsoFile);
		*IsoFile = 0;
		SaveConf();
		strcpy(IsoFile, temp);
	}

	iso = isoOpen(IsoFile);
	if (iso == NULL) {
		SysMessage("Error loading %s\n", IsoFile);
		return -1;
	}

	if (iso->type == ISOTYPE_DVD) {
		 cdtype = CDVD_TYPE_PS2DVD;
	} else 
	if (iso->type == ISOTYPE_AUDIO) {
		 cdtype = CDVD_TYPE_CDDA;
	} else {
		cdtype = CDVD_TYPE_PS2CD;
	}

	if (BlockDump) {
		char fname[MAX_PATH],ext[MAX_PATH];
		char fname_only[MAX_PATH];
		_splitpath(IsoFile,NULL,NULL,fname,ext);
		_makepath(fname_only,NULL,NULL,fname,NULL);
		strcat(fname_only, ".dump");
		fdump = isoCreate(fname_only, ISOFLAGS_BLOCKDUMP);
		if (fdump) {
			isoSetFormat(fdump, iso->blockofs, iso->blocksize, iso->blocks);
		}
	} else {
		fdump = NULL;
	}

	return 0;
}

void CALLBACK CDVDclose() {
	isoClose(iso);
	if (fdump != NULL) {
		isoClose(fdump);
	}
}

s32  CALLBACK CDVDreadSubQ(u32 lsn, cdvdSubQ* subq) {
	// fake it
	u8 min, sec, frm;
	subq->ctrl		= 4;
	subq->mode		= 1;
	subq->trackNum	= itob(1);
	subq->trackIndex= itob(1);
	
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

s32 CALLBACK CDVDgetTN(cdvdTN *Buffer) {
	Buffer->strack = 1;
	Buffer->etrack = 1;

	return 0;
}

s32 CALLBACK CDVDgetTD(u8 Track, cdvdTD *Buffer) {
	if (Track == 0) {
		Buffer->lsn = iso->blocks;
	} else {
		Buffer->type = CDVD_MODE1_TRACK;
		Buffer->lsn = 0;
	}

	return 0;
}

s32  CALLBACK CDVDgetTOC(void* toc) {
	u8 type = CDVDgetDiskType();
	u8* tocBuff = (u8*)toc;
	
	if(	type == CDVD_TYPE_DVDV ||
		type == CDVD_TYPE_PS2DVD)
	{
		// get dvd structure format
		// scsi command 0x43
		memset(tocBuff, 0, 2048);
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
	}
	else if(type == CDVD_TYPE_CDDA ||
			type == CDVD_TYPE_PS2CDDA ||
			type == CDVD_TYPE_PS2CD ||
			type == CDVD_TYPE_PSCDDA ||
			type == CDVD_TYPE_PSCD)
	{
		// cd toc
		// (could be replaced by 1 command that reads the full toc)
		u8 min, sec, frm;
		s32 i, err;
		cdvdTN diskInfo;
		cdvdTD trackInfo;
		memset(tocBuff, 0, 1024);
		if (CDVDgetTN(&diskInfo) == -1)	{ diskInfo.etrack = 0;diskInfo.strack = 1; }
		if (CDVDgetTD(0, &trackInfo) == -1) trackInfo.lsn = 0;
		
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
		
		for (i=diskInfo.strack; i<=diskInfo.etrack; i++)
		{
			err = CDVDgetTD(i, &trackInfo);
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

s32 CALLBACK CDVDreadTrack(u32 lsn, int mode) {
	int _lsn = lsn;
	if (_lsn < 0) {
//		lsn = 2097152 + (-_lsn);
		lsn = iso->blocks - (-_lsn);
	}
//	printf ("CDRreadTrack %d\n", lsn);

	isoReadBlock(iso, cdbuffer, lsn);
	if (fdump != NULL) {
		isoWriteBlock(fdump, cdbuffer, lsn);
	}

	pbuffer = cdbuffer;
	switch (mode) {
		case CDVD_MODE_2352: break;
		case CDVD_MODE_2340: pbuffer+= 12; break;
		case CDVD_MODE_2328: pbuffer+= 24; break;
		case CDVD_MODE_2048: pbuffer+= 24; break;
	}

	return 0;
}

u8* CALLBACK CDVDgetBuffer() {
	return pbuffer;
}

s32  CALLBACK CDVDgetDiskType() {
	return cdtype;
}

s32  CALLBACK CDVDgetTrayStatus() {
	return CDVD_TRAY_CLOSE;
}

s32  CALLBACK CDVDctrlTrayOpen() {
	return 0;
}
s32  CALLBACK CDVDctrlTrayClose() {
	return 0;
}


s32 CALLBACK CDVDtest() {
	if (*IsoFile == 0)
		return 0;

	iso = isoOpen(IsoFile);
	if (iso == NULL) return -1;
	isoClose(iso);

	return 0;
}

