/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2009  PCSX2 Dev Team
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
#include "MainFrame.h"

AppCoreThread::AppCoreThread( PluginManager& plugins ) :
	SysCoreThread( plugins )
,	m_kevt()
{
}

AppCoreThread::~AppCoreThread() throw()
{
}

void AppCoreThread::Suspend( bool isBlocking )
{
	_parent::Suspend( isBlocking );
	if( HasMainFrame() )
		GetMainFrame().ApplySettings();

	// Clear the sticky key statuses, because hell knows what'll change while the PAD
	// plugin is suspended.

	m_kevt.m_shiftDown		= false;
	m_kevt.m_controlDown	= false;
	m_kevt.m_altDown		= false;
}

void AppCoreThread::Resume()
{
	// Thread control (suspend / resume) should only be performed from the main/gui thread.
	if( !AllowFromMainThreadOnly() ) return;

	if( sys_resume_lock )
	{
		Console.WriteLn( "SysResume: State is locked, ignoring Resume request!" );
		return;
	}
	_parent::Resume();
}

void AppCoreThread::OnResumeReady()
{
	if( m_shortSuspend ) return;

	ApplySettings( g_Conf->EmuOptions );

	if( GSopen2 != NULL )
		wxGetApp().OpenGsFrame();

	if( HasMainFrame() )
		GetMainFrame().ApplySettings();
}

// Called whenever the thread has terminated, for either regular or irregular reasons.
// Typically the thread handles all its own errors, so there's no need to have error
// handling here.  However it's a good idea to update the status of the GUI to reflect
// the new (lack of) thread status, so this posts a message to the App to do so.
void AppCoreThread::OnThreadCleanup()
{
	wxCommandEvent evt( pxEVT_AppCoreThreadFinished );
	wxGetApp().AddPendingEvent( evt );
	_parent::OnThreadCleanup();
}

#ifdef __WXGTK__
	extern int TranslateGDKtoWXK( u32 keysym );
#endif

void AppCoreThread::StateCheck( bool isCancelable )
{
	_parent::StateCheck( isCancelable );

	const keyEvent* ev = PADkeyEvent();
	if( ev == NULL || (ev->key == 0) ) return;

	m_plugins.KeyEvent( *ev );
	m_kevt.SetEventType( ( ev->evt == KEYPRESS ) ? wxEVT_KEY_DOWN : wxEVT_KEY_UP );
	const bool isDown = (ev->evt == KEYPRESS);

	#ifdef __WXMSW__
		const int vkey = wxCharCodeMSWToWX( ev->key );
	#elif defined( __WXGTK__ )
		const int vkey = TranslateGDKtoWXK( ev->key );
	#else
	#	error Unsupported Target Platform.
	#endif

	switch (vkey)
	{
		case WXK_SHIFT:		m_kevt.m_shiftDown		= isDown; return;
		case WXK_CONTROL:	m_kevt.m_controlDown	= isDown; return;
		case WXK_MENU:		m_kevt.m_altDown		= isDown; return;
	}

	m_kevt.m_keyCode = vkey;
	wxGetApp().PostPadKey( m_kevt );
}

// To simplify settings application rules and re-entry conditions, the main App's implementation
// of ApplySettings requires that the caller manually ensure that the thread has been properly
// suspended.  If the thread has mot been suspended, this call will fail *silently*.
void AppCoreThread::ApplySettings( const Pcsx2Config& src )
{
	if( !IsSuspended() ) return;

	// Re-entry guard protects against cases where code wants to manually set core settings
	// which are not part of g_Conf.  The subsequent call to apply g_Conf settings (which is
	// usually the desired behavior) will be ignored.

	static int localc = 0;
	RecursionGuard guard( localc );
	if(guard.IsReentrant()) return;
	SysCoreThread::ApplySettings( src );
}

void AppCoreThread::ExecuteTask()
{
	try
	{
		SysCoreThread::ExecuteTask();
	}
	// ----------------------------------------------------------------------------
	catch( Exception::FileNotFound& ex )
	{
		m_plugins.Close();
		if( ex.StreamName == g_Conf->FullpathToBios() )
		{
			m_plugins.Close();
			bool result = Msgbox::OkCancel( ex.FormatDisplayMessage() +
				_("\n\nPress Ok to go to the BIOS Configuration Panel.") );

			if( result )
			{
				if( wxGetApp().ThreadedModalDialog( DialogId_BiosSelector ) == wxID_CANCEL )
				{
					// fixme: handle case where user cancels the settings dialog. (should return FALSE).
				}
				else
				{
					// fixme: automatically re-try emu startup here...
				}
			}
		}
	}
	// ----------------------------------------------------------------------------
	catch( Exception::PluginError& ex )
	{
		m_plugins.Close();
		Console.Error( ex.FormatDiagnosticMessage() );
		Msgbox::Alert( ex.FormatDisplayMessage(), _("Plugin Open Error") );

		/*if( HandlePluginError( ex ) )
		{
		// fixme: automatically re-try emu startup here...
		}*/
	}
	// ----------------------------------------------------------------------------
	// [TODO] : Add exception handling here for debuggable PS2 exceptions that allows
	// invocation of the PCSX2 debugger and such.
	//
	catch( Exception::BaseException& ex )
	{
		// Sent the exception back to the main gui thread?
		m_plugins.Close();
		Msgbox::Alert( ex.FormatDisplayMessage() );
	}
}
