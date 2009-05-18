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

#pragma once

#include "GPUState.h"
#include "GSVertexList.h"

struct GPURendererSettings
{
	int m_filter;
	int m_dither;
	int m_aspectratio;
	bool m_vsync;
	GSVector2i m_scale;
};

class GPURendererBase : public GPUState, protected GPURendererSettings
{
protected:
	HWND m_hWnd;
	WNDPROC m_wndproc;
	static map<HWND, GPURendererBase*> m_wnd2gpu;

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		map<HWND, GPURendererBase*>::iterator i = m_wnd2gpu.find(hWnd);

		if(i != m_wnd2gpu.end())
		{
			return (*i).second->OnMessage(message, wParam, lParam);
		}

		ASSERT(0);

		return 0;
	}

	LRESULT OnMessage(UINT message, WPARAM wParam, LPARAM lParam)
	{
		if(message == WM_KEYUP)
		{
			switch(wParam)
			{
			case VK_DELETE:
				m_filter = (m_filter + 1) % 3;
				return 0;
			case VK_END:
				m_dither = m_dither ? 0 : 1;
				return 0;
			case VK_NEXT:
				m_aspectratio = (m_aspectratio + 1) % 3;
				return 0;
			}
		}

		return m_wndproc(m_hWnd, message, wParam, lParam);
	}

public:
	GPURendererBase(const GPURendererSettings& rs)
		: GPUState(rs.m_scale)
		, m_hWnd(NULL)
		, m_wndproc(NULL)
	{
		m_filter = rs.m_filter;
		m_dither = rs.m_dither;
		m_aspectratio = rs.m_aspectratio;
		m_vsync = rs.m_vsync;
		m_scale = m_mem.GetScale();
	}

	virtual ~GPURendererBase()
	{
		if(m_wndproc)
		{
			SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, (LONG_PTR)m_wndproc);

			m_wnd2gpu.erase(m_hWnd);
		}
	}

	virtual bool Create(HWND hWnd)
	{
		m_hWnd = hWnd;

		m_wndproc = (WNDPROC)GetWindowLongPtr(hWnd, GWLP_WNDPROC);
		SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)WndProc);

		m_wnd2gpu[hWnd] = this;

		DWORD style = GetWindowLong(hWnd, GWL_STYLE);
		style |= WS_OVERLAPPEDWINDOW;
		SetWindowLong(hWnd, GWL_STYLE, style);
		UpdateWindow(hWnd);

		ShowWindow(hWnd, SW_SHOWNORMAL);

		return true;
	}

	virtual void VSync() = 0;
	virtual bool MakeSnapshot(const string& path) = 0;
};

template<class Device, class Vertex> 
class GPURenderer : public GPURendererBase
{
protected:
	typedef typename Device::Texture Texture;

	Vertex* m_vertices;
	int m_count;
	int m_maxcount;
	GSVertexList<Vertex> m_vl;

	void Reset()
	{
		m_count = 0;
		m_vl.RemoveAll();

		__super::Reset();
	}

	void VertexKick()
	{
		if(m_vl.GetCount() < (int)m_env.PRIM.VTX)
		{
			return;
		}

		if(m_count > m_maxcount)
		{
			m_maxcount = max(10000, m_maxcount * 3/2);
			m_vertices = (Vertex*)_aligned_realloc(m_vertices, sizeof(Vertex) * m_maxcount, 16);
			m_maxcount -= 100;
		}

		Vertex* v = &m_vertices[m_count];

		int count = 0;

		switch(m_env.PRIM.TYPE)
		{
		case GPU_POLYGON:
			m_vl.GetAt(0, v[0]);
			m_vl.GetAt(1, v[1]);
			m_vl.GetAt(2, v[2]);
			m_vl.RemoveAll();
			count = 3;
			break;
		case GPU_LINE:
			m_vl.GetAt(0, v[0]);
			m_vl.GetAt(1, v[1]);
			m_vl.RemoveAll();
			count = 2;
			break;
		case GPU_SPRITE:
			m_vl.GetAt(0, v[0]);
			m_vl.GetAt(1, v[1]);
			m_vl.RemoveAll();
			count = 2;
			break;
		default:
			ASSERT(0);
			m_vl.RemoveAll();
			count = 0;
			break;
		}

		(this->*m_fpDrawingKickHandlers[m_env.PRIM.TYPE])(v, count);

		m_count += count;
	}

	typedef void (GPURenderer<Device, Vertex>::*DrawingKickHandler)(Vertex* v, int& count);

	DrawingKickHandler m_fpDrawingKickHandlers[4];

	void DrawingKickNull(Vertex* v, int& count)
	{
		ASSERT(0);
	}

	void ResetPrim()
	{
		m_vl.RemoveAll();
	}

	void FlushPrim() 
	{
		if(m_count > 0)
		{
			/*
			Dump("db");

			if(m_env.PRIM.TME)
			{
				GSVector4i r;

				r.left = m_env.STATUS.TX << 6;
				r.top = m_env.STATUS.TY << 8;
				r.right = r.left + 256;
				r.bottom = r.top + 256;

				Dump(format("da_%d_%d_%d_%d_%d", m_env.STATUS.TP, r).c_str(), m_env.STATUS.TP, r, false);
			}
			*/

			Draw();

			m_count = 0;

			//Dump("dc", false);
		}
	}

	virtual void ResetDevice() {}
	virtual void Draw() = 0;
	virtual bool GetOutput(Texture& t) = 0;

	bool Merge()
	{
		Texture st[2];

		if(!GetOutput(st[0]))
		{
			return false;
		}

		GSVector2i s = st[0].GetSize();
		
		GSVector4 sr[2];

		sr[0].x = 0;
		sr[0].y = 0;
		sr[0].z = 1.0f;
		sr[0].w = 1.0f;

		GSVector4 dr[2];

		dr[0].x = 0;
		dr[0].y = 0;
		dr[0].z = (float)s.x;
		dr[0].w = (float)s.y;

		GSVector4 c(0, 0, 0, 1);

		m_dev.Merge(st, sr, dr, s, 1, 1, c);

		return true;
	}

public:
	Device m_dev;

public:
	GPURenderer(const GPURendererSettings& rs)
		: GPURendererBase(rs)
		, m_count(0)
		, m_maxcount(10000)
	{
		m_vertices = (Vertex*)_aligned_malloc(sizeof(Vertex) * m_maxcount, 16);
		m_maxcount -= 100;

		for(int i = 0; i < countof(m_fpDrawingKickHandlers); i++)
		{
			m_fpDrawingKickHandlers[i] = &GPURenderer<Device, Vertex>::DrawingKickNull;
		}
	}

	virtual ~GPURenderer()
	{
		if(m_vertices) _aligned_free(m_vertices);
	}

	virtual bool Create(HWND hWnd)
	{
		if(!__super::Create(hWnd))
		{
			return false;
		}

		if(!m_dev.Create(hWnd, m_vsync))
		{
			return false;
		}

		Reset();

		return true;
	}

	virtual void VSync()
	{
		GSPerfMonAutoTimer pmat(m_perfmon);

		// m_env.STATUS.LCF = ~m_env.STATUS.LCF; // ?

		if(!IsWindow(m_hWnd))
		{
			return;
		}

		Flush();

		m_perfmon.Put(GSPerfMon::Frame);

		if(!Merge())
		{
			return;
		}

		// osd 

		static uint64 s_frame = 0;
		static string s_stats;

		if(m_perfmon.GetFrame() - s_frame >= 30)
		{
			m_perfmon.Update();

			s_frame = m_perfmon.GetFrame();

			double fps = 1000.0f / m_perfmon.Get(GSPerfMon::Frame);

			GSVector4i r = m_env.GetDisplayRect();

			int w = r.width() << m_scale.x;
			int h = r.height() << m_scale.y;

			s_stats = format(
				"%I64d | %d x %d | %.2f fps (%d%%) | %d/%d | %d%% CPU | %.2f | %.2f", 
				m_perfmon.GetFrame(), w, h, fps, (int)(100.0 * fps / m_env.GetFPS()),
				(int)m_perfmon.Get(GSPerfMon::Prim),
				(int)m_perfmon.Get(GSPerfMon::Draw),
				m_perfmon.CPU(),
				m_perfmon.Get(GSPerfMon::Swizzle) / 1024,
				m_perfmon.Get(GSPerfMon::Unswizzle) / 1024
			);

			double fillrate = m_perfmon.Get(GSPerfMon::Fillrate);

			if(fillrate > 0)
			{
				s_stats = format("%s | %.2f mpps", s_stats.c_str(), fps * fillrate / (1024 * 1024));
			}

			SetWindowText(m_hWnd, s_stats.c_str());
		}

		if(m_dev.IsLost())
		{
			ResetDevice();
		}

		GSVector4i r;
		
		GetClientRect(m_hWnd, r);

		m_dev.Present(r.fit(m_aspectratio));
	}

	virtual bool MakeSnapshot(const string& path)
	{
		time_t t = time(NULL);

		char buff[16];

		if(!strftime(buff, sizeof(buff), "%Y%m%d%H%M%S", localtime(&t)))
		{
			return false;
		}

		return m_dev.SaveCurrent(format("%s_%s.bmp", path.c_str(), buff));
	}
};
