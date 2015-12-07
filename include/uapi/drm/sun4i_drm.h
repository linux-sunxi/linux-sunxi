/*
 * Copyright (C) 2015 Free Electrons
 * Copyright (C) 2015 NextThing Co
 *
 * Maxime Ripard <maxime.ripard@free-electrons.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#ifndef _UAPI_SUN4I_DRM_H_
#define _UAPI_SUN4I_DRM_H_

#include <drm/drm.h>

struct drm_sun4i_gem_create {
	__u64 size;
	__u32 flags;
	__u32 handle;
};

#define DRM_SUN4I_GEM_CREATE		0x00

#define DRM_IOCTL_SUN4I_GEM_CREATE	DRM_IOWR(DRM_COMMAND_BASE + DRM_SUN4I_GEM_CREATE, \
						 struct drm_sun4i_gem_create)

#endif
