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

#include <stdlib.h>

#include "util/u_debug.h"
#include "util/u_inlines.h"

#include "tegra/tegra_context.h"
#include "tegra/tegra_resource.h"
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
tegra_bind_blend_state(struct pipe_context *pcontext,
		       void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, so=%p)\n", __func__, pcontext, so);

	context->gpu->bind_blend_state(context->gpu, so);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_delete_blend_state(struct pipe_context *pcontext,
			 void *so)
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
tegra_delete_sampler_state(struct pipe_context *pcontext,
			   void *so)
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
tegra_bind_rasterizer_state(struct pipe_context *pcontext,
			    void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, so=%p)\n", __func__, pcontext, so);

	context->gpu->bind_rasterizer_state(context->gpu, so);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_delete_rasterizer_state(struct pipe_context *pcontext,
			      void *so)
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
tegra_bind_depth_stencil_alpha_state(struct pipe_context *pcontext,
				     void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, so=%p)\n", __func__, pcontext, so);

	context->gpu->bind_depth_stencil_alpha_state(context->gpu, so);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_delete_depth_stencil_alpha_state(struct pipe_context *pcontext,
				       void *so)
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
tegra_bind_fs_state(struct pipe_context *pcontext,
		    void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, so=%p)\n", __func__, pcontext, so);

	context->gpu->bind_fs_state(context->gpu, so);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_delete_fs_state(struct pipe_context *pcontext,
		      void *so)
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
tegra_bind_vs_state(struct pipe_context *pcontext,
		    void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, so=%p)\n", __func__, pcontext, so);

	context->gpu->bind_vs_state(context->gpu, so);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_delete_vs_state(struct pipe_context *pcontext,
		      void *so)
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
tegra_bind_gs_state(struct pipe_context *pcontext,
		    void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, so=%p)\n", __func__, pcontext, so);

	context->gpu->bind_gs_state(context->gpu, so);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_delete_gs_state(struct pipe_context *pcontext,
		      void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, so=%p)\n", __func__, pcontext, so);

	context->gpu->delete_gs_state(context->gpu, so);

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
tegra_bind_vertex_elements_state(struct pipe_context *pcontext,
				 void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, so=%p)\n", __func__, pcontext, so);

	context->gpu->bind_vertex_elements_state(context->gpu, so);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_delete_vertex_elements_state(struct pipe_context *pcontext,
				   void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, so=%p)\n", __func__, pcontext, so);

	context->gpu->delete_vertex_elements_state(context->gpu, so);

	debug_printf("< %s()\n", __func__);
}

static void
tegra_set_constant_buffer(struct pipe_context *pcontext,
			  uint shader,
			  uint index,
			  struct pipe_constant_buffer *buf)
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
tegra_set_shader_images(struct pipe_context *pcontext, unsigned shader,
			unsigned start, unsigned count,
			struct pipe_image_view **views)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, shader=%u, start=%u, count=%u, views=%p)\n",
		     __func__, pcontext, shader, start, count, views);

	context->gpu->set_shader_images(context->gpu, shader, start, count,
					views);

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

static void
tegra_flush_resource(struct pipe_context *pcontext,
		     struct pipe_resource *presource)
{
	struct tegra_resource *resource = to_tegra_resource(presource);
	struct tegra_context *context = to_tegra_context(pcontext);

	debug_printf("> %s(pcontext=%p, presource=%p)\n", __func__, pcontext,
		     presource);

	context->gpu->flush_resource(context->gpu, resource->gpu);

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

struct pipe_context *
tegra_context_create(struct pipe_screen *pscreen, void *priv, unsigned flags)
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

	context->base.create_vertex_elements_state = tegra_create_vertex_elements_state;
	context->base.bind_vertex_elements_state = tegra_bind_vertex_elements_state;
	context->base.delete_vertex_elements_state = tegra_delete_vertex_elements_state;

	context->base.set_constant_buffer = tegra_set_constant_buffer;
	context->base.set_framebuffer_state = tegra_set_framebuffer_state;
	context->base.set_polygon_stipple = tegra_set_polygon_stipple;
	context->base.set_scissor_states = tegra_set_scissor_states;
	context->base.set_viewport_states = tegra_set_viewport_states;
	context->base.set_sampler_views = tegra_set_sampler_views;

	context->base.set_shader_images = tegra_set_shader_images;
	context->base.set_vertex_buffers = tegra_set_vertex_buffers;
	context->base.set_index_buffer = tegra_set_index_buffer;

	context->base.create_stream_output_target = tegra_create_stream_output_target;
	context->base.stream_output_target_destroy = tegra_stream_output_target_destroy;
	context->base.set_stream_output_targets = tegra_set_stream_output_targets;

	context->base.blit = tegra_blit;
	context->base.clear = tegra_clear;
	context->base.flush = tegra_flush;

	context->base.create_sampler_view = tegra_create_sampler_view;
	context->base.sampler_view_destroy = tegra_sampler_view_destroy;

	context->base.flush_resource = tegra_flush_resource;

	context->base.create_surface = tegra_create_surface;
	context->base.surface_destroy = tegra_surface_destroy;

	context->base.transfer_map = tegra_transfer_map;
	context->base.transfer_unmap = tegra_transfer_unmap;
	context->base.transfer_inline_write = tegra_transfer_inline_write;

	debug_printf("< %s() = %p\n", __func__, &context->base);
	return &context->base;
}
