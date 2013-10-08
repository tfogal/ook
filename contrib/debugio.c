#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include "io-interface.h"

/* since we need to return a valid pointer (NULL would indicate an error), we
 * return the filename given to us as an argument.  We don't own this memory,
 * but it means that our other functions get the filename as an argument, so
 * they can print out more informative messages. */
static void*
dbg_open(const char* fn, const enum OOKMODE mode, const void* state)
{
  (void) mode; (void) state;
  fprintf(stderr, "[ook] opening %s\n", fn);
  return (char*)fn;
}

static int
dbg_read(void* fd, const off_t offset, const size_t len, void* buf)
{
  (void) buf;
  fprintf(stderr, "[ook] reading %zu bytes at %ld from %s\n", len, offset,
          (const char*)fd);
  return 0;
}

static int
dbg_write(void* fd, const off_t offset, const size_t len, const void* buf)
{
  (void) buf;
  fprintf(stderr, "[ook] writing %zu bytes at %ld into %s\n", len, offset,
          (const char*)fd);
  return 0;
}

static int
dbg_close(void* fd)
{
  fprintf(stderr, "[ook] closing %s\n", (const char*)fd);
  return 0;
}

static void
dbg_prealloc(void* fd, off_t len)
{
  fprintf(stderr, "[ook] preallocating %lu bytes for %s\n", len,
          (const char*)fd);
}

struct io DebugIO = {
  .open = dbg_open,
  .read = dbg_read,
  .write = dbg_write,
  .close = dbg_close,
  .preallocate = dbg_prealloc
};
