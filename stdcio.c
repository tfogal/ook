#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include "io-interface.h"

static void*
stdc_open(const char* fn, const enum OOKMODE mode)
{
  const char* access = "r";
  if(mode == OOK_RDWR) { access = "r+"; }
  FILE* fp = fopen(fn, access);

	if(NULL == fp) {
    errno = -EINVAL;
    return NULL;
	}
  return fp;
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

int
stdc_close(void* fd)
{
  FILE* fp = (FILE*) fd;
  if(fclose(fp) != 0) {
    return errno;
  }
  return 0;
}

struct io StdCIO = {
  .open = stdc_open,
  .read = stdc_read,
  .close = stdc_close
};
