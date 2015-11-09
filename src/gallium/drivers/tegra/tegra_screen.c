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
#include <fcntl.h>
#include <stdio.h>

#include <sys/stat.h>

#ifdef HAVE_LIBUDEV
#include <libudev.h>
#endif

#include "util/u_debug.h"

#include "tegra/tegra_context.h"
#include "tegra/tegra_resource.h"
#include "tegra/tegra_screen.h"

/* TODO: obtain from include file */
struct pipe_screen *nouveau_drm_screen_create(int fd);

static const char *
tegra_get_name(struct pipe_screen *pscreen)
{
	return "tegra";
}

static const char *
tegra_get_vendor(struct pipe_screen *pscreen)
{
	return "tegra";
}

static void tegra_screen_destroy(struct pipe_screen *pscreen)
{
	struct tegra_screen *screen = to_tegra_screen(pscreen);

	debug_printf("> %s(pscreen=%p)\n", __func__, pscreen);

	screen->gpu->destroy(screen->gpu);
	free(pscreen);

	debug_printf("< %s()\n", __func__);
}

static int
tegra_screen_get_param(struct pipe_screen *pscreen, enum pipe_cap param)
{
	struct tegra_screen *screen = to_tegra_screen(pscreen);
	int ret;

	debug_printf("> %s(pscreen=%p, param=%d)\n", __func__, pscreen,
		     param);

	ret = screen->gpu->get_param(screen->gpu, param);

	debug_printf("< %s() = %d\n", __func__, ret);
	return ret;
}

static float
tegra_screen_get_paramf(struct pipe_screen *pscreen, enum pipe_capf param)
{
	struct tegra_screen *screen = to_tegra_screen(pscreen);
	float ret;

	debug_printf("> %s(pscreen=%p, param=%d)\n", __func__, pscreen,
		     param);

	ret = screen->gpu->get_paramf(screen->gpu, param);

	debug_printf("< %s() = %f\n", __func__, ret);
	return ret;
}

static int
tegra_screen_get_shader_param(struct pipe_screen *pscreen,
			      unsigned shader,
			      enum pipe_shader_cap param)
{
	struct tegra_screen *screen = to_tegra_screen(pscreen);
	int ret;

	debug_printf("> %s(pscreen=%p, param=%d)\n", __func__, pscreen,
		     param);

	ret = screen->gpu->get_shader_param(screen->gpu, shader, param);

	debug_printf("< %s() = %d\n", __func__, ret);
	return ret;
}

static boolean
tegra_screen_is_format_supported(struct pipe_screen *pscreen,
				 enum pipe_format format,
				 enum pipe_texture_target target,
				 unsigned sample_count,
				 unsigned usage)
{
	struct tegra_screen *screen = to_tegra_screen(pscreen);
	boolean ret;

	debug_printf("> %s(pscreen=%p, format=%d, target=%d, sample_count=%u, usage=%x)\n",
	       __func__, pscreen, format, target, sample_count, usage);

	ret = screen->gpu->is_format_supported(screen->gpu, format, target,
					       sample_count, usage);

	debug_printf("< %s() = %d\n", __func__, ret);
	return ret;
}

static void
tegra_fence_reference(struct pipe_screen *pscreen,
		      struct pipe_fence_handle **ptr,
		      struct pipe_fence_handle *fence)
{
	struct tegra_screen *screen = to_tegra_screen(pscreen);

	debug_printf("> %s(pscreen=%p, ptr=%p, fence=%p)\n", __func__,
		     pscreen, ptr, fence);

	screen->gpu->fence_reference(screen->gpu, ptr, fence);

	debug_printf("< %s()\n", __func__);
}

static boolean
tegra_fence_finish(struct pipe_screen *pscreen,
		   struct pipe_fence_handle *fence,
		   uint64_t timeout)
{
	struct tegra_screen *screen = to_tegra_screen(pscreen);
	boolean ret;

	debug_printf("> %s(pscreen=%p, fence=%p, timeout=%llu)\n", __func__,
		     pscreen, fence, timeout);

	ret = screen->gpu->fence_finish(screen->gpu, fence, timeout);

	debug_printf("< %s() = %d\n", __func__, ret);
	return ret;
}

static struct udev_device *udev_device_new_from_fd(struct udev *udev, int fd)
{
	struct udev_device *device;
	struct stat stat;
	int err;

	err = fstat(fd, &stat);
	if (err < 0) {
		fprintf(stderr, "fstat() failed: %s\n", strerror(errno));
		return NULL;
	}

	device = udev_device_new_from_devnum(udev, 'c', stat.st_rdev);
	if (!device) {
		fprintf(stderr, "udev_device_new_from_devnum() failed\n");
		return NULL;
	}

	return device;
}

static struct udev_device *udev_device_get_root(struct udev_device *device)
{
	struct udev_device *parent;

	debug_printf("> %s(device=%p)\n", __func__, device);
	debug_printf("  syspath: %s\n", udev_device_get_syspath(device));

	while (true) {
		parent = udev_device_get_parent(device);
		if (!parent)
			break;

		debug_printf("  parent: %p\n", parent);
		debug_printf("    syspath: %s\n", udev_device_get_syspath(parent));
		device = parent;
	}

	debug_printf("< %s() = %p\n", __func__, device);
	return device;
}

static bool udev_device_match(struct udev_device *x, struct udev_device *y)
{
	const char *p1 = udev_device_get_syspath(x);
	const char *p2 = udev_device_get_syspath(y);

	return strcmp(p1, p2) == 0;
}

static int tegra_open_render_node(int fd)
{
	struct udev_device *display, *parent, *root;
	struct udev_list_entry *list, *entry;
	struct udev_enumerate *enumerate;
	struct udev *udev;

	udev = udev_new();
	if (!udev)
		return -ENOMEM;

	display = udev_device_new_from_fd(udev, fd);
	if (!display) {
		udev_unref(udev);
		return -ENODEV;
	}

	debug_printf("path: %s\n", udev_device_get_devpath(display));

	parent = udev_device_get_parent(display);
	if (!parent) {
		udev_device_unref(display);
		udev_unref(udev);
		return -ENODEV;
	}

	debug_printf("parent: %s\n", udev_device_get_syspath(parent));

	display = parent;

	root = udev_device_get_root(display);
	if (!root) {
		udev_device_unref(display);
		udev_unref(udev);
		return -ENODEV;
	}

	debug_printf("root: %s\n", udev_device_get_syspath(root));

	enumerate = udev_enumerate_new(udev);
	if (!enumerate) {
		udev_device_unref(display);
		udev_unref(udev);
		return -ENOMEM;
	}

	udev_enumerate_add_match_subsystem(enumerate, "drm");
	udev_enumerate_add_match_sysname(enumerate, "render*");
	udev_enumerate_scan_devices(enumerate);

	list = udev_enumerate_get_list_entry(enumerate);

	debug_printf("devices:\n");

	udev_list_entry_foreach(entry, list) {
		const char *path = udev_list_entry_get_name(entry);
		struct udev_device *device, *bus;

		device = udev_device_new_from_syspath(udev, path);
		if (!device)
			continue;

		path = udev_device_get_devnode(device);

		parent = udev_device_get_parent(device);
		if (!parent) {
			udev_device_unref(device);
			continue;
		}

		/* do not match if the render nodes shares the same parent */
		if (udev_device_match(parent, display)) {
			udev_device_unref(parent);
			udev_device_unref(device);
			continue;
		}

		bus = udev_device_get_root(device);
		if (!bus) {
			udev_device_unref(parent);
			udev_device_unref(device);
			continue;
		}

		/* both devices need to be on the same bus, though */
		if (udev_device_match(bus, root)) {
			debug_printf("match found: %s\n", path);

			fd = open(path, O_RDWR);
			if (fd < 0)
				fd = -errno;

			break;
		}
	}

	udev_enumerate_unref(enumerate);
	udev_unref(udev);

	return open("/dev/dri/renderD128", O_RDWR);
}

struct pipe_screen *
tegra_screen_create(int fd)
{
	struct tegra_screen *screen;

	debug_printf("> %s()\n", __func__);

	screen = calloc(1, sizeof(*screen));
	if (!screen)
		return NULL;

	screen->fd = fd;

	screen->gpu_fd = tegra_open_render_node(screen->fd);
	if (screen->gpu_fd < 0) {
		fprintf(stderr, "failed to open GPU device: %s\n",
			strerror(errno));
		free(screen);
		return NULL;
	}

	screen->gpu = nouveau_drm_screen_create(screen->gpu_fd);
	if (!screen->gpu) {
		fprintf(stderr, "failed to create GPU screen\n");
		close(screen->gpu_fd);
		free(screen);
		return NULL;
	}

	debug_printf("GPU: %p\n", screen->gpu);
	debug_printf("  fd: %d\n", screen->gpu_fd);

	screen->base.get_name = tegra_get_name;
	screen->base.get_vendor = tegra_get_vendor;
	screen->base.destroy = tegra_screen_destroy;
	screen->base.get_param = tegra_screen_get_param;
	screen->base.get_paramf = tegra_screen_get_paramf;
	screen->base.get_shader_param = tegra_screen_get_shader_param;
	screen->base.context_create = tegra_context_create;
	screen->base.is_format_supported = tegra_screen_is_format_supported;

	screen->base.resource_create = tegra_resource_create;
	screen->base.resource_from_handle = tegra_resource_from_handle;
	screen->base.resource_get_handle = tegra_resource_get_handle;
	screen->base.resource_destroy = tegra_resource_destroy;

	screen->base.fence_reference = tegra_fence_reference;
	screen->base.fence_finish = tegra_fence_finish;

	debug_printf("< %s() = %p\n", __func__, &screen->base);
	return &screen->base;
}
