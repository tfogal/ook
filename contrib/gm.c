#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <magick/api.h>
#include "io-interface.h"

static bool magick_initialized = false;

/* 'graphics magick image' */
struct gmi {
  ImageInfo* info;
  Image* data;
};

static void*
img_open(const char* fn, const enum OOKMODE mode)
{
  if(!magick_initialized) {
    InitializeMagick(NULL);
  }
  struct gmi* img = calloc(1, sizeof(struct gmi));
  img->info = CloneImageInfo(0);
  img->data = NULL;
  ExceptionInfo exception;
  GetExceptionInfo(&exception);
  strcpy(img->info->filename, fn);
  /* just make sure we can read this. */
  Image* tmp = PingImage(img->info, &except);
  if(NULL == tmp) {
    CatchException(&except);
    DestroyImageInfo(iinfo);
    return NULL;
  }
  DestroyImage(tmp);
  return img;
}

static int
img_read(void* fd, const off_t offset, const size_t len, void* buf)
{
  struct gmi* img = (struct gmi*) fd;
  ExceptionInfo exc;
  GetExceptionInfo(&exc);
  if(img->data == NULL) { /* first call, never read any data */
    img->data = ReadImage(img->info, &exc);
    if(img->data == NULL) { /* read failed. */
      CatchException(&exc);
      return 0;
    }
  }
  const PixelPacket* pixels = AcquireImagePixels(img->data, 
}

struct io ImageIO = {
  .open = img_open,
  .read = img_read,
  .write = NULL,
  .close = NULL,
  .preallocate = NULL
};
