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
#include "ConfigurationPanels.h"

#include <wx/stdpaths.h>

using namespace wxHelpers;
static const int BetweenFolderSpace = 5;

// ------------------------------------------------------------------------
Panels::BasePathsPanel::BasePathsPanel( wxWindow& parent, int idealWidth ) :
	wxPanelWithHelpers( &parent, idealWidth-12 )
,	s_main( *new wxBoxSizer( wxVERTICAL ) )
{
}

Panels::DirPickerPanel& Panels::BasePathsPanel::AddDirPicker( wxBoxSizer& sizer,
	FoldersEnum_t folderid, const wxString& label, const wxString& popupLabel )
{
	DirPickerPanel* dpan = new DirPickerPanel( this, folderid, label, popupLabel );
	sizer.Add( dpan, SizerFlags::SubGroup() );
	return *dpan;
}

// ------------------------------------------------------------------------
Panels::StandardPathsPanel::StandardPathsPanel( wxWindow& parent, __unused int idealWidth ) :
	BasePathsPanel( parent )
{
	s_main.AddSpacer( BetweenFolderSpace );
	AddDirPicker( s_main, FolderId_Savestates,
		_("Savestates:"),
		_("Select folder for Savestates") ).
		SetToolTip( pxE( ".Tooltips:Folders:Savestates",
			L"This folder is where PCSX2 records savestates; which are recorded either by using "
			L"menus/toolbars, or by pressing F1/F3 (load/save)."
		) );

	s_main.AddSpacer( BetweenFolderSpace );
	AddDirPicker( s_main, FolderId_Snapshots,
		_("Snapshots:"),
		_("Select a folder for Snapshots") ).
		SetToolTip( pxE( ".Tooltips:Folders:Snapshots",
			L"This folder is where PCSX2 saves screenshots.  Actual screenshot image format and style "
			L"may vary depending on the GS plugin being used."
		) );

	s_main.AddSpacer( BetweenFolderSpace );
	AddDirPicker( s_main, FolderId_Logs,
		_("Logs/Dumps:" ),
		_("Select a folder for logs/dumps") ).
		SetToolTip( pxE( ".Tooltips:Folders:Logs",
			L"This folder is where PCSX2 saves its logfiles and diagnostic dumps.  Most plugins will "
			L"also adhere to this folder, however some older plugins may ignore it."
		) );

	s_main.AddSpacer( BetweenFolderSpace );
	AddDirPicker( s_main, FolderId_MemoryCards,
		_("Memorycards:"),
		_("Select a default Memorycards folder") ).
		SetToolTip( pxE( ".Tooltips:Folders:Memorycards",
			L"This is the default path where PCSX2 loads or creates its memory cards, and can be "
			L"overridden in the MemoryCard Configuration by using absolute filenames."
		) );

	s_main.AddSpacer( 5 );
	SetSizer( &s_main );
}

