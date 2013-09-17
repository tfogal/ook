#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "io-interface.h"
#include "ook.h"

struct ookfile {
  void* fd;
  size_t bricksize[3];
  uint64_t volsize[3];
};
struct io iop;

#ifndef NDEBUG
static int test();
#endif

bool
ookinit(struct io impl)
{
#ifndef NDEBUG
  test();
#endif
  iop = impl;
  return true;
}

struct ookfile*
ookread(const char* fn, const uint64_t voxels[3], const size_t bsize[3])
{
  /* bricks can't be larger than data size. */
  if(bsize[0] > voxels[0] ||
     bsize[1] > voxels[1] ||
     bsize[2] > voxels[2]) {
    errno = -EINVAL;
    return NULL;
  }
  struct ookfile* of = calloc(1, sizeof(struct ookfile));
  if(of == NULL) {
    errno = -ENOMEM;
    return NULL;
  }
  of->fd = iop.open(fn, OOK_RDONLY);
  if(of->fd == NULL) {
    free(of);
    errno = -EINVAL;
    return NULL;
  }
  memcpy(of->volsize, voxels, sizeof(uint64_t)*3);
  memcpy(of->bricksize, bsize, sizeof(size_t)*3);
  return of;
}

/* returns the brick layout (number of bricks per dimension) for the given
 * file. */
static void
blayout(const struct ookfile* of, size_t nbricks[3])
{
  assert(of->volsize[0] >= of->bricksize[0]);
  assert(of->volsize[1] >= of->bricksize[1]);
  assert(of->volsize[2] >= of->bricksize[2]);
  nbricks[0] = (size_t) ceil((double)of->volsize[0] / of->bricksize[0]);
  nbricks[1] = (size_t) ceil((double)of->volsize[1] / of->bricksize[1]);
  nbricks[2] = (size_t) ceil((double)of->volsize[2] / of->bricksize[2]);
}

size_t
ookbricks(const struct ookfile* ook)
{
  size_t nbricks[3];
  blayout(ook, nbricks);
  return nbricks[0] * nbricks[1] * nbricks[2];
}

void
ookmaxbricksize(const struct ookfile* of, size_t bs[3])
{
  memcpy(bs, of->bricksize, sizeof(size_t)*3);
}

void ookbrick(struct ookfile* of, size_t id, void* data)
{
  (void)of;
  (void)id;
  (void)data;
}
void
ookdimensions(const struct ookfile* of, uint64_t voxels[3])
{
  memcpy(voxels, of->volsize, sizeof(uint64_t)*3);
}

static size_t
width(enum OOKTYPE t)
{
  switch(t) {
    case OOK_I8: return 1;
    case OOK_U8: return 1;
    case OOK_I16: return 2;
    case OOK_U16: return 2;
    case OOK_I32: return 4;
    case OOK_U32: return 4;
    case OOK_I64: return 8;
    case OOK_U64: return 8;
    case OOK_FLOAT: return 4;
    case OOK_DOUBLE: return 8;
  }
  return -42;
}

struct ookfile*
ookcreate(const char* filename, const uint64_t dims[3], const size_t bsize[3],
          enum OOKTYPE type, size_t components)
{
  /* bricks can't be larger than data size. */
  if(bsize[0] > dims[0] ||
     bsize[1] > dims[1] ||
     bsize[2] > dims[2]) {
    errno = -EINVAL;
    return NULL;
  }
  struct ookfile* of = calloc(1, sizeof(struct ookfile));
  if(of == NULL) {
    errno = -ENOMEM;
    return NULL;
  }
  of->fd = iop.open(filename, OOK_RDWR);
  if(of->fd == NULL) {
    free(of);
    errno = -EINVAL;
    return NULL;
  }
  memcpy(of->volsize, dims, sizeof(uint64_t)*3);
  memcpy(of->bricksize, bsize, sizeof(size_t)*3);

  if(iop.preallocate) {
    const off_t sz = width(type) * components * dims[0]*dims[1]*dims[2];
    iop.preallocate(of->fd, sz);
  }
  return of;
}

static void
bidxto3d(const size_t id, const size_t layout[3], size_t brick[3])
{
  assert(layout[0] > 0 && layout[1] > 0 && layout[2] > 0);
  assert(id < layout[0]*layout[1]*layout[2] && "within dataset");
  brick[0] = id % layout[0];
  brick[1] = (id / layout[0]) % layout[1];
  brick[2] = (id / (layout[0]*layout[1])) % layout[2];
  assert(brick[0] < layout[0]);
  assert(brick[1] < layout[1]);
  assert(brick[2] < layout[2]);
}

void
ookbricksize(struct ookfile* of, const size_t id, size_t bsize[3])
{
  const double nbricks[3] = {
    of->volsize[0] / of->bricksize[0],
    of->volsize[1] / of->bricksize[1],
    of->volsize[2] / of->bricksize[2]
  };
  size_t layout[3];
  blayout(of, layout);
  size_t bid[3];
  bidxto3d(id, layout, bid);
  memcpy(bsize, of->bricksize, sizeof(size_t)*3);
  for(size_t i=0; i < 3; ++i) {
		if(bid[i] == nbricks[i]) {
			bsize[i] = of->volsize[i] % of->bricksize[i];
		}
  }
}

void
ookwrite(struct ookfile* of, const size_t id, const void* from, size_t n)
{
  (void)of;
  (void)id;
  (void)from;
  (void)n;
}

int
ookclose(struct ookfile* of)
{
  return iop.close(of->fd);
}

#ifndef NDEBUG
static int
test()
{
  {
    struct ookfile of;
    of.volsize[0] = of.volsize[1] = of.volsize[2] = 1000;
    of.bricksize[0] = of.bricksize[1] = of.bricksize[2] = 100;
    size_t bs[3];
    for(size_t i=0; i < 10*10*10; ++i) {
			ookbricksize(&of, i, bs);
			assert(bs[0] == 100);
			assert(bs[1] == 100);
			assert(bs[2] == 100);
    }
  }
  {
    struct ookfile of;
    of.volsize[0] = of.volsize[1] = of.volsize[2] = 40;
    of.bricksize[0] = of.bricksize[1] = of.bricksize[2] = 16;
    size_t bs[3];
    ookbricksize(&of, 0, bs);
    assert(bs[0] == 16 && bs[1] == 16 && bs[2] == 16);
    ookbricksize(&of, 1, bs);
    assert(bs[0] == 16 && bs[1] == 16 && bs[2] == 16);
    ookbricksize(&of, 2, bs);
    assert(bs[0] == 8 && bs[1] == 16 && bs[2] == 16);
    ookbricksize(&of, 3, bs);
    assert(bs[0] == 16 && bs[1] == 16 && bs[2] == 16);
    ookbricksize(&of, 4, bs);
    assert(bs[0] == 16 && bs[1] == 16 && bs[2] == 16);
    ookbricksize(&of, 5, bs);
    assert(bs[0] == 8 && bs[1] == 16 && bs[2] == 16);
    ookbricksize(&of, 6, bs);
    assert(bs[0] == 16 && bs[1] == 8 && bs[2] == 16);
    ookbricksize(&of, 7, bs);
    assert(bs[0] == 16 && bs[1] == 8 && bs[2] == 16);
    ookbricksize(&of, 8, bs);
    assert(bs[0] == 8 && bs[1] == 8 && bs[2] == 16);

    ookbricksize(&of, 17, bs);
    assert(bs[0] == 8 && bs[1] == 8 && bs[2] == 16);
    ookbricksize(&of, 18, bs);
    assert(bs[0] == 16 && bs[1] == 16 && bs[2] == 8);
    ookbricksize(&of, 19, bs);
    assert(bs[0] == 16 && bs[1] == 16 && bs[2] == 8);
    ookbricksize(&of, 20, bs);
    assert(bs[0] == 8 && bs[1] == 16 && bs[2] == 8);
    ookbricksize(&of, 21, bs);
    assert(bs[0] == 16 && bs[1] == 16 && bs[2] == 8);
    ookbricksize(&of, 22, bs);
    assert(bs[0] == 16 && bs[1] == 16 && bs[2] == 8);
    ookbricksize(&of, 23, bs);
    assert(bs[0] == 8 && bs[1] == 16 && bs[2] == 8);
    ookbricksize(&of, 24, bs);
    assert(bs[0] == 16 && bs[1] == 8 && bs[2] == 8);
    ookbricksize(&of, 25, bs);
    assert(bs[0] == 16 && bs[1] == 8 && bs[2] == 8);
    ookbricksize(&of, 26, bs);
    assert(bs[0] == 8 && bs[1] == 8 && bs[2] == 8);
  }
  return 1;
}
#endif