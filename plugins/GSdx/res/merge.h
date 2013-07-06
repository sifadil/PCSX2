/*
 *  This file was generated by glsl2h.pl script
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

#pragma once

#include "stdafx.h"

static const char* merge_glsl =
	"//#version 420 // Keep it for editor detection\n"
	"\n"
	"struct vertex_basic\n"
	"{\n"
	"    vec4 p;\n"
	"    vec2 t;\n"
	"};\n"
	"\n"
	"#if __VERSION__ > 140 && !(defined(NO_STRUCT))\n"
	"layout(location = 0) in vertex_basic PSin;\n"
	"#define PSin_p (PSin.p)\n"
	"#define PSin_t (PSin.t)\n"
	"#else\n"
	"#ifdef DISABLE_SSO\n"
	"in vec4 SHADERp;\n"
	"in vec2 SHADERt;\n"
	"#else\n"
	"layout(location = 0) in vec4 SHADERp;\n"
	"layout(location = 1) in vec2 SHADERt;\n"
	"#endif\n"
	"#define PSin_p SHADERp\n"
	"#define PSin_t SHADERt\n"
	"#endif\n"
	"#ifdef FRAGMENT_SHADER\n"
	"\n"
	"layout(location = 0) out vec4 SV_Target0;\n"
	"\n"
	"#ifdef DISABLE_GL42\n"
	"layout(std140) uniform cb10\n"
	"#else\n"
	"layout(std140, binding = 10) uniform cb10\n"
	"#endif\n"
	"{\n"
	"    vec4 BGColor;\n"
	"};\n"
	"\n"
	"#ifdef DISABLE_GL42\n"
	"uniform sampler2D TextureSampler;\n"
	"#else\n"
	"layout(binding = 0) uniform sampler2D TextureSampler;\n"
	"#endif\n"
	"\n"
	"void ps_main0()\n"
	"{\n"
	"    vec4 c = texture(TextureSampler, PSin_t);\n"
	"	c.a = min(c.a * 2, 1.0);\n"
	"    SV_Target0 = c;\n"
	"}\n"
	"\n"
	"void ps_main1()\n"
	"{\n"
	"    vec4 c = texture(TextureSampler, PSin_t);\n"
	"	c.a = BGColor.a;\n"
	"    SV_Target0 = c;\n"
	"}\n"
	"\n"
	"#endif\n"
	;
