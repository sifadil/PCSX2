/*
 *	Copyright (C) 2011-2013 Gregory hainaut
 *	Copyright (C) 2007-2009 Gabest
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
#include "GSShaderOGL.h"

GSShaderOGL::GSShaderOGL(bool debug, bool sso, bool glsl420) :
	m_vs(0),
	m_ps(0),
	m_gs(0),
	m_prog(0),
	m_debug_shader(debug),
	m_sso(sso),
	m_glsl420(glsl420)
{
	m_single_prog.clear();
	if (sso) {
		gl_GenProgramPipelines(1, &m_pipeline);
		gl_BindProgramPipeline(m_pipeline);
	}
}

GSShaderOGL::~GSShaderOGL()
{
	if (m_sso)
		gl_DeleteProgramPipelines(1, &m_pipeline);

	for (auto it = m_single_prog.begin(); it != m_single_prog.end() ; it++) gl_DeleteProgram(it->second);
	m_single_prog.clear();
}

void GSShaderOGL::VS(GLuint s)
{
	if (m_vs != s)
	{
		m_vs = s;
		if (m_sso)
			gl_UseProgramStages(m_pipeline, GL_VERTEX_SHADER_BIT, s);
	}
}

void GSShaderOGL::PS(GLuint s)
{
	if (m_ps != s)
	{
		m_ps = s;
		if (m_sso)
			gl_UseProgramStages(m_pipeline, GL_FRAGMENT_SHADER_BIT, s);
	}
}

void GSShaderOGL::GS(GLuint s)
{
	if (m_gs != s)
	{
		m_gs = s;
		if (m_sso)
			gl_UseProgramStages(m_pipeline, GL_GEOMETRY_SHADER_BIT, s);
	}
}

void GSShaderOGL::SetUniformBinding(GLuint prog, GLchar* name, GLuint binding)
{
	GLuint index;
	index = gl_GetUniformBlockIndex(prog, name);
	if (index != GL_INVALID_INDEX) {
		gl_UniformBlockBinding(prog, index, binding);
	}
}

void GSShaderOGL::SetSamplerBinding(GLuint prog, GLchar* name, GLuint binding)
{
	GLint loc = gl_GetUniformLocation(prog, name);
	if (loc != -1) {
		if (m_sso) {
			gl_ProgramUniform1i(prog, loc, binding);
		} else {
			gl_Uniform1i(loc, binding);
		}
	}
}

void GSShaderOGL::SetupUniform()
{
	if (m_glsl420) return;

	if (m_sso) {
		SetUniformBinding(m_vs, "cb20", 20);
		SetUniformBinding(m_ps, "cb21", 21);

		SetUniformBinding(m_ps, "cb10", 10);
		SetUniformBinding(m_ps, "cb11", 11);
		SetUniformBinding(m_ps, "cb12", 12);
		SetUniformBinding(m_ps, "cb13", 13);

		SetSamplerBinding(m_ps, "TextureSampler", 0);
		SetSamplerBinding(m_ps, "PaletteSampler", 1);
		SetSamplerBinding(m_ps, "RTCopySampler", 2);
	} else {
		SetUniformBinding(m_prog, "cb20", 20);
		SetUniformBinding(m_prog, "cb21", 21);

		SetUniformBinding(m_prog, "cb10", 10);
		SetUniformBinding(m_prog, "cb11", 11);
		SetUniformBinding(m_prog, "cb12", 12);
		SetUniformBinding(m_prog, "cb13", 13);

		SetSamplerBinding(m_prog, "TextureSampler", 0);
		SetSamplerBinding(m_prog, "PaletteSampler", 1);
		SetSamplerBinding(m_prog, "RTCopySampler", 2);
	}
}

bool GSShaderOGL::ValidateShader(GLuint s)
{
	if (!m_debug_shader) return true;

	GLint status;
	gl_GetShaderiv(s, GL_COMPILE_STATUS, &status);
	if (status) return true;

	GLint log_length = 0;
	gl_GetShaderiv(s, GL_INFO_LOG_LENGTH, &log_length);
	if (log_length > 0) {
		char* log = new char[log_length];
		gl_GetShaderInfoLog(s, log_length, NULL, log);
		fprintf(stderr, "%s", log);
		delete[] log;
	}
	fprintf(stderr, "\n");

	return false;
}

bool GSShaderOGL::ValidateProgram(GLuint p)
{
	if (!m_debug_shader) return true;

	GLint status;
	gl_GetProgramiv(p, GL_LINK_STATUS, &status);
	if (status) return true;

	GLint log_length = 0;
	gl_GetProgramiv(p, GL_INFO_LOG_LENGTH, &log_length);
	if (log_length > 0) {
		char* log = new char[log_length];
		gl_GetProgramInfoLog(p, log_length, NULL, log);
		fprintf(stderr, "%s", log);
		delete[] log;
	}
	fprintf(stderr, "\n");

	return false;
}

bool GSShaderOGL::ValidatePipeline(GLuint p)
{
	if (!m_debug_shader) return true;

	// FIXME: might be mandatory to validate the pipeline
	gl_ValidateProgramPipeline(p);

	GLint status;
	gl_GetProgramPipelineiv(p, GL_VALIDATE_STATUS, &status);
	if (status) return true;

	GLint log_length = 0;
	gl_GetProgramPipelineiv(p, GL_INFO_LOG_LENGTH, &log_length);
	if (log_length > 0) {
		char* log = new char[log_length];
		gl_GetProgramPipelineInfoLog(p, log_length, NULL, log);
		fprintf(stderr, "%s", log);
		delete[] log;
	}
	fprintf(stderr, "\n");

	return false;
}

GLuint GSShaderOGL::LinkNewProgram()
{
	GLuint p = gl_CreateProgram();
	if (m_vs) gl_AttachShader(p, m_vs);
	if (m_ps) gl_AttachShader(p, m_ps);
	if (m_gs) gl_AttachShader(p, m_gs);

	gl_LinkProgram(p);

	ValidateProgram(p);

	return p;
}

void GSShaderOGL::UseProgram()
{
	hash_map<uint64, GLuint >::iterator it;
	if (!m_sso) {
		// Note: shader are integer lookup pointer. They start from 1 and incr
		// every time you create a new shader OR a new program.
		// Note2: vs & gs are precompiled at startup. FGLRX and radeon got value < 128.
		// We migth be able to pack the value in a 32bits int
		// I would need to check the behavior on Nvidia (pause/resume).
		uint64 sel = (uint64)m_vs << 40 | (uint64)m_gs << 20 | m_ps;
		it = m_single_prog.find(sel);
		if (it == m_single_prog.end()) {
			m_prog = LinkNewProgram();
			m_single_prog[sel] = m_prog;
		} else {
			m_prog = it->second;
		}

		gl_UseProgram(m_prog);
	} else {
		ValidateProgram(m_pipeline);
	}
}

GLuint GSShaderOGL::Compile(const std::string& glsl_file, const std::string& entry, GLenum type, const char* glsl_h_code, const std::string& macro_sel)
{
	// Not supported
	if (type == GL_GEOMETRY_SHADER && !GLLoader::found_geometry_shader) {
		return 0;
	}

	// *****************************************************
	// Build a header string
	// *****************************************************
	// First select the version (must be the first line so we need to generate it
	std::string version;
	if (GLLoader::found_only_gl30) {
		version = "#version 130\n";
	} else {
		version = "#version 330\n";
	}
	if (m_glsl420) {
		version += "#extension GL_ARB_shading_language_420pack: require\n";
	} else {
		version += "#define DISABLE_GL42\n";
	}
	if (m_sso) {
		version += "#extension GL_ARB_separate_shader_objects : require\n";
	} else {
		if (!GLLoader::fglrx_buggy_driver)
			version += "#define DISABLE_SSO\n";
	}
	if (GLLoader::found_only_gl30) {
		// Need version 330
		version += "#extension GL_ARB_explicit_attrib_location : require\n";
		// Need version 140
		version += "#extension GL_ARB_uniform_buffer_object : require\n";
	}
#ifdef ENABLE_OGL_STENCIL_DEBUG
	version += "#define ENABLE_OGL_STENCIL_DEBUG 1\n";
#endif

	// Allow to puts several shader in 1 files
	std::string shader_type;
	switch (type) {
		case GL_VERTEX_SHADER:
			shader_type = "#define VERTEX_SHADER 1\n";
			break;
		case GL_GEOMETRY_SHADER:
			shader_type = "#define GEOMETRY_SHADER 1\n";
			break;
		case GL_FRAGMENT_SHADER:
			shader_type = "#define FRAGMENT_SHADER 1\n";
			break;
		default: ASSERT(0);
	}

	// Select the entry point ie the main function
	std::string entry_main = format("#define %s main\n", entry.c_str());

	std::string header = version + shader_type + entry_main + macro_sel;

	// Note it is better to separate header and source file to have the good line number
	// in the glsl compiler report
	const char** sources_array = (const char**)malloc(2*sizeof(char*));

	char* header_str = (char*)malloc(header.size() + 1);
	sources_array[0] = header_str;
	header.copy(header_str, header.size(), 0);
	header_str[header.size()] = '\0';

	if (glsl_h_code)
		sources_array[1] = glsl_h_code;
	else
		sources_array[1] = '\0';


	GLuint program;
	if (m_sso) {
	#if 0
		// Could be useful one day
		const GLchar* ShaderSource[1];
		ShaderSource[0] = header.append(source).c_str();
		program = gl_CreateShaderProgramv(type, 1, &ShaderSource[0]);
	#else
		program = gl_CreateShaderProgramv(type, 2, sources_array);
	#endif
	} else {
		program = gl_CreateShader(type);
		gl_ShaderSource(program, 2, sources_array, NULL);
		gl_CompileShader(program);
	}

	free(header_str);
	free(sources_array);

	bool status;
	if (m_sso)
		status = ValidateProgram(program);
	else
		status = ValidateShader(program);

	if (!status) {
		// print extra info
		fprintf(stderr, "%s (entry %s, prog %d) :", glsl_file.c_str(), entry.c_str(), program);
		fprintf(stderr, "\n%s", macro_sel.c_str());
		fprintf(stderr, "\n");
	}
	return program;
}

void GSShaderOGL::Delete(GLuint s)
{
	if (m_sso) {
		gl_DeleteProgram(s);
	} else {
		gl_DeleteShader(s);
	}
}
