#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <vips/vips.h>
#include "io-interface.h"

static void*
img_open(const char* fn, const enum OOKMODE mode)
{
  const char* access = "rd";
  if(mode == OOK_RDWR) { access = "w"; }
  IMAGE* img = im_open(fn, access);
  if(NULL == img) {
    errno = -EINVAL;
    return NULL;
  }
  return img;
}

static int
img_read(void* fd, const off_t offset, const size_t len, void* buf)
{
  IMAGE* img = (IMAGE*)fd;
  REGION* reg = vips_region_new(img);
  if(NULL == reg) {
    return -ENOMEM;
  }

  const size_t width = img->Xsize;
  const size_t height = img->Ysize;
  const size_t y = offset / width;
  const size_t x = offset - y*width;

  VipsRect r = { .left = x, .top = y, .width = width, .height = height };
  vips_region_prepare(reg, &r);
  memcpy(buf, VIPS_REGION_ADDR(reg, x,y), len);
  g_object_unref(reg);
  return 0;
}

static int
img_close(void* fd)
{
  IMAGE* img = (IMAGE*)fd;
  vips_image_invalidate_all(img);
  return im_close(img);
}

struct io ImageIO = {
  .open = img_open,
  .read = img_read,
  .write = NULL,
  .close = img_close,
  .preallocate = NULL
};
