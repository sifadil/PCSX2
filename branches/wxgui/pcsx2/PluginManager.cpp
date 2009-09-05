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

#include "PrecompiledHeader.h"
#include "Utilities/ScopedPtr.h"

#include <wx/dir.h>
#include <wx/file.h>

#include "IopCommon.h"
#include "GS.h"
#include "HostGui.h"
#include "CDVD/CDVDisoReader.h"

// ----------------------------------------------------------------------------
// Yay, order of this array shouldn't be important. :)
//
const PluginInfo tbl_PluginInfo[] =
{
	{ "GS",		PluginId_GS,	PS2E_LT_GS,		PS2E_GS_VERSION		},
	{ "PAD",	PluginId_PAD,	PS2E_LT_PAD,	PS2E_PAD_VERSION	},
	{ "SPU2",	PluginId_SPU2,	PS2E_LT_SPU2,	PS2E_SPU2_VERSION	},
	{ "CDVD",	PluginId_CDVD,	PS2E_LT_CDVD,	PS2E_CDVD_VERSION	},
	{ "USB",	PluginId_USB,	PS2E_LT_USB,	PS2E_USB_VERSION	},
	{ "FW",		PluginId_FW,	PS2E_LT_FW,		PS2E_FW_VERSION		},
	{ "DEV9",	PluginId_DEV9,	PS2E_LT_DEV9,	PS2E_DEV9_VERSION	},

	{ NULL }

	// SIO is currently unused (legacy?)
	//{ "SIO",	PluginId_SIO,	PS2E_LT_SIO,	PS2E_SIO_VERSION }

};

typedef void CALLBACK VoidMethod();
typedef void CALLBACK vMeth();		// shorthand for VoidMethod

// ----------------------------------------------------------------------------
struct LegacyApi_CommonMethod
{
	const char*		MethodName;

	// fallback is used if the method is null.  If the method is null and fallback is null
	// also, the plugin is considered incomplete or invalid, and an error is generated.
	VoidMethod*	Fallback;

	// returns the method name as a wxString, converted from UTF8.
	wxString GetMethodName( PluginsEnum_t pid ) const
	{
		return tbl_PluginInfo[pid].GetShortname() + wxString::FromUTF8( MethodName );
	}
};

// ----------------------------------------------------------------------------
struct LegacyApi_ReqMethod
{
	const char*		MethodName;
	VoidMethod**	Dest;		// Target function where the binding is saved.

	// fallback is used if the method is null.  If the method is null and fallback is null
	// also, the plugin is considered incomplete or invalid, and an error is generated.
	VoidMethod*	Fallback;

	// returns the method name as a wxString, converted from UTF8.
	wxString GetMethodName( ) const
	{
		return wxString::FromUTF8( MethodName );
	}
};

// ----------------------------------------------------------------------------
struct LegacyApi_OptMethod
{
	const char*		MethodName;
	VoidMethod**	Dest;		// Target function where the binding is saved.

	// returns the method name as a wxString, converted from UTF8.
	wxString GetMethodName() const { return wxString::FromUTF8( MethodName ); }
};


static s32  CALLBACK fallback_freeze(int mode, freezeData *data) { data->size = 0; return 0; }
static void CALLBACK fallback_keyEvent(keyEvent *ev) {}
static void CALLBACK fallback_configure() {}
static void CALLBACK fallback_about() {}
static s32  CALLBACK fallback_test() { return 0; }

_GSvsync           GSvsync;
_GSopen            GSopen;
_GSgifTransfer1    GSgifTransfer1;
_GSgifTransfer2    GSgifTransfer2;
_GSgifTransfer3    GSgifTransfer3;
_GSgifSoftReset    GSgifSoftReset;
_GSreadFIFO        GSreadFIFO;
_GSreadFIFO2       GSreadFIFO2;
_GSchangeSaveState GSchangeSaveState;
_GSmakeSnapshot	   GSmakeSnapshot;
_GSmakeSnapshot2   GSmakeSnapshot2;
_GSirqCallback 	   GSirqCallback;
_GSprintf      	   GSprintf;
_GSsetBaseMem 	   GSsetBaseMem;
_GSsetGameCRC		GSsetGameCRC;
_GSsetFrameSkip	   GSsetFrameSkip;
_GSsetFrameLimit   GSsetFrameLimit;
_GSsetupRecording	GSsetupRecording;
_GSreset		   GSreset;
_GSwriteCSR		   GSwriteCSR;

static void CALLBACK GS_makeSnapshot(const char *path) {}
static void CALLBACK GS_irqCallback(void (*callback)()) {}
static void CALLBACK GS_printf(int timeout, char *fmt, ...)
{
	va_list list;
	char msg[512];

	va_start(list, fmt);
	vsprintf(msg, fmt, list);
	va_end(list);

	Console::WriteLn(msg);
}

// PAD
_PADinit           PADinit;
_PADopen           PADopen;
_PADstartPoll      PADstartPoll;
_PADpoll           PADpoll;
_PADquery          PADquery;
_PADupdate         PADupdate;
_PADkeyEvent       PADkeyEvent;
_PADsetSlot        PADsetSlot;
_PADqueryMtap      PADqueryMtap;

static void PAD_update( u32 padslot ) { }

// SPU2
_SPU2open          SPU2open;
_SPU2write         SPU2write;
_SPU2read          SPU2read;
_SPU2readDMA4Mem   SPU2readDMA4Mem;
_SPU2writeDMA4Mem  SPU2writeDMA4Mem;
_SPU2interruptDMA4 SPU2interruptDMA4;
_SPU2readDMA7Mem   SPU2readDMA7Mem;
_SPU2writeDMA7Mem  SPU2writeDMA7Mem;
_SPU2setDMABaseAddr SPU2setDMABaseAddr;
_SPU2interruptDMA7 SPU2interruptDMA7;
_SPU2ReadMemAddr   SPU2ReadMemAddr;
_SPU2setupRecording SPU2setupRecording;
_SPU2WriteMemAddr   SPU2WriteMemAddr;
_SPU2irqCallback   SPU2irqCallback;

_SPU2setClockPtr   SPU2setClockPtr;
_SPU2async         SPU2async;


// DEV9
_DEV9open          DEV9open;
_DEV9read8         DEV9read8;
_DEV9read16        DEV9read16;
_DEV9read32        DEV9read32;
_DEV9write8        DEV9write8;
_DEV9write16       DEV9write16;
_DEV9write32       DEV9write32;
_DEV9readDMA8Mem   DEV9readDMA8Mem;
_DEV9writeDMA8Mem  DEV9writeDMA8Mem;
_DEV9irqCallback   DEV9irqCallback;
_DEV9irqHandler    DEV9irqHandler;

// USB
_USBopen           USBopen;
_USBread8          USBread8;
_USBread16         USBread16;
_USBread32         USBread32;
_USBwrite8         USBwrite8;
_USBwrite16        USBwrite16;
_USBwrite32        USBwrite32;
_USBasync          USBasync;

_USBirqCallback    USBirqCallback;
_USBirqHandler     USBirqHandler;
_USBsetRAM         USBsetRAM;

// FW
_FWopen            FWopen;
_FWread32          FWread32;
_FWwrite32         FWwrite32;
_FWirqCallback     FWirqCallback;

DEV9handler dev9Handler;
USBhandler usbHandler;
uptr pDsp;

static s32 CALLBACK _hack_PADinit()
{
	return PADinit( 3 );
}

// ----------------------------------------------------------------------------
// Important: Contents of this array must match the order of the contents of the
// LegacyPluginAPI_Common structure defined in Plugins.h.
//
static const LegacyApi_CommonMethod s_MethMessCommon[] =
{
	{	"init",			NULL	},
	{	"close",		NULL	},
	{	"shutdown",		NULL	},

	{	"freeze",		(vMeth*)fallback_freeze	},
	{	"test",			(vMeth*)fallback_test	},
	{	"configure",	fallback_configure	},
	{	"about",		fallback_about	},

	{ NULL }

};

// ----------------------------------------------------------------------------
//  GS Mess!
// ----------------------------------------------------------------------------
static const LegacyApi_ReqMethod s_MethMessReq_GS[] =
{
	{	"GSopen",			(vMeth**)&GSopen,			NULL	},
	{	"GSvsync",			(vMeth**)&GSvsync,			NULL	},
	{	"GSgifTransfer1",	(vMeth**)&GSgifTransfer1,	NULL	},
	{	"GSgifTransfer2",	(vMeth**)&GSgifTransfer2,	NULL	},
	{	"GSgifTransfer3",	(vMeth**)&GSgifTransfer3,	NULL	},
	{	"GSreadFIFO2",		(vMeth**)&GSreadFIFO2,		NULL	},

	{	"GSmakeSnapshot",	(vMeth**)&GSmakeSnapshot,	(vMeth*)GS_makeSnapshot },
	{	"GSirqCallback",	(vMeth**)&GSirqCallback,	(vMeth*)GS_irqCallback },
	{	"GSprintf",			(vMeth**)&GSprintf,			(vMeth*)GS_printf },
	{	"GSsetBaseMem",		(vMeth**)&GSsetBaseMem,		NULL	},
	{	"GSwriteCSR",		(vMeth**)&GSwriteCSR,		NULL	},
	{ NULL }
};

static const LegacyApi_OptMethod s_MethMessOpt_GS[] =
{
	{	"GSreset",			(vMeth**)&GSreset			},
	{	"GSsetupRecording",	(vMeth**)&GSsetupRecording	},
	{	"GSsetGameCRC",		(vMeth**)&GSsetGameCRC		},
	{	"GSsetFrameSkip",	(vMeth**)&GSsetFrameSkip	},
	{	"GSsetFrameLimit",	(vMeth**)&GSsetFrameLimit	},
	{	"GSchangeSaveState",(vMeth**)&GSchangeSaveState	},
	{	"GSmakeSnapshot2",	(vMeth**)&GSmakeSnapshot2	},
	{	"GSgifSoftReset",	(vMeth**)&GSgifSoftReset	},
	{	"GSreadFIFO",		(vMeth**)&GSreadFIFO		},
	{ NULL }
};

// ----------------------------------------------------------------------------
//  PAD Mess!
// ----------------------------------------------------------------------------
static const LegacyApi_ReqMethod s_MethMessReq_PAD[] =
{
	{	"PADopen",			(vMeth**)&PADopen,		NULL },
	{	"PADstartPoll",		(vMeth**)&PADstartPoll,	NULL },
	{	"PADpoll",			(vMeth**)&PADpoll,		NULL },
	{	"PADquery",			(vMeth**)&PADquery,		NULL },
	{	"PADkeyEvent",		(vMeth**)&PADkeyEvent,	NULL },

	// fixme - Following functions are new as of some revison post-0.9.6, and
	// are for multitap support only.  They should either be optional or offer
	// NOP fallbacks, to allow older plugins to retain functionality.
	{	"PADsetSlot",		(vMeth**)&PADsetSlot,	NULL },
	{	"PADqueryMtap",		(vMeth**)&PADqueryMtap,	NULL },
	{ NULL },
};

static const LegacyApi_OptMethod s_MethMessOpt_PAD[] =
{
	{	"PADupdate",		(vMeth**)&PADupdate },
	{ NULL },
};

// ----------------------------------------------------------------------------
//  CDVD Mess!
// ----------------------------------------------------------------------------
void CALLBACK CDVD_newDiskCB(void (*callback)()) {}

extern int lastReadSize, lastLSN;
static s32 CALLBACK CDVD_getBuffer2(u8* buffer)
{
	// TEMP: until I fix all the plugins to use this function style
	u8* pb = CDVD->getBuffer();
	if(pb == NULL) return -2;

	memcpy_fast( buffer, pb, lastReadSize );
	return 0;
}

static s32 CALLBACK CDVD_readSector(u8* buffer, u32 lsn, int mode)
{
	if(CDVD->readTrack(lsn,mode) < 0)
		return -1;

	// TEMP: until all the plugins use the new CDVDgetBuffer style
	switch (mode)
	{
	case CDVD_MODE_2352:
		lastReadSize = 2352;
		break;
	case CDVD_MODE_2340:
		lastReadSize = 2340;
		break;
	case CDVD_MODE_2328:
		lastReadSize = 2328;
		break;
	case CDVD_MODE_2048:
		lastReadSize = 2048;
		break;
	}

	lastLSN = lsn;
	return CDVD->getBuffer2(buffer);
}

static s32 CALLBACK CDVD_getDualInfo(s32* dualType, u32* layer1Start)
{
	u8 toc[2064];

	// if error getting toc, settle for single layer disc ;)
	if(CDVD->getTOC(toc))
		return 0;

	if(toc[14] & 0x60)
	{
		if(toc[14] & 0x10)
		{
			// otp dvd
			*dualType = 2;
			*layer1Start = (toc[25]<<16) + (toc[26]<<8) + (toc[27]) - 0x30000 + 1;
		}
		else
		{
			// ptp dvd
			*dualType = 1;
			*layer1Start = (toc[21]<<16) + (toc[22]<<8) + (toc[23]) - 0x30000 + 1;
		}
	}
	else
	{
		// single layer dvd
		*dualType = 0;
		*layer1Start = (toc[21]<<16) + (toc[22]<<8) + (toc[23]) - 0x30000 + 1;
	}

	return 1;
}

static void CALLBACK CDVDplugin_Close()
{
	g_plugins->Close( PluginId_CDVD );
}

CDVD_API CDVDapi_Plugin =
{
	CDVDplugin_Close,

	// The rest are filled in by the plugin manager
	NULL
};

CDVD_API* CDVD			= NULL;

static const LegacyApi_ReqMethod s_MethMessReq_CDVD[] =
{
	{	"CDVDopen",			(vMeth**)&CDVDapi_Plugin.open,			NULL },
	{	"CDVDreadTrack",	(vMeth**)&CDVDapi_Plugin.readTrack,		NULL },
	{	"CDVDgetBuffer",	(vMeth**)&CDVDapi_Plugin.getBuffer,		NULL },
	{	"CDVDreadSubQ",		(vMeth**)&CDVDapi_Plugin.readSubQ,		NULL },
	{	"CDVDgetTN",		(vMeth**)&CDVDapi_Plugin.getTN,			NULL },
	{	"CDVDgetTD",		(vMeth**)&CDVDapi_Plugin.getTD,			NULL },
	{	"CDVDgetTOC",		(vMeth**)&CDVDapi_Plugin.getTOC,		NULL },
	{	"CDVDgetDiskType",	(vMeth**)&CDVDapi_Plugin.getDiskType,	NULL },
	{	"CDVDgetTrayStatus",(vMeth**)&CDVDapi_Plugin.getTrayStatus,	NULL },
	{	"CDVDctrlTrayOpen",	(vMeth**)&CDVDapi_Plugin.ctrlTrayOpen,	NULL },
	{	"CDVDctrlTrayClose",(vMeth**)&CDVDapi_Plugin.ctrlTrayClose,	NULL },
	{	"CDVDnewDiskCB",	(vMeth**)&CDVDapi_Plugin.newDiskCB,		(vMeth*)CDVD_newDiskCB },

	{	"CDVDreadSector",	(vMeth**)&CDVDapi_Plugin.readSector,	(vMeth*)CDVD_readSector },
	{	"CDVDgetBuffer2",	(vMeth**)&CDVDapi_Plugin.getBuffer2,	(vMeth*)CDVD_getBuffer2 },
	{	"CDVDgetDualInfo",	(vMeth**)&CDVDapi_Plugin.getDualInfo,	(vMeth*)CDVD_getDualInfo },

	{ NULL }
};

static const LegacyApi_OptMethod s_MethMessOpt_CDVD[] =
{
	{ NULL }
};

// ----------------------------------------------------------------------------
//  SPU2 Mess!
// ----------------------------------------------------------------------------
static const LegacyApi_ReqMethod s_MethMessReq_SPU2[] =
{
	{	"SPU2open",				(vMeth**)&SPU2open,			NULL },
	{	"SPU2write",			(vMeth**)&SPU2write,		NULL },
	{	"SPU2read",				(vMeth**)&SPU2read,			NULL },
	{	"SPU2readDMA4Mem",		(vMeth**)&SPU2readDMA4Mem,	NULL },
	{	"SPU2readDMA7Mem",		(vMeth**)&SPU2readDMA7Mem,	NULL },
	{	"SPU2writeDMA4Mem",		(vMeth**)&SPU2writeDMA4Mem,	NULL },
	{	"SPU2writeDMA7Mem",		(vMeth**)&SPU2writeDMA7Mem,	NULL },
	{	"SPU2interruptDMA4",	(vMeth**)&SPU2interruptDMA4,NULL },
	{	"SPU2interruptDMA7",	(vMeth**)&SPU2interruptDMA7,NULL },
	{	"SPU2setDMABaseAddr",	(vMeth**)&SPU2setDMABaseAddr,NULL },
	{	"SPU2ReadMemAddr",		(vMeth**)&SPU2ReadMemAddr,	NULL },
	{	"SPU2irqCallback",		(vMeth**)&SPU2irqCallback,	NULL },

	{ NULL }
};

static const LegacyApi_OptMethod s_MethMessOpt_SPU2[] =
{
	{	"SPU2setClockPtr",		(vMeth**)&SPU2setClockPtr	},
	{	"SPU2async",			(vMeth**)&SPU2async			},
	{	"SPU2WriteMemAddr",		(vMeth**)&SPU2WriteMemAddr	},
	{	"SPU2setupRecording",	(vMeth**)&SPU2setupRecording},

	{ NULL }
};

// ----------------------------------------------------------------------------
//  DEV9 Mess!
// ----------------------------------------------------------------------------
static const LegacyApi_ReqMethod s_MethMessReq_DEV9[] =
{
	{	"DEV9open",			(vMeth**)&DEV9open,			NULL },
	{	"DEV9read8",		(vMeth**)&DEV9read8,		NULL },
	{	"DEV9read16",		(vMeth**)&DEV9read16,		NULL },
	{	"DEV9read32",		(vMeth**)&DEV9read32,		NULL },
	{	"DEV9write8",		(vMeth**)&DEV9write8,		NULL },
	{	"DEV9write16",		(vMeth**)&DEV9write16,		NULL },
	{	"DEV9write32",		(vMeth**)&DEV9write32,		NULL },
	{	"DEV9readDMA8Mem",	(vMeth**)&DEV9readDMA8Mem,	NULL },
	{	"DEV9writeDMA8Mem",	(vMeth**)&DEV9writeDMA8Mem,	NULL },
	{	"DEV9irqCallback",	(vMeth**)&DEV9irqCallback,	NULL },
	{	"DEV9irqHandler",	(vMeth**)&DEV9irqHandler,	NULL },

	{ NULL }
};

static const LegacyApi_OptMethod s_MethMessOpt_DEV9[] =
{
	{ NULL }
};

// ----------------------------------------------------------------------------
//  USB Mess!
// ----------------------------------------------------------------------------
static const LegacyApi_ReqMethod s_MethMessReq_USB[] =
{
	{	"USBopen",			(vMeth**)&USBopen,			NULL },
	{	"USBread8",			(vMeth**)&USBread8,			NULL },
	{	"USBread16",		(vMeth**)&USBread16,		NULL },
	{	"USBread32",		(vMeth**)&USBread32,		NULL },
	{	"USBwrite8",		(vMeth**)&USBwrite8,		NULL },
	{	"USBwrite16",		(vMeth**)&USBwrite16,		NULL },
	{	"USBwrite32",		(vMeth**)&USBwrite32,		NULL },
	{	"USBirqCallback",	(vMeth**)&USBirqCallback,	NULL },
	{	"USBirqHandler",	(vMeth**)&USBirqHandler,	NULL },
	{ NULL }
};

static const LegacyApi_OptMethod s_MethMessOpt_USB[] =
{
	{	"USBasync",		(vMeth**)&USBasync },
	{ NULL }
};

// ----------------------------------------------------------------------------
//  FW Mess!
// ----------------------------------------------------------------------------
static const LegacyApi_ReqMethod s_MethMessReq_FW[] =
{
	{	"FWopen",			(vMeth**)&FWopen,			NULL },
	{	"FWread32",			(vMeth**)&FWread32,			NULL },
	{	"FWwrite32",		(vMeth**)&FWwrite32,		NULL },
	{	"FWirqCallback",	(vMeth**)&FWirqCallback,	NULL },
	{ NULL }
};

static const LegacyApi_OptMethod s_MethMessOpt_FW[] =
{
	{ NULL }
};

static const LegacyApi_ReqMethod* const s_MethMessReq[] =
{
	s_MethMessReq_GS,
	s_MethMessReq_PAD,
	s_MethMessReq_SPU2,
	s_MethMessReq_CDVD,
	s_MethMessReq_USB,
	s_MethMessReq_FW,
	s_MethMessReq_DEV9
};

static const LegacyApi_OptMethod* const s_MethMessOpt[] =
{
	s_MethMessOpt_GS,
	s_MethMessOpt_PAD,
	s_MethMessOpt_SPU2,
	s_MethMessOpt_CDVD,
	s_MethMessOpt_USB,
	s_MethMessOpt_FW,
	s_MethMessOpt_DEV9
};

PluginManager *g_plugins = NULL;

//////////////////////////////////////////////////////////////////////////////////////////

Exception::PluginLoadError::PluginLoadError( PluginsEnum_t pid, const wxString& objname, const char* eng )
{
	BaseException::InitBaseEx( eng );
	StreamName = objname;
	PluginId = pid;
}

Exception::PluginLoadError::PluginLoadError( PluginsEnum_t pid, const wxString& objname,
	const wxString& eng_msg, const wxString& xlt_msg )
{
	BaseException::InitBaseEx( eng_msg, xlt_msg );
	StreamName = objname;
	PluginId = pid;
}

wxString Exception::PluginLoadError::FormatDiagnosticMessage() const
{
	return wxsFormat( m_message_diag, tbl_PluginInfo[PluginId].GetShortname().c_str() ) +
		L"\n\n" + StreamName;
}

wxString Exception::PluginLoadError::FormatDisplayMessage() const
{
	return wxsFormat( m_message_user, tbl_PluginInfo[PluginId].GetShortname().c_str() ) +
		L"\n\n" + StreamName;
}

wxString Exception::PluginError::FormatDiagnosticMessage() const
{
	return wxsFormat( m_message_diag, tbl_PluginInfo[PluginId].GetShortname().c_str() );
}

wxString Exception::PluginError::FormatDisplayMessage() const
{
	return wxsFormat( m_message_user, tbl_PluginInfo[PluginId].GetShortname().c_str() );
}

//////////////////////////////////////////////////////////////////////////////////////////

PluginManager::PluginManager( const wxString (&folders)[PluginId_Count] )
{
	const PluginInfo* pi = tbl_PluginInfo-1;
	while( ++pi, pi->shortname != NULL )
	{
		const PluginsEnum_t pid = pi->id;

		if( folders[pid].IsEmpty() )
			throw Exception::InvalidArgument( "Empty plugin filename." );

		m_info[pid].Filename = folders[pid];

		if( !wxFile::Exists( folders[pid] ) )
			throw Exception::PluginLoadError( pid, folders[pid],
				wxLt("The configured %s plugin file was not found")
			);

		if( !m_info[pid].Lib.Load( folders[pid] ) )
			throw Exception::PluginLoadError( pid, folders[pid],
				wxLt("The configured %s plugin file is not a valid dynamic library")
			);

		// Try to enumerate the new v2.0 plugin interface first.
		// If that fails, fall back on the old style interface.

		//m_libs[i].GetSymbol( L"PS2E_InitAPI" );

		// Bind Required Functions
		// (generate critical error if binding fails)

		BindCommon( pid );
		BindRequired( pid );
		BindOptional( pid );

		// Bind Optional Functions
		// (leave pointer null and do not generate error)
	}

	// Hack for PAD's stupid parameter passed on Init
	PADinit = (_PADinit)m_info[PluginId_PAD].CommonBindings.Init;
	m_info[PluginId_PAD].CommonBindings.Init = _hack_PADinit;
}

PluginManager::~PluginManager()
{
	Close();
	Shutdown();

	// All library unloading done automatically.
}

void PluginManager::BindCommon( PluginsEnum_t pid )
{
	const LegacyApi_CommonMethod* current = s_MethMessCommon;
	VoidMethod** target = (VoidMethod**)&m_info[pid].CommonBindings;

	wxDoNotLogInThisScope please;

	while( current->MethodName != NULL )
	{
		*target = (VoidMethod*)m_info[pid].Lib.GetSymbol( current->GetMethodName( pid ) );

		if( *target == NULL )
			*target = current->Fallback;

		if( *target == NULL )
		{
			throw Exception::PluginLoadError( pid, m_info[pid].Filename,
				wxLt( "Configured plugin is not a PCSX2 plugin, or is for an older unsupported version of PCSX2." ) );
		}

		target++;
		current++;
	}
}

void PluginManager::BindRequired( PluginsEnum_t pid )
{
	const LegacyApi_ReqMethod* current = s_MethMessReq[pid];
	const wxDynamicLibrary& lib = m_info[pid].Lib;

	wxDoNotLogInThisScope please;

	while( current->MethodName != NULL )
	{
		*(current->Dest) = (VoidMethod*)lib.GetSymbol( current->GetMethodName() );

		if( *(current->Dest) == NULL )
			*(current->Dest) = current->Fallback;

		if( *(current->Dest) == NULL )
		{
			throw Exception::PluginLoadError( pid, m_info[pid].Filename,
				wxLt( "Configured plugin is not a valid PCSX2 plugin, or is for an older unsupported version of PCSX2." ) );
		}

		current++;
	}
}

void PluginManager::BindOptional( PluginsEnum_t pid )
{
	const LegacyApi_OptMethod* current = s_MethMessOpt[pid];
	const wxDynamicLibrary& lib = m_info[pid].Lib;

	wxDoNotLogInThisScope please;

	while( current->MethodName != NULL )
	{
		*(current->Dest) = (VoidMethod*)lib.GetSymbol( current->GetMethodName() );
		current++;
	}
}

// Exceptions:
//   FileNotFound - Thrown if one of the configured plugins doesn't exist.
//   NotPcsxPlugin - Thrown if one of the configured plugins is an invalid or unsupported DLL

extern bool renderswitch;
extern void spu2DMA4Irq();
extern void spu2DMA7Irq();
extern void spu2Irq();

static bool OpenPlugin_CDVD()
{
	if( CDVDapi_Plugin.open( NULL ) ) return false;
	CDVDapi_Plugin.newDiskCB( cdvdNewDiskCB );
	return true;
}

static bool OpenPlugin_GS()
{
	if( mtgsThread == NULL )
	{
		mtgsOpen();	// mtgsOpen raises its own exception on error
		return true;
	}

	if( !mtgsThread->IsSelf() ) return true;	// already opened?
	
	return !GSopen( (void*)&pDsp, "PCSX2", renderswitch ? 2 : 1 );

	// Note: renderswitch is us abusing the isMultiThread parameter for that so
	// we don't need a new callback
}

static bool OpenPlugin_PAD()
{
	return !PADopen( (void*)&pDsp );
}

static bool OpenPlugin_SPU2()
{
	if( SPU2open( (void*)&pDsp ) ) return false;

	SPU2irqCallback( spu2Irq, spu2DMA4Irq, spu2DMA7Irq );
	if( SPU2setDMABaseAddr != NULL ) SPU2setDMABaseAddr((uptr)psxM);
	if( SPU2setClockPtr != NULL ) SPU2setClockPtr(&psxRegs.cycle);
	return true;
}

static bool OpenPlugin_DEV9()
{
	dev9Handler = NULL;

	if( DEV9open( (void*)&pDsp ) ) return false;
	DEV9irqCallback( dev9Irq );
	dev9Handler = DEV9irqHandler();
	return true;
}

static bool OpenPlugin_USB()
{
	usbHandler = NULL;

	if( USBopen( (void*)&pDsp ) ) return false;
	USBirqCallback( usbIrq );
	usbHandler = USBirqHandler();
	if( USBsetRAM != NULL )
		USBsetRAM(psxM);
	return true;
}

static bool OpenPlugin_FW()
{
	if( FWopen( (void*)&pDsp ) ) return false;
	FWirqCallback( fwIrq );
	return true;
}

void PluginManager::Open( PluginsEnum_t pid )
{
	if( m_info[pid].IsOpened ) return;

	// Each Open needs to be called explicitly. >_<

	bool result = true;
	switch( pid )
	{
		case PluginId_CDVD:	result = DoCDVDopen();		break;
		case PluginId_GS:	result = OpenPlugin_GS();	break;
		case PluginId_PAD:	result = OpenPlugin_PAD();	break;
		case PluginId_SPU2:	result = OpenPlugin_SPU2();	break;
		case PluginId_USB:	result = OpenPlugin_USB();	break;
		case PluginId_FW:	result = OpenPlugin_FW();	break;
		case PluginId_DEV9:	result = OpenPlugin_DEV9();	break;
	}
	if( !result )
		throw Exception::PluginOpenError( pid );

	m_info[pid].IsOpened = true;
}

void PluginManager::Open()
{
	const PluginInfo* pi = tbl_PluginInfo-1;
	while( ++pi, pi->shortname != NULL )
		g_plugins->Open( pi->id );
		
	cdvdDetectDisk();
}

void PluginManager::Close( PluginsEnum_t pid )
{
	if( !m_info[pid].IsOpened ) return;

	if( pid == PluginId_GS )
	{
		if( mtgsThread == NULL ) return;

		if( !mtgsThread->IsSelf() )
		{
			// force-close PAD before GS, because the PAD depends on the GS window.
			Close( PluginId_PAD );

			safe_delete( mtgsThread );
			return;
		}
	}
	else if( pid == PluginId_CDVD )
	{
		DoCDVDclose();
		return;
	}

	m_info[pid].IsOpened = false;
	m_info[pid].CommonBindings.Close();
}

void PluginManager::Close( bool closegs )
{
	// Close plugins in reverse order of the initialization procedure.

	for( int i=PluginId_Count-1; i>=0; --i )
	{
		if( closegs || (tbl_PluginInfo[i].id != PluginId_GS) )
			Close( tbl_PluginInfo[i].id );
	}
}

// Initializes all plugins.  Plugin initialization should be done once for every new emulation
// session.  During a session emulation can be paused/resumed using Open/Close, and should be
// terminated using Shutdown().
//
// In a purist emulation sense, Init() and Shutdown() should only ever need be called for when
// the PS2's hardware has received a *full* hard reset.  Soft resets typically should rely on
// the PS2's bios/kernel to re-initialize hardware on the fly.
//
void PluginManager::Init()
{
	const PluginInfo* pi = tbl_PluginInfo-1;
	while( ++pi, pi->shortname != NULL )
	{
		const PluginsEnum_t pid = pi->id;

		if( m_info[pid].IsInitialized ) continue;
		m_info[pid].IsInitialized = true;
		if( 0 != m_info[pid].CommonBindings.Init() )
			throw Exception::PluginInitError( pid );
	}
}

// Shuts down all plugins.  Plugins are closed first, if necessary.
//
// In a purist emulation sense, Init() and Shutdown() should only ever need be called for when
// the PS2's hardware has received a *full* hard reset.  Soft resets typically should rely on
// the PS2's bios/kernel to re-initialize hardware on the fly.
//
void PluginManager::Shutdown()
{
	Close();

	// Shutdown plugins in reverse order (probably doesn't matter...
	//  ... but what the heck, right?)

	for( int i=PluginId_Count-1; i>=0; --i )
	{
		const PluginsEnum_t pid = tbl_PluginInfo[i].id;
		if( !m_info[pid].IsInitialized ) continue;
		m_info[pid].IsInitialized = false;
		m_info[pid].CommonBindings.Shutdown();
	}
}

void PluginManager::Freeze( PluginsEnum_t pid, int mode, freezeData* data )
{
	m_info[pid].CommonBindings.Freeze( mode, data );
}

// ----------------------------------------------------------------------------
// Thread Safety:
//   This function should only be called by the Main GUI thread and the GS thread (for GS states only),
//   as it has special handlers to ensure that GS freeze commands are executed appropriately on the
//   GS thread.
//
void PluginManager::Freeze( PluginsEnum_t pid, SaveState& state )
{
	if( pid == PluginId_GS && wxThread::IsMain() )
	{
		// Need to send the GS freeze request on the GS thread.
	}
	else
	{
		state.FreezePlugin( tbl_PluginInfo[pid].shortname, m_info[pid].CommonBindings.Freeze );
	}
}

// ----------------------------------------------------------------------------
// This overload of Freeze performs savestate freeze operation on *all* plugins,
// as according to the order in PluignsEnum_t.
//
// Thread Safety:
//   This function should only be called by the Main GUI thread and the GS thread (for GS states only),
//   as it has special handlers to ensure that GS freeze commands are executed appropriately on the
//   GS thread.
//
void PluginManager::Freeze( SaveState& state )
{
	const PluginInfo* pi = tbl_PluginInfo-1;
	while( ++pi, pi->shortname != NULL )
		Freeze( pi->id, state );
}

void PluginManager::Configure( PluginsEnum_t pid )
{
	m_info[pid].CommonBindings.Configure();
}

// Creates an instance of a plugin manager, using the specified plugin filenames for sources.
// Impl Note: Have to use this stupid effing 'friend' declaration because static members of
// classes can't access their own protected members.  W-T-F?
//
PluginManager* PluginManager_Create( const wxString (&folders)[PluginId_Count] )
{
	PluginManager* retval = new PluginManager( folders );
	retval->Init();
	return retval;
}

PluginManager* PluginManager_Create( const wxChar* (&folders)[PluginId_Count] )
{
	wxString passins[PluginId_Count];

	const PluginInfo* pi = tbl_PluginInfo-1;
	while( ++pi, pi->shortname != NULL )
		passins[pi->id] = folders[pi->id];

	return PluginManager_Create( passins );
}

static PluginManagerBase s_pluginman_placebo;

// retrieves a handle to the current plugin manager.  Plugin manager is assumed to be valid,
// and debug-level assertions are performed on the validity of the handle.
PluginManagerBase& GetPluginManager()
{
	if( g_plugins == NULL ) return s_pluginman_placebo;
	return *g_plugins;
}
