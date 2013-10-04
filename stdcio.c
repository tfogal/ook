#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include "io-interface.h"

static void*
stdc_open(const char* fn, const enum OOKMODE mode, struct metadata md)
{
  (void) md;
  const char* access = "r";
  if(mode == OOK_RDWR) { access = "wb"; }
  FILE* fp = fopen(fn, access);

	if(NULL == fp) {
    errno = -EINVAL;
    return NULL;
	}
  return fp;
}

static void*
stdc_open_dbg(const char* fn, const enum OOKMODE mode, struct metadata md)
{
  void* rv = stdc_open(fn, mode, md);
  fprintf(stderr, "[ook] opened %p\n", rv);
  return rv;
}

static int
stdc_read(void* fd, const off_t offset, const size_t len, void* buf)
{
  FILE* fp = (FILE*) fd;
  if(fseek(fp, offset, SEEK_SET) != 0) {
    return errno;
  }
  if(fread(buf, 1, len, fp) != len) {
    return errno;
  }
  return 0;
}

static int
stdc_read_dbg(void* fd, const off_t offset, const size_t len, void* buf)
{
  fprintf(stderr, "[ook] reading %zu bytes at %ld from %p\n", len, offset, fd);
  return stdc_read(fd, offset, len, buf);
}

static int
stdc_write(void* fd, const off_t offset, const size_t len, const void* buf)
{
  FILE* fp = (FILE*) fd;
  if(fseek(fp, offset, SEEK_SET) != 0) {
    return errno;
  }
  if(fwrite(buf, 1, len, fp) != len) {
    return errno;
  }
  return 0;
}

static int
stdc_write_dbg(void* fd, const off_t offset, const size_t len, const void* buf)
{
  fprintf(stderr, "[ook] writing %zu bytes at %ld into %p\n", len, offset, fd);
  return stdc_write(fd, offset, len, buf);
}

static int
stdc_close(void* fd)
{
  FILE* fp = (FILE*) fd;
  if(fclose(fp) != 0) {
    return errno;
  }
  return 0;
}

static int
stdc_close_dbg(void* fd)
{
  fprintf(stderr, "[ook] closing %p\n", fd);
  return stdc_close(fd);
}

struct io StdCIO = {
  .open = stdc_open,
  .read = stdc_read,
  .write = stdc_write,
  .close = stdc_close,
  .preallocate = NULL
};

struct io StdCIO_debug = {
  .open = stdc_open_dbg,
  .read = stdc_read_dbg,
  .write = stdc_write_dbg,
  .close = stdc_close_dbg,
  .preallocate = NULL
};
