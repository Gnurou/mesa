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

#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include <drm/tegra_drm.h>
#include <libdrm/tegra.h>
#include <xf86drm.h>

#include "pipe/p_state.h"
#include "util/u_debug.h"
#include "util/u_format.h"
#include "util/u_inlines.h"

#include "state_tracker/drm_driver.h"

#include "tegra/tegra_context.h"
#include "tegra/tegra_resource.h"
#include "tegra/tegra_screen.h"

struct pipe_resource *
tegra_resource_create(struct pipe_screen *pscreen,
		      const struct pipe_resource *template)
{
	struct tegra_screen *screen = to_tegra_screen(pscreen);
	struct tegra_resource *resource;

	debug_printf("> %s(pscreen=%p, template=%p)\n", __func__, pscreen,
		     template);
	/*
	debug_printf("  template: %p\n", template);
	debug_printf("    target: %u\n", template->target);
	debug_printf("    format: %u\n", template->format);
	debug_printf("    width: %u\n", template->width0);
	debug_printf("    height: %u\n", template->height0);
	debug_printf("    depth: %u\n", template->depth0);
	debug_printf("    array_size: %u\n", template->array_size);
	debug_printf("    last_level: %u\n", template->last_level);
	debug_printf("    nr_samples: %u\n", template->nr_samples);
	debug_printf("    usage: %u\n", template->usage);
	debug_printf("    bind: %x\n", template->bind);
	debug_printf("    flags: %x\n", template->flags);
	*/

	resource = calloc(1, sizeof(*resource));
	if (!resource)
		return NULL;

	/* import scanout buffers for display */
	if (template->bind & PIPE_BIND_SCANOUT) {
		struct drm_tegra_bo_tiling tiling;
		struct winsys_handle handle;
		boolean status;
		size_t size;
		int fd, err;

		resource->gpu = screen->gpu->resource_create(screen->gpu,
							     template);
		if (!resource->gpu)
			goto free;

		memset(&handle, 0, sizeof(handle));
		handle.type = DRM_API_HANDLE_TYPE_FD;

		status = screen->gpu->resource_get_handle(screen->gpu,
							  resource->gpu,
							  &handle);
		if (!status)
			goto destroy;

		size = handle.stride * resource->gpu->height0;
		resource->stride = handle.stride;
		fd = handle.handle;

		err = drmPrimeFDToHandle(screen->fd, fd, &resource->handle);
		if (err < 0) {
			fprintf(stderr, "drmPrimeFDToHandle() failed: %s\n",
				strerror(errno));
			close(fd);
			goto destroy;
		}

		close(fd);

		err = drm_tegra_bo_wrap(&resource->bo, screen->device,
					resource->handle, 0, size);
		if (err < 0) {
			fprintf(stderr, "failed to create buffer object: %s\n",
				strerror(-err));
			goto destroy;
		}

		tiling.mode = DRM_TEGRA_GEM_TILING_MODE_BLOCK;
		tiling.value = 4;

		err = drm_tegra_bo_set_tiling(resource->bo, &tiling);
		if (err < 0) {
			fprintf(stderr,
				"failed to set tiling for buffer object: %s\n",
				strerror(-err));
			goto unref;
		}
	} else {
		resource->gpu = screen->gpu->resource_create(screen->gpu,
							     template);
		if (!resource->gpu)
			goto free;
	}

	debug_printf("  gpu: %p\n", resource->gpu);

	memcpy(&resource->base, resource->gpu, sizeof(*resource->gpu));
	pipe_reference_init(&resource->base.reference, 1);
	resource->base.screen = &screen->base;

	debug_printf("< %s() = %p\n", __func__, &resource->base);
	return &resource->base;

unref:
	drm_tegra_bo_unref(resource->bo);
destroy:
	screen->gpu->resource_destroy(screen->gpu, resource->gpu);
free:
	free(resource);
	return NULL;
}

struct pipe_resource *
tegra_resource_from_handle(struct pipe_screen *pscreen,
			   const struct pipe_resource *template,
			   struct winsys_handle *handle)
{
	struct tegra_screen *screen = to_tegra_screen(pscreen);
	struct tegra_resource *resource;

	_debug_printf("> %s(pscreen=%p, template=%p, handle=%p)\n", __func__,
		      pscreen, template);

	resource = calloc(1, sizeof(*resource));
	if (!resource)
		return NULL;

	resource->gpu = screen->gpu->resource_from_handle(screen->gpu,
							  template,
							  handle);
	if (!resource->gpu) {
		free(resource);
		return NULL;
	}

	memcpy(&resource->base, resource->gpu, sizeof(*resource->gpu));
	pipe_reference_init(&resource->base.reference, 1);
	resource->base.screen = &screen->base;

	_debug_printf("< %s() = %p\n", __func__, &resource->base);
	return &resource->base;
}

boolean
tegra_resource_get_handle(struct pipe_screen *pscreen,
			  struct pipe_resource *presource,
			  struct winsys_handle *handle)
{
	struct tegra_resource *resource = to_tegra_resource(presource);
	struct tegra_screen *screen = to_tegra_screen(pscreen);
	boolean ret = TRUE;

	debug_printf("> %s(pscreen=%p, presource=%p, handle=%p)\n", __func__,
		     pscreen, presource, handle);

	if (presource->bind & PIPE_BIND_SCANOUT) {
		handle->handle = resource->handle;
		handle->stride = resource->stride;
	} else {
		ret = screen->gpu->resource_get_handle(screen->gpu,
						       resource->gpu,
						       handle);
	}

	debug_printf("< %s() = %d\n", __func__, ret);
	return ret;
}

void
tegra_resource_destroy(struct pipe_screen *pscreen,
		       struct pipe_resource *presource)
{
	struct tegra_resource *resource = to_tegra_resource(presource);

	debug_printf("> %s(pscreen=%p, presource=%p)\n", __func__, pscreen,
		     presource);

	pipe_resource_reference(&resource->gpu, NULL);
	drm_tegra_bo_unref(resource->bo);
	free(resource);

	debug_printf("< %s()\n", __func__);
}

struct pipe_surface *
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

void
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
