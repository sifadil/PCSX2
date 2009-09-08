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
#include "HostGui.h"
#include "MainFrame.h"

#include "Dump.h"
#include "Elfheader.h"
#include "SaveState.h"
#include "GS.h"

// This API is likely obsolete for the most part, so I've just included a few dummies
// to keep things compiling until I can get to the point of tying up loose ends.

// renderswitch - tells GSdx to go into dx9 sw if "renderswitch" is set.
bool renderswitch = false;

namespace HostGui
{
	// Sets the status bar message without mirroring the output to the console.
	void SetStatusMsg( const wxString& text )
	{
		wxGetApp().GetMainWindow()->SetStatusText( text );
	}

	void Notice( const wxString& text )
	{
		// mirror output to the console!
		Console::Status( text.c_str() );
		SetStatusMsg( text );
	}
}

static const int NUM_STATES = 10;
static int StatesC = 0;
static int g_Pcsx2Recording = 0; // true 1 if recording video and sound

static wxString GetGSStateFilename()
{
	return Path::Combine( g_Conf->Folders.Savestates, wxsFormat( L"/%8.8X.%d.gs", ElfCRC, StatesC ) );
}

void Pcsx2App::OnKeyDown( wxKeyEvent& evt )
{
	switch( evt.GetKeyCode() )
	{
		case WXK_ESCAPE:
			if( g_Conf->CloseGSonEsc )
				StateRecovery::MakeGsOnly();

			SysEndExecution();
		break;
	
		case WXK_F1:
			States_Save( StatesC );
		break;

		case WXK_F2:
			if( evt.GetModifiers() & wxMOD_SHIFT )
				StatesC = (StatesC+NUM_STATES-1) % NUM_STATES;
			else
				StatesC = (StatesC+1) % NUM_STATES;

			Console::Notice( " > Selected savestate slot %d", StatesC);

			if( GSchangeSaveState != NULL )
				GSchangeSaveState(StatesC, SaveState::GetFilename(StatesC).mb_str());
		break;

		case WXK_F3:
			States_Load( StatesC );
		break;

		case WXK_F4:
			// FIXME : Reimplement framelimiting using new oolean system
			//CycleFrameLimit(keymod->shift ? -1 : 1);
		break;

		// note: VK_F5-VK_F7 are reserved for GS
		case WXK_F8:
			GSmakeSnapshot( g_Conf->Folders.Snapshots.ToAscii().data() );
		break;

		case WXK_F9: //gsdx "on the fly" renderer switching

			SysSuspend();
			StateRecovery::MakeGsOnly();

			GetPluginManager().Close( PluginId_GS );
			renderswitch = !renderswitch;

			StateRecovery::Recover();
			SysResume();
		break;
			
#ifdef PCSX2_DEVBUILD
		case WXK_F10:
			// There's likely a better way to implement this, but this seemed useful.
			// I might add turning EE, VU0, and VU1 recs on and off by hotkey at some point, too.
			// --arcum42
			enableLogging = !enableLogging;

			if (enableLogging)
				GSprintf(10, "Logging Enabled.");
			else
				GSprintf(10,"Logging Disabled.");

			break;

		case WXK_F11:
			Console::Notice( "Cannot make gsstates in MTGS mode" );

			// fixme : fix up gsstate mess and make it mtgs compatible -- air
#ifdef _STGS_GSSTATE_CODE
			wxString Text;
			if( strgametitle[0] != 0 )
			{
				// only take the first two words
				wxString gsText;

				wxStringTokenizer parts( strgametitle, L" " );

				wxString name( parts.GetNextToken() );	// first part
				wxString part2( parts.GetNextToken() );

				if( !!part2 )
					name += L"_" + part2;

				gsText.Printf( L"%s.%d.gs", name.c_str(), StatesC );
				Text = Path::Combine( g_Conf->Folders.Savestates, gsText );
			}
			else
			{
				Text = GetGSStateFilename();
			}
			break;
#endif
#endif

		case WXK_F12:
			if( evt.GetModifiers() & wxMOD_SHIFT )
			{
#ifdef PCSX2_DEVBUILD
				iDumpRegisters(cpuRegs.pc, 0);
				Console::Notice("hardware registers dumped EE:%x, IOP:%x\n", cpuRegs.pc, psxRegs.pc);
#endif
			}
			else
			{
				g_Pcsx2Recording ^= 1;

				mtgsThread->SendSimplePacket(GS_RINGTYPE_RECORD, g_Pcsx2Recording, 0, 0);
				if( SPU2setupRecording != NULL ) SPU2setupRecording(g_Pcsx2Recording, NULL);
			}
			break;
	}
}
