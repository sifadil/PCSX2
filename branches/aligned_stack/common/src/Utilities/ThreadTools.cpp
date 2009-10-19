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

#ifdef _WIN32
#	include <wx/msw/wrapwin.h>	// for thread renaming features
#endif

#ifdef __LINUX__
#	include <signal.h>		// for pthread_kill, which is in pthread.h on w32-pthreads
#endif

#include "Threading.h"
#include "wxBaseTools.h"
#include "ThreadingInternal.h"

// 100ms interval for waitgui (issued from blocking semaphore waits on the main thread,
// to avoid gui deadlock).
const wxTimeSpan	Threading::def_yieldgui_interval( 0, 0, 0, 100 );

// three second interval for deadlock protection on waitgui.
const wxTimeSpan	Threading::def_deadlock_timeout( 0, 0, 3, 0 );

// (intended for internal use only)
// Returns true if the Wait is recursive, or false if the Wait is safe and should be
// handled via normal yielding methods.
bool Threading::_WaitGui_RecursionGuard( const char* guardname )
{
	// In order to avoid deadlock we need to make sure we cut some time to handle messages.
	// But this can result in recursive yield calls, which would crash the app.  Protect
	// against them here and, if recursion is detected, perform a standard blocking wait.

	static int __Guard = 0;
	RecursionGuard guard( __Guard );

	if( guard.IsReentrant() )
	{
		Console.WriteLn( "(Thread Log) Possible yield recursion detected in %s; performing blocking wait.", guardname );
		return true;
	}
	return false;
}

__forceinline void Threading::Timeslice()
{
	sched_yield();
}

void Threading::PersistentThread::_pt_callback_cleanup( void* handle )
{
	((PersistentThread*)handle)->_ThreadCleanup();
}

Threading::PersistentThread::PersistentThread() :
	m_name( L"PersistentThread" )
,	m_thread()
,	m_sem_event()
,	m_lock_InThread()
,	m_lock_start()
,	m_detached( true )		// start out with m_thread in detached/invalid state
,	m_running( false )
{
}

// This destructor performs basic "last chance" cleanup, which is a blocking join
// against the thread. Extending classes should almost always implement their own
// thread closure process, since any PersistentThread will, by design, not terminate
// unless it has been properly canceled.
//
// Thread safety: This class must not be deleted from its own thread.  That would be
// like marrying your sister, and then cheating on her with your daughter.
Threading::PersistentThread::~PersistentThread() throw()
{
	try
	{
		DevCon.WriteLn( L"(Thread Log) Executing destructor for " + m_name );

		if( m_running )
		{
			DevCon.WriteLn( L"\tWaiting for running thread to end...");
			m_lock_InThread.Wait();
		}
		Threading::Sleep( 1 );
		Detach();
	}
	catch( Exception::ThreadTimedOut& ex )
	{
		// Windows allows for a thread to be terminated forcefully, but it's not really
		// a safe thing to do since typically threads are acquiring and releasing locks
		// and semaphores all the time.  And terminating threads isn't really cross-platform
		// either so let's just not bother.

		// Additionally since this is a destructor most of our derived class info is lost,
		// so we can't allow for customized deadlock handlers, least not in any useful
		// context.  So let's just log the condition and move on.

		Console.Error( wxsFormat(L"\tThread destructor for '%s' timed out with error:\n\t",
			m_name.c_str(), ex.FormatDiagnosticMessage().c_str() ) );
	}
	DESTRUCTOR_CATCHALL
}

void Threading::PersistentThread::FrankenMutex( MutexLock& mutex )
{
	if( mutex.RecreateIfLocked() )
	{
		// Our lock is bupkis, which means  the previous thread probably deadlocked.
		// Let's create a new mutex lock to replace it.

		Console.Error( wxsFormat(
			L"(Thread Log) Possible deadlock detected on restarted mutex belonging to '%s'.", m_name.c_str() )
		);
	}
}

// Main entry point for starting or e-starting a persistent thread.  This function performs necessary
// locks and checks for avoiding race conditions, and then calls OnStart() immediately before
// the actual thread creation.  Extending classes should generally not override Start(), and should
// instead override DoPrepStart instead.
//
// This function should not be called from the owner thread.
void Threading::PersistentThread::Start()
{
	// Prevents sudden parallel startup, and or parallel startup + cancel:
	ScopedLock startlock( m_lock_start );
	if( m_running ) return;

	Detach();		// clean up previous thread handle, if one exists.
	OnStart();

	if( pthread_create( &m_thread, NULL, _internal_callback, this ) != 0 )
		throw Exception::ThreadCreationError();

	m_detached = false;
}

// Returns: TRUE if the detachment was performed, or FALSE if the thread was
// already detached or isn't running at all.
// This function should not be called from the owner thread.
bool Threading::PersistentThread::Detach()
{
	pxAssertMsg( !IsSelf(), "Thread affinity error." );		// not allowed from our own thread.

	if( _InterlockedExchange( &m_detached, true ) ) return false;
	pthread_detach( m_thread );
	return true;
}

// Remarks:
//   Provision of non-blocking Cancel() is probably academic, since destroying a PersistentThread
//   object performs a blocking Cancel regardless of if you explicitly do a non-blocking Cancel()
//   prior, since the ExecuteTaskInThread() method requires a valid object state.  If you really need
//   fire-and-forget behavior on threads, use pthreads directly for now.
//
// This function should not be called from the owner thread.
//
// Parameters:
//   isBlocking - indicates if the Cancel action should block for thread completion or not.
//
void Threading::PersistentThread::Cancel( bool isBlocking )
{
	pxAssertMsg( !IsSelf(), "Thread affinity error." );

	{
		// Prevent simultaneous startup and cancel:
		ScopedLock startlock( m_lock_start );
		if( !m_running ) return;

		if( m_detached )
		{
			Console.Notice( "(Thread Warning) Ignoring attempted cancellation of detached thread." );
			return;
		}

		pthread_cancel( m_thread );

	}

	if( isBlocking )
	{
		m_lock_InThread.Wait();
		Detach();
	}
}

// Blocks execution of the calling thread until this thread completes its task.  The
// caller should make sure to signal the thread to exit, or else blocking may deadlock the
// calling thread.  Classes which extend PersistentThread should override this method
// and signal any necessary thread exit variables prior to blocking.
//
// Returns the return code of the thread.
// This method is roughly the equivalent of pthread_join().
//
void Threading::PersistentThread::Block()
{
	pxAssertDev( !IsSelf(), "Thread deadlock detected; Block() should never be called by the owner thread." );

	m_lock_InThread.Wait();
}

bool Threading::PersistentThread::IsSelf() const
{
	// Detached threads may have their pthread handles recycled as newer threads, causing
	// false IsSelf reports.
	return !m_detached && (pthread_self() == m_thread);
}

bool Threading::PersistentThread::IsRunning() const
{
    return !!m_running;
}

// Throws an exception if the thread encountered one.  Uses the BaseException's Rethrow() method,
// which ensures the exception type remains consistent.  Debuggable stacktraces will be lost, since
// the thread will have allowed itself to terminate properly.
void Threading::PersistentThread::RethrowException() const
{
	if( !m_except ) return;
	m_except->Rethrow();
}

// This helper function is a deadlock-safe method of waiting on a semaphore in a PersistentThread.  If the
// thread is terminated or canceled by another thread or a nested action prior to the semaphore being
// posted, this function will detect that and throw a ThreadTimedOut exception.
//
// Note: Use of this function only applies to semaphores which are posted by the worker thread.  Calling
// this function from the context of the thread itself is an error, and a dev assertion will be generated.
//
// Exceptions:
//   ThreadTimedOut
//
void Threading::PersistentThread::WaitOnSelf( Semaphore& sem )
{
	if( !pxAssertDev( !IsSelf(), "WaitOnSelf called from inside the thread (invalid operation!)" ) ) return;

	while( true )
	{
		if( sem.Wait( wxTimeSpan(0, 0, 0, 250) ) ) return;
		if( !m_running )
		{
			wxString msg( m_name + L": thread was terminated while another thread was waiting on a semaphore." );
			throw Exception::ThreadTimedOut( msg, msg );
		}
	}
}

// This helper function is a deadlock-safe method of waiting on a mutex in a PersistentThread.  If the
// thread is terminated or canceled by another thread or a nested action prior to the mutex being
// unlocked, this function will detect that and throw a ThreadTimedOut exception.
//
// Note: Use of this function only applies to semaphores which are posted by the worker thread.  Calling
// this function from the context of the thread itself is an error, and a dev assertion will be generated.
//
// Exceptions:
//   ThreadTimedOut
//
void Threading::PersistentThread::WaitOnSelf( MutexLock& mutex )
{
	if( !pxAssertDev( !IsSelf(), "WaitOnSelf called from inside the thread (invalid operation!)" ) ) return;

	while( true )
	{
		if( mutex.Wait( wxTimeSpan(0, 0, 0, 250) ) ) return;
		if( !m_running )
		{
			wxString msg( m_name + L": thread was terminated while another thread was waiting on a mutex." );
			throw Exception::ThreadTimedOut( msg, msg );
		}
	}
}


// Inserts a thread cancellation point.  If the thread has received a cancel request, this
// function will throw an SEH exception designed to exit the thread (so make sure to use C++
// object encapsulation for anything that could leak resources, to ensure object unwinding
// and cleanup, or use the DoThreadCleanup() override to perform resource cleanup).
void Threading::PersistentThread::TestCancel()
{
	pxAssert( IsSelf() );
	pthread_testcancel();
}

// Executes the virtual member method
void Threading::PersistentThread::_try_virtual_invoke( void (PersistentThread::*method)() )
{
	try {
		(this->*method)();
	}

	// ----------------------------------------------------------------------------
	// Neat repackaging for STL Runtime errors...
	//
	catch( std::runtime_error& ex )
	{
		m_except = new Exception::RuntimeError(
			// Diagnostic message:
			wxsFormat( L"(thread: %s) STL Runtime Error: %s\n\t%s",
				GetName().c_str(), fromUTF8( ex.what() ).c_str()
			),

			// User Message (not translated, std::exception doesn't have that kind of fancy!
			wxsFormat( L"A runtime error occurred in %s:\n\n%s (STL)",
				GetName().c_str(), fromUTF8( ex.what() ).c_str()
			)
		);
	}

	// ----------------------------------------------------------------------------
	catch( Exception::RuntimeError& ex )
	{
		m_except = ex.Clone();
		m_except->DiagMsg() = wxsFormat( L"(thread:%s) ", GetName().c_str() ) + m_except->DiagMsg();
	}
#ifndef PCSX2_DEVBUILD
	// ----------------------------------------------------------------------------
	// Allow logic errors to propagate out of the thread in release builds, so that they might be
	// handled in non-fatal ways.  On Devbuilds let them loose, so that they produce debug stack
	// traces and such.
	catch( std::logic_error& ex )
	{
		throw Exception::LogicError( wxsFormat( L"(thread: %s) STL Logic Error: %s\n\t%s",
			GetName().c_str(), fromUTF8( ex.what() ).c_str() )
		);
	}
	catch( Exception::LogicError& ex )
	{
		m_except = ex.Clone();
		m_except->DiagMsg() = wxsFormat( L"(thread:%s) ", GetName().c_str() ) + m_except->DiagMsg();
	}
	// ----------------------------------------------------------------------------
	// Bleh... don't bother with std::exception.  std::logic_error and runtime_error should catch
	// anything coming out of the core STL libraries anyway.
	/*catch( std::exception& ex )
	{
		throw Exception::BaseException( wxsFormat( L"(thread: %s) STL exception: %s\n\t%s",
			GetName().c_str(), fromUTF8( ex.what() ).c_str() )
		);
	}*/
	// ----------------------------------------------------------------------------
	// BaseException --  same deal as LogicErrors.
	//
	catch( Exception::BaseException& ex )
	{
		m_except = ex.Clone();
		m_except->DiagMsg() = wxsFormat( L"(thread:%s) ", GetName().c_str() ) + m_except->DiagMsg();
	}
#endif
}

// invoked internally when canceling or exiting the thread.  Extending classes should implement
// OnCleanupInThread() to extend cleanup functionality.
void Threading::PersistentThread::_ThreadCleanup()
{
	pxAssertMsg( IsSelf(), "Thread affinity error." );	// only allowed from our own thread, thanks.

	_try_virtual_invoke( &PersistentThread::OnCleanupInThread );

	m_lock_InThread.Unlock();
}

wxString Threading::PersistentThread::GetName() const
{
	return m_name;
}

// This override is called by PeristentThread when the thread is first created, prior to
// calling ExecuteTaskInThread.  This is useful primarily for "base" classes that extend
// from PersistentThread, giving them the ability to bind startup code to all threads that
// derive from them.  (the alternative would have been to make ExecuteTaskInThread a
// private member, and provide a new Task executor by a different name).
void Threading::PersistentThread::OnStartInThread()
{
	m_running = true;
}

void Threading::PersistentThread::_internal_execute()
{
	m_lock_InThread.Lock();
	_DoSetThreadName( m_name );

	OnStartInThread();

	_try_virtual_invoke( &PersistentThread::ExecuteTaskInThread );
}

void Threading::PersistentThread::OnStart()
{
	FrankenMutex( m_lock_InThread );
	m_sem_event.Reset();
}

void Threading::PersistentThread::OnCleanupInThread()
{
	m_running = false;
}

// passed into pthread_create, and is used to dispatch the thread's object oriented
// callback function
void* Threading::PersistentThread::_internal_callback( void* itsme )
{
	jASSUME( itsme != NULL );
	PersistentThread& owner = *((PersistentThread*)itsme);

	pthread_cleanup_push( _pt_callback_cleanup, itsme );
	owner._internal_execute();
	pthread_cleanup_pop( true );
	return NULL;
}

void Threading::PersistentThread::_DoSetThreadName( const wxString& name )
{
	_DoSetThreadName( name.ToUTF8() );
}

void Threading::PersistentThread::_DoSetThreadName( __unused const char* name )
{
	pxAssertMsg( IsSelf(), "Thread affinity error." );	// only allowed from our own thread, thanks.

	// This feature needs Windows headers and MSVC's SEH support:

#if defined(_WINDOWS_) && defined (_MSC_VER)

	// This code sample was borrowed form some obscure MSDN article.
	// In a rare bout of sanity, it's an actual Micrsoft-published hack
	// that actually works!

	static const int MS_VC_EXCEPTION = 0x406D1388;

	#pragma pack(push,8)
	struct THREADNAME_INFO
	{
		DWORD dwType; // Must be 0x1000.
		LPCSTR szName; // Pointer to name (in user addr space).
		DWORD dwThreadID; // Thread ID (-1=caller thread).
		DWORD dwFlags; // Reserved for future use, must be zero.
	};
	#pragma pack(pop)

	THREADNAME_INFO info;
	info.dwType		= 0x1000;
	info.szName		= name;
	info.dwThreadID	= GetCurrentThreadId();
	info.dwFlags	= 0;

	__try
	{
		RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info );
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
	}
#endif
}

// --------------------------------------------------------------------------------------
//  BaseTaskThread Implementations
// --------------------------------------------------------------------------------------

// Tells the thread to exit and then waits for thread termination.
void Threading::BaseTaskThread::Block()
{
	if( !IsRunning() ) return;
	m_Done = true;
	m_sem_event.Post();
	PersistentThread::Block();
}

// Initiates the new task.  This should be called after your own StartTask has
// initialized internal variables / preparations for task execution.
void Threading::BaseTaskThread::PostTask()
{
	pxAssert( !m_detached );

	ScopedLock locker( m_lock_TaskComplete );
	m_TaskPending = true;
	m_post_TaskComplete.Reset();
	m_sem_event.Post();
}

// Blocks current thread execution pending the completion of the parallel task.
void Threading::BaseTaskThread::WaitForResult()
{
	if( m_detached || !m_running ) return;
	if( m_TaskPending )
	#ifdef wxUSE_GUI
		m_post_TaskComplete.Wait();
	#else
		m_post_TaskComplete.WaitRaw();
	#endif

	m_post_TaskComplete.Reset();
}

void Threading::BaseTaskThread::ExecuteTaskInThread()
{
	while( !m_Done )
	{
		// Wait for a job -- or get a pthread_cancel.  I'm easy.
		m_sem_event.WaitRaw();

		Task();
		m_lock_TaskComplete.Lock();
		m_TaskPending = false;
		m_post_TaskComplete.Post();
		m_lock_TaskComplete.Unlock();
	};

	return;
}

// --------------------------------------------------------------------------------------
//  pthread Cond is an evil api that is not suited for Pcsx2 needs.
//  Let's not use it. (Air)
// --------------------------------------------------------------------------------------

#if 0
Threading::WaitEvent::WaitEvent()
{
	int err = 0;

	err = pthread_cond_init(&cond, NULL);
	err = pthread_mutex_init(&mutex, NULL);
}

Threading::WaitEvent::~WaitEvent() throw()
{
	pthread_cond_destroy( &cond );
	pthread_mutex_destroy( &mutex );
}

void Threading::WaitEvent::Set()
{
	pthread_mutex_lock( &mutex );
	pthread_cond_signal( &cond );
	pthread_mutex_unlock( &mutex );
}

void Threading::WaitEvent::Wait()
{
	pthread_mutex_lock( &mutex );
	pthread_cond_wait( &cond, &mutex );
	pthread_mutex_unlock( &mutex );
}
#endif


// --------------------------------------------------------------------------------------
//  InterlockedExchanges / AtomicExchanges (PCSX2's Helper versions)
// --------------------------------------------------------------------------------------
// define some overloads for InterlockedExchanges for commonly used types, like u32 and s32.

__forceinline u32 Threading::AtomicExchange( volatile u32& Target, u32 value )
{
	return _InterlockedExchange( (volatile long*)&Target, value );
}

__forceinline u32 Threading::AtomicExchangeAdd( volatile u32& Target, u32 value )
{
	return _InterlockedExchangeAdd( (volatile long*)&Target, value );
}

__forceinline u32 Threading::AtomicIncrement( volatile u32& Target )
{
	return _InterlockedExchangeAdd( (volatile long*)&Target, 1 );
}

__forceinline u32 Threading::AtomicDecrement( volatile u32& Target )
{
	return _InterlockedExchangeAdd( (volatile long*)&Target, -1 );
}

__forceinline s32 Threading::AtomicExchange( volatile s32& Target, s32 value )
{
	return _InterlockedExchange( (volatile long*)&Target, value );
}

__forceinline s32 Threading::AtomicExchangeAdd( volatile s32& Target, u32 value )
{
	return _InterlockedExchangeAdd( (volatile long*)&Target, value );
}

__forceinline s32 Threading::AtomicIncrement( volatile s32& Target )
{
	return _InterlockedExchangeAdd( (volatile long*)&Target, 1 );
}

__forceinline s32 Threading::AtomicDecrement( volatile s32& Target )
{
	return _InterlockedExchangeAdd( (volatile long*)&Target, -1 );
}
