/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2010  PCSX2 Dev Team
 *
 *  PCSX2 is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with PCSX2.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include "PrecompiledHeader.h"
#include "ConfigurationPanels.h"
#include "MemoryCardPanels.h"

#include "Dialogs/ConfigurationDialog.h"

#include "Utilities/IniInterface.h"

using namespace pxSizerFlags;
using namespace Panels;

// --------------------------------------------------------------------------------------
//  BaseMcdListView  (implementations)
// --------------------------------------------------------------------------------------

void BaseMcdListView::SetMcdProvider( IMcdList* face )
{
	m_CardProvider = face;
	SetCardCount( m_CardProvider ? m_CardProvider->GetLength() : 0 );
}

const IMcdList& BaseMcdListView::GetMcdProvider() const
{
	pxAssume( m_CardProvider );
	return *m_CardProvider;
}

void BaseMcdListView::SetTargetedItem( int sel )
{
	if( m_TargetedItem == sel ) return;

	if( m_TargetedItem >= 0 ) RefreshItem( m_TargetedItem );
	m_TargetedItem = sel;
	RefreshItem( sel );
}

void BaseMcdListView::LoadSaveColumns( IniInterface& ini )
{
	FastFormatUnicode label;
	uint colcnt = GetColumnCount();
	for( uint col=0; col<colcnt; ++col )
	{
		const ListViewColumnInfo& cinfo = GetDefaultColumnInfo(col);
		label.Clear();
		label.Write( L"ColumnWidth_%s", cinfo.name );
		int width = GetColumnWidth(col);
		ini.Entry( label, width, cinfo.width );
		
		if (ini.IsLoading())
			SetColumnWidth(col, width);
	}
}

// --------------------------------------------------------------------------------------
//  MemoryCardListView_Simple  (implementations)
// --------------------------------------------------------------------------------------
enum McdColumnType_Simple
{
	McdColS_PortSlot,	// port and slot of the card
	McdColS_Status,		// either Enabled/Disabled, or Missing (no card).
	McdColS_Size,
	McdColS_Formatted,
	McdColS_DateModified,
	McdColS_DateCreated,
	McdColS_Filename,
	McdColS_Count
};

MemoryCardListView_Simple::MemoryCardListView_Simple( wxWindow* parent )
	: _parent( parent )
{
	// I'd love to put this in the base class, and use the virtual GetDefaultColumnInfo method, but
	// you can't call virtual functions from constructors in C++ reliably.  -_-  --air
	CreateColumns();
}

void MemoryCardListView_Simple::CreateColumns()
{
	for( int i=0; i<McdColS_Count; ++i )
	{
		const ListViewColumnInfo& info = GetDefaultColumnInfo(i);
		InsertColumn( i, pxGetTranslation(info.name), info.align, info.width );
	}
}

const ListViewColumnInfo& MemoryCardListView_Simple::GetDefaultColumnInfo( uint idx ) const
{
	static const ListViewColumnInfo columns[] =
	{
		{ L"Slot",			48,		wxLIST_FORMAT_CENTER	},
		{ L"Status",		96,		wxLIST_FORMAT_CENTER	},
		{ L"Size",			72,		wxLIST_FORMAT_LEFT		},
		{ L"Formatted",		96,		wxLIST_FORMAT_CENTER	},
		{ L"Modified",		120,	wxLIST_FORMAT_LEFT		},
		{ L"Created",		120,	wxLIST_FORMAT_LEFT		},
		{ L"Filename",		256,	wxLIST_FORMAT_LEFT		},
	};

	pxAssumeDev( idx < ArraySize(columns), "ListView column index is out of bounds." );
	return columns[idx];
}


void MemoryCardListView_Simple::SetCardCount( int length )
{
	if( !m_CardProvider ) length = 0;
	SetItemCount( length );
	Refresh();
}

// return the text for the given column of the given item
wxString MemoryCardListView_Simple::OnGetItemText(long item, long column) const
{
	if( !m_CardProvider ) return _parent::OnGetItemText(item, column);

	const McdListItem& it( m_CardProvider->GetCard(item) );

	switch( column )
	{
		case McdColS_PortSlot:		return pxsFmt( L"%u", item+1);
		case McdColS_Status:		return it.IsPresent ? ( it.IsEnabled ? _("Enabled") : _("Disabled")) : _("Missing");
		case McdColS_Size:			return it.IsPresent ? pxsFmt( L"%u MB", it.SizeInMB ) : (wxString)_("N/A");
		case McdColS_Formatted:		return it.IsFormatted ? _("Yes") : _("No");
		case McdColS_DateModified:	return it.IsPresent ? it.DateModified.FormatDate()	: (wxString)_("N/A");
		case McdColS_DateCreated:	return it.IsPresent ? it.DateCreated.FormatDate()	: (wxString)_("N/A");

		case McdColS_Filename:
		{
			wxDirName filepath( it.Filename.GetPath() );
			
			if (filepath.SameAs(g_Conf->Folders.MemoryCards))
				return it.Filename.GetFullName();
			else
				return it.Filename.GetFullPath();
		}
	}

	pxFail( "Unknown column index in MemoryCardListView -- returning an empty string." );
	return wxEmptyString;
}

// return the icon for the given item. In report view, OnGetItemImage will
// only be called for the first column. See OnGetItemColumnImage for
// details.
int MemoryCardListView_Simple::OnGetItemImage(long item) const
{
	return _parent::OnGetItemImage( item );
}

// return the icon for the given item and column.
int MemoryCardListView_Simple::OnGetItemColumnImage(long item, long column) const
{
	return _parent::OnGetItemColumnImage( item, column );
}

static wxListItemAttr m_ItemAttr;

// return the attribute for the item (may return NULL if none)
wxListItemAttr* MemoryCardListView_Simple::OnGetItemAttr(long item) const
{
	//m_disabled.SetTextColour( wxLIGHT_GREY );
	//m_targeted.SetBackgroundColour( wxColour(L"Yellow") );

	if( !m_CardProvider ) return _parent::OnGetItemAttr(item);
	const McdListItem& it( m_CardProvider->GetCard(item) );

	m_ItemAttr = wxListItemAttr();		// Wipe it clean!

	if( !it.IsPresent )
		m_ItemAttr.SetTextColour( *wxLIGHT_GREY );

	if( m_TargetedItem == item )
		m_ItemAttr.SetBackgroundColour( wxColour(L"Wheat") );

	return &m_ItemAttr;
}