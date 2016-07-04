/*
 * Copyright © 2014 NVIDIA Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <inttypes.h>
#include <stdlib.h>

#include "util/u_debug.h"
#include "util/u_inlines.h"

#include "tegra/tegra_context.h"
#include "tegra/tegra_screen.h"

static void
tegra_destroy(struct pipe_context *pcontext)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p)\n", __func__, pcontext);

	context->gpu->destroy(context->gpu);
	free(context);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_draw_vbo(struct pipe_context *pcontext,
	       const struct pipe_draw_info *pinfo)
{
	struct tegra_context *context = to_tegra_context(pcontext);
	struct pipe_draw_info info;

	debug_printf("> %s(pcontext=%p, pinfo=%p)\n", __func__, pcontext,
		     pinfo);
	debug_printf("  pinfo:\n");
	debug_printf("    indexed: %d\n", pinfo->indexed);
	debug_printf("    mode: %x\n", pinfo->mode);
	debug_printf("    start: %u\n", pinfo->start);
	debug_printf("    count: %u\n", pinfo->count);

	if (pinfo && pinfo->indirect) {
		debug_printf("  unwrapping pipe_draw_info\n");
		memcpy(&info, pinfo, sizeof(info));
		info.indirect = tegra_resource_unwrap(info.indirect);
		pinfo = &info;
	}

	context->gpu->draw_vbo(context->gpu, pinfo);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_render_condition(struct pipe_context *pcontext,
		       struct pipe_query *query,
		       boolean condition,
		       unsigned int mode)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, query=%p, condition=%d, mode=%u)\n",
		     __func__, pcontext, query, condition, mode);

	context->gpu->render_condition(context->gpu, query, condition, mode);

	debug_printf("< %s()\n", __func__);
}

static struct pipe_query *
tegra_create_query(struct pipe_context *pcontext,
		   unsigned int query_type,
		   unsigned int index)
{
	struct tegra_context *context = to_tegra_context(pcontext);
	struct pipe_query *query;

	debug_printf("> %s(pcontext=%p, query_type=%u, index=%u)\n", __func__,
		     pcontext, query_type, index);

	query = context->gpu->create_query(context->gpu, query_type, index);

	debug_printf("< %s() = %p\n", __func__, query);
	return query;
}

static struct pipe_query *
tegra_create_batch_query(struct pipe_context *pcontext,
			 unsigned int num_queries,
			 unsigned int *queries)
{
	struct tegra_context *context = to_tegra_context(pcontext);
	struct pipe_query *query;

	debug_printf("> %s(pcontext=%p, num_queries=%u, queries=%p)\n",
		     __func__, pcontext, num_queries, queries);

	query = context->gpu->create_batch_query(context->gpu, num_queries,
						 queries);

	debug_printf("< %s() = %p\n", __func__, query);
	return query;
}

static void
tegra_destroy_query(struct pipe_context *pcontext, struct pipe_query *query)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, query=%p)\n", __func__, pcontext,
		     query);

	context->gpu->destroy_query(context->gpu, query);

	debug_printf("< %s()\n", __func__);
}

static boolean
tegra_begin_query(struct pipe_context *pcontext, struct pipe_query *query)
{
	struct tegra_context *context = to_tegra_context(pcontext);
	boolean ret;

	debug_printf("> %s(pcontext=%p, query=%p)\n", __func__, pcontext,
		     query);

	ret = context->gpu->begin_query(context->gpu, query);

	debug_printf("< %s() = %d\n", __func__, ret);
	return ret;
}

static bool
tegra_end_query(struct pipe_context *pcontext, struct pipe_query *query)
{
	struct tegra_context *context = to_tegra_context(pcontext);
	bool ret;

	debug_printf("> %s(pcontext=%p, query=%p)\n", __func__, pcontext,
		     query);

	ret = context->gpu->end_query(context->gpu, query);

	debug_printf("< %s()\n", __func__);
	return ret;
}

static boolean
tegra_get_query_result(struct pipe_context *pcontext,
		       struct pipe_query *query,
		       boolean wait,
		       union pipe_query_result *result)
{
	struct tegra_context *context = to_tegra_context(pcontext);
	boolean ret;

	debug_printf("> %s(pcontext=%p, query=%p, wait=%d, result=%p)\n",
		     __func__, pcontext, query, wait, result);

	ret = context->gpu->get_query_result(context->gpu, query, wait,
					     result);

	debug_printf("< %s() = %d\n", __func__, ret);
	return ret;
}

static void
tegra_get_query_result_resource(struct pipe_context *pcontext,
				struct pipe_query *query,
				boolean wait,
				enum pipe_query_value_type result_type,
				int index,
				struct pipe_resource *resource,
				unsigned int offset)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, query=%p, wait=%d, result_type=%d, index=%d, resource=%p, offset=%u)\n",
		     __func__, pcontext, query, wait, result_type, index,
		     resource, offset);

	context->gpu->get_query_result_resource(context->gpu, query, wait,
						result_type, index, resource,
						offset);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_set_active_query_state(struct pipe_context *pcontext, boolean enable)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, enable=%d)\n", __func__, pcontext,
		     enable);

	context->gpu->set_active_query_state(pcontext, enable);

	debug_printf("< %s()\n", __func__);
}

static void *
tegra_create_blend_state(struct pipe_context *pcontext,
			 const struct pipe_blend_state *cso)
{
	struct tegra_context *context = to_tegra_context(pcontext);
	void *so;

	debug_printf("> %s(pcontext=%p, cso=%p)\n", __func__, pcontext, cso);

	so = context->gpu->create_blend_state(context->gpu, cso);

	debug_printf("< %s() = %p\n", __func__, so);
	return so;
}

static void
tegra_bind_blend_state(struct pipe_context *pcontext, void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, so=%p)\n", __func__, pcontext, so);

	context->gpu->bind_blend_state(context->gpu, so);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_delete_blend_state(struct pipe_context *pcontext, void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, so=%p)\n", __func__, pcontext, so);

	context->gpu->delete_blend_state(context->gpu, so);

	debug_printf("< %s()\n", __func__);
}

static void *
tegra_create_sampler_state(struct pipe_context *pcontext,
			   const struct pipe_sampler_state *cso)
{
	struct tegra_context *context = to_tegra_context(pcontext);
	void *so;

	debug_printf("> %s(pcontext=%p, cso=%p)\n", __func__, pcontext, cso);

	so = context->gpu->create_sampler_state(context->gpu, cso);

	debug_printf("< %s() = %p\n", __func__, so);
	return so;
}

static void
tegra_bind_sampler_states(struct pipe_context *pcontext,
			  unsigned shader,
			  unsigned start_slot,
			  unsigned num_samplers,
			  void **samplers)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, shader=%u, start_slot=%u, num_samplers=%u, samplers=%p)\n",
		     __func__, pcontext, shader, start_slot, num_samplers,
		     samplers);

	context->gpu->bind_sampler_states(context->gpu, shader, start_slot,
					  num_samplers, samplers);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_delete_sampler_state(struct pipe_context *pcontext, void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, so=%p)\n", __func__, pcontext, so);

	context->gpu->delete_sampler_state(context->gpu, so);

	debug_printf("< %s()\n", __func__);
}

static void *
tegra_create_rasterizer_state(struct pipe_context *pcontext,
			      const struct pipe_rasterizer_state *cso)
{
	struct tegra_context *context = to_tegra_context(pcontext);
	void *so;

	debug_printf("> %s(pcontext=%p, cso=%p)\n", __func__, pcontext, cso);

	so = context->gpu->create_rasterizer_state(context->gpu, cso);

	debug_printf("< %s() = %p\n", __func__, so);
	return so;
}

static void
tegra_bind_rasterizer_state(struct pipe_context *pcontext, void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, so=%p)\n", __func__, pcontext, so);

	context->gpu->bind_rasterizer_state(context->gpu, so);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_delete_rasterizer_state(struct pipe_context *pcontext, void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, so=%p)\n", __func__, pcontext, so);

	context->gpu->delete_rasterizer_state(context->gpu, so);

	debug_printf("< %s()\n", __func__);
}

static void *
tegra_create_depth_stencil_alpha_state(struct pipe_context *pcontext,
				       const struct pipe_depth_stencil_alpha_state *cso)
{
	struct tegra_context *context = to_tegra_context(pcontext);
	void *so;

	debug_printf("> %s(pcontext=%p, cso=%p)\n", __func__, pcontext, cso);

	so = context->gpu->create_depth_stencil_alpha_state(context->gpu,
							    cso);

	debug_printf("< %s() = %p\n", __func__, so);
	return so;
}

static void
tegra_bind_depth_stencil_alpha_state(struct pipe_context *pcontext, void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, so=%p)\n", __func__, pcontext, so);

	context->gpu->bind_depth_stencil_alpha_state(context->gpu, so);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_delete_depth_stencil_alpha_state(struct pipe_context *pcontext, void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, so=%p)\n", __func__, pcontext, so);

	context->gpu->delete_depth_stencil_alpha_state(context->gpu, so);

	debug_printf("< %s()\n", __func__);
}

static void *
tegra_create_fs_state(struct pipe_context *pcontext,
		      const struct pipe_shader_state *cso)
{
	struct tegra_context *context = to_tegra_context(pcontext);
	void *so;

	debug_printf("> %s(pcontext=%p, cso=%p)\n", __func__, pcontext, cso);

	so = context->gpu->create_fs_state(context->gpu, cso);

	debug_printf("< %s() = %p\n", __func__, so);
	return so;
}

static void
tegra_bind_fs_state(struct pipe_context *pcontext, void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, so=%p)\n", __func__, pcontext, so);

	context->gpu->bind_fs_state(context->gpu, so);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_delete_fs_state(struct pipe_context *pcontext, void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, so=%p)\n", __func__, pcontext, so);

	context->gpu->delete_fs_state(context->gpu, so);

	debug_printf("< %s()\n", __func__);
}

static void *
tegra_create_vs_state(struct pipe_context *pcontext,
		      const struct pipe_shader_state *cso)
{
	struct tegra_context *context = to_tegra_context(pcontext);
	void *so;

	debug_printf("> %s(pcontext=%p, cso=%p)\n", __func__, pcontext, cso);

	so = context->gpu->create_vs_state(context->gpu, cso);

	debug_printf("< %s() = %p\n", __func__, so);
	return so;
}

static void
tegra_bind_vs_state(struct pipe_context *pcontext, void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, so=%p)\n", __func__, pcontext, so);

	context->gpu->bind_vs_state(context->gpu, so);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_delete_vs_state(struct pipe_context *pcontext, void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, so=%p)\n", __func__, pcontext, so);

	context->gpu->delete_vs_state(context->gpu, so);

	debug_printf("< %s()\n", __func__);
}

static void *
tegra_create_gs_state(struct pipe_context *pcontext,
		      const struct pipe_shader_state *cso)
{
	struct tegra_context *context = to_tegra_context(pcontext);
	void *so;

	debug_printf("> %s(pcontext=%p, cso=%p)\n", __func__, pcontext, cso);

	so = context->gpu->create_gs_state(context->gpu, cso);

	debug_printf("< %s() = %p\n", __func__, so);
	return so;
}

static void
tegra_bind_gs_state(struct pipe_context *pcontext, void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, so=%p)\n", __func__, pcontext, so);

	context->gpu->bind_gs_state(context->gpu, so);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_delete_gs_state(struct pipe_context *pcontext, void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, so=%p)\n", __func__, pcontext, so);

	context->gpu->delete_gs_state(context->gpu, so);

	debug_printf("< %s()\n", __func__);
}

static void *
tegra_create_tcs_state(struct pipe_context *pcontext,
		       const struct pipe_shader_state *cso)
{
	struct tegra_context *context = to_tegra_context(pcontext);
	void *so;

	debug_printf("> %s(pcontext=%p, cso=%p)\n", __func__, pcontext, cso);

	so = context->gpu->create_tcs_state(context->gpu, cso);

	debug_printf("< %s() = %p\n", __func__, so);
	return so;
}

static void
tegra_bind_tcs_state(struct pipe_context *pcontext, void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, so=%p)\n", __func__, pcontext, so);

	context->gpu->bind_tcs_state(context->gpu, so);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_delete_tcs_state(struct pipe_context *pcontext, void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, so=%p)\n", __func__, pcontext, so);

	context->gpu->delete_tcs_state(context->gpu, so);

	debug_printf("< %s()\n", __func__);
}

static void *
tegra_create_tes_state(struct pipe_context *pcontext,
		       const struct pipe_shader_state *cso)
{
	struct tegra_context *context = to_tegra_context(pcontext);
	void *so;

	debug_printf("> %s(pcontext=%p, cso=%p)\n", __func__, pcontext, cso);

	so = context->gpu->create_tes_state(context->gpu, cso);

	debug_printf("< %s() = %p\n", __func__, so);
	return so;
}

static void
tegra_bind_tes_state(struct pipe_context *pcontext, void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, so=%p)\n", __func__, pcontext, so);

	context->gpu->bind_tes_state(context->gpu, so);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_delete_tes_state(struct pipe_context *pcontext, void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, so=%p)\n", __func__, pcontext, so);

	context->gpu->delete_tes_state(context->gpu, so);

	debug_printf("< %s()\n", __func__);
}

static void *
tegra_create_vertex_elements_state(struct pipe_context *pcontext,
				   unsigned num_elements,
				   const struct pipe_vertex_element *elements)
{
	struct tegra_context *context = to_tegra_context(pcontext);
	void *so;

	debug_printf("> %s(pcontext=%p, num_elements=%u, elements=%p)\n",
		     __func__, pcontext, num_elements, elements);

	so = context->gpu->create_vertex_elements_state(context->gpu,
							num_elements,
							elements);

	debug_printf("< %s() = %p\n", __func__, so);
	return so;
}

static void
tegra_bind_vertex_elements_state(struct pipe_context *pcontext, void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, so=%p)\n", __func__, pcontext, so);

	context->gpu->bind_vertex_elements_state(context->gpu, so);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_delete_vertex_elements_state(struct pipe_context *pcontext, void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, so=%p)\n", __func__, pcontext, so);

	context->gpu->delete_vertex_elements_state(context->gpu, so);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_set_blend_color(struct pipe_context *pcontext,
		      const struct pipe_blend_color *color)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, color=%p)\n", __func__, pcontext,
		     color);

	context->gpu->set_blend_color(context->gpu, color);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_set_stencil_ref(struct pipe_context *pcontext,
		      const struct pipe_stencil_ref *ref)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, ref=%p)\n", __func__, pcontext, ref);

	context->gpu->set_stencil_ref(context->gpu, ref);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_set_sample_mask(struct pipe_context *pcontext, unsigned int mask)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, mask=%x)\n", __func__, pcontext,
		     mask);

	context->gpu->set_sample_mask(context->gpu, mask);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_set_min_samples(struct pipe_context *pcontext, unsigned int samples)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, samples=%x)\n", __func__, pcontext,
		     samples);

	context->gpu->set_min_samples(context->gpu, samples);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_set_clip_state(struct pipe_context *pcontext,
		     const struct pipe_clip_state *state)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, state=%p)\n", __func__, pcontext,
		     state);

	context->gpu->set_clip_state(context->gpu, state);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_set_constant_buffer(struct pipe_context *pcontext,
			  unsigned int shader,
			  unsigned int index,
			  const struct pipe_constant_buffer *buf)
{
	struct tegra_context *context = to_tegra_context(pcontext);
	struct pipe_constant_buffer buffer;

	debug_printf("> %s(pcontext=%p, shader=%u, index=%u, buf=%p)\n",
		     __func__, pcontext, shader, index, buf);

	if (buf && buf->buffer) {
		memcpy(&buffer, buf, sizeof(buffer));
		debug_printf("  buffer: %p -> %p\n", buffer.buffer,
			     tegra_resource_unwrap(buffer.buffer));
		buffer.buffer = tegra_resource_unwrap(buffer.buffer);
		buf = &buffer;
	}

	context->gpu->set_constant_buffer(context->gpu, shader, index, buf);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_set_framebuffer_state(struct pipe_context *pcontext,
			    const struct pipe_framebuffer_state *fb)
{
	struct tegra_context *context = to_tegra_context(pcontext);
	struct pipe_framebuffer_state state;
	unsigned i;

	debug_printf("> %s(pcontext=%p, fb=%p)\n", __func__, pcontext, fb);

	if (fb) {
		memcpy(&state, fb, sizeof(state));

		for (i = 0; i < fb->nr_cbufs; i++) {
			state.cbufs[i] = tegra_surface_unwrap(fb->cbufs[i]);
			debug_printf("  %u: %p -> %p\n", i, fb->cbufs[i],
				     state.cbufs[i]);
		}

		while (i < PIPE_MAX_COLOR_BUFS)
			state.cbufs[i++] = NULL;

		state.zsbuf = tegra_surface_unwrap(fb->zsbuf);
		debug_printf("  zsbuf: %p -> %p\n", fb->zsbuf, state.zsbuf);

		fb = &state;
	}

	context->gpu->set_framebuffer_state(context->gpu, fb);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_set_polygon_stipple(struct pipe_context *pcontext,
			  const struct pipe_poly_stipple *stipple)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, stipple=%p)\n", __func__, pcontext,
		     stipple);

	context->gpu->set_polygon_stipple(context->gpu, stipple);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_set_scissor_states(struct pipe_context *pcontext,
			 unsigned start_slot,
			 unsigned num_scissors,
			 const struct pipe_scissor_state *scissors)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, start_slot=%u, num_scissors=%u, scissors=%p)\n",
		     __func__, pcontext, start_slot, num_scissors, scissors);

	context->gpu->set_scissor_states(context->gpu, start_slot,
					 num_scissors, scissors);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_set_window_rectangles(struct pipe_context *pcontext,
			    boolean include,
			    unsigned int num_rectangles,
			    const struct pipe_scissor_state *rectangles)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, include=%d, num_rectangles=%u, rectangles=%p)\n",
		     __func__, pcontext, include, num_rectangles, rectangles);

	context->gpu->set_window_rectangles(context->gpu, include,
					    num_rectangles, rectangles);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_set_viewport_states(struct pipe_context *pcontext,
			  unsigned start_slot,
			  unsigned num_viewports,
			  const struct pipe_viewport_state *viewports)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, start_slot=%u, num_viewports=%u, viewports=%p)\n",
		     __func__, pcontext, start_slot, num_viewports,
		     viewports);

	context->gpu->set_viewport_states(context->gpu, start_slot,
					  num_viewports, viewports);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_set_sampler_views(struct pipe_context *pcontext,
			unsigned shader,
			unsigned start_slot,
			unsigned num_views,
			struct pipe_sampler_view **pviews)
{
	struct pipe_sampler_view *views[PIPE_MAX_SHADER_SAMPLER_VIEWS];
	struct tegra_context *context = to_tegra_context(pcontext);
	unsigned i;

	debug_printf("> %s(pcontext=%p, shader=%u, start_slot=%u, num_views=%u, pviews=%p)\n",
		     __func__, pcontext, shader, start_slot, num_views,
		     pviews);

	for (i = 0; i < num_views; i++)
		views[i] = tegra_sampler_view_unwrap(pviews[i]);

	context->gpu->set_sampler_views(context->gpu, shader, start_slot,
					num_views, views);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_set_tess_state(struct pipe_context *pcontext,
		     const float default_outer_level[4],
		     const float default_inner_level[2])
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, default_outer_level=%p, default_inner_level=%p)\n",
		     __func__, pcontext, default_outer_level,
		     default_inner_level);

	context->gpu->set_tess_state(context->gpu, default_outer_level,
				     default_inner_level);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_set_debug_callback(struct pipe_context *pcontext,
			 const struct pipe_debug_callback *callback)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, callback=%p)\n", __func__, pcontext,
		     callback);

	context->gpu->set_debug_callback(context->gpu, callback);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_set_shader_buffers(struct pipe_context *pcontext,
			 unsigned int shader,
			 unsigned start,
			 unsigned count,
			 const struct pipe_shader_buffer *buffers)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, shader=%u, start=%u, count=%u, buffers=%p)\n",
		     __func__, pcontext, shader, start, count, buffers);

	context->gpu->set_shader_buffers(context->gpu, shader, start, count,
					 buffers);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_set_shader_images(struct pipe_context *pcontext,
			unsigned int shader,
			unsigned start,
			unsigned count,
			const struct pipe_image_view *images)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, shader=%u, start=%u, count=%u, images=%p)\n",
		     __func__, pcontext, shader, start, count, images);

	context->gpu->set_shader_images(context->gpu, shader, start, count,
					images);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_set_vertex_buffers(struct pipe_context *pcontext,
			 unsigned start_slot,
			 unsigned num_buffers,
			 const struct pipe_vertex_buffer *buffers)
{
	struct tegra_context *context = to_tegra_context(pcontext);
	struct pipe_vertex_buffer buf[PIPE_MAX_SHADER_INPUTS];
	unsigned i;

	debug_printf("> %s(pcontext=%p, start_slot=%u, num_buffers=%u, buffers=%p)\n",
		     __func__, pcontext, start_slot, num_buffers, buffers);

	if (num_buffers && buffers) {
		for (i = 0; i < num_buffers; i++) {
			debug_printf("  %u:\n", i);
			debug_printf("    stride: %u\n", buffers[i].stride);
			debug_printf("    offset: %u\n", buffers[i].buffer_offset);
			debug_printf("    buffer: %p\n", buffers[i].buffer);
			debug_printf("    user: %p\n", buffers[i].user_buffer);
		}

		memcpy(buf, buffers, num_buffers * sizeof(struct pipe_vertex_buffer));

		for (i = 0; i < num_buffers; i++) {
			debug_printf("  %u: %p -> %p\n", i, buf[i].buffer,
				     tegra_resource_unwrap(buf[i].buffer));
			buf[i].buffer = tegra_resource_unwrap(buf[i].buffer);
		}

		buffers = buf;
	}

	context->gpu->set_vertex_buffers(context->gpu, start_slot,
					 num_buffers, buffers);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_set_index_buffer(struct pipe_context *pcontext,
		       const struct pipe_index_buffer *buffer)
{
	struct tegra_context *context = to_tegra_context(pcontext);
	struct pipe_index_buffer buf;

	debug_printf("> %s(pcontext=%p, buffer=%p)\n", __func__, pcontext,
		     buffer);

	if (buffer) {
		memcpy(&buf, buffer, sizeof(buf));
		debug_printf("  buffer: %p -> %p\n", buf.buffer,
			     tegra_resource_unwrap(buf.buffer));
		buf.buffer = tegra_resource_unwrap(buf.buffer);
		buffer = &buf;
	}

	context->gpu->set_index_buffer(context->gpu, buffer);

	debug_printf("< %s()\n", __func__);
}

static struct pipe_stream_output_target *
tegra_create_stream_output_target(struct pipe_context *pcontext,
				  struct pipe_resource *presource,
				  unsigned buffer_offset,
				  unsigned buffer_size)
{
	struct tegra_resource *resource = to_tegra_resource(presource);
	struct tegra_context *context = to_tegra_context(pcontext);
	struct pipe_stream_output_target *target;

	debug_printf("> %s(pcontext=%p, presource=%p, buffer_offset=%u, buffer_size=%u)\n",
		     __func__, pcontext, presource, buffer_offset,
		     buffer_size);

	target = context->gpu->create_stream_output_target(context->gpu,
							   resource->gpu,
							   buffer_offset,
							   buffer_size);

	debug_printf("< %s() = %p\n", __func__, target);
	return target;
}

static void
tegra_stream_output_target_destroy(struct pipe_context *pcontext,
				   struct pipe_stream_output_target *target)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, target=%p)\n", __func__, pcontext,
		     target);

	context->gpu->stream_output_target_destroy(context->gpu, target);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_set_stream_output_targets(struct pipe_context *pcontext,
				unsigned num_targets,
				struct pipe_stream_output_target **targets,
				const unsigned *offsets)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, num_targets=%u, targets=%p, offsets=%p)\n",
		     __func__, pcontext, num_targets, targets, offsets);

	context->gpu->set_stream_output_targets(context->gpu, num_targets,
						targets, offsets);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_resource_copy_region(struct pipe_context *pcontext,
			   struct pipe_resource *dst,
			   unsigned int dst_level,
			   unsigned int dstx,
			   unsigned int dsty,
			   unsigned int dstz,
			   struct pipe_resource *src,
			   unsigned int src_level,
			   const struct pipe_box *src_box)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, dst=%p, dst_level=%u, dstx=%x, dsty=%x, dstz=%u, src=%p, src_level=%u, src_box=%p)\n",
		     __func__, pcontext, dst, dst_level, dstx, dsty, dstz,
		     src, src_level, src_box);

	context->gpu->resource_copy_region(pcontext, dst, dst_level, dstx,
					   dsty, dstz, src, src_level,
					   src_box);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_blit(struct pipe_context *pcontext,
	   const struct pipe_blit_info *pinfo)
{
	struct tegra_context *context = to_tegra_context(pcontext);
	struct pipe_blit_info info;

	debug_printf("> %s(pcontext=%p, pinfo=%p)\n", __func__, pcontext,
		     pinfo);

	if (pinfo) {
		memcpy(&info, pinfo, sizeof(info));
		info.dst.resource = tegra_resource_unwrap(info.dst.resource);
		info.src.resource = tegra_resource_unwrap(info.src.resource);
		pinfo = &info;
	}

	context->gpu->blit(context->gpu, pinfo);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_clear(struct pipe_context *pcontext,
	    unsigned buffers,
	    const union pipe_color_union *color,
	    double depth,
	    unsigned stencil)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, buffers=%x, color=%p, depth=%f, stencil=%u)\n",
		     __func__, pcontext, buffers, color, depth, stencil);

	context->gpu->clear(context->gpu, buffers, color, depth, stencil);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_clear_render_target(struct pipe_context *pcontext,
			  struct pipe_surface *dst,
			  const union pipe_color_union *color,
			  unsigned int dstx,
			  unsigned int dsty,
			  unsigned int width,
			  unsigned int height)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, dst=%p, color=%p, dstx=%u, dsty=%u, width=%u, height=%u)\n",
		     __func__, pcontext, dst, color, dstx, dsty, width,
		     height);

	context->gpu->clear_render_target(pcontext, dst, color, dstx, dsty,
					  width, height);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_clear_depth_stencil(struct pipe_context *pcontext,
			  struct pipe_surface *dst,
			  unsigned int flags,
			  double depth,
			  unsigned int stencil,
			  unsigned int dstx,
			  unsigned int dsty,
			  unsigned int width,
			  unsigned int height)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, dst=%p, flags=%x, depth=%f, stencil=%u, dstx=%u, dsty=%u, width=%u, height=%u)\n",
		     __func__, pcontext, dst, flags, depth, stencil, dstx,
		     dsty, width, height);

	context->gpu->clear_depth_stencil(pcontext, dst, flags, depth,
					  stencil, dstx, dsty, width, height);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_clear_texture(struct pipe_context *pcontext,
		    struct pipe_resource *res,
		    unsigned int level,
		    const struct pipe_box *box,
		    const void *data)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, res=%p, level=%u, box=%p, data=%p)\n",
		     __func__, pcontext, res, level, box, data);

	context->gpu->clear_texture(pcontext, res, level, box, data);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_clear_buffer(struct pipe_context *pcontext,
		   struct pipe_resource *res,
		   unsigned int offset,
		   unsigned int size,
		   const void *value,
		   int value_size)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, res=%p, offset=%u, size=%u, value=%p, value_size=%d)\n",
		     __func__, pcontext, res, offset, size, value,
		     value_size);

	context->gpu->clear_buffer(pcontext, res, offset, size, value,
				   value_size);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_flush(struct pipe_context *pcontext,
	    struct pipe_fence_handle **fence,
	    unsigned flags)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, fence=%p, flags=%x)\n", __func__,
		     pcontext, fence, flags);

	context->gpu->flush(context->gpu, fence, flags);

	debug_printf("< %s()\n", __func__);
}

static struct pipe_sampler_view *
tegra_create_sampler_view(struct pipe_context *pcontext,
			  struct pipe_resource *ptexture,
			  const struct pipe_sampler_view *template)
{
	struct tegra_resource *texture = to_tegra_resource(ptexture);
	struct tegra_context *context = to_tegra_context(pcontext);
	struct tegra_sampler_view *view;

	debug_printf("> %s(pcontext=%p, ptexture=%p, template=%p)\n",
		     __func__, pcontext, ptexture, template);

	view = calloc(1, sizeof(*view));
	if (!view)
		return NULL;

	view->gpu = context->gpu->create_sampler_view(context->gpu,
						      texture->gpu,
						      template);
	memcpy(&view->base, view->gpu, sizeof(*view->gpu));
	/* overwrite to prevent reference from being released */
	view->base.texture = NULL;

	pipe_reference_init(&view->base.reference, 1);
	pipe_resource_reference(&view->base.texture, ptexture);
	view->base.context = pcontext;

	debug_printf("< %s() = %p\n", __func__, &view->base);
	return &view->base;
}

static void
tegra_sampler_view_destroy(struct pipe_context *pcontext,
			   struct pipe_sampler_view *pview)
{
	struct tegra_sampler_view *view = to_tegra_sampler_view(pview);

	debug_printf("> %s(pcontext=%p, view=%p)\n", __func__, pcontext,
		     view);

	pipe_resource_reference(&view->base.texture, NULL);
	pipe_sampler_view_reference(&view->gpu, NULL);
	free(view);

	debug_printf("< %s()\n", __func__);
}

static struct pipe_surface *
tegra_create_surface(struct pipe_context *pcontext,
		     struct pipe_resource *presource,
		     const struct pipe_surface *template)
{
	struct tegra_resource *resource = to_tegra_resource(presource);
	struct tegra_context *context = to_tegra_context(pcontext);
	struct tegra_surface *surface;

	debug_printf("> %s(pcontext=%p, presource=%p, template=%p)\n",
		     __func__, pcontext, presource, template);

	surface = calloc(1, sizeof(*surface));
	if (!surface)
		return NULL;

	surface->gpu = context->gpu->create_surface(context->gpu,
						    resource->gpu,
						    template);
	if (!surface->gpu) {
		free(surface);
		return NULL;
	}

	memcpy(&surface->base, surface->gpu, sizeof(*surface->gpu));
	/* overwrite to prevent reference from being released */
	surface->base.texture = NULL;

	pipe_reference_init(&surface->base.reference, 1);
	pipe_resource_reference(&surface->base.texture, presource);
	surface->base.context = &context->base;

	debug_printf("  gpu: %p\n", surface->gpu);
	debug_printf("< %s() = %p\n", __func__, &surface->base);
	return &surface->base;
}

static void
tegra_surface_destroy(struct pipe_context *pcontext,
		      struct pipe_surface *psurface)
{
	struct tegra_surface *surface = to_tegra_surface(psurface);

	debug_printf("> %s(pcontext=%p, psurface=%p)\n", __func__, pcontext,
		     psurface);

	pipe_resource_reference(&surface->base.texture, NULL);
	pipe_surface_reference(&surface->gpu, NULL);
	free(surface);

	debug_printf("< %s()\n", __func__);
}

static void *
tegra_transfer_map(struct pipe_context *pcontext,
		   struct pipe_resource *presource,
		   unsigned level,
		   unsigned usage,
		   const struct pipe_box *box,
		   struct pipe_transfer **ptransfer)
{
	struct tegra_resource *resource = to_tegra_resource(presource);
	struct tegra_context *context = to_tegra_context(pcontext);
	struct tegra_transfer *transfer;

	debug_printf("> %s(pcontext=%p, presource=%p, level=%u, usage=%x, box=%p, ptransfer=%p)\n",
		     __func__, pcontext, presource, level, usage, box,
		     ptransfer);

	transfer = calloc(1, sizeof(*transfer));
	if (!transfer)
		return NULL;

	transfer->map = context->gpu->transfer_map(context->gpu,
						   resource->gpu,
						   level,
						   usage,
						   box,
						   &transfer->gpu);
	memcpy(&transfer->base, transfer->gpu, sizeof(*transfer->gpu));
	if (transfer->base.resource != NULL)
		debug_printf("  resource: %p\n", transfer->base.resource);
	transfer->base.resource = NULL;
	pipe_resource_reference(&transfer->base.resource, presource);

	*ptransfer = &transfer->base;

	debug_printf("< %s() = %p\n", __func__, transfer->map);
	return transfer->map;
}

static void
tegra_transfer_flush_region(struct pipe_context *pcontext,
			    struct pipe_transfer *transfer,
			    const struct pipe_box *box)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, transfer=%p, box=%p)\n", __func__,
		     pcontext, transfer, box);

	context->gpu->transfer_flush_region(context->gpu, transfer, box);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_transfer_unmap(struct pipe_context *pcontext,
		     struct pipe_transfer *ptransfer)
{
	struct tegra_transfer *transfer = to_tegra_transfer(ptransfer);
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, ptransfer=%p)\n", __func__, pcontext,
		     ptransfer);

	context->gpu->transfer_unmap(context->gpu, transfer->gpu);
	pipe_resource_reference(&transfer->base.resource, NULL);
	free(transfer);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_transfer_inline_write(struct pipe_context *pcontext,
			    struct pipe_resource *presource,
			    unsigned level,
			    unsigned usage,
			    const struct pipe_box *box,
			    const void *data,
			    unsigned stride,
			    unsigned layer_stride)
{
	struct tegra_resource *resource = to_tegra_resource(presource);
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, presource=%p, level=%u, usage=%x, box=%p, data=%p, stride=%u, layer_stride=%u)\n",
		     __func__, pcontext, presource, level, usage, box, data,
		     stride, layer_stride);

	context->gpu->transfer_inline_write(context->gpu, resource->gpu,
					    level, usage, box, data, stride,
					    layer_stride);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_texture_barrier(struct pipe_context *pcontext)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p)\n", __func__, pcontext);

	context->gpu->texture_barrier(context->gpu);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_memory_barrier(struct pipe_context *pcontext, unsigned int flags)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, flags=%x)\n", __func__, pcontext,
		     flags);

	context->gpu->memory_barrier(context->gpu, flags);

	debug_printf("< %s()\n", __func__);
}

static struct pipe_video_codec *
tegra_create_video_codec(struct pipe_context *pcontext,
			 const struct pipe_video_codec *template)
{
	struct tegra_context *context = to_tegra_context(pcontext);
	struct pipe_video_codec *codec;

	debug_printf("> %s(pcontext=%p, template=%p)\n", __func__, pcontext,
		     template);

	codec = context->gpu->create_video_codec(context->gpu, template);

	debug_printf("< %s() = %p\n", __func__, codec);
	return codec;
}

static struct pipe_video_buffer *
tegra_create_video_buffer(struct pipe_context *pcontext,
			  const struct pipe_video_buffer *template)
{
	struct tegra_context *context = to_tegra_context(pcontext);
	struct pipe_video_buffer *buffer;

	debug_printf("> %s(pcontext=%p, template=%p)\n", __func__, pcontext,
		     template);

	buffer = context->gpu->create_video_buffer(context->gpu, template);

	debug_printf("< %s() = %p\n", __func__, buffer);
	return buffer;
}

static void *
tegra_create_compute_state(struct pipe_context *pcontext,
			   const struct pipe_compute_state *template)
{
	struct tegra_context *context = to_tegra_context(pcontext);
	void *so;

	debug_printf("> %s(pcontext=%p, template=%p)\n", __func__, pcontext,
		     template);

	so = context->gpu->create_compute_state(context->gpu, template);

	debug_printf("< %s() = %p\n", __func__, so);
	return so;
}

static void
tegra_bind_compute_state(struct pipe_context *pcontext, void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, so=%p)\n", __func__, pcontext, so);

	context->gpu->bind_compute_state(context->gpu, so);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_delete_compute_state(struct pipe_context *pcontext, void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, so=%p)\n", __func__, pcontext, so);

	context->gpu->delete_compute_state(context->gpu, so);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_set_compute_resources(struct pipe_context *pcontext,
			    unsigned int start,
			    unsigned int count,
			    struct pipe_surface **resources)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, start=%u, count=%u, resources=%p)\n",
		     __func__, pcontext, start, count, resources);

	context->gpu->set_compute_resources(context->gpu, start, count,
					    resources);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_set_global_binding(struct pipe_context *pcontext,
			 unsigned int first,
			 unsigned int count,
			 struct pipe_resource **resources,
			 uint32_t **handles)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, first=%u, count=%u, resources=%p, handles=%p)\n",
		     __func__, pcontext, first, count, resources, handles);

	context->gpu->set_global_binding(context->gpu, first, count,
					 resources, handles);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_launch_grid(struct pipe_context *pcontext,
		  const struct pipe_grid_info *info)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, info=%p)\n", __func__, pcontext, info);

	context->gpu->launch_grid(context->gpu, info);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_get_sample_position(struct pipe_context *pcontext,
			  unsigned int count,
			  unsigned int index,
			  float *value)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, count=%u, index=%u, value=%p)\n",
		     __func__, pcontext, count, index, value);

	context->gpu->get_sample_position(context->gpu, count, index, value);

	debug_printf("< %s()\n", __func__);
}

static uint64_t
tegra_get_timestamp(struct pipe_context *pcontext)
{
	struct tegra_context *context = to_tegra_context(pcontext);
	uint64_t timestamp;

	debug_printf("> %s(pcontext=%p)\n", __func__, pcontext);

	timestamp = context->gpu->get_timestamp(context->gpu);

	debug_printf("< %s() = %" PRIu64 "\n", __func__, timestamp);
	return timestamp;
}

static void
tegra_flush_resource(struct pipe_context *pcontext,
		     struct pipe_resource *resource)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, resource=%p)\n", __func__, pcontext,
		     resource);

	context->gpu->flush_resource(context->gpu, resource);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_invalidate_resource(struct pipe_context *pcontext,
			  struct pipe_resource *resource)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, resource=%p)\n", __func__, pcontext,
		     resource);

	context->gpu->invalidate_resource(context->gpu, resource);

	debug_printf("< %s()\n", __func__);
}

static enum pipe_reset_status
tegra_get_device_reset_status(struct pipe_context *pcontext)
{
	struct tegra_context *context = to_tegra_context(pcontext);
	enum pipe_reset_status status;

	debug_printf("> %s(pcontext=%p)\n", __func__, pcontext);

	status = context->gpu->get_device_reset_status(context->gpu);

	debug_printf("< %s() = %d\n", __func__, status);
	return status;
}

static void
tegra_dump_debug_state(struct pipe_context *pcontext, FILE *stream,
		       unsigned int flags)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, stream=%p, flags=%x)\n", __func__,
		     pcontext, stream, flags);

	context->gpu->dump_debug_state(context->gpu, stream, flags);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_emit_string_marker(struct pipe_context *pcontext, const char *string,
			 int length)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, string=%p, length=%d)\n", __func__,
		     pcontext, string, length);

	context->gpu->emit_string_marker(context->gpu, string, length);

	debug_printf("< %s()\n", __func__);
}

static boolean
tegra_generate_mipmap(struct pipe_context *pcontext,
		      struct pipe_resource *resource,
		      enum pipe_format format,
		      unsigned int base_level,
		      unsigned int last_level,
		      unsigned int first_layer,
		      unsigned int last_layer)
{
	struct tegra_context *context = to_tegra_context(pcontext);
	boolean ret;

	debug_printf("> %s(pcontext=%p, resource=%p, format=%d, base_level=%u, last_level=%u, first_layer=%u, last_layer=%u)\n",
		     __func__, pcontext, resource, format, base_level,
		     last_level, first_layer, last_layer);

	ret = context->gpu->generate_mipmap(context->gpu, resource, format,
					    base_level, last_level,
					    first_layer, last_layer);

	debug_printf("< %s() = %d\n", __func__, ret);
	return ret;
}

struct pipe_context *
tegra_screen_context_create(struct pipe_screen *pscreen, void *priv,
			    unsigned int flags)
{
	struct tegra_screen *screen = to_tegra_screen(pscreen);
	struct tegra_context *context;

	debug_printf("> %s(pscreen=%p, priv=%p, flags=%x)\n", __func__,
		     pscreen, priv, flags);

	context = calloc(1, sizeof(*context));
	if (!context)
		return NULL;

	context->gpu = screen->gpu->context_create(screen->gpu, priv, flags);
	if (!context->gpu) {
		debug_error("failed to create GPU context\n");
		free(context);
		return NULL;
	}

	context->base.screen = &screen->base;
	context->base.priv = priv;

	context->base.destroy = tegra_destroy;

	context->base.draw_vbo = tegra_draw_vbo;

	context->base.render_condition = tegra_render_condition;

	context->base.create_query = tegra_create_query;
	context->base.create_batch_query = tegra_create_batch_query;
	context->base.destroy_query = tegra_destroy_query;
	context->base.begin_query = tegra_begin_query;
	context->base.end_query = tegra_end_query;
	context->base.get_query_result = tegra_get_query_result;
	context->base.get_query_result_resource = tegra_get_query_result_resource;
	context->base.set_active_query_state = tegra_set_active_query_state;

	context->base.create_blend_state = tegra_create_blend_state;
	context->base.bind_blend_state = tegra_bind_blend_state;
	context->base.delete_blend_state = tegra_delete_blend_state;

	context->base.create_sampler_state = tegra_create_sampler_state;
	context->base.bind_sampler_states = tegra_bind_sampler_states;
	context->base.delete_sampler_state = tegra_delete_sampler_state;

	context->base.create_rasterizer_state = tegra_create_rasterizer_state;
	context->base.bind_rasterizer_state = tegra_bind_rasterizer_state;
	context->base.delete_rasterizer_state = tegra_delete_rasterizer_state;

	context->base.create_depth_stencil_alpha_state = tegra_create_depth_stencil_alpha_state;
	context->base.bind_depth_stencil_alpha_state = tegra_bind_depth_stencil_alpha_state;
	context->base.delete_depth_stencil_alpha_state = tegra_delete_depth_stencil_alpha_state;

	context->base.create_fs_state = tegra_create_fs_state;
	context->base.bind_fs_state = tegra_bind_fs_state;
	context->base.delete_fs_state = tegra_delete_fs_state;

	context->base.create_vs_state = tegra_create_vs_state;
	context->base.bind_vs_state = tegra_bind_vs_state;
	context->base.delete_vs_state = tegra_delete_vs_state;

	context->base.create_gs_state = tegra_create_gs_state;
	context->base.bind_gs_state = tegra_bind_gs_state;
	context->base.delete_gs_state = tegra_delete_gs_state;

	context->base.create_tcs_state = tegra_create_tcs_state;
	context->base.bind_tcs_state = tegra_bind_tcs_state;
	context->base.delete_tcs_state = tegra_delete_tcs_state;

	context->base.create_tes_state = tegra_create_tes_state;
	context->base.bind_tes_state = tegra_bind_tes_state;
	context->base.delete_tes_state = tegra_delete_tes_state;

	context->base.create_vertex_elements_state = tegra_create_vertex_elements_state;
	context->base.bind_vertex_elements_state = tegra_bind_vertex_elements_state;
	context->base.delete_vertex_elements_state = tegra_delete_vertex_elements_state;

	context->base.set_blend_color = tegra_set_blend_color;
	context->base.set_stencil_ref = tegra_set_stencil_ref;
	context->base.set_sample_mask = tegra_set_sample_mask;
	context->base.set_min_samples = tegra_set_min_samples;
	context->base.set_clip_state = tegra_set_clip_state;

	context->base.set_constant_buffer = tegra_set_constant_buffer;
	context->base.set_framebuffer_state = tegra_set_framebuffer_state;
	context->base.set_polygon_stipple = tegra_set_polygon_stipple;
	context->base.set_scissor_states = tegra_set_scissor_states;
	context->base.set_window_rectangles = tegra_set_window_rectangles;
	context->base.set_viewport_states = tegra_set_viewport_states;
	context->base.set_sampler_views = tegra_set_sampler_views;
	context->base.set_tess_state = tegra_set_tess_state;

	context->base.set_debug_callback = tegra_set_debug_callback;

	context->base.set_shader_buffers = tegra_set_shader_buffers;
	context->base.set_shader_images = tegra_set_shader_images;
	context->base.set_vertex_buffers = tegra_set_vertex_buffers;
	context->base.set_index_buffer = tegra_set_index_buffer;

	context->base.create_stream_output_target = tegra_create_stream_output_target;
	context->base.stream_output_target_destroy = tegra_stream_output_target_destroy;
	context->base.set_stream_output_targets = tegra_set_stream_output_targets;

	context->base.resource_copy_region = tegra_resource_copy_region;
	context->base.blit = tegra_blit;
	context->base.clear = tegra_clear;
	context->base.clear_render_target = tegra_clear_render_target;
	context->base.clear_depth_stencil = tegra_clear_depth_stencil;
	context->base.clear_texture = tegra_clear_texture;
	context->base.clear_buffer = tegra_clear_buffer;
	context->base.flush = tegra_flush;

	context->base.create_sampler_view = tegra_create_sampler_view;
	context->base.sampler_view_destroy = tegra_sampler_view_destroy;

	context->base.create_surface = tegra_create_surface;
	context->base.surface_destroy = tegra_surface_destroy;

	context->base.transfer_map = tegra_transfer_map;
	context->base.transfer_flush_region = tegra_transfer_flush_region;
	context->base.transfer_unmap = tegra_transfer_unmap;
	context->base.transfer_inline_write = tegra_transfer_inline_write;

	context->base.texture_barrier = tegra_texture_barrier;
	context->base.memory_barrier = tegra_memory_barrier;

	context->base.create_video_codec = tegra_create_video_codec;
	context->base.create_video_buffer = tegra_create_video_buffer;

	context->base.create_compute_state = tegra_create_compute_state;
	context->base.bind_compute_state = tegra_bind_compute_state;
	context->base.delete_compute_state = tegra_delete_compute_state;
	context->base.set_compute_resources = tegra_set_compute_resources;
	context->base.set_global_binding = tegra_set_global_binding;
	context->base.launch_grid = tegra_launch_grid;
	context->base.get_sample_position = tegra_get_sample_position;
	context->base.get_timestamp = tegra_get_timestamp;

	context->base.flush_resource = tegra_flush_resource;
	context->base.invalidate_resource = tegra_invalidate_resource;

	context->base.get_device_reset_status = tegra_get_device_reset_status;
	context->base.dump_debug_state = tegra_dump_debug_state;
	context->base.emit_string_marker = tegra_emit_string_marker;

	context->base.generate_mipmap = tegra_generate_mipmap;

	debug_printf("< %s() = %p\n", __func__, &context->base);
	return &context->base;
}
