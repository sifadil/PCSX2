/* 
 *	Copyright (C) 2007-2009 Gabest
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
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "StdAfx.h"
#include "GSDevice.h"

GSDevice::GSDevice() 
	: m_wnd(NULL)
	, m_rbswapped(false)
	, m_backbuffer(NULL)
	, m_merge(NULL)
	, m_weavebob(NULL)
	, m_blend(NULL)
	, m_1x1(NULL)
{
}

GSDevice::~GSDevice() 
{
	for_each(m_pool.begin(), m_pool.end(), delete_object());
	
	delete m_backbuffer;
	delete m_merge; 
	delete m_weavebob;
	delete m_blend;
	delete m_1x1;
}

bool GSDevice::Create(GSWnd* wnd, bool vsync)
{
	m_wnd = wnd;
	m_vsync = vsync;

	return true;
}

bool GSDevice::Reset(int w, int h, int mode)
{
	for_each(m_pool.begin(), m_pool.end(), delete_object());
	
	m_pool.clear();
	
	delete m_backbuffer;
	delete m_merge; 
	delete m_weavebob;
	delete m_blend;
	delete m_1x1;

	m_backbuffer = NULL;
	m_merge = NULL;
	m_weavebob = NULL;
	m_blend = NULL;
	m_1x1 = NULL;

	m_current = NULL; // current is special, points to other textures, no need to delete

	return true;
}

void GSDevice::Present(const GSVector4i& r, int shader, bool limit)
{
	GSVector4i cr = m_wnd->GetClientRect();

	int w = std::max(cr.width(), 1);
	int h = std::max(cr.height(), 1);

	if(!m_backbuffer || m_backbuffer->m_size.x != w || m_backbuffer->m_size.y != h)
	{
		if(!Reset(w, h, DontCare))
		{
			return;
		}
	}

	ClearRenderTarget(m_backbuffer, 0);

	if(m_current)
	{
		static int s_shader[3] = {0, 5, 6}; // FIXME

		StretchRect(m_current, m_backbuffer, GSVector4(r), s_shader[shader]);
	}

	Flip(limit);
}

GSTexture* GSDevice::Fetch(int type, int w, int h, int format)
{
	GSVector2i size(w, h);

	for(list<GSTexture*>::iterator i = m_pool.begin(); i != m_pool.end(); i++)
	{
		GSTexture* t = *i;

		if(t->GetType() == type && t->GetFormat() == format && t->GetSize() == size)
		{
			m_pool.erase(i);

			return t;
		}
	}

	return Create(type, w, h, format);
}

void GSDevice::Recycle(GSTexture* t)
{
	if(t)
	{
		m_pool.push_front(t);

		while(m_pool.size() > 200)
		{
			delete m_pool.back();

			m_pool.pop_back();
		}
	}
}

GSTexture* GSDevice::CreateRenderTarget(int w, int h, int format)
{
	return Fetch(GSTexture::RenderTarget, w, h, format);
}

GSTexture* GSDevice::CreateDepthStencil(int w, int h, int format)
{
	return Fetch(GSTexture::DepthStencil, w, h, format);
}

GSTexture* GSDevice::CreateTexture(int w, int h, int format)
{
	return Fetch(GSTexture::Texture, w, h, format);
}

GSTexture* GSDevice::CreateOffscreen(int w, int h, int format)
{
	return Fetch(GSTexture::Offscreen, w, h, format);
}

void GSDevice::StretchRect(GSTexture* st, GSTexture* dt, const GSVector4& dr, int shader, bool linear)
{
	StretchRect(st, GSVector4(0, 0, 1, 1), dt, dr, shader, linear);
}

GSTexture* GSDevice::GetCurrent()
{
	return m_current;
}

void GSDevice::Merge(GSTexture* st[2], GSVector4* sr, GSVector4* dr, const GSVector2i& fs, bool slbg, bool mmod, const GSVector4& c)
{
	if(!m_merge || !(m_merge->GetSize() == fs))
	{
		m_merge = CreateRenderTarget(fs.x, fs.y);
	}

	// TODO: m_1x1

	// KH:COM crashes at startup when booting *through the bios* due to m_merge being NULL.
	// (texture appears to be non-null, and is being re-created at a size around like 1700x340,
	// dunno if that's relevant) -- air
	
	if(m_merge)
	{
		DoMerge(st, sr, dr, m_merge, slbg, mmod, c);
	}
	else
	{
		printf("GSdx: m_merge is NULL!\n");
	}

	m_current = m_merge;
}

void GSDevice::Interlace(const GSVector2i& ds, int field, int mode, float yoffset)
{
	if(!m_weavebob || !(m_weavebob->GetSize() == ds))
	{
		m_weavebob = CreateRenderTarget(ds.x, ds.y);
	}

	if(mode == 0 || mode == 2) // weave or blend
	{
		// weave first

		DoInterlace(m_merge, m_weavebob, field, false, 0);

		if(mode == 2)
		{
			// blend

			if(!m_blend || !(m_blend->GetSize() == ds))
			{
				m_blend = CreateRenderTarget(ds.x, ds.y);
			}

			DoInterlace(m_weavebob, m_blend, 2, false, 0);

			m_current = m_blend;
		}
		else
		{
			m_current = m_weavebob;
		}
	}
	else if(mode == 1) // bob
	{
		DoInterlace(m_merge, m_weavebob, 3, true, yoffset * field);

		m_current = m_weavebob;
	}
	else
	{
		m_current = m_merge;
	}
}

bool GSDevice::ResizeTexture(GSTexture** t, int w, int h)
{
	if(t == NULL) {ASSERT(0); return false;}

	GSTexture* t2 = *t;

	if(t2 == NULL || t2->m_size.x != w || t2->m_size.y != h)
	{
		delete t2;

		t2 = CreateTexture(w, h);

		*t = t2;
	}

	return t2 != NULL;
}

bool GSDevice::SetFeatureLevel(D3D_FEATURE_LEVEL level, bool compat_mode)
{
	m_shader.level = level;

	switch(level)
	{
	case D3D_FEATURE_LEVEL_9_1:
	case D3D_FEATURE_LEVEL_9_2:
		m_shader.model = "0x200";
		m_shader.vs = compat_mode ? "vs_4_0_level_9_1" : "vs_2_0";
		m_shader.ps = compat_mode ? "ps_4_0_level_9_1" : "ps_2_0";
		break;
	case D3D_FEATURE_LEVEL_9_3:
		m_shader.model = "0x300";
		m_shader.vs = compat_mode ? "vs_4_0_level_9_3" : "vs_3_0";
		m_shader.ps = compat_mode ? "ps_4_0_level_9_3" : "ps_3_0";
		break;
	case D3D_FEATURE_LEVEL_10_0:
		m_shader.model = "0x400";
		m_shader.vs = "vs_4_0";
		m_shader.gs = "gs_4_0";
		m_shader.ps = "ps_4_0";
		break;
	case D3D_FEATURE_LEVEL_10_1:
		m_shader.model = "0x401";
		m_shader.vs = "vs_4_1";
		m_shader.gs = "gs_4_1";
		m_shader.ps = "ps_4_1";
		break;
	case D3D_FEATURE_LEVEL_11_0:
		m_shader.model = "0x500";
		m_shader.vs = "vs_5_0";
		m_shader.gs = "gs_5_0";
		m_shader.ps = "ps_5_0";
		break;
	default:
		ASSERT(0);
		return false;
	}

	return true;
}
