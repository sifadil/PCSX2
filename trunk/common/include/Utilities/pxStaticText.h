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

#pragma once

#include <wx/wx.h>
#include "wxGuiTools.h"

// --------------------------------------------------------------------------------------
//  pxStaticText
// --------------------------------------------------------------------------------------
// This class's purpose is to overcome two fundamental annoyances in wxWidgets design:
//
//   * An inability to wrap text to conform to a fitted window (a limitation imposed by
//     wxWidgets inability to fit individual directions, ie fit widths and then fit heights,
//     which would allow a textbox to wrap text to a sizer-determined width, and then grow
//     the sizers vertically to fit the calcuated text-wrapped height).
//
//   * Textbox alignment requires aligning both the textbox contents, and aligning the text
//     control within it's containing sizer.  If both alignment flags do not match the result
//     is typically undesirable.
//
class pxStaticText : public wxPanel
{
	typedef wxPanel _parent;

protected:
	wxAlignment		m_align;
	wxString		m_wrappedLabel;
	bool			m_autowrap;
	int				m_wrappedWidth;
	int				m_heightInLines;

	int				m_paddingPix_horiz;
	int				m_paddingPix_vert;
	float			m_paddingPct_horiz;
	float			m_paddingPct_vert;

protected:
	explicit pxStaticText( wxWindow* parent=NULL );

public:
	pxStaticText( wxWindow* parent, const wxString& label, wxAlignment align=wxALIGN_CENTRE_HORIZONTAL );
	pxStaticText( wxWindow* parent, int heightInLines, const wxString& label, wxAlignment align=wxALIGN_CENTRE_HORIZONTAL );
	virtual ~pxStaticText() throw() {}

	wxFont GetFontOk() const;

	virtual void SetLabel(const wxString& label);
	pxStaticText& SetHeight( int lines );
	pxStaticText& Bold();
	pxStaticText& WrapAt( int width );

	pxStaticText& Unwrapped();

	pxStaticText& PaddingPixH( int pixels );
	pxStaticText& PaddingPixV( int pixels );

	pxStaticText& PaddingPctH( float pct );
	pxStaticText& PaddingPctV( float pct );
	//pxStaticText& DoBestGuessHeight();

protected:
	void SetPaddingDefaults();
	void Init( const wxString& label );

	wxSize GetBestWrappedSize( const wxClientDC& dc ) const;
	wxSize DoGetBestSize() const;

	int calcPaddingWidth( int newWidth ) const;
	int calcPaddingHeight( int newHeight ) const;

	void paintEvent(wxPaintEvent& evt);

	void UpdateWrapping( bool textChanged );
	bool _updateWrapping( bool textChanged );
};


class pxStaticHeading : public pxStaticText
{
	typedef pxStaticText _parent;

public:
	pxStaticHeading( wxWindow* parent=NULL, const wxString& label=wxEmptyString );
	pxStaticHeading( wxWindow* parent, int heightInLines, const wxString& label=wxEmptyString );
	virtual ~pxStaticHeading() throw() {}

protected:
	void SetPaddingDefaults();
};
