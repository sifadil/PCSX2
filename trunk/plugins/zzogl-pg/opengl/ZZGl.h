#ifndef ZZGL_H_INCLUDED
#define ZZGL_H_INCLUDED

#include "PS2Etypes.h"
#include "PS2Edefs.h"

// Need this before gl.h
#ifdef _WIN32

#include <windows.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include "glprocs.h"

#else

// adding glew support instead of glXGetProcAddress (thanks to scaught)
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>

inline void* wglGetProcAddress(const char* x)
{
	return (void*)glXGetProcAddress((const GLubyte*)x);
}

#endif

#include <Cg/cg.h>
#include <Cg/cgGL.h>
#include <cstring>
#include "zerogsmath.h"

// Defines

#ifndef GL_DEPTH24_STENCIL8_EXT // allows FBOs to support stencils
#	define GL_DEPTH_STENCIL_EXT 0x84F9
#	define GL_UNSIGNED_INT_24_8_EXT 0x84FA
#	define GL_DEPTH24_STENCIL8_EXT 0x88F0
#	define GL_TEXTURE_STENCIL_SIZE_EXT 0x88F1
#endif

#define GL_STENCILFUNC(func, ref, mask) { \
	s_stencilfunc  = func; \
	s_stencilref = ref; \
	s_stencilmask = mask; \
	glStencilFunc(func, ref, mask); \
}

#define GL_STENCILFUNC_SET() glStencilFunc(s_stencilfunc, s_stencilref, s_stencilmask)


// sets the data stream
#define SET_STREAM() { \
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(VertexGPU), (void*)8); \
	glSecondaryColorPointerEXT(4, GL_UNSIGNED_BYTE, sizeof(VertexGPU), (void*)12); \
	glTexCoordPointer(3, GL_FLOAT, sizeof(VertexGPU), (void*)16); \
	glVertexPointer(4, GL_SHORT, sizeof(VertexGPU), (void*)0); \
}

extern const char* ShaderCallerName;
extern const char* ShaderHandleName;

inline void SetShaderCaller(const char* Name)
{
	ShaderCallerName = Name;
}

inline void SetHandleName(const char* Name)
{
	ShaderHandleName = Name;
}

extern void HandleCgError(CGcontext ctx, CGerror err, void* appdata);
extern void ZZcgSetParameter4fv(CGparameter param, const float* v, const char* name);

#define SETVERTEXSHADER(prog) { \
	if( (prog) != g_vsprog ) { \
		cgGLBindProgram(prog); \
		g_vsprog = prog; \
	} \
} \
 
#define SETPIXELSHADER(prog) { \
	if( (prog) != g_psprog ) { \
		cgGLBindProgram(prog); \
		g_psprog = prog; \
	} \
} \


#define SAFE_RELEASE_PROG(x) { if( (x) != NULL ) { cgDestroyProgram(x); x = NULL; } }
#define SAFE_RELEASE_TEX(x) { if( (x) != 0 ) { glDeleteTextures(1, &(x)); x = 0; } }

// inline for an extremely often used sequence
// This is turning off all gl functions. Safe to do updates.
inline void DisableAllgl()
{
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(0);
	glDisable(GL_STENCIL_TEST);
	glColorMask(1, 1, 1, 1);
}


//--------------------- Dummies

#ifdef _WIN32
extern void (__stdcall *zgsBlendEquationSeparateEXT)(GLenum, GLenum);
extern void (__stdcall *zgsBlendFuncSeparateEXT)(GLenum, GLenum, GLenum, GLenum);
#else
extern void (APIENTRY *zgsBlendEquationSeparateEXT)(GLenum, GLenum);
extern void (APIENTRY *zgsBlendFuncSeparateEXT)(GLenum, GLenum, GLenum, GLenum);
#endif


// ------------------------ Types -------------------------

struct FRAGMENTSHADER
{
	FRAGMENTSHADER() : prog(0), sMemory(0), sFinal(0), sBitwiseANDX(0), sBitwiseANDY(0), sInterlace(0), sCLUT(0), sOneColor(0), sBitBltZ(0),
			fTexAlpha2(0), fTexOffset(0), fTexDims(0), fTexBlock(0), fClampExts(0), fTexWrapMode(0),
			fRealTexDims(0), fTestBlack(0), fPageOffset(0), fTexAlpha(0) {}

	CGprogram prog;
	CGparameter sMemory, sFinal, sBitwiseANDX, sBitwiseANDY, sInterlace, sCLUT;
	CGparameter sOneColor, sBitBltZ, sInvTexDims;
	CGparameter fTexAlpha2, fTexOffset, fTexDims, fTexBlock, fClampExts, fTexWrapMode, fRealTexDims, fTestBlack, fPageOffset, fTexAlpha;

	void set_uniform_param(CGparameter &var, const char *name)
	{
		CGparameter p;
		p = cgGetNamedParameter(prog, name);

		if (p != NULL && cgIsParameterUsed(p, prog) == CG_TRUE) var = p;
	}

	bool set_texture(GLuint texobj, const char *name)
	{
		CGparameter p;

		p = cgGetNamedParameter(prog, name);

		if (p != NULL && cgIsParameterUsed(p, prog) == CG_TRUE)
		{
			cgGLSetTextureParameter(p, texobj);
			cgGLEnableTextureParameter(p);
			return true;
		}

		return false;
	}

	bool connect(CGparameter &tex, const char *name)
	{
		CGparameter p;

		p = cgGetNamedParameter(prog, name);

		if (p != NULL && cgIsParameterUsed(p, prog) == CG_TRUE)
		{
			cgConnectParameter(tex, p);
			return true;
		}

		return false;
	}

	bool set_texture(CGparameter &tex, const char *name)
	{
		CGparameter p;

		p = cgGetNamedParameter(prog, name);

		if (p != NULL && cgIsParameterUsed(p, prog) == CG_TRUE)
		{
			//cgGLEnableTextureParameter(p);
			tex = p;
			return true;
		}

		return false;
	}

	bool set_shader_const(Vector v, const char *name)
	{
		CGparameter p;

		p = cgGetNamedParameter(prog, name);

		if (p != NULL && cgIsParameterUsed(p, prog) == CG_TRUE)
		{
			cgGLSetParameter4fv(p, v);
			return true;
		}

		return false;
	}
};

struct VERTEXSHADER
{
	VERTEXSHADER() : prog(0), sBitBltPos(0), sBitBltTex(0) {}

	CGprogram prog;
	CGparameter sBitBltPos, sBitBltTex, fBitBltTrans;		 // vertex shader constants
};

// GL prototypes
extern PFNGLISRENDERBUFFEREXTPROC glIsRenderbufferEXT;
extern PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbufferEXT;
extern PFNGLDELETERENDERBUFFERSEXTPROC glDeleteRenderbuffersEXT;
extern PFNGLGENRENDERBUFFERSEXTPROC glGenRenderbuffersEXT;
extern PFNGLRENDERBUFFERSTORAGEEXTPROC glRenderbufferStorageEXT;
extern PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC glGetRenderbufferParameterivEXT;
extern PFNGLISFRAMEBUFFEREXTPROC glIsFramebufferEXT;
extern PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXT;
extern PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffersEXT;
extern PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersEXT;
extern PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT;
extern PFNGLFRAMEBUFFERTEXTURE1DEXTPROC glFramebufferTexture1DEXT;
extern PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT;
extern PFNGLFRAMEBUFFERTEXTURE3DEXTPROC glFramebufferTexture3DEXT;
extern PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbufferEXT;
extern PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC glGetFramebufferAttachmentParameterivEXT;
extern PFNGLGENERATEMIPMAPEXTPROC glGenerateMipmapEXT;
extern PFNGLDRAWBUFFERSPROC glDrawBuffers;

#endif // ZZGL_H_INCLUDED
