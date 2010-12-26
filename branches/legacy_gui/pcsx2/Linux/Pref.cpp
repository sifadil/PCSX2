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

#include "Linux.h"

FILE *pref_file;
char *data;
char cfgfile[g_MaxPath];

static void SetValue(const char *name, char *var)
{
	fprintf(pref_file, "%s = %s\n", name, var);
}

static void SetValuel(const char *name, s32 var)
{
	fprintf(pref_file, "%s = %x\n", name, var);
}

#define GetValue(name, var) {\
	char * tmp; \
	tmp = strstr(data, name); \
	if (tmp != NULL) { \
		tmp+=strlen(name); \
		while ((*tmp == ' ') || (*tmp == '=')) tmp++; \
		if (*tmp != '\n') sscanf(tmp, "%s", var); \
	} \
}

#define GetValuel(name, var) {\
	char * tmp; \
	tmp = strstr(data, name); \
	if (tmp != NULL) { \
		tmp+=strlen(name); \
		while ((*tmp == ' ') || (*tmp == '=')) tmp++; \
		if (*tmp != '\n') sscanf(tmp, "%x", &var); \
	} \
}

int LoadConfig()
{
	struct stat buf;
	int size;

	if (stat(cfgfile, &buf) == -1) return -1;
	size = buf.st_size;

	pref_file = fopen(cfgfile, "r");
	if (pref_file == NULL) return -1;

	data = (char*)malloc(size);
	if (data == NULL) return -1;

	fread(data, 1, size, pref_file);
	fclose(pref_file);

	GetValue("Bios", Config.Bios);
	Config.Lang[0] = 0;
	GetValue("Lang", Config.Lang);
	GetValuel("Ps2Out",     Config.PsxOut);
	GetValuel("cdvdPrint",     Config.cdvdPrint);
	GetValue("PluginsDir", Config.Paths.Plugins);
	GetValue("BiosDir",    Config.Paths.Bios);
	
	GetValuel("EnabledCard1", Config.Mcd[0].Enabled);
	GetValue("Mcd1", Config.Mcd[0].Filename);
	if (strcmp(Config.Mcd[0].Filename,"") == 0) strcpy(Config.Mcd[0].Filename, MEMCARDS_DIR "/" DEFAULT_MEMCARD1);
	
	GetValuel("EnabledCard2", Config.Mcd[1].Enabled);
	GetValue("Mcd2", Config.Mcd[1].Filename);
	if (strcmp(Config.Mcd[1].Filename,"") == 0) strcpy(Config.Mcd[1].Filename, MEMCARDS_DIR "/" DEFAULT_MEMCARD2);
	
	GetValuel("McdEnableEject", Config.McdEnableEject);
	
	GetValue("GS",   Config.Plugins.GS);
	GetValue("SPU2", Config.Plugins.SPU2);
	GetValue("CDVD", Config.Plugins.CDVD);
	GetValue("PAD1", Config.Plugins.PAD1);
	GetValue("PAD2", Config.Plugins.PAD2);
	GetValue("DEV9", Config.Plugins.DEV9);
	GetValue("USB",  Config.Plugins.USB);
	GetValue("FW",	 Config.Plugins.FW);

	GetValuel("Patch",      Config.Patch);
#ifdef PCSX2_DEVBUILD
	GetValuel("varLog", varLog);
#endif
	GetValuel("Options", Config.Options);

	GetValuel("EECycleRate", Config.Hacks.EECycleRate);
	if (Config.Hacks.EECycleRate > 2)
		Config.Hacks.EECycleRate = 2;
	GetValuel("IOPCycleDouble",	Config.Hacks.IOPCycleDouble);
	//GetValuel("WaitCycleExt",	Config.Hacks.WaitCycleExt);
	GetValuel("INTCSTATSlow",	Config.Hacks.INTCSTATSlow);
	GetValuel("VUCycleSteal",	Config.Hacks.VUCycleSteal);
	GetValuel("vuFlagHack",		Config.Hacks.vuFlagHack);
	GetValuel("vuMinMax",		Config.Hacks.vuMinMax);
	GetValuel("IdleLoopFF",		Config.Hacks.IdleLoopFF);
	//GetValuel("BlockDump", 		Config.Blockdump);
	GetValuel("ESCExits",		Config.Hacks.ESCExits);

	if (Config.Hacks.VUCycleSteal < 0 || Config.Hacks.VUCycleSteal > 4)
		Config.Hacks.VUCycleSteal = 0;
	GetValuel("Fixes",        Config.GameFixes);

	GetValuel("CustomFps",      Config.CustomFps);
	GetValuel("CustomFrameskip",      Config.CustomFrameSkip);
	GetValuel("CustomConsecutiveFrames",      Config.CustomConsecutiveFrames);
	GetValuel("CustomConsecutiveSkip",      Config.CustomConsecutiveSkip);

	// Note - order is currently important.
	GetValuel("sseMXCSR",        Config.sseMXCSR);
	GetValuel("sseVUMXCSR",    Config.sseVUMXCSR);
	GetValuel("eeOptions",        Config.eeOptions);
	GetValuel("vuOptions",   	    Config.vuOptions);

	free(data);

#ifdef ENABLE_NLS
	if (Config.Lang[0])
	{
		extern int _nl_msg_cat_cntr;

		setenv("LANGUAGE", Config.Lang, 1);
		++_nl_msg_cat_cntr;
	}
#endif

	return 0;
}

/////////////////////////////////////////////////////////

void SaveConfig()
{

	pref_file = fopen(cfgfile, "w");
	if (pref_file == NULL) return;

	SetValue("Bios", Config.Bios);
	SetValue("Lang",    Config.Lang);
	SetValue("PluginsDir", Config.Paths.Plugins);
	SetValue("BiosDir",    Config.Paths.Bios);
	
	SetValuel("Ps2Out",     Config.PsxOut);
	SetValuel("cdvdPrint",     Config.cdvdPrint);
	
	SetValuel("EnabledCard1", Config.Mcd[0].Enabled);
	SetValue("Mcd1", Config.Mcd[0].Filename);
	
	SetValuel("EnabledCard2", Config.Mcd[1].Enabled);
	SetValue("Mcd2", Config.Mcd[1].Filename);
	SetValuel("McdEnableEject", Config.McdEnableEject);

	SetValue("GS",   Config.Plugins.GS);
	SetValue("SPU2", Config.Plugins.SPU2);
	SetValue("CDVD", Config.Plugins.CDVD);
	SetValue("PAD1", Config.Plugins.PAD1);
	SetValue("PAD2", Config.Plugins.PAD2);
	SetValue("DEV9", Config.Plugins.DEV9);
	SetValue("USB",  Config.Plugins.USB);
	SetValue("FW",  Config.Plugins.FW);

	SetValuel("Options",        Config.Options);

	SetValuel("EECycleRate", Config.Hacks.EECycleRate);
	SetValuel("IOPCycleDouble", Config.Hacks.IOPCycleDouble);
	//SetValuel("WaitCycleExt", Config.Hacks.WaitCycleExt);
	SetValuel("INTCSTATSlow", Config.Hacks.INTCSTATSlow);
	SetValuel("VUCycleSteal",  Config.Hacks.VUCycleSteal);
	SetValuel("IdleLoopFF", Config.Hacks.IdleLoopFF);
	//SetValuel("BlockDump", Config.Blockdump);
	SetValuel("ESCExits", Config.Hacks.ESCExits);

	SetValuel("Fixes",        Config.GameFixes);

	SetValuel("Patch",      Config.Patch);

	SetValuel("CustomFps",      Config.CustomFps);
	SetValuel("CustomFrameskip",      Config.CustomFrameSkip);
	SetValuel("CustomConsecutiveFrames",      Config.CustomConsecutiveFrames);
	SetValuel("CustomConsecutiveSkip",      Config.CustomConsecutiveSkip);

	SetValuel("sseMXCSR",        Config.sseMXCSR);
	SetValuel("sseVUMXCSR",        Config.sseVUMXCSR);
	SetValuel("eeOptions",        Config.eeOptions);
	SetValuel("vuOptions",   	    Config.vuOptions);

#ifdef PCSX2_DEVBUILD
	SetValuel("varLog", varLog);
#endif

	fclose(pref_file);

	return;
}