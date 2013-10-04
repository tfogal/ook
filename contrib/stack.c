#define _POSIX_C_SOURCE 200112L
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
};

static void
cleanup_images(array* imgs)
{
  for(size_t i=0; i < ca_size(imgs); ++i) {
    ImageIO.close(ca_elem(imgs, i));
  }
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
stack_open(const char* fn, const enum OOKMODE mode, struct metadata md)
{
  DIR* d = opendir(fn);
  if(d == NULL) {
    return NULL; /* errno is already set by opendir. */
  }

  struct stack* stk = calloc(1, sizeof(struct stack));
  memcpy(stk->dims, md.voxels, sizeof(uint64_t)*3);
  stk->images = carray(2);
  {
    struct dirent cur;
    struct dirent* pcur;
    while(readdir_r(d, &cur, &pcur) == 0) {
      if(pcur == NULL) { break; } /* EOF */
      if(cur.d_name[0] == '.') { continue; } /* skip hidden files. */
      char* fname = dirconcat(fn, cur.d_name);
      void* img = ImageIO.open(fname, mode, md);
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

static int
stack_read(void* fd, const off_t offset, const size_t len, void* buf)
{
  (void) fd; (void) offset; (void) len; (void) buf;
  return -EINVAL;
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
