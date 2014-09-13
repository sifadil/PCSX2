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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "stdafx.h"
#include "GSRendererCL.h"

#define LOG 0

static FILE* s_fp = LOG ? fopen("c:\\temp1\\_.txt", "w") : NULL;

#define MAX_FRAME_SIZE 2048
#define MAX_PRIM_COUNT 4096u
#define MAX_PRIM_PER_BATCH_BITS 5
#define MAX_PRIM_PER_BATCH (1u << MAX_PRIM_PER_BATCH_BITS)
#define BATCH_COUNT(prim_count) (((prim_count) + (MAX_PRIM_PER_BATCH - 1)) / MAX_PRIM_PER_BATCH)
#define MAX_BATCH_COUNT BATCH_COUNT(MAX_PRIM_COUNT)
#define BIN_SIZE_BITS 4
#define BIN_SIZE (1u << BIN_SIZE_BITS)
#define MAX_BIN_PER_BATCH ((MAX_FRAME_SIZE / BIN_SIZE) * (MAX_FRAME_SIZE / BIN_SIZE))
#define MAX_BIN_COUNT (MAX_BIN_PER_BATCH * MAX_BATCH_COUNT)

#if MAX_PRIM_PER_BATCH == 64u
#define BIN_TYPE cl_ulong
#elif MAX_PRIM_PER_BATCH == 32u
#define BIN_TYPE cl_uint
#else
#error "MAX_PRIM_PER_BATCH != 32u OR 64u"
#endif

#pragma pack(push, 1)

typedef struct
{
	GSVertexCL v[4];
} gs_prim;

typedef struct
{
	cl_float4 dx, dy;
	cl_float4 zero;
	cl_float4 reject_corner;
} gs_barycentric;

typedef struct
{
	cl_uint batch_counter;
	cl_uint _pad[7];
	struct { cl_uint first, last; } bounds[MAX_BIN_PER_BATCH];
	BIN_TYPE bin[MAX_BIN_COUNT];
	cl_uchar4 bbox[MAX_PRIM_COUNT];
	gs_prim prim[MAX_PRIM_COUNT];
	gs_barycentric barycentric[MAX_PRIM_COUNT];
} gs_env;

#pragma pack(pop)

GSRendererCL::GSRendererCL()
	: m_vb_count(0)
{
	m_nativeres = true; // ignore ini, sw is always native

//	s_dump = 1;
//	s_save = 1;

	// TODO: m_tc = new GSTextureCacheCL(this);

	memset(m_texture, 0, sizeof(m_texture));

	m_output = (uint8*)_aligned_malloc(1024 * 1024 * sizeof(uint32), 32);

	memset(m_rw_pages, 0, sizeof(m_rw_pages));

	#define InitCVB(P) \
		m_cvb[P][0][0] = &GSRendererCL::ConvertVertexBuffer<P, 0, 0>; \
		m_cvb[P][0][1] = &GSRendererCL::ConvertVertexBuffer<P, 0, 1>; \
		m_cvb[P][1][0] = &GSRendererCL::ConvertVertexBuffer<P, 1, 0>; \
		m_cvb[P][1][1] = &GSRendererCL::ConvertVertexBuffer<P, 1, 1>; \

	InitCVB(GS_POINT_CLASS);
	InitCVB(GS_LINE_CLASS);
	InitCVB(GS_TRIANGLE_CLASS);
	InitCVB(GS_SPRITE_CLASS);

	m_cl.vm = cl::Buffer(m_cl.context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, (size_t)m_mem.m_vmsize, m_mem.m_vm8, NULL);
}

GSRendererCL::~GSRendererCL()
{
	// TODO: delete m_tc;

	for(size_t i = 0; i < countof(m_texture); i++)
	{
		delete m_texture[i];
	}

	_aligned_free(m_output);
}

void GSRendererCL::Reset()
{
	Sync(-1);

	// TODO: m_tc->RemoveAll();

	GSRenderer::Reset();
}

void GSRendererCL::VSync(int field)
{
	Sync(0); // IncAge might delete a cached texture in use

	GSRenderer::VSync(field);

	// TODO: m_tc->IncAge();

	//memset(m_mem.m_vm8, 0, (size_t)m_mem.m_vmsize);
}

void GSRendererCL::ResetDevice()
{
	for(size_t i = 0; i < countof(m_texture); i++)
	{
		delete m_texture[i];

		m_texture[i] = NULL;
	}
}

GSTexture* GSRendererCL::GetOutput(int i)
{
	Sync(1);

	const GSRegDISPFB& DISPFB = m_regs->DISP[i].DISPFB;

	int w = DISPFB.FBW * 64;
	int h = GetFrameRect(i).bottom;

	// TODO: round up bottom

	if(m_dev->ResizeTexture(&m_texture[i], w, h))
	{
		static int pitch = 1024 * 4;

		GSVector4i r(0, 0, w, h);

		const GSLocalMemory::psm_t& psm = GSLocalMemory::m_psm[DISPFB.PSM];

		(m_mem.*psm.rtx)(m_mem.GetOffset(DISPFB.Block(), DISPFB.FBW, DISPFB.PSM), r.ralign<Align_Outside>(psm.bs), m_output, pitch, m_env.TEXA);

		m_texture[i]->Update(r, m_output, pitch);

		if(s_dump)
		{
			if(s_save && s_n >= s_saven)
			{
				m_texture[i]->Save(format("c:\\temp1\\_%05d_f%lld_fr%d_%05x_%d.bmp", s_n, m_perfmon.GetFrame(), i, (int)DISPFB.Block(), (int)DISPFB.PSM));
			}

			s_n++;
		}
	}

	return m_texture[i];
}

const GSVector4 g_pos_scale(1.0f / 16, 1.0f / 16, 1.0f, 1.0f);

template<uint32 primclass, uint32 tme, uint32 fst>
void GSRendererCL::ConvertVertexBuffer(GSVertexCL* RESTRICT dst, const GSVertex* RESTRICT src, size_t count)
{
	GSVector4i o = (GSVector4i)m_context->XYOFFSET;
	GSVector4 st_scale = GSVector4(1 << m_context->TEX0.TW, 1 << m_context->TEX0.TH, 1, 0);

	for(int i = (int)m_vertex.next; i > 0; i--, src++, dst++)
	{
		GSVector4 stcq = GSVector4::load<true>(&src->m[0]); // s t rgba q

		#if _M_SSE >= 0x401

		GSVector4i xyzuvf(src->m[1]);

		GSVector4i xy = xyzuvf.upl16() - o;
		GSVector4i zf = xyzuvf.ywww().min_u32(GSVector4i::xffffff00());

		#else

		uint32 z = src->XYZ.Z;

		GSVector4i xy = GSVector4i::load((int)src->XYZ.u32[0]).upl16() - o;
		GSVector4i zf = GSVector4i((int)std::min<uint32>(z, 0xffffff00), src->FOG); // NOTE: larger values of z may roll over to 0 when converting back to uint32 later

		#endif

		dst->p = GSVector4(xy).xyxy(GSVector4(zf) + (GSVector4::m_x4f800000 & GSVector4::cast(zf.sra32(31)))) * g_pos_scale;

		GSVector4 t = GSVector4::zero();

		if(tme)
		{
			if(fst)
			{
				#if _M_SSE >= 0x401

				t = GSVector4(xyzuvf.uph16()) * g_pos_scale;
					
				#else

				t = GSVector4(GSVector4i::load(src->UV).upl16()) * g_pos_scale;

				#endif
			}
			else
			{
				t = stcq.xyww() * st_scale;
			}
		}

		dst->t = t.insert32<2, 3>(stcq);
	}
}

void GSRendererCL::Draw()
{
	const GSDrawingContext* context = m_context;

	GSVector4i scissor = GSVector4i(context->scissor.in);
	GSVector4i bbox = GSVector4i(m_vt.m_min.p.floor().xyxy(m_vt.m_max.p.ceil()));

	// points and lines may have zero area bbox (example: single line 0,0->256,0)

	if(m_vt.m_primclass == GS_POINT_CLASS || m_vt.m_primclass == GS_LINE_CLASS)
	{
		if(bbox.x == bbox.z) bbox.z++;
		if(bbox.y == bbox.w) bbox.w++;
	}

	scissor.z = std::min<int>(scissor.z, (int)context->FRAME.FBW * 64); // TODO: find a game that overflows and check which one is the right behaviour

	GSVector4i rect = bbox.rintersect(scissor);

	if(rect.rempty())
	{
		return;
	}

	try
	{
		// simplified texture cache idea: mirror 4 MB vmem in cl::Buffer, track rendered pages, update invalidated texture pages as needed, the kernel does the unswizzling of each texel

		size_t vb_size = m_vertex.next * sizeof(GSVertexCL);
		size_t ib_size = m_index.tail * sizeof(uint32);
		size_t pb_size = sizeof(TFXParameter);

		if(m_cl.vb.tail + vb_size > m_cl.vb.size || m_cl.ib.tail + ib_size > m_cl.ib.size || m_cl.pb.tail + pb_size > m_cl.pb.size)
		{
			if(vb_size > m_cl.vb.size || ib_size > m_cl.ib.size)
			{
				// buffer too small for even one batch, allow twice the size (at least 1 MB)

				Sync(2); // must sync, reallocating the input buffers

				m_cl.Unmap();

				m_cl.vb.size = 0;
				m_cl.ib.size = 0;

				size_t size = std::max(vb_size * 2, 2u << 20);

				printf("growing vertex/index buffer %d\n", size);

				m_cl.vb.buff[0] = cl::Buffer(m_cl.context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, size);
				m_cl.vb.buff[1] = cl::Buffer(m_cl.context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, size);
				m_cl.vb.size = size;

				size = std::max(size / sizeof(GSVertex) * 3 * sizeof(uint32), 1u << 20); // worst case, three times the vertex count

				ASSERT(size >= ib_size);

				if(size < ib_size) size = ib_size; // should not happen

				m_cl.ib.buff[0] = cl::Buffer(m_cl.context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, size);
				m_cl.ib.buff[1] = cl::Buffer(m_cl.context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, size);
				m_cl.ib.size = size;
			}
			else
			{
				Enqueue();

				m_cl.Unmap();

				// make the write queue wait until the rendering queue is ready, it may still use the device buffers

				std::vector<cl::Event> el(1);

				m_cl.queue[2].enqueueMarkerWithWaitList(NULL, &el[0]);
				m_cl.wq->enqueueBarrierWithWaitList(&el, NULL);

				m_cl.wqidx = (m_cl.wqidx + 1) & 1;
				m_cl.wq = &m_cl.queue[m_cl.wqidx];
			}

			m_cl.vb.head = m_cl.vb.tail = 0;
			m_cl.ib.head = m_cl.ib.tail = 0;
			m_cl.pb.head = m_cl.pb.tail = 0;

			m_cl.Map();
		}
		else
		{
			// only allow batches of the same primclass in Enqueue

			if(!m_jobs.empty() && m_jobs.front().sel.prim != (uint32)m_vt.m_primclass)
			{
				Enqueue();
			}
		}

		//

		GSVertexCL* vb = (GSVertexCL*)(m_cl.vb.ptr + m_cl.vb.tail);
		uint32* ib = (uint32*)(m_cl.ib.ptr + m_cl.ib.tail);
		TFXParameter* pb = (TFXParameter*)(m_cl.pb.ptr + m_cl.pb.tail);

		pb->scissor = scissor;
		pb->bbox = bbox;
		pb->rect = rect;

		(this->*m_cvb[m_vt.m_primclass][PRIM->TME][PRIM->FST])(vb, m_vertex.buff, m_vertex.next); // TODO: upload in GSVertex format and extract the fields in the kernel? 

		if(m_jobs.empty())
		{
			memcpy(ib, m_index.buff, m_index.tail * sizeof(uint32));

			m_vb_start = m_cl.vb.tail;
		}
		else
		{
			// TODO: SIMD

			uint32 vb_count = m_vb_count;

			for(size_t i = 0; i < m_index.tail; i++)
			{
				ib[i] = m_index.buff[i] + vb_count;
			}
		}

		m_vb_count += m_vertex.next;

		if(!SetupParameter(pb, vb, m_vertex.next, m_index.buff, m_index.tail))
		{
			return;
		}

		TFXJob job;

		job.rect.x = rect.x;
		job.rect.y = rect.y;
		job.rect.z = rect.z;
		job.rect.w = rect.w;
		job.sel = pb->sel;
		job.ib_start = m_cl.ib.tail;
		job.ib_count = m_index.tail;
		job.pb_start = m_cl.pb.tail;

		m_jobs.push_back(job);

		m_cl.vb.tail += vb_size;
		m_cl.ib.tail += ib_size;
		m_cl.pb.tail += pb_size;

		// mark pages for writing

		if(pb->sel.fb)
		{
			uint8 flag = pb->sel.fb;

			const uint32* pages = m_context->offset.fb->GetPages(rect, m_tmp_pages);

			for(const uint32* p = pages; *p != GSOffset::EOP; p++)
			{
				m_rw_pages[*p] |= flag;
			}
		}

		if(pb->sel.zb)
		{
			uint8 flag = pb->sel.zb;

			const uint32* pages = m_context->offset.zb->GetPages(rect, m_tmp_pages);

			for(const uint32* p = pages; *p != GSOffset::EOP; p++)
			{
				m_rw_pages[*p] |= flag;
			}
		}

		// don't buffer too much data, feed them to the device if there is enough

		if(m_cl.vb.tail - m_cl.vb.head >= 256 * 4096 || m_jobs.size() >= 64)
		{
			Enqueue();
		}

		/*
		// check if the texture is not part of a target currently in use

		if(CheckSourcePages(data))
		{
		Sync(4);
		}

		// addref source and target pages

		data->UsePages(fb_pages, m_context->offset.fb->psm, zb_pages, m_context->offset.zb->psm);
		*/

		// update previously invalidated parts

		//data->UpdateSource();
		/*
		if(LOG)
		{
		fprintf(s_fp, "[%d] queue %05x %d (%d) %05x %d (%d) %05x %d %dx%d (%d %d %d) | %d %d %d\n",
		sd->counter,
		m_context->FRAME.Block(), m_context->FRAME.PSM, gd.sel.fwrite,
		m_context->ZBUF.Block(), m_context->ZBUF.PSM, gd.sel.zwrite,
		PRIM->TME ? m_context->TEX0.TBP0 : 0xfffff, m_context->TEX0.PSM, (int)m_context->TEX0.TW, (int)m_context->TEX0.TH, m_context->TEX0.CSM, m_context->TEX0.CPSM, m_context->TEX0.CSA,
		PRIM->PRIM, sd->vertex_count, sd->index_count);

		fflush(s_fp);
		}
		*/

		//printf("q %p %d (%d %d %d %d)\n", pb, pb->ib_count, r.x, r.y, r.z, r.w);

		/*
		// invalidate new parts rendered onto

		if(sd->global.sel.fwrite)
		{
			m_tc->InvalidatePages(sd->m_fb_pages, sd->m_fpsm);
		}

		if(sd->global.sel.zwrite)
		{
			m_tc->InvalidatePages(sd->m_zb_pages, sd->m_zpsm);
		}
		*/
	}
	catch(cl::Error err)
	{
		printf("%s (%d)\n", err.what(), err.err());

		return;
	}
	catch(std::exception err)
	{
		printf("%s\n", err.what());

		return;
	}
}

void GSRendererCL::Sync(int reason)
{
	//printf("sync %d\n", reason);

	GSPerfMonAutoTimer pmat(&m_perfmon, GSPerfMon::Sync);

	Enqueue();

	m_cl.queue[2].finish();

	memset(m_rw_pages, 0, sizeof(m_rw_pages));

	// TODO: sync buffers created with CL_MEM_USE_HOST_PTR (on m_mem.m_vm8) by a simple map/unmap, 
	// though it does not seem to be necessary even with GPU devices where it might be cached, 
	// needs more testing...

	//void* ptr = m_cl.queue->enqueueMapBuffer(m_cl.vm, CL_TRUE, CL_MAP_READ, 0, m_mem.m_vmsize);
	//m_cl.queue->enqueueUnmapMemObject(m_cl.vm, ptr);
}

void GSRendererCL::InvalidateVideoMem(const GIFRegBITBLTBUF& BITBLTBUF, const GSVector4i& r)
{
	if(LOG) {fprintf(s_fp, "w %05x %d %d, %d %d %d %d\n", BITBLTBUF.DBP, BITBLTBUF.DBW, BITBLTBUF.DPSM, r.x, r.y, r.z, r.w); fflush(s_fp);}
	
	GSOffset* o = m_mem.GetOffset(BITBLTBUF.DBP, BITBLTBUF.DBW, BITBLTBUF.DPSM);

	o->GetPages(r, m_tmp_pages);

	if(m_cl.pb.head < m_cl.pb.tail)
	{
		for(uint32* RESTRICT p = m_tmp_pages; *p != GSOffset::EOP; p++)
		{
			if(m_rw_pages[*p] & 3) // rw
			{
				Sync(3);

				break;
			}
		}
	}

	// TODO: invalidate texture cache pages by m_tmp_pages
}

void GSRendererCL::InvalidateLocalMem(const GIFRegBITBLTBUF& BITBLTBUF, const GSVector4i& r, bool clut)
{
	if(LOG) {fprintf(s_fp, "%s %05x %d %d, %d %d %d %d\n", clut ? "rp" : "r", BITBLTBUF.SBP, BITBLTBUF.SBW, BITBLTBUF.SPSM, r.x, r.y, r.z, r.w); fflush(s_fp);}
	
	if(m_cl.pb.head < m_cl.pb.tail)
	{
		GSOffset* o = m_mem.GetOffset(BITBLTBUF.SBP, BITBLTBUF.SBW, BITBLTBUF.SPSM);

		o->GetPages(r, m_tmp_pages);

		for(uint32* RESTRICT p = m_tmp_pages; *p != GSOffset::EOP; p++)
		{
			if(m_rw_pages[*p] & 1) // w
			{
				Sync(4);

				break;
			}
		}
	}
}
/*
bool GSRendererCL::CheckSourcePages(RasterizerData* data)
{
	// TODO: if(!m_rl->IsSynced()) // TODO: all callbacks from the issued drawings reported in => in-sync
	{
		for(size_t i = 0; data->m_tex[i].t != NULL; i++)
		{
			data->m_tex[i].t->m_offset->GetPages(data->m_tex[i].r, m_tmp_pages);

			uint32* pages = m_tmp_pages; // data->m_tex[i].t->m_pages.n;

			for(const uint32* p = pages; *p != GSOffset::EOP; p++)
			{
				// TODO: 8H 4HL 4HH texture at the same place as the render target (24 bit, or 32-bit where the alpha channel is masked, Valkyrie Profile 2)

				if(m_fzb_pages[*p]) // currently being drawn to? => sync
				{
					return true;
				}
			}
		}
	}

	return false;
}
*/

//#include "GSTextureCL.h"

void GSRendererCL::Enqueue()
{
	if(m_jobs.empty()) return;

	try
	{
		ASSERT(m_cl.vb.tail > m_cl.vb.head);
		ASSERT(m_cl.ib.tail > m_cl.ib.head);
		ASSERT(m_cl.pb.tail > m_cl.pb.head);

		int primclass = m_jobs.front().sel.prim;

		uint32 n;

		switch(primclass)
		{
		case GS_POINT_CLASS: n = 1; break;
		case GS_LINE_CLASS: n = 2; break;
		case GS_TRIANGLE_CLASS: n = 3; break;
		case GS_SPRITE_CLASS: n = 2; break;
		default: __assume(0);
		}

		PrimSelector psel;

		psel.key = 0;
		psel.prim = primclass;

		cl::Kernel& pk = m_cl.GetPrimKernel(psel);

		pk.setArg(1, m_cl.vb.buff[m_cl.wqidx]);
		pk.setArg(2, m_cl.ib.buff[m_cl.wqidx]);

		TileSelector tsel;

		tsel.key = 0;
		tsel.prim = primclass;

		tsel.mode = 0;

		cl::Kernel& tk_32 = m_cl.GetTileKernel(tsel);

		tsel.mode = 1;

		cl::Kernel& tk_16 = m_cl.GetTileKernel(tsel);

		tsel.mode = 2;

		cl::Kernel& tk_8 = m_cl.GetTileKernel(tsel);

		tsel.mode = 3;

		cl::Kernel& tk = m_cl.GetTileKernel(tsel);

		tsel.key = 0;
		tsel.clear = 1;

		cl::Kernel& tk_clear = m_cl.GetTileKernel(tsel);

		//

		m_cl.Unmap();

		std::vector<cl::Event> el2(1);

		m_cl.wq->enqueueMarkerWithWaitList(NULL, &el2[0]);
		m_cl.queue[2].enqueueBarrierWithWaitList(&el2, NULL);

		//

		cl_kernel tfx_prev = NULL;

		auto head = m_jobs.begin();

		while(head != m_jobs.end())
		{
			uint32 total_prim_count = 0;

			auto next = head;

			while(next != m_jobs.end())
			{
				auto job = next++;

				uint32 cur_prim_count = job->ib_count / n;

				total_prim_count += cur_prim_count;

				if(total_prim_count >= MAX_PRIM_COUNT || next == m_jobs.end())
				{
					uint32 prim_count = std::min(total_prim_count, MAX_PRIM_COUNT);

					pk.setArg(3, (cl_uint)m_vb_start);
					pk.setArg(4, (cl_uint)head->ib_start);

					m_cl.queue[2].enqueueNDRangeKernel(pk, cl::NullRange, cl::NDRange(prim_count), cl::NullRange);

					if(0)
					{
						gs_env* ptr = (gs_env*)m_cl.queue[2].enqueueMapBuffer(m_cl.env, CL_TRUE, CL_MAP_READ, 0, sizeof(gs_env));
						m_cl.queue[2].enqueueUnmapMemObject(m_cl.env, ptr);
					}

					GSVector4i rect = GSVector4i::zero();

					for(auto i = head; i != next; i++)
					{
						rect = rect.runion(GSVector4i::load<false>(&i->rect));
					}

					rect = rect.ralign<Align_Outside>(GSVector2i(BIN_SIZE, BIN_SIZE)) >> BIN_SIZE_BITS;

					int bin_w = rect.width();
					int bin_h = rect.height();

					uint32 batch_count = BATCH_COUNT(prim_count);
					uint32 bin_count = bin_w * bin_h;

					cl_uchar4 bin_dim;

					bin_dim.s[0] = (cl_uchar)rect.x;
					bin_dim.s[1] = (cl_uchar)rect.y;
					bin_dim.s[2] = (cl_uchar)bin_w;
					bin_dim.s[3] = (cl_uchar)bin_h;

					if(1)//bin_w > 1 || bin_h > 1) // && not just one sprite covering the whole area
					{
						m_cl.queue[2].enqueueNDRangeKernel(tk_clear, cl::NullRange, cl::NDRange(bin_count), cl::NullRange);

						if(bin_count <= 32 && m_cl.WIs >= 256)
						{
							uint32 item_count;
							uint32 group_count;
							cl::Kernel* k;

							if(bin_count <= 8)
							{
								item_count = std::min(prim_count, 32u);
								group_count = ((prim_count + 31) >> 5) * item_count;
								k = &tk_32;
							}
							else if(bin_count <= 16)
							{
								item_count = std::min(prim_count, 16u);
								group_count = ((prim_count + 15) >> 4) * item_count;
								k = &tk_16;
							}
							else
							{
								item_count = std::min(prim_count, 8u);
								group_count = ((prim_count + 7) >> 3) * item_count;
								k = &tk_8;
							}

							k->setArg(1, (cl_uint)prim_count);
							k->setArg(2, (cl_uint)bin_count);
							k->setArg(3, bin_dim);

							m_cl.queue[2].enqueueNDRangeKernel(*k, cl::NullRange, cl::NDRange(bin_w, bin_h, group_count), cl::NDRange(bin_w, bin_h, item_count));
						}
						else
						{
							uint32 item_count = std::min(bin_count, m_cl.WIs);
							uint32 group_count = std::min(batch_count, m_cl.CUs) * item_count;

							tk.setArg(1, (cl_uint)prim_count);
							tk.setArg(2, (cl_uint)batch_count);
							tk.setArg(3, (cl_uint)bin_count);
							tk.setArg(4, bin_dim);

							m_cl.queue[2].enqueueNDRangeKernel(tk, cl::NullRange, cl::NDRange(group_count), cl::NDRange(item_count));
						}

						if(0)
						{
							gs_env* ptr = (gs_env*)m_cl.queue[2].enqueueMapBuffer(m_cl.env, CL_TRUE, CL_MAP_READ, 0, sizeof(gs_env));
							m_cl.queue[2].enqueueUnmapMemObject(m_cl.env, ptr);
						}
					}

					//

					uint32 prim_start = 0;

					for(auto i = head; i != next; i++)
					{
						ASSERT(prim_start < MAX_PRIM_COUNT);

						uint32 prim_count_inner = std::min(i->ib_count / n, MAX_PRIM_COUNT - prim_start);

						// TODO: update the needed pages of the texture cache buffer with enqueueCopyBuffer (src=this->vm),
						// changed by tfx in the previous loop or marked by InvalidateVideoMem

						// TODO: tile level z test

						// TODO: merge sequential tfx calls having the same i->sel and texture and no dependencies, and pass the param index for each prim in the prim buffer (set by pk earlier)

						cl::Kernel& tfx = m_cl.GetTFXKernel(i->sel);

						if(tfx_prev != tfx())
						{
							tfx.setArg(2, sizeof(m_cl.pb.buff[m_cl.wqidx]), &m_cl.pb.buff[m_cl.wqidx]);

							tfx_prev = tfx();
						}

						tfx.setArg(3, (cl_uint)i->pb_start);
						tfx.setArg(4, (cl_uint)prim_start);
						tfx.setArg(5, (cl_uint)prim_count_inner);
						tfx.setArg(6, (cl_uint)batch_count);
						tfx.setArg(7, (cl_uint)bin_count);
						tfx.setArg(8, bin_dim);

						//m_cl.queue[2].enqueueNDRangeKernel(tfx, cl::NullRange, cl::NDRange(std::min(bin_count * 4, CUs) * 256), cl::NDRange(256));

						// TODO: limit to i->rect

						//printf("%d %d %d %d\n", rect.width() << BIN_SIZE_BITS, rect.height() << BIN_SIZE_BITS, i->rect.z - i->rect.x, i->rect.w - i->rect.y);

						GSVector4i r = GSVector4i::load<false>(&i->rect);

						r = r.ralign<Align_Outside>(GSVector2i(BIN_SIZE, BIN_SIZE));

						if(i->sel.IsSolidRect()) // TODO: simple mem fill
							;//printf("%d %d %d %d\n", r.left, r.top, r.width(), r.height());
						else
							m_cl.queue[2].enqueueNDRangeKernel(tfx, cl::NDRange(r.left, r.top), cl::NDRange(r.width(), r.height()), cl::NDRange(16, 16));

						// TODO: invalidate texture cache pages

						prim_start += prim_count_inner;
					}

					//

					if(total_prim_count > MAX_PRIM_COUNT)
					{
						prim_count = cur_prim_count - (total_prim_count - MAX_PRIM_COUNT);

						job->ib_start += prim_count * n * sizeof(uint32);
						job->ib_count -= prim_count * n;

						next = job; // try again for the reminder
					}

					break;
				}
			}

			head = next;
		}
	}
	catch(cl::Error err)
	{
		printf("%s (%d)\n", err.what(), err.err());
	}

	m_jobs.clear();

	m_vb_count = 0;

	m_cl.vb.head = m_cl.vb.tail;
	m_cl.ib.head = m_cl.ib.tail;
	m_cl.pb.head = m_cl.pb.tail;

	m_cl.Map();
}

bool GSRendererCL::SetupParameter(TFXParameter* pb, GSVertexCL* vertex, size_t vertex_count, const uint32* index, size_t index_count)
{
	const GSDrawingEnvironment& env = m_env;
	const GSDrawingContext* context = m_context;
	const GS_PRIM_CLASS primclass = m_vt.m_primclass;

	pb->sel.key = 0;

	pb->sel.atst = ATST_ALWAYS;
	pb->sel.tfx = TFX_NONE;
	pb->sel.ababcd = 0xff;
	pb->sel.prim = primclass;

	uint32 fm = context->FRAME.FBMSK;
	uint32 zm = context->ZBUF.ZMSK || context->TEST.ZTE == 0 ? 0xffffffff : 0;

	if(context->TEST.ZTE && context->TEST.ZTST == ZTST_NEVER)
	{
		fm = 0xffffffff;
		zm = 0xffffffff;
	}

	if(PRIM->TME)
	{
		if(GSLocalMemory::m_psm[context->TEX0.PSM].pal > 0)
		{
			m_mem.m_clut.Read32(context->TEX0, env.TEXA);
		}
	}

	if(context->TEST.ATE)
	{
		if(!TryAlphaTest(fm, zm))
		{
			pb->sel.atst = context->TEST.ATST;
			pb->sel.afail = context->TEST.AFAIL;

			pb->aref = (int)context->TEST.AREF;

			switch(pb->sel.atst)
			{
			case ATST_LESS:
				pb->sel.atst = ATST_LEQUAL;
				pb->aref -= 1;
				break;
			case ATST_GREATER:
				pb->sel.atst = ATST_GEQUAL;
				pb->aref += 1;
				break;
			}
		}
	}

	bool fwrite = fm != 0xffffffff;
	bool ftest = pb->sel.atst != ATST_ALWAYS || context->TEST.DATE && context->FRAME.PSM != PSM_PSMCT24;

	bool zwrite = zm != 0xffffffff;
	bool ztest = context->TEST.ZTE && context->TEST.ZTST > ZTST_ALWAYS;
	
	//printf("%05x %d %05x %d %05x %d %dx%d\n", 
		//fwrite || ftest ? m_context->FRAME.Block() : 0xfffff, m_context->FRAME.PSM,
		//zwrite || ztest ? m_context->ZBUF.Block() : 0xfffff, m_context->ZBUF.PSM,
		//PRIM->TME ? m_context->TEX0.TBP0 : 0xfffff, m_context->TEX0.PSM, (int)m_context->TEX0.TW, (int)m_context->TEX0.TH);
	
	if(!fwrite && !zwrite) return false;

	pb->sel.fwrite = fwrite;
	pb->sel.ftest = ftest;

	if(fwrite || ftest)
	{
		switch(context->FRAME.PSM)
		{
		default:
		case PSM_PSMCT32: pb->sel.fpsm = 0; break;
		case PSM_PSMCT24: pb->sel.fpsm = 1; break;
		case PSM_PSMCT16: pb->sel.fpsm = 2; break;
		case PSM_PSMCT16S: pb->sel.fpsm = 3; break;
		case PSM_PSMZ32: pb->sel.fpsm = 4; break;
		case PSM_PSMZ24: pb->sel.fpsm = 5; break;
		case PSM_PSMZ16: pb->sel.fpsm = 6; break;
		case PSM_PSMZ16S: pb->sel.fpsm = 7; break;
		}

		if((primclass == GS_LINE_CLASS || primclass == GS_TRIANGLE_CLASS) && m_vt.m_eq.rgba != 0xffff)
		{
			pb->sel.iip = PRIM->IIP;
		}

		if(PRIM->TME)
		{
			pb->sel.tfx = context->TEX0.TFX;
			pb->sel.tcc = context->TEX0.TCC;
			pb->sel.fst = PRIM->FST;
			pb->sel.ltf = m_vt.IsLinear();

			if(GSLocalMemory::m_psm[context->TEX0.PSM].pal > 0)
			{
				pb->sel.tlu = 1;

				memcpy(pb->clut, (const uint32*)m_mem.m_clut, sizeof(uint32) * GSLocalMemory::m_psm[context->TEX0.PSM].pal);
			}

			pb->sel.wms = context->CLAMP.WMS;
			pb->sel.wmt = context->CLAMP.WMT;

			if(pb->sel.tfx == TFX_MODULATE && pb->sel.tcc && m_vt.m_eq.rgba == 0xffff && m_vt.m_min.c.eq(GSVector4i(128)))
			{
				// modulate does not do anything when vertex color is 0x80

				pb->sel.tfx = TFX_DECAL;
			}

			// TODO: GSTextureCacheSW::Texture* t = m_tc->Lookup(context->TEX0, env.TEXA);

			// TODO: if(t == NULL) {ASSERT(0); return false;}

			GSVector4i r;

			GetTextureMinMax(r, context->TEX0, context->CLAMP, pb->sel.ltf);

			// TODO: data->SetSource(t, r, 0);

			// TODO: pb->sel.tw = t->m_tw - 3;

			if(m_mipmap && context->TEX1.MXL > 0 && context->TEX1.MMIN >= 2 && context->TEX1.MMIN <= 5 && m_vt.m_lod.y > 0)
			{
				// TEX1.MMIN
				// 000 p
				// 001 l
				// 010 p round
				// 011 p tri
				// 100 l round
				// 101 l tri

				if(m_vt.m_lod.x > 0)
				{
					pb->sel.ltf = context->TEX1.MMIN >> 2;
				}
				else
				{
					// TODO: isbilinear(mmag) != isbilinear(mmin) && m_vt.m_lod.x <= 0 && m_vt.m_lod.y > 0
				}

				pb->sel.mmin = (context->TEX1.MMIN & 1) + 1; // 1: round, 2: tri
				pb->sel.lcm = context->TEX1.LCM;

				int mxl = std::min<int>((int)context->TEX1.MXL, 6) << 16;
				int k = context->TEX1.K << 12;

				if((int)m_vt.m_lod.x >= (int)context->TEX1.MXL)
				{
					k = (int)m_vt.m_lod.x << 16; // set lod to max level

					pb->sel.lcm = 1; // lod is constant
					pb->sel.mmin = 1; // tri-linear is meaningless
				}

				if(pb->sel.mmin == 2)
				{
					mxl--; // don't sample beyond the last level (TODO: add a dummy level instead?)
				}

				if(pb->sel.fst)
				{
					ASSERT(pb->sel.lcm == 1);
					ASSERT(((m_vt.m_min.t.uph(m_vt.m_max.t) == GSVector4::zero()).mask() & 3) == 3); // ratchet and clank (menu)

					pb->sel.lcm = 1;
				}

				if(pb->sel.lcm)
				{
					int lod = std::max<int>(std::min<int>(k, mxl), 0);

					if(pb->sel.mmin == 1)
					{
						lod = (lod + 0x8000) & 0xffff0000; // rounding
					}

					pb->lod = lod;

					// TODO: lot to optimize when lod is constant
				}
				else
				{
					pb->mxl = mxl;
					pb->l = (float)(-0x10000 << context->TEX1.L);
					pb->k = (float)k;
				}

				GIFRegTEX0 MIP_TEX0 = context->TEX0;
				GIFRegCLAMP MIP_CLAMP = context->CLAMP;

				GSVector4 tmin = m_vt.m_min.t;
				GSVector4 tmax = m_vt.m_max.t;

				static int s_counter = 0;

				for(int i = 1, j = std::min<int>((int)context->TEX1.MXL, 6); i <= j; i++)
				{
					switch(i)
					{
					case 1:
						MIP_TEX0.TBP0 = context->MIPTBP1.TBP1;
						MIP_TEX0.TBW = context->MIPTBP1.TBW1;
						break;
					case 2:
						MIP_TEX0.TBP0 = context->MIPTBP1.TBP2;
						MIP_TEX0.TBW = context->MIPTBP1.TBW2;
						break;
					case 3:
						MIP_TEX0.TBP0 = context->MIPTBP1.TBP3;
						MIP_TEX0.TBW = context->MIPTBP1.TBW3;
						break;
					case 4:
						MIP_TEX0.TBP0 = context->MIPTBP2.TBP4;
						MIP_TEX0.TBW = context->MIPTBP2.TBW4;
						break;
					case 5:
						MIP_TEX0.TBP0 = context->MIPTBP2.TBP5;
						MIP_TEX0.TBW = context->MIPTBP2.TBW5;
						break;
					case 6:
						MIP_TEX0.TBP0 = context->MIPTBP2.TBP6;
						MIP_TEX0.TBW = context->MIPTBP2.TBW6;
						break;
					default:
						__assume(0);
					}

					if(MIP_TEX0.TW > 0) MIP_TEX0.TW--;
					if(MIP_TEX0.TH > 0) MIP_TEX0.TH--;

					MIP_CLAMP.MINU >>= 1;
					MIP_CLAMP.MINV >>= 1;
					MIP_CLAMP.MAXU >>= 1;
					MIP_CLAMP.MAXV >>= 1;

					m_vt.m_min.t *= 0.5f;
					m_vt.m_max.t *= 0.5f;

					// TODO: GSTextureCacheSW::Texture* t = m_tc->Lookup(MIP_TEX0, env.TEXA, pb->sel.tw + 3);

					// TODO: if(t == NULL) {ASSERT(0); return false;}

					GSVector4i r;

					GetTextureMinMax(r, MIP_TEX0, MIP_CLAMP, pb->sel.ltf);

					// TODO: data->SetSource(t, r, i);
				}

				s_counter++;

				m_vt.m_min.t = tmin;
				m_vt.m_max.t = tmax;
			}
			else
			{
				if(pb->sel.fst == 0)
				{
					// skip per pixel division if q is constant

					GSVertexCL* RESTRICT v = vertex;

					if(m_vt.m_eq.q)
					{
						pb->sel.fst = 1;

						const GSVector4& t = v[index[0]].t;

						if(t.z != 1.0f)
						{
							GSVector4 w = t.zzzz().rcpnr();

							for(int i = 0, j = vertex_count; i < j; i++)
							{
								GSVector4 t = v[i].t;

								v[i].t = (t * w).xyzw(t);
							}
						}
					}
					else if(primclass == GS_SPRITE_CLASS)
					{
						pb->sel.fst = 1;

						for(int i = 0, j = vertex_count; i < j; i += 2)
						{
							GSVector4 t0 = v[i + 0].t;
							GSVector4 t1 = v[i + 1].t;

							GSVector4 w = t1.zzzz().rcpnr();

							v[i + 0].t = (t0 * w).xyzw(t0);
							v[i + 1].t = (t1 * w).xyzw(t1);
						}
					}
				}

				if(pb->sel.ltf && pb->sel.fst) // TODO: quite slow, do this in the prim kernel?
				{
					// if q is constant we can do the half pel shift for bilinear sampling on the vertices

					// TODO: but not when mipmapping is used!!!

					GSVector4 half(0.5f, 0.5f);

					GSVertexCL* RESTRICT v = vertex;

					for(int i = 0, j = vertex_count; i < j; i++)
					{
						GSVector4 t = v[i].t;

						v[i].t = (t - half).xyzw(t);
					}
				}
			}

			uint16 tw = 1u << context->TEX0.TW;
			uint16 th = 1u << context->TEX0.TH;

			switch(context->CLAMP.WMS)
			{
			case CLAMP_REPEAT:
				pb->minu = tw - 1;
				pb->maxu = 0;
				//gd.t.mask.u32[0] = 0xffffffff;
				break;
			case CLAMP_CLAMP:
				pb->minu = 0;
				pb->maxu = tw - 1;
				//gd.t.mask.u32[0] = 0;
				break;
			case CLAMP_REGION_CLAMP:
				pb->minu = std::min<uint16>(context->CLAMP.MINU, tw - 1);
				pb->maxu = std::min<uint16>(context->CLAMP.MAXU, tw - 1);
				//gd.t.mask.u32[0] = 0;
				break;
			case CLAMP_REGION_REPEAT:
				pb->minu = context->CLAMP.MINU & (tw - 1);
				pb->maxu = context->CLAMP.MAXU & (tw - 1);
				//gd.t.mask.u32[0] = 0xffffffff;
				break;
			default:
				__assume(0);
			}

			switch(context->CLAMP.WMT)
			{
			case CLAMP_REPEAT:
				pb->minv = th - 1;
				pb->maxv = 0;
				//gd.t.mask.u32[2] = 0xffffffff;
				break;
			case CLAMP_CLAMP:
				pb->minv = 0;
				pb->maxv = th - 1;
				//gd.t.mask.u32[2] = 0;
				break;
			case CLAMP_REGION_CLAMP:
				pb->minv = std::min<uint16>(context->CLAMP.MINV, th - 1);
				pb->maxv = std::min<uint16>(context->CLAMP.MAXV, th - 1); // ffx anima summon scene, when the anchor appears (th = 256, maxv > 256)
				//gd.t.mask.u32[2] = 0;
				break;
			case CLAMP_REGION_REPEAT:
				pb->minv = context->CLAMP.MINV & (th - 1); // skygunner main menu water texture 64x64, MINV = 127
				pb->maxv = context->CLAMP.MAXV & (th - 1);
				//gd.t.mask.u32[2] = 0xffffffff;
				break;
			default:
				__assume(0);
			}
		}

		if(PRIM->FGE)
		{
			pb->sel.fge = 1;
			pb->fog = env.FOGCOL.u32[0];
		}

		if(context->FRAME.PSM != PSM_PSMCT24)
		{
			pb->sel.date = context->TEST.DATE;
			pb->sel.datm = context->TEST.DATM;
		}

		if(!IsOpaque())
		{
			pb->sel.abe = PRIM->ABE;
			pb->sel.ababcd = context->ALPHA.u32[0];

			if(env.PABE.PABE)
			{
				pb->sel.pabe = 1;
			}

			if(m_aa1 && PRIM->AA1 && (primclass == GS_LINE_CLASS || primclass == GS_TRIANGLE_CLASS))
			{
				pb->sel.aa1 = 1;
			}

			pb->afix = context->ALPHA.FIX;
		}

		if(pb->sel.date
		|| pb->sel.aba == 1 || pb->sel.abb == 1 || pb->sel.abc == 1 || pb->sel.abd == 1
		|| pb->sel.atst != ATST_ALWAYS && pb->sel.afail == AFAIL_RGB_ONLY
		|| (pb->sel.fpsm & 3) == 0 && fm != 0 && fm != 0xffffffff
		|| (pb->sel.fpsm & 3) == 1 && (fm & 0x00ffffff) != 0 && (fm & 0x00ffffff) != 0x00ffffff
		|| (pb->sel.fpsm & 3) >= 2 && (fm & 0x80f8f8f8) != 0 && (fm & 0x80f8f8f8) != 0x80f8f8f8)
		{
			pb->sel.rfb = 1;
		}

		pb->sel.colclamp = env.COLCLAMP.CLAMP;
		pb->sel.fba = context->FBA.FBA;

		if(env.DTHE.DTHE)
		{
			pb->sel.dthe = 1;

			pb->dimx[0] = env.dimx[1].sll32(16).sra32(16);
			pb->dimx[1] = env.dimx[3].sll32(16).sra32(16);
			pb->dimx[2] = env.dimx[5].sll32(16).sra32(16);
			pb->dimx[3] = env.dimx[7].sll32(16).sra32(16);
		}
	}

	pb->sel.zwrite = zwrite;
	pb->sel.ztest = ztest;

	if(zwrite || ztest)
	{
		switch(context->ZBUF.PSM)
		{
		case PSM_PSMCT32: pb->sel.zpsm = 0; break;
		case PSM_PSMCT24: pb->sel.zpsm = 1; break;
		case PSM_PSMCT16: pb->sel.zpsm = 2; break;
		case PSM_PSMCT16S: pb->sel.zpsm = 3; break;
		default:
		case PSM_PSMZ32: pb->sel.zpsm = 4; break;
		case PSM_PSMZ24: pb->sel.zpsm = 5; break;
		case PSM_PSMZ16: pb->sel.zpsm = 6; break;
		case PSM_PSMZ16S: pb->sel.zpsm = 7; break;
		}

		pb->sel.ztst = ztest ? context->TEST.ZTST : ZTST_ALWAYS;
		pb->sel.zoverflow = GSVector4i(m_vt.m_max.p).z == 0x80000000;
	}

	pb->fm = fm;
	pb->zm = zm;

	if((pb->sel.fpsm & 3) == 1)
	{
		pb->fm |= 0xff000000;
	}
	else if((pb->sel.fpsm & 3) >= 2)
	{
		uint32 rb = pb->fm & 0x00f800f8;
		uint32 ga = pb->fm & 0x8000f800;

		pb->fm = (ga >> 16) | (rb >> 9) | (ga >> 6) | (rb >> 3) | 0xffff0000;
	}

	if((pb->sel.zpsm & 3) == 1)
	{
		pb->zm |= 0xff000000;
	}
	else if((pb->sel.zpsm & 3) >= 2)
	{
		pb->zm |= 0xffff0000;
	}

	if(pb->sel.prim == GS_SPRITE_CLASS && !pb->sel.ftest && !pb->sel.ztest && pb->bbox.eq(pb->bbox.rintersect(pb->scissor)))
	{
		pb->sel.notest = 1;

		uint32 ofx = context->XYOFFSET.OFX;
	}

	pb->fbp = context->FRAME.Block();
	pb->zbp = context->ZBUF.Block();
	pb->bw = context->FRAME.FBW;

	return true;
}

//////////

//#define IOCL_DEBUG

GSRendererCL::CL::CL()
{
#ifdef IOCL_DEBUG
	std::vector<cl::Platform> platforms;

	cl::Platform::get(&platforms);

	for(auto& p : platforms)
	{
		std::string vendor = p.getInfo<CL_PLATFORM_VENDOR>();

		printf("%s\n", vendor.c_str());

		std::vector<cl::Device> devices;

		p.getDevices(CL_DEVICE_TYPE_ALL, &devices);

		for(auto& d : devices)
		{
			cl_device_type type = d.getInfo<CL_DEVICE_TYPE>();

			switch(type)
			{
			case CL_DEVICE_TYPE_GPU: printf("GPU\n"); break;
			case CL_DEVICE_TYPE_CPU: printf("CPU\n"); break;
			}

			if(type == CL_DEVICE_TYPE_CPU && strstr(vendor.c_str(), "Intel") != NULL)
			{
				device = d;
				context = cl::Context(device);
			}
		}
	}
#else
	context = cl::Context(CL_DEVICE_TYPE_GPU);
	//context = cl::Context(CL_DEVICE_TYPE_CPU);
	//context = cl::Context(CL_DEVICE_TYPE_DEFAULT);

	std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();

	if(devices.empty())
	{
		throw new std::exception("OpenCL device not found");
	}

	device = devices[0];
#endif
	CUs = device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
	WIs = device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>();

	queue[0] = cl::CommandQueue(context, device);
	queue[1] = cl::CommandQueue(context, device);
	queue[2] = cl::CommandQueue(context, device);

	vector<unsigned char> buff;

	if(theApp.LoadResource(IDR_TFX_CL, buff))
	{
		kernel_str = std::string((const char*)buff.data(), buff.size());
	}

	vb.head = vb.tail = vb.size = 0;
	ib.head = ib.tail = ib.size = 0;
	pb.head = pb.tail = pb.size = 0;

	vb.mapped_ptr = vb.ptr = NULL;
	ib.mapped_ptr = ib.ptr = NULL;
	pb.mapped_ptr = pb.ptr = NULL;

	pb.size = sizeof(TFXParameter) * 256;
	pb.buff[0] = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, pb.size);
	pb.buff[1] = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, pb.size);

	env = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(gs_env));

	wqidx = 0;
	wq = &queue[0];
}

GSRendererCL::CL::~CL()
{
	Unmap();
}

void GSRendererCL::CL::Map()
{
	Unmap();

	if(vb.head < vb.size)
	{
		vb.mapped_ptr = wq->enqueueMapBuffer(vb.buff[wqidx], CL_TRUE, CL_MAP_WRITE_INVALIDATE_REGION, vb.head, vb.size - vb.head);
		vb.ptr = (unsigned char*)vb.mapped_ptr - vb.head;
		ASSERT(((size_t)vb.ptr & 15) == 0);
		ASSERT((((size_t)vb.ptr + sizeof(GSVertexCL)) & 15) == 0);
	}

	if(ib.head < ib.size)
	{
		ib.mapped_ptr = wq->enqueueMapBuffer(ib.buff[wqidx], CL_TRUE, CL_MAP_WRITE_INVALIDATE_REGION, ib.head, ib.size - ib.head);
		ib.ptr = (unsigned char*)ib.mapped_ptr - ib.head;
		ASSERT(((size_t)ib.ptr & 15) == 0);
	}

	if(pb.head < pb.size)
	{
		pb.mapped_ptr = wq->enqueueMapBuffer(pb.buff[wqidx], CL_TRUE, CL_MAP_WRITE_INVALIDATE_REGION, pb.head, pb.size - pb.head);
		pb.ptr = (unsigned char*)pb.mapped_ptr - pb.head;
		ASSERT(((size_t)pb.ptr & 15) == 0);
		ASSERT((((size_t)pb.ptr + sizeof(TFXParameter)) & 15) == 0);
	}
}

void GSRendererCL::CL::Unmap()
{
	if(vb.mapped_ptr != NULL) wq->enqueueUnmapMemObject(vb.buff[wqidx], vb.mapped_ptr);
	if(ib.mapped_ptr != NULL) wq->enqueueUnmapMemObject(ib.buff[wqidx], ib.mapped_ptr);
	if(pb.mapped_ptr != NULL) wq->enqueueUnmapMemObject(pb.buff[wqidx], pb.mapped_ptr);

	vb.mapped_ptr = vb.ptr = NULL;
	ib.mapped_ptr = ib.ptr = NULL;
	pb.mapped_ptr = pb.ptr = NULL;
}

static void AddDefs(ostringstream& opt)
{
	opt << "-D MAX_FRAME_SIZE=" << MAX_FRAME_SIZE << "u ";
	opt << "-D MAX_PRIM_COUNT=" << MAX_PRIM_COUNT << "u ";
	opt << "-D MAX_PRIM_PER_BATCH_BITS=" << MAX_PRIM_PER_BATCH_BITS << "u ";
	opt << "-D MAX_PRIM_PER_BATCH=" << MAX_PRIM_PER_BATCH << "u ";
	opt << "-D MAX_BATCH_COUNT=" << MAX_BATCH_COUNT << "u ";
	opt << "-D BIN_SIZE_BITS=" << BIN_SIZE_BITS << " ";
	opt << "-D BIN_SIZE=" << BIN_SIZE << "u ";
	opt << "-D MAX_BIN_PER_BATCH=" << MAX_BIN_PER_BATCH << "u ";	
	opt << "-D MAX_BIN_COUNT=" << MAX_BIN_COUNT << "u ";
#ifdef IOCL_DEBUG
	opt << "-g -s \"E:\\Progs\\pcsx2\\plugins\\GSdx\\res\\tfx.cl\" ";
#endif
}

cl::Kernel& GSRendererCL::CL::GetPrimKernel(const PrimSelector& sel)
{
	auto i = prim_map.find(sel);

	if(i != prim_map.end())
	{
		return i->second;
	}

	char entry[256];

	sprintf(entry, "prim_%02x", sel);

	cl::Program program = cl::Program(context, kernel_str);

	try
	{
		ostringstream opt;

		opt << "-D KERNEL_PRIM=" << entry << " ";
		opt << "-D PRIM=" << sel.prim << " ";

		AddDefs(opt);

		program.build(opt.str().c_str());
	}
	catch(cl::Error err)
	{
		auto s = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);

		printf("kernel (%s) build error: %s\n", entry, s.c_str());

		throw err;
	}

	cl::Kernel k(program, entry);

	prim_map[sel] = k;

	k.setArg(0, env);

	return prim_map[sel];
}

cl::Kernel& GSRendererCL::CL::GetTileKernel(const TileSelector& sel)
{
	auto i = tile_map.find(sel);

	if(i != tile_map.end())
	{
		return i->second;
	}

	char entry[256];

	sprintf(entry, "tile_%02x", sel);

	cl::Program program = cl::Program(context, kernel_str);

	try
	{
		ostringstream opt;

		opt << "-D KERNEL_TILE=" << entry << " ";
		opt << "-D PRIM=" << sel.prim << " ";
		opt << "-D MODE=" << sel.mode << " ";
		opt << "-D CLEAR=" << sel.clear << " ";

		AddDefs(opt);

		program.build(opt.str().c_str());
	}
	catch(cl::Error err)
	{
		auto s = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);

		printf("kernel (%s) build error: %s\n", entry, s.c_str());

		throw err;
	}

	cl::Kernel k(program, entry);

	tile_map[sel] = k;

	k.setArg(0, env);

	return tile_map[sel];
}

cl::Kernel& GSRendererCL::CL::GetTFXKernel(const TFXSelector& sel)
{
	auto i = tfx_map.find(sel);

	if(i != tfx_map.end())
	{
		return i->second;
	}

	char entry[256];

	sprintf(entry, "tfx_%016x", sel);

	cl::Program program = cl::Program(context, kernel_str);

	try
	{
		ostringstream opt;

		opt << "-D KERNEL_TFX=" << entry << " ";
		opt << "-D FPSM=" << sel.fpsm << " ";
		opt << "-D ZPSM=" << sel.zpsm << " ";
		opt << "-D ZTST=" << sel.ztst << " ";
		opt << "-D ATST=" << sel.atst << " ";
		opt << "-D AFAIL=" << sel.afail << " ";
		opt << "-D IIP=" << sel.iip << " ";
		opt << "-D TFX=" << sel.tfx << " ";
		opt << "-D TCC=" << sel.tcc << " ";
		opt << "-D FST=" << sel.fst << " ";
		opt << "-D LTF=" << sel.ltf << " ";
		opt << "-D TLU=" << sel.tlu << " ";
		opt << "-D FGE=" << sel.fge << " ";
		opt << "-D DATE=" << sel.date << " ";
		opt << "-D ABE=" << sel.abe << " ";
		opt << "-D ABA=" << sel.aba << " ";
		opt << "-D ABB=" << sel.abb << " ";
		opt << "-D ABC=" << sel.abc << " ";
		opt << "-D ABD=" << sel.abd << " ";
		opt << "-D PABE=" << sel.pabe << " ";
		opt << "-D AA1=" << sel.aa1 << " ";
		opt << "-D FWRITE=" << sel.fwrite << " ";
		opt << "-D FTEST=" << sel.ftest << " ";
		opt << "-D RFB=" << sel.rfb << " ";
		opt << "-D ZWRITE=" << sel.zwrite << " ";
		opt << "-D ZTEST=" << sel.ztest << " ";
		opt << "-D ZOVERFLOW=" << sel.zoverflow << " ";
		opt << "-D WMS=" << sel.wms << " ";
		opt << "-D WMT=" << sel.wmt << " ";
		opt << "-D DATM=" << sel.datm << " ";
		opt << "-D COLCLAMP=" << sel.colclamp << " ";
		opt << "-D FBA=" << sel.fba << " ";
		opt << "-D DTHE=" << sel.dthe << " ";
		opt << "-D PRIM=" << sel.prim << " ";
		opt << "-D TW=" << sel.tw << " ";
		opt << "-D LCM=" << sel.lcm << " ";
		opt << "-D MMIN=" << sel.mmin << " ";
		opt << "-D NOTEST=" << sel.notest << " ";
		opt << "-D FB=" << sel.fb << " ";
		opt << "-D ZB=" << sel.zb << " ";

		AddDefs(opt);

		program.build(opt.str().c_str());
	}
	catch(cl::Error err)
	{
		auto s = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);

		printf("kernel (%s) build error: %s\n", entry, s.c_str());

		throw err;
	}

	cl::Kernel k(program, entry);

	tfx_map[sel] = k;

	k.setArg(0, env);
	k.setArg(1, vm);

	return tfx_map[sel];
}
