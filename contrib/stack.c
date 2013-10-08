#define _POSIX_C_SOURCE 200112L
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include "stack.h"
#include "carr.h"
#include "imgio.h"

struct stack {
  array* images;
  uint64_t dims[3];
  size_t components;
  size_t width;
};

static void
cleanup_images(array* imgs)
{
  for(size_t i=0; i < ca_size(imgs); ++i) {
    ImageIO.close(ca_elem(imgs, i));
  }
}

#ifdef __GNUC__
# define CONST __attribute__((const))
#else
# define CONST /* unavailable */
#endif

CONST static int64_t
MIN(const int64_t a, const int64_t b)
{
  return a < b ? a : b;
}

/*allocs-memory*/ static char*
dirconcat(const char* prefix, const char* suffix)
{
  const size_t n = strlen(prefix) + strlen(suffix) + 2;
  char* rv = calloc(n, sizeof(char));
  strncpy(rv, prefix, n);
  strncat(rv, "/", n);
  strncat(rv, suffix, n);
  return rv;
}

static void*
stack_open(const char* fn, const enum OOKMODE mode, const void* state)
{
  if(state == NULL) {
    errno = EINVAL;
    return NULL;
  }
  const struct metadata* md = state;

  DIR* d = opendir(fn);
  if(d == NULL) {
    return NULL; /* errno is already set by opendir. */
  }

  struct stack* stk = calloc(1, sizeof(struct stack));
  memcpy(stk->dims, md->voxels, sizeof(uint64_t)*3);
  stk->components = md->components;
  stk->width = md->width;
  stk->images = carray(2);
  {
    struct dirent cur;
    struct dirent* pcur;
    while(readdir_r(d, &cur, &pcur) == 0) {
      if(pcur == NULL) { break; } /* EOF */
      if(cur.d_name[0] == '.') { continue; } /* skip hidden files. */
      char* fname = dirconcat(fn, cur.d_name);
      void* img = ImageIO.open(fname, mode, md->voxels);
      free(fname);
      if(img == NULL) { /* image failed to open... :-( */
        int err = errno;
        cleanup_images(stk->images);
        ca_destroy(stk->images);
        free(stk);
        errno = err;
        return NULL;
      }
      stk->images = ca_append(stk->images, img);
    }
  }
  if(closedir(d) != 0) {
    int err = errno;
    cleanup_images(stk->images);
    ca_destroy(stk->images);
    free(stk);
    errno = err;
    return NULL;
  }

  return stk;
}

/* This is a bit confusing.  We only get 1D offsets, but we need 3D offsets to
 * do the reads correctly: for one, we need to know the Z offset because that
 * tells us which slice to read from.  Secondly, we need to loop because we
 * might have to read from multiple slices.  Instead of an explicit loop, we
 * solve the issue by recursing, with a base case of the desired reading
 * fitting on a single slice. */
static int
stack_read(void* fd, const off_t offset, const size_t len, void* buf)
{
  const struct stack* stk = (struct stack*)fd;
  assert(stk->components > 0);
  assert(stk->width > 0);

  /* the offset is a byte offset, but we need the element# to be able to
   * calculate the slice. */
  const size_t start = offset / (stk->components*stk->width);
  const off_t start3[3] = {
    (start % stk->dims[0]),
    (start / stk->dims[0]) % stk->dims[1],
    (start / (stk->dims[0]*stk->dims[1])) % stk->dims[2]
  };
  assert(start3[0] >= 0 && start3[1] >= 0 && start3[2] >= 0);
  assert((uint64_t)start3[0] < stk->dims[0] &&
         (uint64_t)start3[1] < stk->dims[1] &&
         (uint64_t)start3[2] < stk->dims[2]);
  const size_t end = (offset+len) / (stk->components*stk->width);
  assert(start < end);
  const off_t end3[3] = {
    (end % stk->dims[0]),
    (end / stk->dims[0]) % stk->dims[1],
    (end / (stk->dims[0]*stk->dims[1])) % stk->dims[2]
  };
  assert(end3[0] >= 0 && end3[1] >= 0 && end3[2] >= 0);
  assert((uint64_t)end3[0] < stk->dims[0] &&
         (uint64_t)end3[1] < stk->dims[1] &&
         (uint64_t)end3[2] < stk->dims[2]);

  /* read only up to the end of this slice. */
  const uint64_t slice_sz = stk->dims[0]*stk->dims[1]*stk->components*stk->width;
  const size_t length =
    MIN(len, slice_sz - (int64_t)offset) > 0 ?
      (size_t)MIN(len, slice_sz - (int64_t)offset) : len;
  assert(length <= len || length <= slice_sz);
  assert(length > 0);

  void* img = ca_elem(stk->images, start3[2]);
  ImageIO.read(img, start3[0]*start3[1]*stk->components*stk->width, length,
               buf);
  /* do we need to continue onto the next slice? */
  if((len - length) > 0) {
    return stack_read(fd, offset+length, len-length, buf+length);
  }
  return 0;
}

static int
stack_close(void* fd)
{
  struct stack* stk = (struct stack*)fd;
  cleanup_images(stk->images);
  ca_destroy(stk->images);
  free(stk);
  return 0;
}

struct io StackIO = {
  .open = stack_open,
  .read = stack_read,
  .write = NULL,
  .close = stack_close,
  .preallocate = NULL
};
