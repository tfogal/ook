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
  struct io iop;
  size_t bricksize[3];
  uint64_t volsize[3];
  enum OOKTYPE type;
  size_t components;
};

#ifndef NDEBUG
static int test();
#endif

#ifdef __GNUC__
#  define CONST __attribute__((const))
#  define MALLOC __attribute__((malloc))
#  define PURE __attribute__((pure))
#else
#  define CONST /* no const function support */
#  define MALLOC /* no malloc function support */
#  define PURE /* no pure function support */
#endif

/* returns the brick layout (number of bricks per dimension) for the given
 * file. */
static void blayout(const struct ookfile* of, size_t nbricks[3]);
/** @return the number of bytes a given type needs. */
PURE static size_t width(enum OOKTYPE t);
/** converts brick 1D ID to 3D ID.
 * @param id the 1-dimensional brick ID
 * @param layout the layout of bricks in the dataset (# bricks per dim)
 * @param[out] brick where the 3D index (output) is stored */
static void bidxto3d(const size_t id, const size_t layout[3], size_t brick[3]);

/* a read or write operation on the opaque ook interface. */
typedef int (rwop)(void* fd, const off_t offset, const size_t len, void* buf);
/** identifies the location of data within the larger set, and moves data
 * between the two places. */
static void srcop(rwop* op, const struct ookfile* of, size_t id, void* buffer);

bool
ookinit()
{
#ifndef NDEBUG
  test();
#endif
  return true;
}

struct ookfile*
ookread(struct io iop, const char* fn, const uint64_t voxels[3],
        const size_t bsize[3], const enum OOKTYPE type, const size_t components)
{
  if(fn == NULL) { errno = EINVAL; return NULL; }
  /* bricks can't be larger than data size. */
  if(bsize[0] > voxels[0] ||
     bsize[1] > voxels[1] ||
     bsize[2] > voxels[2]) {
    errno = EINVAL;
    return NULL;
  }
  struct ookfile* of = calloc(1, sizeof(struct ookfile));
  if(of == NULL) {
    errno = ENOMEM;
    return NULL;
  }
  of->iop = iop;
  of->fd = iop.open(fn, OOK_RDONLY, of->iop.state);

  if(of->fd == NULL) {
    const int err = errno;
    free(of);
    errno = err;
    return NULL;
  }
  memcpy(of->volsize, voxels, sizeof(uint64_t)*3);
  memcpy(of->bricksize, bsize, sizeof(size_t)*3);
  of->type = type;
  of->components = components;
  return of;
}

size_t
ookbricks(const struct ookfile* ook)
{
  if(ook == NULL) { errno = EINVAL; return 0; }
  size_t nbricks[3];
  blayout(ook, nbricks);
  return nbricks[0] * nbricks[1] * nbricks[2];
}

void
ooklayout(const struct ookfile* of, size_t layout[3])
{
  if(of == NULL) { errno = EINVAL; return; }
  blayout(of, layout);
}

void
ookmaxbricksize(const struct ookfile* of, size_t bs[3])
{
  if(of == NULL) { errno = EINVAL; return; }
  memcpy(bs, of->bricksize, sizeof(size_t)*3);
}

/* copies data from the appropriate part of the file into 'target'. */
int
ookbrick(const struct ookfile* of, size_t id, void* target)
{
  errno = 0;
  srcop(of->iop.read, of, id, target);
  return errno;
}

/* copies data for the given brick into 'data'. */
int
ookbrick3(const struct ookfile* of, const size_t id[3], void* data)
{
  errno = 0;
  /* Somewhat humorously, 'srcop' needs the 1D index---which it just goes and
   * converts to a 3D index anyway.  To satisfy the interface, for now, we just
   * convert 3D to 1D and let 'srcop' convert it back. */
  size_t layout[3];
  blayout(of, layout);
  const size_t bid = id[2]*layout[0]*layout[1] + id[1]*layout[0] + id[0];
  srcop(of->iop.read, of, bid, data);
  return errno;
}

void
ookdimensions(const struct ookfile* of, uint64_t voxels[3])
{
  if(of == NULL) { errno = EINVAL; return; }
  memcpy(voxels, of->volsize, sizeof(uint64_t)*3);
}

struct ookfile*
ookcreate(struct io iop, const char* filename,
          const uint64_t dims[3], const size_t bsize[3],
          enum OOKTYPE type, size_t components)
{
  if(filename == NULL) { errno = EINVAL; return NULL; }
  /* bricks can't be larger than data size. */
  if(bsize[0] > dims[0] ||
     bsize[1] > dims[1] ||
     bsize[2] > dims[2]) {
    errno = EINVAL;
    return NULL;
  }
  struct ookfile* of = calloc(1, sizeof(struct ookfile));
  if(of == NULL) {
    errno = ENOMEM;
    return NULL;
  }
  of->iop = iop;
  of->fd = iop.open(filename, OOK_RDWR, iop.state);
  if(of->fd == NULL) {
    free(of);
    errno = EINVAL;
    return NULL;
  }
  memcpy(of->volsize, dims, sizeof(uint64_t)*3);
  memcpy(of->bricksize, bsize, sizeof(size_t)*3);
  of->type = type;
  of->components = components;

  if(of->iop.preallocate) {
    const off_t sz = width(type) * components * dims[0]*dims[1]*dims[2];
    of->iop.preallocate(of->fd, sz);
  }
  return of;
}

/* size of a brick, per-dimension.
 * basically we assume it is the global brick size, but calculate if the brick
 * is at the edge of the domain.  if it is, then we only give the 'remainder'
 * number of voxels. */
void
ookbricksize(const struct ookfile* of, const size_t id, size_t bsize[3])
{
  if(of == NULL) { errno = EINVAL; return; }
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
ookbricksize3(const struct ookfile* of, const size_t bid[3], size_t bsize[3])
{
  if(of == NULL) { errno = EINVAL; return; }
  const double nbricks[3] = {
    of->volsize[0] / of->bricksize[0],
    of->volsize[1] / of->bricksize[1],
    of->volsize[2] / of->bricksize[2]
  };
  if(bid[0] > nbricks[0] || bid[1] > nbricks[1] || bid[2] > nbricks[2]) {
    errno = EINVAL;
    return;
  }
  memcpy(bsize, of->bricksize, sizeof(size_t)*3);
  for(size_t i=0; i < 3; ++i) {
		if(bid[i] == nbricks[i]) {
			bsize[i] = of->volsize[i] % of->bricksize[i];
		}
  }
}

void
ookwrite(struct ookfile* of, const size_t id, const void* from)
{
  /* 'srcop' is defined for a 'read' buffer, which doesn't have the same
   * "const"s: hence the casting. */
  srcop((rwop*)of->iop.write, of, id, (void*)from);
}

int
ookclose(struct ookfile* of)
{
  if(of == NULL) { return EINVAL; }
  int errcode = of->iop.close(of->fd);
  free(of);
  return errcode;
}

/* returns the brick layout (number of bricks per dimension) for the given
 * file. */
static void
blayout(const struct ookfile* of, size_t nbricks[3])
{
  assert(of != NULL);
  assert(of->volsize[0] >= of->bricksize[0]);
  assert(of->volsize[1] >= of->bricksize[1]);
  assert(of->volsize[2] >= of->bricksize[2]);
  nbricks[0] = (size_t) ceil((double)of->volsize[0] / of->bricksize[0]);
  nbricks[1] = (size_t) ceil((double)of->volsize[1] / of->bricksize[1]);
  nbricks[2] = (size_t) ceil((double)of->volsize[2] / of->bricksize[2]);
}

/** @return the number of bytes a given type needs. */
PURE static size_t
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
  assert(false && "unreachable");
  return -42;
}

/** converts brick 1D ID to 3D ID.
 * @param id the 1-dimensional brick ID
 * @param layout the layout of bricks in the dataset (# bricks per dim)
 * @param[out] brick where the 3D index (output) is stored */
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

/** identifies the location of data within the larger set, and moves data
 * between the two places. */
static void
srcop(rwop* op, const struct ookfile* of, size_t id, void* buffer)
{
  assert(op);
  if(of == NULL || buffer == NULL) { errno = EINVAL; return; }

  size_t bsize[3];
  ookbricksize(of, id, bsize);

  size_t layout[3];
  blayout(of, layout);
  size_t brickid[3];
  bidxto3d(id, layout, brickid);

  /* just for typing convenience: */
  const uint64_t vol[3] = { of->volsize[0], of->volsize[1], of->volsize[2] };

  size_t src_offset[3] = {
    (brickid[0] % layout[0]) * of->bricksize[0],
    (brickid[1] % layout[1]) * of->bricksize[1],
    (brickid[2] % layout[2]) * of->bricksize[2],
  };
  const size_t original_src_offset[3] = {
    src_offset[0], src_offset[1], src_offset[2]
  };

  const size_t c = of->components; /* convenience */
  const size_t w = width(of->type); /* convenience */
  /* our copy size/scanline size is the width of our target brick. */
  const size_t scanline = bsize[0] * c * w;
  for(size_t z=0; z < bsize[2]; ++z) {
    for(size_t y=0; y < bsize[1]; ++y) {
      const off_t tgt_offs = (z*bsize[1]*bsize[0] + y*bsize[0] + 0) * c * w;
      const off_t src_offs = (src_offset[2]*vol[1]*vol[0] +
                              src_offset[1]*vol[0] + src_offset[0]) * c * w;
      int errcode = op(of->fd, src_offs, scanline, buffer+tgt_offs); /* copy */
      if(errcode != 0) { errno = errcode; return; }
      src_offset[1]++; /* follows y's increment.. */
    }
    src_offset[1] = original_src_offset[1];
    src_offset[2]++;
  }
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
