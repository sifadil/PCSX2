/*
 *	Copyright (C) 2007-2012 Gabest
 *	http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "stdafx.h"
#include "GSWndEGL.h"

// Need at least MESA 9.0 (plan for october/november 2012)
// So force the destiny to at least check the compilation
#ifndef EGL_KHR_create_context
#define EGL_KHR_create_context 1
#define EGL_CONTEXT_MAJOR_VERSION_KHR			    EGL_CONTEXT_CLIENT_VERSION
#define EGL_CONTEXT_MINOR_VERSION_KHR			    0x30FB
#define EGL_CONTEXT_FLAGS_KHR				    0x30FC
#define EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR		    0x30FD
#define EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_KHR  0x31BD
#define EGL_NO_RESET_NOTIFICATION_KHR			    0x31BE
#define EGL_LOSE_CONTEXT_ON_RESET_KHR			    0x31BF
#define EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR		    0x00000001
#define EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE_BIT_KHR	    0x00000002
#define EGL_CONTEXT_OPENGL_ROBUST_ACCESS_BIT_KHR	    0x00000004
#define EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR		    0x00000001
#define EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT_KHR    0x00000002
#endif

#if defined(_LINUX) && defined (EGL_API)
GSWndEGL::GSWndEGL()
	: m_NativeWindow(0), m_NativeDisplay(NULL), m_ctx_attached(false)
{
}

bool GSWndEGL::CreateContext(int major, int minor)
{
	EGLConfig eglConfig;
	EGLint numConfigs;
	EGLint contextAttribs[] =
	{
		EGL_CONTEXT_MAJOR_VERSION_KHR, major,
		EGL_CONTEXT_MINOR_VERSION_KHR, minor,
		// Keep compatibility for old cruft
		EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR, EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT_KHR,
		//EGL_CONTEXT_FLAGS_KHR, EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR | EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE_BIT_KHR,
		// FIXME : Request a debug context to ease opengl development
#if defined(ZEROGS_DEVBUILD) || defined(_DEBUG)
		EGL_CONTEXT_FLAGS_KHR, EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR,
#endif
		EGL_NONE
	};
	EGLint attrList[] = {
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_DEPTH_SIZE, 24,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
		EGL_NONE
	};

	eglBindAPI(EGL_OPENGL_API);

	if ( !eglChooseConfig(m_eglDisplay, attrList, &eglConfig, 1, &numConfigs) )
	{
		fprintf(stderr,"EGL: Failed to get a frame buffer config!");
		return EGL_FALSE;
	}

	m_eglSurface = eglCreateWindowSurface(m_eglDisplay, eglConfig, m_NativeWindow, NULL);
	if ( m_eglSurface == EGL_NO_SURFACE )
	{
		fprintf(stderr,"EGL: Failed to get a window surface");
		return EGL_FALSE;
	}

	m_eglContext = eglCreateContext(m_eglDisplay, eglConfig, EGL_NO_CONTEXT, contextAttribs );
	if ( m_eglContext == EGL_NO_CONTEXT )
	{
		fprintf(stderr,"EGL: Failed to create the context");
		fprintf(stderr,"EGL STATUS: %x", eglGetError());
		return EGL_FALSE;
	}

	if ( !eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext) )
	{
		return EGL_FALSE;
	}

	return EGL_TRUE;
}

void GSWndEGL::AttachContext()
{
	if (!IsContextAttached()) {
		//fprintf(stderr, "Attach the context\n");
		eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext);
		m_ctx_attached = true;
	}
}

void GSWndEGL::DetachContext()
{
	if (IsContextAttached()) {
		//fprintf(stderr, "Detach the context\n");
		eglMakeCurrent(m_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		m_ctx_attached = false;
	}
}

void GSWndEGL::CheckContext()
{
	fprintf(stderr,"EGL: %s : %s", eglQueryString(m_eglDisplay, EGL_VENDOR) , eglQueryString(m_eglDisplay, EGL_VERSION) );
	fprintf(stderr,"EGL: extensions supported: %s", eglQueryString(m_eglDisplay, EGL_EXTENSIONS));
}

bool GSWndEGL::Attach(void* handle, bool managed)
{
	m_NativeWindow = *(Window*)handle;
	m_managed = managed;

	m_NativeDisplay = XOpenDisplay(NULL);
	if (!OpenEGLDisplay()) return false;

	// Note: 4.2 crash on latest nvidia drivers!
	if (!CreateContext(3, 3)) return false;

	AttachContext();

	CheckContext();

	return true;
}

void GSWndEGL::Detach()
{
	// Actually the destructor is not called when there is only a GSclose/GSshutdown
	// The window still need to be closed
	DetachContext();
	CloseEGLDisplay();

	if (m_NativeDisplay) {
		XCloseDisplay(m_NativeDisplay);
		m_NativeDisplay = NULL;
	}
}

bool GSWndEGL::Create(const string& title, int w, int h)
{
	if(m_NativeWindow) return false;

	if(w <= 0 || h <= 0) {
		w = theApp.GetConfig("ModeWidth", 640);
		h = theApp.GetConfig("ModeHeight", 480);
	}

	m_managed = true;

	// note this part must be only executed when replaying .gs debug file
	m_NativeDisplay = XOpenDisplay(NULL);

#if 0
	int attrListDbl[] = { GLX_RGBA, GLX_DOUBLEBUFFER,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_DEPTH_SIZE, 24,
		None
	};
	XVisualInfo* vi = glXChooseVisual(m_NativeDisplay, DefaultScreen(m_NativeDisplay), attrListDbl);

	/* create a color map */
	XSetWindowAttributes attr;
	attr.colormap = XCreateColormap(m_NativeDisplay, RootWindow(m_NativeDisplay, vi->screen),
			vi->visual, AllocNone);
	attr.border_pixel = 0;
	attr.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask |
		StructureNotifyMask | SubstructureRedirectMask | SubstructureNotifyMask |
		EnterWindowMask | LeaveWindowMask | FocusChangeMask ;

	// Create a window at the last position/size
	m_NativeWindow = XCreateWindow(m_NativeDisplay, RootWindow(m_NativeDisplay, vi->screen),
			0 , 0 , w, h, 0, vi->depth, InputOutput, vi->visual,
			CWBorderPixel | CWColormap | CWEventMask, &attr);

	XFree(vi);
#else
	m_NativeWindow = XCreateSimpleWindow(m_NativeDisplay, DefaultRootWindow(m_NativeDisplay), 0, 0, w, h, 0, 0, 0);
#endif

	XMapWindow (m_NativeDisplay, m_NativeWindow);

	if (!CreateContext(3, 3)) return false;

	AttachContext();

	return (m_NativeWindow != 0);
}

void* GSWndEGL::GetDisplay()
{
	// note this part must be only executed when replaying .gs debug file
	return (void*)m_NativeDisplay;
}

GSVector4i GSWndEGL::GetClientRect()
{
	unsigned int h = 480;
	unsigned int w = 640;

	unsigned int borderDummy;
	unsigned int depthDummy;
	Window winDummy;
    int xDummy;
    int yDummy;

	if (!m_NativeDisplay) m_NativeDisplay = XOpenDisplay(NULL);
	XGetGeometry(m_NativeDisplay, m_NativeWindow, &winDummy, &xDummy, &yDummy, &w, &h, &borderDummy, &depthDummy);

	return GSVector4i(0, 0, (int)w, (int)h);
}

// Returns FALSE if the window has no title, or if th window title is under the strict
// management of the emulator.

bool GSWndEGL::SetWindowText(const char* title)
{
	if (!m_managed) return true;

	XTextProperty prop;

	memset(&prop, 0, sizeof(prop));

	char* ptitle = (char*)title;
	if (XStringListToTextProperty(&ptitle, 1, &prop)) {
		XSetWMName(m_NativeDisplay, m_NativeWindow, &prop);
	}

	XFree(prop.value);
	XFlush(m_NativeDisplay);

	return true;
}

void GSWndEGL::SetVSync(bool enable)
{
	// 0 -> disable vsync
	// n -> wait n frame
	eglSwapInterval(m_eglDisplay, enable);
}

void GSWndEGL::Flip()
{
	eglSwapBuffers(m_eglDisplay, m_eglSurface);
}

void GSWndEGL::Show()
{
	XMapRaised(m_NativeDisplay, m_NativeWindow);
	XFlush(m_NativeDisplay);
}

void GSWndEGL::Hide()
{
	XUnmapWindow(m_NativeDisplay, m_NativeWindow);
	XFlush(m_NativeDisplay);
}

void GSWndEGL::HideFrame()
{
	// TODO
}

void GSWndEGL::CloseEGLDisplay()
{
	eglTerminate(m_eglDisplay);
}

EGLBoolean GSWndEGL::OpenEGLDisplay()
{
	// Create an EGL display from the native display
	m_eglDisplay = eglGetDisplay((EGLNativeDisplayType)m_NativeDisplay);
	if ( m_eglDisplay == EGL_NO_DISPLAY ) return EGL_FALSE;

	if ( !eglInitialize(m_eglDisplay, NULL, NULL) ) return EGL_FALSE;

	return EGL_TRUE;
}


#endif
