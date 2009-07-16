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
#include "App.h"
#include "MainFrame.h"
#include "Utilities/Console.h"
#include "DebugTools/Debug.h"

#include <wx/file.h>
#include <wx/textfile.h>

// This code was 'borrowed' from wxWidgets built in console log class and then heavily
// modified to suite our needs.  I would have used some crafty subclassing instead except
// who ever wrote the code of wxWidgets had a peculiar love of the 'private' keyword,
// thus killing any possibility of subclassing in a useful manner.  (sigh)


DECLARE_EVENT_TYPE(wxEVT_LOG_Write, -1)
DECLARE_EVENT_TYPE(wxEVT_LOG_Newline, -1)
DECLARE_EVENT_TYPE(wxEVT_SetTitleText, -1)

DEFINE_EVENT_TYPE(wxEVT_LOG_Write)
DEFINE_EVENT_TYPE(wxEVT_LOG_Newline)
DEFINE_EVENT_TYPE(wxEVT_SetTitleText)
DEFINE_EVENT_TYPE(wxEVT_DockConsole)

using Console::Colors;

//////////////////////////////////////////////////////////////////////////////////////////
// pass an uninitialized file object, the function will ask the user for the
// filename and try to open it, returns true on success (file was opened),
// false if file couldn't be opened/created and -1 if the file selection
// dialog was canceled
//
static bool OpenLogFile(wxFile& file, wxString& filename, wxWindow *parent)
{
    filename = wxSaveFileSelector(L"log", L"txt", L"log.txt", parent);
    if ( !filename ) return false; // canceled

    if( wxFile::Exists(filename) )
    {
        bool bAppend = false;
        wxString strMsg;
        strMsg.Printf(L"Append log to file '%s' (choosing [No] will overwrite it)?",
                      filename.c_str());

        switch ( wxMessageBox(strMsg, L"Question", wxICON_QUESTION | wxYES_NO | wxCANCEL) )
		{
			case wxYES:
				bAppend = true;
			break;

			case wxNO:
				bAppend = false;
			break;

			case wxCANCEL:
				return false;

			default:
				wxFAIL_MSG( L"invalid message box return value" );
        }

		return ( bAppend ) ?
			file.Open(filename, wxFile::write_append) :
            file.Create(filename, true /* overwrite */);
    }

	return file.Create(filename);
}

// ------------------------------------------------------------------------
ConsoleLogFrame::ColorArray::ColorArray() :
	m_table( 8 )
{
	// Standard R, G, B format:
	new (&m_table[Color_Black]) wxTextAttr( wxColor(   0,   0,   0 ) );
	new (&m_table[Color_Red])	wxTextAttr( wxColor( 128,   0,   0 ) );
	new (&m_table[Color_Green])	wxTextAttr( wxColor(   0, 128,   0 ) );
	new (&m_table[Color_Blue])	wxTextAttr( wxColor(   0,   0, 128 ) );
	new (&m_table[Color_Yellow])wxTextAttr( wxColor( 180, 180,   0 ) );
	new (&m_table[Color_Cyan])	wxTextAttr( wxColor(   0, 160, 160 ) );
	new (&m_table[Color_Magenta])wxTextAttr( wxColor( 160,   0, 160 ) );
	new (&m_table[Color_White])	wxTextAttr( wxColor( 160, 160, 160 ) );
}

void ConsoleLogFrame::ColorArray::SetFont( const wxFont& font )
{
	for( int i=0; i<8; ++i )
		m_table[i].SetFont( font );
}

// ------------------------------------------------------------------------
ConsoleLogFrame::ConsoleLogFrame(MainEmuFrame *parent, const wxString& title) :
	wxFrame(parent, wxID_ANY, title)
,	m_TextCtrl( *new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
		wxTE_MULTILINE | wxHSCROLL | wxTE_READONLY | wxTE_RICH2 ) )
,	m_ColorTable()
,	m_curcolor( Color_Black )
{
	m_TextCtrl.SetBackgroundColour( wxColor( 238, 240, 248 ) ); //wxColor( 48, 48, 64 ) );

    // create menu
    wxMenuBar *pMenuBar = new wxMenuBar;
    wxMenu *pMenu = new wxMenu;
    pMenu->Append(wxID_SAVE,  L"&Save...", _("Save log contents to file"));
    pMenu->Append(wxID_CLEAR, L"C&lear", _("Clear the log contents"));
    pMenu->AppendSeparator();
    pMenu->Append(wxID_CLOSE, L"&Close", _("Close this window"));

    pMenuBar->Append(pMenu, L"&Log");
    SetMenuBar(pMenuBar);

    // status bar for menu prompts
    CreateStatusBar();
    ClearColor();
    
    // Sync with configuration settings:
    
	wxASSERT_MSG( g_Conf != NULL, L"Console Log Frames must be created AFTER the configuration has been loaded" );
	if( g_Conf != NULL )
	{
		SetSize( wxRect( g_Conf->ConLogBox.DisplayPosition, g_Conf->ConLogBox.DisplaySize ) );
		Show( g_Conf->ConLogBox.Visible );
	}

	// Bind Events:

	Connect( wxID_OPEN,  wxEVT_COMMAND_MENU_SELECTED, wxMenuEventHandler( ConsoleLogFrame::OnOpen ) );
    Connect( wxID_CLOSE, wxEVT_COMMAND_MENU_SELECTED, wxMenuEventHandler( ConsoleLogFrame::OnClose ) );
    Connect( wxID_SAVE,  wxEVT_COMMAND_MENU_SELECTED, wxMenuEventHandler( ConsoleLogFrame::OnSave ) );
    Connect( wxID_CLEAR, wxEVT_COMMAND_MENU_SELECTED, wxMenuEventHandler( ConsoleLogFrame::OnClear ) );

    Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler(ConsoleLogFrame::OnCloseWindow) );
	Connect( wxEVT_MOVE, wxMoveEventHandler(ConsoleLogFrame::OnMoveAround) );
	Connect( wxEVT_SIZE, wxSizeEventHandler(ConsoleLogFrame::OnResize) );
	
	Connect( wxEVT_LOG_Write, wxCommandEventHandler(ConsoleLogFrame::OnWrite) );
	Connect( wxEVT_LOG_Newline, wxCommandEventHandler(ConsoleLogFrame::OnNewline) );
	Connect( wxEVT_SetTitleText, wxCommandEventHandler(ConsoleLogFrame::OnSetTitle) );
	Connect( wxEVT_DockConsole, wxCommandEventHandler(ConsoleLogFrame::OnDockedMove) );
}

ConsoleLogFrame::~ConsoleLogFrame() { }

void ConsoleLogFrame::OnMoveAround( wxMoveEvent& evt )
{
	// Docking check!  If the window position is within some amount
	// of the main window, enable docking.

	if( wxFrame* main = wxGetApp().GetMainWindow() )
	{
		wxPoint topright( main->GetRect().GetTopRight() );
		wxRect snapzone( topright - wxSize( 8,8 ), wxSize( 16,16 ) );

		g_Conf->ConLogBox.AutoDock = snapzone.Contains( GetPosition() );
		if( g_Conf->ConLogBox.AutoDock )
		{
			SetPosition( topright + wxSize( 1,0 ) );
			g_Conf->ConLogBox.AutoDock = true;
		}
	}
	evt.Skip();
}

void ConsoleLogFrame::OnResize( wxSizeEvent& evt )
{
	g_Conf->ConLogBox.DisplaySize = GetSize();
	evt.Skip();
}

void ConsoleLogFrame::DoClose()
{
    // instead of closing just hide the window to be able to Show() it later
    Show(false);
	if( wxFrame* main = wxGetApp().GetMainWindow() )
		wxStaticCast( main, MainEmuFrame )->OnLogBoxHidden();
}

void ConsoleLogFrame::OnOpen(wxMenuEvent& WXUNUSED(event))
{
	Show(true);
}

void ConsoleLogFrame::OnClose( wxMenuEvent& event )
{
	
	DoClose();
}
void ConsoleLogFrame::OnCloseWindow(wxCloseEvent& event)
{
	if( event.CanVeto() )
		DoClose();
	else
		event.Skip();
}

void ConsoleLogFrame::OnSave(wxMenuEvent& WXUNUSED(event))
{
    wxString filename;
    wxFile file;
    bool rc = OpenLogFile( file, filename, this );
    if ( !rc )
    {
        // canceled
        return;
    }

    // retrieve text and save it
    // -------------------------
    int nLines = m_TextCtrl.GetNumberOfLines();
    for ( int nLine = 0; nLine < nLines; nLine++ )
    {
		if( !file.Write(m_TextCtrl.GetLineText(nLine) + wxTextFile::GetEOL()) )
		{
			wxLogError( L"Can't save log contents to file." );
			return;
		}
	}

	wxLogStatus(this, L"Log saved to the file '%s'.", filename.c_str());
}

void ConsoleLogFrame::OnClear(wxMenuEvent& WXUNUSED(event))
{
    m_TextCtrl.Clear();
}

void ConsoleLogFrame::SetColor( Colors color )
{
	m_TextCtrl.SetDefaultStyle( m_ColorTable[color] );
}

void ConsoleLogFrame::ClearColor()
{
	m_TextCtrl.SetDefaultStyle( m_ColorTable.Default() );
}

void ConsoleLogFrame::OnWrite( wxCommandEvent& event )
{
	Colors color = (Colors)event.GetExtraLong();
	
	if( color != m_curcolor )
		m_TextCtrl.SetDefaultStyle( m_ColorTable[m_curcolor=color] );

	Write( event.GetString() );
}

void ConsoleLogFrame::OnNewline( wxCommandEvent& event )
{
	Write( L"\n" );
}

void ConsoleLogFrame::OnSetTitle( wxCommandEvent& event )
{
	SetTitle( event.GetString() );
}

void ConsoleLogFrame::OnDockedMove( wxCommandEvent& event )
{
	if( g_Conf != NULL )
		SetPosition( g_Conf->ConLogBox.DisplayPosition );
}

void ConsoleLogFrame::Write( const wxString& text )
{
	// remove selection (WriteText is in fact ReplaceSelection)
#ifdef __WXMSW__
	wxTextPos nLen = m_TextCtrl.GetLastPosition();
	m_TextCtrl.SetSelection(nLen, nLen);
#endif

	m_TextCtrl.AppendText( text );
}

namespace Console
{
	__threadlocal Colors th_CurrentColor = Color_Black;

	// ------------------------------------------------------------------------
	void __fastcall SetTitle( const wxString& title )
	{
		wxCommandEvent evt( wxEVT_SetTitleText );
		evt.SetString( title );
		wxGetApp().ProgramLog_PostEvent( evt );
	}

	// ------------------------------------------------------------------------
	void __fastcall SetColor( Colors color )
	{
		th_CurrentColor = color;
	}

	// ------------------------------------------------------------------------
	void ClearColor()
	{
		th_CurrentColor = Color_Black;
	}

	// ------------------------------------------------------------------------
	bool Newline()
	{
		if( emuLog != NULL )
			fputs( "\n", emuLog );

		wxCommandEvent evt( wxEVT_LOG_Newline );
		wxGetApp().ProgramLog_PostEvent( evt );

		return false;
	}

	// ------------------------------------------------------------------------
	bool __fastcall Write( const char* fmt )
	{
		if( emuLog != NULL )
			fputs( fmt, emuLog );

		wxCommandEvent evt( wxEVT_LOG_Write );
		evt.SetString( wxString::FromAscii( fmt ) );
		evt.SetExtraLong( th_CurrentColor );
		wxGetApp().ProgramLog_PostEvent( evt );

		return false;
	}

	// ------------------------------------------------------------------------
	bool __fastcall Write( const wxString& fmt )
	{
		if( emuLog != NULL )
			fputs( fmt.ToAscii().data(), emuLog );

		wxCommandEvent evt( wxEVT_LOG_Write );
		evt.SetString( fmt );
		evt.SetExtraLong( th_CurrentColor );
		wxGetApp().ProgramLog_PostEvent( evt );

		return false;
	}
}

namespace Msgbox
{
	bool Alert( const wxString& text )
	{
		wxMessageBox( text, L"Pcsx2 Message", wxOK, wxGetApp().GetTopWindow() );
		return false;
	}

	bool OkCancel( const wxString& text )
	{
		int result = wxMessageBox( text, L"Pcsx2 Message", wxOK | wxCANCEL, wxGetApp().GetTopWindow() );
		return result == wxOK;
	}
}
