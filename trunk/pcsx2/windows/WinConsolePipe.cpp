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
#include "Win32.h"

#include "App.h"
#include "ConsoleLogger.h"


// --------------------------------------------------------------------------------------
//  Exception::Win32Error
// --------------------------------------------------------------------------------------

namespace Exception
{
	class Win32Error : public RuntimeError
	{
	public:
		int		ErrorId;
	
	public:
		DEFINE_EXCEPTION_COPYTORS( Win32Error )

		Win32Error( const char* msg="" )
		{
			ErrorId = GetLastError();
			BaseException::InitBaseEx( msg );
		}
		
		wxString GetMsgFromWindows() const
		{
			if (!ErrorId)
				return wxString();

			const DWORD BUF_LEN = 2048;
			TCHAR t_Msg[BUF_LEN];
			if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, ErrorId, 0, t_Msg, BUF_LEN, 0))
				return wxsFormat( L"Win32 Error #%d: %s", ErrorId, t_Msg );

			return wxsFormat( L"Win32 Error #%d (no text msg available)", ErrorId );
		}

		virtual wxString FormatDisplayMessage() const
		{
			return m_message_user + L"\n\n" + GetMsgFromWindows();
		}

		virtual wxString FormatDiagnosticMessage() const
		{
			return m_message_diag + L"\n\t" + GetMsgFromWindows();
		}
	};
}

// --------------------------------------------------------------------------------------
//  Win32 Console Pipes
//  As a courtesy and convenience, we redirect stdout/stderr to the console and logfile.
// --------------------------------------------------------------------------------------

using namespace Threading;

static __forceinline void __CreatePipe( HANDLE& ph_ReadPipe, HANDLE& ph_WritePipe )
{
	SECURITY_ATTRIBUTES k_Secur;
	k_Secur.nLength              = sizeof(SECURITY_ATTRIBUTES);
	k_Secur.lpSecurityDescriptor = 0;
	k_Secur.bInheritHandle       = TRUE;

	if( 0 == CreatePipe( &ph_ReadPipe, &ph_WritePipe, &k_Secur, 2048 ) )
		throw Exception::Win32Error( "CreatePipe failed." );

	if (!ConnectNamedPipe(ph_ReadPipe, NULL))
	{
		if (GetLastError() != ERROR_PIPE_CONNECTED)
			throw Exception::Win32Error( "ConnectNamedPipe failed." );
	}

	SetHandleInformation(ph_WritePipe, HANDLE_FLAG_INHERIT, 0);
}

// Reads from the Pipe and appends the read data to ps_Data
// returns TRUE if something was printed to console, or false if the stdout/err were idle.
static __forceinline bool ReadPipe(HANDLE h_Pipe, ConsoleColors color )
{
	if( h_Pipe == INVALID_HANDLE_VALUE ) return false;

	// IMPORTANT: Check if there is data that can be read.
	// The first console output will be lost if ReadFile() is called before data becomes available!
	// It does not make any sense but the following 5 lines are indispensable!!
	DWORD u32_Avail = 0;
	if (!PeekNamedPipe(h_Pipe, 0, 0, 0, &u32_Avail, 0))
		throw Exception::Win32Error( "Error peeking Pipe." );

	if (!u32_Avail)
		return false;

	char s8_Buf[2049];
	DWORD u32_Read = 0;
	do
	{
		if (!ReadFile(h_Pipe, s8_Buf, sizeof(s8_Buf)-1, &u32_Read, NULL))
		{
			if (GetLastError() != ERROR_IO_PENDING)
				throw Exception::Win32Error( "ReadFile from pipe failed." );
		}

		// ATTENTION: The Console always prints ANSI to the pipe independent if compiled as UNICODE or MBCS!
		s8_Buf[u32_Read] = 0;
		OemToCharA(s8_Buf, s8_Buf);			// convert DOS codepage -> ANSI
		Console.Write( color, s8_Buf );	// convert ANSI -> Unicode if compiled as Unicode
	}
	while (u32_Read == sizeof(s8_Buf)-1);

	return true;
}

// --------------------------------------------------------------------------------------
//  WinPipeThread
// --------------------------------------------------------------------------------------
class WinPipeThread : public PersistentThread
{
	typedef PersistentThread _parent;

protected:
	const HANDLE& m_outpipe;
	const ConsoleColors m_color;

public:
	WinPipeThread( const HANDLE& outpipe, ConsoleColors color ) :
		m_outpipe( outpipe )
	,	m_color( color )
	{
		m_name = (m_color == Color_Red) ? L"Redirect_Stderr" : L"Redirect_Stdout";
	}
	
	virtual ~WinPipeThread() throw()
	{
		_parent::Cancel();
	}
	
protected:
	void OnStart() {}

	void ExecuteTaskInThread()
	{
		try
		{
			SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL );
			while( true )
			{
				Yield( 100 );
				ReadPipe( m_outpipe, m_color );
			}
		}
		catch( Exception::RuntimeError& ex )
		{
			// Log error, and fail silently.  It's not really important if the
			// pipe fails.  PCSX2 will run fine without it in any case.
			Console.Error( ex.FormatDiagnosticMessage() );
		}
	}

	void OnCleanupInThread() { }
};

// --------------------------------------------------------------------------------------
//  WinPipeRedirection
// --------------------------------------------------------------------------------------
class WinPipeRedirection : public PipeRedirectionBase
{
	DeclareNoncopyableObject( WinPipeRedirection );

protected:
	HANDLE		m_readpipe;
	HANDLE		m_writepipe;
	int			m_crtFile;
	FILE*		m_fp;

	WinPipeThread m_Thread;

public:
	WinPipeRedirection( FILE* stdstream );
	virtual ~WinPipeRedirection() throw();
	
	void Cleanup() throw();
};

WinPipeRedirection::WinPipeRedirection( FILE* stdstream ) :
	m_readpipe(INVALID_HANDLE_VALUE)
,	m_writepipe(INVALID_HANDLE_VALUE)
,	m_crtFile(-1)
,	m_fp(NULL)
,	m_Thread( m_readpipe, (stdstream == stderr) ? Color_Red : Color_Black )
{
	try
	{
		pxAssert( (stdstream == stderr) || (stdstream == stdout) );
		DWORD stdhandle = ( stdstream == stderr ) ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE;

		__CreatePipe( m_readpipe, m_writepipe );
		if( 0 == SetStdHandle( stdhandle, m_writepipe ) )
			throw Exception::Win32Error( "SetStdHandle failed." );

		// In some cases GetStdHandle can fail, even when the one we just assigned above is valid.
		HANDLE newhandle = GetStdHandle(stdhandle);
		if( newhandle == INVALID_HANDLE_VALUE )
			throw Exception::Win32Error( "GetStdHandle failed." );

		if( newhandle == NULL )
			throw Exception::RuntimeError( "GetStdHandle returned NULL." );		// not a Win32error (no error code)

		m_crtFile = _open_osfhandle( (intptr_t)newhandle, _O_TEXT );
		if( m_crtFile == -1 ) 
			throw Exception::RuntimeError( "_open_osfhandle returned -1." );

		m_fp = _fdopen( m_crtFile, "w" );
		if( m_fp == NULL )
			throw Exception::RuntimeError( "_fdopen returned NULL." );

		*stdstream = *m_fp;
		setvbuf( stdstream, NULL, _IONBF, 0 );

		m_Thread.Start();
	}
	catch( Exception::BaseException& ex )
	{
		Cleanup();
		ex.DiagMsg() = (wxString)((stdstream==stdout) ? L"STDOUT" : L"STDERR") + L" Redirection Init failed: " + ex.DiagMsg();
		throw;
	}
	catch( ... )
	{
		throw;
	}
}

WinPipeRedirection::~WinPipeRedirection()
{
	Cleanup();
}

void WinPipeRedirection::Cleanup() throw()
{
	m_Thread.Cancel();

	if( m_fp != NULL )
	{
		fclose( m_fp );
		m_fp = NULL;

		m_crtFile	= -1;						// crtFile is closed implicitly when closing m_fp
	}
	
	if( m_crtFile != -1 )
	{
		_close( m_crtFile );
		m_crtFile	= -1;		// m_file is closed implicitly when closing crtFile
	}

	if( m_readpipe != INVALID_HANDLE_VALUE )
	{
		CloseHandle( m_readpipe );
		m_readpipe = m_writepipe = INVALID_HANDLE_VALUE;
	}

	if( m_writepipe != INVALID_HANDLE_VALUE )
	{
		CloseHandle( m_writepipe );
		m_readpipe = m_writepipe = INVALID_HANDLE_VALUE;
	}
}

// The win32 specific implementation of PipeRedirection.
PipeRedirectionBase* NewPipeRedir( FILE* stdstream )
{
	try
	{
		return new WinPipeRedirection( stdstream );
	}
	catch( Exception::RuntimeError& ex )
	{
		// Entirely non-critical errors.  Log 'em and move along.
		Console.Error( ex.FormatDiagnosticMessage() );
	}
	
	return NULL;
}
