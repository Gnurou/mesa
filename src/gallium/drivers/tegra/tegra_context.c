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

	context->gpu->destroy(context->gpu);
	free(context);
}

static void
tegra_draw_vbo(struct pipe_context *pcontext,
	       const struct pipe_draw_info *pinfo)
{
	struct tegra_context *context = to_tegra_context(pcontext);
	struct pipe_draw_info info;

	if (pinfo && pinfo->indirect) {
		memcpy(&info, pinfo, sizeof(info));
		info.indirect = tegra_resource_unwrap(info.indirect);
		pinfo = &info;
	}

	context->gpu->draw_vbo(context->gpu, pinfo);
}

static void *
tegra_create_blend_state(struct pipe_context *pcontext,
			 const struct pipe_blend_state *cso)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	return context->gpu->create_blend_state(context->gpu, cso);
}

static void
tegra_bind_blend_state(struct pipe_context *pcontext,
		       void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	context->gpu->bind_blend_state(context->gpu, so);
}

static void
tegra_delete_blend_state(struct pipe_context *pcontext,
			 void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	context->gpu->delete_blend_state(context->gpu, so);
}

static void *
tegra_create_sampler_state(struct pipe_context *pcontext,
			   const struct pipe_sampler_state *cso)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	return context->gpu->create_sampler_state(context->gpu, cso);
}

static void
tegra_bind_sampler_states(struct pipe_context *pcontext,
			  unsigned shader,
			  unsigned start_slot,
			  unsigned num_samplers,
			  void **samplers)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	context->gpu->bind_sampler_states(context->gpu, shader, start_slot,
					  num_samplers, samplers);
}

static void
tegra_delete_sampler_state(struct pipe_context *pcontext,
			   void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	context->gpu->delete_sampler_state(context->gpu, so);
}

static void *
tegra_create_rasterizer_state(struct pipe_context *pcontext,
			      const struct pipe_rasterizer_state *cso)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	return context->gpu->create_rasterizer_state(context->gpu, cso);
}

static void
tegra_bind_rasterizer_state(struct pipe_context *pcontext,
			    void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	context->gpu->bind_rasterizer_state(context->gpu, so);
}

static void
tegra_delete_rasterizer_state(struct pipe_context *pcontext,
			      void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	context->gpu->delete_rasterizer_state(context->gpu, so);
}

static void *
tegra_create_depth_stencil_alpha_state(struct pipe_context *pcontext,
				       const struct pipe_depth_stencil_alpha_state *cso)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	return context->gpu->create_depth_stencil_alpha_state(context->gpu,
							      cso);
}

static void
tegra_bind_depth_stencil_alpha_state(struct pipe_context *pcontext,
				     void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	context->gpu->bind_depth_stencil_alpha_state(context->gpu, so);
}

static void
tegra_delete_depth_stencil_alpha_state(struct pipe_context *pcontext,
				       void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	context->gpu->delete_depth_stencil_alpha_state(context->gpu, so);
}

static void *
tegra_create_fs_state(struct pipe_context *pcontext,
		      const struct pipe_shader_state *cso)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	return context->gpu->create_fs_state(context->gpu, cso);
}

static void
tegra_bind_fs_state(struct pipe_context *pcontext,
		    void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	context->gpu->bind_fs_state(context->gpu, so);
}

static void
tegra_delete_fs_state(struct pipe_context *pcontext,
		      void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	context->gpu->delete_fs_state(context->gpu, so);
}

static void *
tegra_create_vs_state(struct pipe_context *pcontext,
		      const struct pipe_shader_state *cso)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	return context->gpu->create_vs_state(context->gpu, cso);
}

static void
tegra_bind_vs_state(struct pipe_context *pcontext,
		    void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	context->gpu->bind_vs_state(context->gpu, so);
}

static void
tegra_delete_vs_state(struct pipe_context *pcontext,
		      void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	context->gpu->delete_vs_state(context->gpu, so);
}

static void *
tegra_create_gs_state(struct pipe_context *pcontext,
		      const struct pipe_shader_state *cso)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	return context->gpu->create_gs_state(context->gpu, cso);
}

static void
tegra_bind_gs_state(struct pipe_context *pcontext,
		    void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	context->gpu->bind_gs_state(context->gpu, so);
}

static void
tegra_delete_gs_state(struct pipe_context *pcontext,
		      void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	context->gpu->delete_gs_state(context->gpu, so);
}

static void *
tegra_create_vertex_elements_state(struct pipe_context *pcontext,
				   unsigned num_elements,
				   const struct pipe_vertex_element *elements)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	return context->gpu->create_vertex_elements_state(context->gpu,
							  num_elements,
							  elements);
}

static void
tegra_bind_vertex_elements_state(struct pipe_context *pcontext,
				 void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	context->gpu->bind_vertex_elements_state(context->gpu, so);
}

static void
tegra_delete_vertex_elements_state(struct pipe_context *pcontext,
				   void *so)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	context->gpu->delete_vertex_elements_state(context->gpu, so);
}

static void
tegra_set_constant_buffer(struct pipe_context *pcontext,
			  uint shader,
			  uint index,
			  struct pipe_constant_buffer *buf)
{
	struct tegra_context *context = to_tegra_context(pcontext);
	struct pipe_constant_buffer buffer;

	if (buf && buf->buffer) {
		memcpy(&buffer, buf, sizeof(buffer));
		buffer.buffer = tegra_resource_unwrap(buffer.buffer);
		buf = &buffer;
	}

	context->gpu->set_constant_buffer(context->gpu, shader, index, buf);
}

static void
tegra_set_framebuffer_state(struct pipe_context *pcontext,
			    const struct pipe_framebuffer_state *fb)
{
	struct tegra_context *context = to_tegra_context(pcontext);
	struct pipe_framebuffer_state state;
	unsigned i;

	if (fb) {
		memcpy(&state, fb, sizeof(state));

		for (i = 0; i < fb->nr_cbufs; i++)
			state.cbufs[i] = tegra_surface_unwrap(fb->cbufs[i]);

		while (i < PIPE_MAX_COLOR_BUFS)
			state.cbufs[i++] = NULL;

		state.zsbuf = tegra_surface_unwrap(fb->zsbuf);

		fb = &state;
	}

	context->gpu->set_framebuffer_state(context->gpu, fb);
}

static void
tegra_set_polygon_stipple(struct pipe_context *pcontext,
			  const struct pipe_poly_stipple *stipple)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	context->gpu->set_polygon_stipple(context->gpu, stipple);
}

static void
tegra_set_scissor_states(struct pipe_context *pcontext,
			 unsigned start_slot,
			 unsigned num_scissors,
			 const struct pipe_scissor_state *scissors)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	context->gpu->set_scissor_states(context->gpu, start_slot,
					 num_scissors, scissors);
}

static void
tegra_set_viewport_states(struct pipe_context *pcontext,
			  unsigned start_slot,
			  unsigned num_viewports,
			  const struct pipe_viewport_state *viewports)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	context->gpu->set_viewport_states(context->gpu, start_slot,
					  num_viewports, viewports);
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

	for (i = 0; i < num_views; i++)
		views[i] = tegra_sampler_view_unwrap(pviews[i]);

	context->gpu->set_sampler_views(context->gpu, shader, start_slot,
					num_views, views);
}

static void
tegra_set_shader_resources(struct pipe_context *pcontext,
			   unsigned start,
			   unsigned count,
			   struct pipe_surface **resources)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	context->gpu->set_shader_resources(context->gpu, start, count,
					   resources);
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

	if (num_buffers && buffers) {
		memcpy(buf, buffers, num_buffers * sizeof(struct pipe_vertex_buffer));

		for (i = 0; i < num_buffers; i++)
			buf[i].buffer = tegra_resource_unwrap(buf[i].buffer);

		buffers = buf;
	}

	context->gpu->set_vertex_buffers(context->gpu, start_slot,
					 num_buffers, buffers);
}

static void
tegra_set_index_buffer(struct pipe_context *pcontext,
		       const struct pipe_index_buffer *buffer)
{
	struct tegra_context *context = to_tegra_context(pcontext);
	struct pipe_index_buffer buf;

	if (buffer) {
		memcpy(&buf, buffer, sizeof(buf));
		buf.buffer = tegra_resource_unwrap(buf.buffer);
		buffer = &buf;
	}

	context->gpu->set_index_buffer(context->gpu, buffer);
}

static struct pipe_stream_output_target *
tegra_create_stream_output_target(struct pipe_context *pcontext,
				  struct pipe_resource *presource,
				  unsigned buffer_offset,
				  unsigned buffer_size)
{
	struct tegra_resource *resource = to_tegra_resource(presource);
	struct tegra_context *context = to_tegra_context(pcontext);

	return context->gpu->create_stream_output_target(context->gpu,
							 resource->gpu,
							 buffer_offset,
							 buffer_size);
}

static void
tegra_stream_output_target_destroy(struct pipe_context *pcontext,
				   struct pipe_stream_output_target *target)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	context->gpu->stream_output_target_destroy(context->gpu, target);
}

static void
tegra_set_stream_output_targets(struct pipe_context *pcontext,
				unsigned num_targets,
				struct pipe_stream_output_target **targets,
				const unsigned *offsets)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	context->gpu->set_stream_output_targets(context->gpu, num_targets,
						targets, offsets);
}

static void
tegra_blit(struct pipe_context *pcontext,
	   const struct pipe_blit_info *pinfo)
{
	struct tegra_context *context = to_tegra_context(pcontext);
	struct pipe_blit_info info;

	if (pinfo) {
		memcpy(&info, pinfo, sizeof(info));
		info.dst.resource = tegra_resource_unwrap(info.dst.resource);
		info.src.resource = tegra_resource_unwrap(info.src.resource);
		pinfo = &info;
	}

	context->gpu->blit(context->gpu, pinfo);
}

static void
tegra_clear(struct pipe_context *pcontext,
	    unsigned buffers,
	    const union pipe_color_union *color,
	    double depth,
	    unsigned stencil)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	context->gpu->clear(context->gpu, buffers, color, depth, stencil);
}

static void
tegra_flush(struct pipe_context *pcontext,
	    struct pipe_fence_handle **fence,
	    unsigned flags)
{
	struct tegra_context *context = to_tegra_context(pcontext);

	context->gpu->flush(context->gpu, fence, flags);
}

static struct pipe_sampler_view *
tegra_create_sampler_view(struct pipe_context *pcontext,
			  struct pipe_resource *ptexture,
			  const struct pipe_sampler_view *template)
{
	struct tegra_resource *texture = to_tegra_resource(ptexture);
	struct tegra_context *context = to_tegra_context(pcontext);
	struct tegra_sampler_view *view;

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

	return &view->base;
}

static void
tegra_sampler_view_destroy(struct pipe_context *pcontext,
			   struct pipe_sampler_view *pview)
{
	struct tegra_sampler_view *view = to_tegra_sampler_view(pview);

	pipe_resource_reference(&view->base.texture, NULL);
	pipe_sampler_view_reference(&view->gpu, NULL);
	free(view);
}

static void
tegra_flush_resource(struct pipe_context *pcontext,
		     struct pipe_resource *presource)
{
	struct tegra_resource *resource = to_tegra_resource(presource);
	struct tegra_context *context = to_tegra_context(pcontext);

	context->gpu->flush_resource(context->gpu, resource->gpu);
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
	transfer->base.resource = NULL;
	pipe_resource_reference(&transfer->base.resource, presource);

	*ptransfer = &transfer->base;

	return transfer->map;
}

static void
tegra_transfer_unmap(struct pipe_context *pcontext,
		     struct pipe_transfer *ptransfer)
{
	struct tegra_transfer *transfer = to_tegra_transfer(ptransfer);
	struct tegra_context *context = to_tegra_context(pcontext);

	context->gpu->transfer_unmap(context->gpu, transfer->gpu);
	pipe_resource_reference(&transfer->base.resource, NULL);
	free(transfer);
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

	context->gpu->transfer_inline_write(context->gpu, resource->gpu,
					    level, usage, box, data, stride,
					    layer_stride);
}

struct pipe_context *
tegra_context_create(struct pipe_screen *pscreen, void *priv)
{
	struct tegra_screen *screen = to_tegra_screen(pscreen);
	struct tegra_context *context;

	context = calloc(1, sizeof(*context));
	if (!context)
		return NULL;

	context->gpu = screen->gpu->context_create(screen->gpu, priv);
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

	context->base.set_shader_resources = tegra_set_shader_resources;
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

	return &context->base;
}
