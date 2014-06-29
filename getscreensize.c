/*
 * Copyright (c) 2012 Arvin Schnell <arvin.schnell@gmail.com>
 * Copyright (c) 2012 Rob Clark <rob@ti.com>
 * Copyright (c) 2014 n3rdopolis (or nerdopolis)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/* Based on a egl cube test app originally written by Arvin Schnell */


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <gbm.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
int x;
int y;
static struct {
  struct gbm_device *dev;
  struct gbm_surface *surface;
} gbm;

static struct {
  int fd;
  drmModeModeInfo *mode;
  uint32_t crtc_id;
  uint32_t connector_id;
} drm;

struct drm_fb {
  struct gbm_bo *bo;
  uint32_t fb_id;
};

int main(void)
{
  static const char *modules[] = {
    "i915", "radeon", "nouveau", "vmwgfx", "omapdrm", "exynos", "msm"
  };
  drmModeRes *resources;
  drmModeConnector *connector = NULL;
  drmModeEncoder *encoder = NULL;
  int i, area;
  
  for (i = 0; i < ARRAY_SIZE(modules); i++) {
    printf("trying to load module %s...", modules[i]);
    drm.fd = drmOpen(modules[i], NULL);
    if (drm.fd < 0) {
      printf("failed.\n");
    } else {
      printf("success.\n");
      break;
    }
  }
  
  if (drm.fd < 0) {
    printf("could not open drm device\n");
    return -1;
  }
  
  resources = drmModeGetResources(drm.fd);
  if (!resources) {
    printf("drmModeGetResources failed: %s\n", strerror(errno));
    return -1;
  }
  
  /* find a connected connector: */
  for (i = 0; i < resources->count_connectors; i++) {
    connector = drmModeGetConnector(drm.fd, resources->connectors[i]);
    if (connector->connection == DRM_MODE_CONNECTED) {
      /* it's connected, let's use this! */
      break;
    }
    drmModeFreeConnector(connector);
    connector = NULL;
  }
  
  if (!connector) {
    /* we could be fancy and listen for hotplug events and wait for
     * a connector..
     */
    printf("no connected connector!\n");
    return -1;
  }
  /* find highest resolution mode: */
  for (i = 0, area = 0; i < connector->count_modes; i++) {
    drmModeModeInfo *current_mode = &connector->modes[i];
    int current_area = current_mode->hdisplay * current_mode->vdisplay;
    if (current_area > area) {
      drm.mode = current_mode;
      area = current_area;
      x=current_mode->hdisplay;
      y=current_mode->vdisplay;
    }
  }
  printf("%dx%d\n", x, y);
  return 0;
}