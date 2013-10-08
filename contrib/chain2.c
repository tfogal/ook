#define _POSIX_C_SOURCE 200112L
#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "chain2.h"

struct func {
  struct io f,g;
  void* fimpl;
  void* gimpl;
};

static void*
ch2_open(const char* fn, const enum OOKMODE mode, const void* state)
{
  if(state == NULL) {
    errno = EINVAL;
    return NULL;
  }
  struct func* copy = malloc(sizeof(struct func));
  memcpy(copy, state, sizeof(struct func));
  copy->fimpl = copy->f.open(fn, mode, state);
  if(copy->fimpl == NULL) {
    free(copy);
    errno = ENOMEM;
    return NULL;
  }
  copy->gimpl = copy->g.open(fn, mode, state);
  if(copy->gimpl == NULL) {
    copy->f.close(copy->fimpl);
    free(copy);
    errno = ENOMEM;
    return NULL;
  }
  free((void*)state);
  return copy;
}

static int
ch2_read(void* fd, const off_t offset, const size_t len, void* buf)
{
  const struct func* chain = (const struct func*) fd;
  int err = chain->f.read(chain->fimpl, offset, len, buf);
  err |= chain->g.read(chain->gimpl, offset, len, buf);
  return err;
}

static int
ch2_write(void* fd, const off_t offset, const size_t len, const void* buf)
{
  const struct func* chain = (const struct func*) fd;
  int err = chain->f.write(chain->fimpl, offset, len, buf);
  err |= chain->g.write(chain->gimpl, offset, len, buf);
  return err;
}

static int
ch2_close(void* fd)
{
  struct func* chain = (struct func*) fd;
  int err = chain->f.close(chain->fimpl);
  err |= chain->g.close(chain->gimpl);
  free(chain);
  chain = NULL;
  return err;
}

static void
ch2_preallocate(void* fd, off_t len)
{
  const struct func* chain = (const struct func*) fd;
  chain->f.preallocate(chain->fimpl, len);
  chain->g.preallocate(chain->gimpl, len);
}

struct io*
chain2(struct io first, struct io second)
{
  struct io* chain = malloc(sizeof(struct io));
  chain->open = ch2_open;
  chain->read = ch2_read;
  chain->write = ch2_write;
  chain->close = ch2_close;
  chain->preallocate = ch2_preallocate;
  struct func* funcs = malloc(sizeof(struct func));
  funcs->f = first;
  funcs->g = second;
  funcs->fimpl = funcs->gimpl = NULL;
  chain->state = funcs;
  return chain;
}
