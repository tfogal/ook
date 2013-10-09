/* Simple ook program to calculate mask one dataset with another.  Usage:
 *    ./mask -i volume.raw -m mask.raw -o out.raw -t i16 -x 128 -y 96 -z 84
 * reads: 'volume.raw' and 'mask.raw'
 * outputs: 'out.raw'
 * assumes: volume and mask are single component.
 *          mask is 8bit.
 *          mask has 0 for 'off' or 'remove' and non-zero for 'keep'. */
#define _POSIX_C_SOURCE 200112L
#include <assert.h>
#include <errno.h>
#include <float.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "chain2.h"
#include "debugio.h"
#include "ook.h"

/* dimensions of the input volume. */
static uint64_t vol[3] = {0};
/* filename given as input */
static char* input = NULL;
/* mask filename */
static char* mask = NULL;
/* filename to output */
static char* output = NULL;
/* input type to assume */
static enum OOKTYPE itype = OOK_I8;
/* verbosity of output.  0 (the default) is terse. */
static uint16_t verbose = 0U;

/* allocation that succeeds or dies. */
static void* xmalloc(const size_t bytes);
/* duplicates a string.  caller must free! */
static char* tjfstrdup(const char* str);
/* identifies the appropriate ook type from a string representation of it. */
static enum OOKTYPE strtotype(const char*);
static size_t bytewidth(const enum OOKTYPE);
static off_t filesize(const char*);

static void
usage(const char* progname)
{
  printf(
"Usage: %s -i input.raw -t type -m mask -x <uint> -y <uint> -z <uint> "
"-o out.raw\n"
"\t-i  input volume to read.  only raw data are supported.\n"
"\t-t  type of input volume. one of: i8,u8,i16,u16,i32,u32,i64,u64,f,d\n"
"\t-m  mask volume to read.  must be raw, 8bit unsigned data.\n"
"\t-o  output volume to create.\n"
"\t-x  number of voxels in input (and output) volume, in X dimension.\n"
"\t-y  ditto, for Y dimension\n"
"\t-z  ditto, for Z dimension\n"
"Type names are generally 'i' for integer, 'u' for unsigned integer, "
"followed by the byte width of the type.  The special types 'f' and 'd' "
"stand for 'float' and 'double', respectively.\n",
  progname);
}

#define MIN(a,b) \
  ({ __typeof__ (a) _a = (a); \
     __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })
#define MAX(a,b) \
  ({ __typeof__ (a) _a = (a); \
     __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

static void
maski8(const void* vol, const uint8_t* mask, void* out, const size_t n)
{
  const int8_t* ivol = (const int8_t*)vol;
  int8_t* ovol = (int8_t*)out;
  for(size_t i=0; i < n; ++i) {
    ovol[i] = mask[i] > 0 ? ivol[i] : 0;
  }
}
static void
masku8(const void* vol, const uint8_t* mask, void* out, const size_t n)
{
  const uint8_t* ivol = (const uint8_t*)vol;
  uint8_t* ovol = (uint8_t*)out;
  for(size_t i=0; i < n; ++i) {
    ovol[i] = mask[i] > 0 ? ivol[i] : 0;
  }
}
static void
maski16(const void* vol, const uint8_t* mask, void* out, const size_t n)
{
  const int16_t* ivol = (const int16_t*)vol;
  uint8_t* ovol = (uint8_t*)out;
  for(size_t i=0; i < n; ++i) {
    ovol[i] = mask[i] > 0 ? ivol[i] : 0;
  }
}
static void
masku16(const void* vol, const uint8_t* mask, void* out, const size_t n)
{
  const uint16_t* ivol = (const uint16_t*)vol;
  uint8_t* ovol = (uint8_t*)out;
  for(size_t i=0; i < n; ++i) {
    ovol[i] = mask[i] > 0 ? ivol[i] : 0;
  }
}

/* sets global variables (options) based on command line options.
 * allocates 'input' and 'output'. */
static void
parseopt(int argc, char* const argv[])
{
  int opt;
  while((opt = getopt(argc, argv, "i:t:m:x:y:z:o:vh")) != -1) {
    switch(opt) {
      case 'i':
        if(input != NULL) { free(input); input = NULL; }
        input = tjfstrdup(optarg);
        break;
      case 't': itype = strtotype(optarg); break;
      case 'm':
        if(mask != NULL) { free(mask); mask = NULL; }
        mask = tjfstrdup(optarg);
        break;
      case 'o':
        if(output != NULL) { free(output); output = NULL; }
        output = tjfstrdup(optarg);
        break;
      case 'x': vol[0] = (uint64_t)atoll(optarg); break;
      case 'y': vol[1] = (uint64_t)atoll(optarg); break;
      case 'z': vol[2] = (uint64_t)atoll(optarg); break;
      case 'v':
        verbose++;
        break;
      case 'h': /* FALL-THROUGH */
      default:
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }
  }
  if(NULL == input) {
    fprintf(stderr, "No input file given!\n");
    usage(argv[0]);
    exit(EXIT_FAILURE);
  }
  if(vol[0] == 0 || vol[1] == 0 || vol[2] == 0) {
    fprintf(stderr, "Volume size is 0.\n");
    usage(argv[0]);
    exit(EXIT_FAILURE);
  }
}

int
main(int argc, char* const argv[])
{
  parseopt(argc, argv);

  if(!ookinit()) {
    fprintf(stderr, "Initialization failed.\n");
    exit(EXIT_FAILURE);
  }
  const off_t in_expect_size = vol[0]*vol[1]*vol[2] * bytewidth(itype);
  const off_t mask_expect_size = vol[0]*vol[1]*vol[2] * sizeof(uint8_t);
  if(filesize(input) < in_expect_size) {
    fprintf(stderr, "Input file is not big enough... something is wrong.\n");
    exit(EXIT_FAILURE);
  } else if(filesize(input) > in_expect_size) {
    fprintf(stderr, "Input is bigger than it needs to be... strange, but "
            "I'll let it slide for now.\n");
  }
  if(filesize(mask) < mask_expect_size) {
    fprintf(stderr, "Mask file is not big enough; something is amiss.\n");
    exit(EXIT_FAILURE);
  } else if(filesize(mask) > mask_expect_size) {
    fprintf(stderr, "Mask is bigger than it needs to be... you are likely to "
            "be eaten by a grue.\n");
  }
  const uint64_t bricksize[3] = { vol[0], 16, 16 };

  struct ookfile* fin = ookread(StdCIO, input, vol, bricksize, itype, 1);
  if(!fin) { perror("ookread"); exit(EXIT_FAILURE); }
  free(input); input = NULL;
  struct ookfile* fmask = ookread(StdCIO, mask, vol, bricksize, OOK_U8, 1);
  if(!fmask) { perror("ookread"); ookclose(fin); exit(EXIT_FAILURE); }
  free(mask); mask = NULL;
  struct ookfile* fout = ookcreate(StdCIO, output, vol, bricksize, itype, 1);
  if(!fout) {
    perror("ookcreate");
    ookclose(fin);
    ookclose(fmask);
    exit(EXIT_FAILURE);
  }
  free(output); output = NULL;

  assert(ookbricks(fin) == ookbricks(fmask) &&
         ookbricks(fmask) == ookbricks(fout));

  size_t bsize[3];
  ookmaxbricksize(fin, bsize);

  const size_t components = 1; /* our program assumes this.. */

  const size_t bytes_brick =
    bytewidth(itype) * components * bsize[0]*bsize[1]*bsize[2];
  void* data = malloc(bytes_brick);
  void* odata = malloc(bytes_brick);
  uint8_t* mdata = malloc(components * bsize[0]*bsize[1]*bsize[2]);

  typedef void (t_func_apply)(const void*, const uint8_t*, void*, const size_t);
  t_func_apply* fqn;
  switch(itype) {
    case OOK_I8: fqn = maski8; break;
    case OOK_U8: fqn = masku8; break;
    case OOK_I16: fqn = maski16; break;
    case OOK_U16: fqn = masku16; break;
    default: assert(false); fqn = NULL; break; /* FIXME! */
  }

  printf("\n");
  for(size_t brick=0; brick < ookbricks(fin); ++brick) {
    size_t bs[3];
    ookbricksize(fin, brick, bs);
    assert(bs[0] > 0 && bs[1] > 0 && bs[2] > 0);
    ookbrick(fin, brick, data);
    ookbrick(fmask, brick, mdata);
    fqn(data, mdata, odata, bs[0]*bs[1]*bs[2]);
    ookwrite(fout, brick, odata);
    printf("\rProcessed brick %5zu / %5zu...", brick, ookbricks(fin));
  }
  printf("\n");

  free(data);
  free(mdata);
  free(odata);
  if(ookclose(fin) != 0) {
    fprintf(stderr, "Error closing input\n");
  }
  if(ookclose(fmask) != 0) {
    fprintf(stderr, "Error closing mask\n");
  }
  if(ookclose(fout) != 0) {
    fprintf(stderr, "Error closing output!\n");
  }
}

static void*
xmalloc(const size_t bytes)
{
  void* rv = malloc(bytes);
  if(rv == NULL) {
    exit(EXIT_FAILURE);
  }
  return rv;
}

static char*
tjfstrdup(const char* str)
{
  const size_t n = strlen(str);
  char* rv = xmalloc(n + 1);
  return strncpy(rv, str, n+1);
}

static enum OOKTYPE
strtotype(const char* str)
{
  if(strcasecmp(str, "i8") == 0) { return OOK_I8;
  } else if(strcasecmp(str, "u8") == 0) { return OOK_U8;
  } else if(strcasecmp(str, "i16") == 0) { return OOK_I16;
  } else if(strcasecmp(str, "u16") == 0) { return OOK_U16;
  } else if(strcasecmp(str, "i32") == 0) { return OOK_I32;
  } else if(strcasecmp(str, "u32") == 0) { return OOK_U32;
  } else if(strcasecmp(str, "i64") == 0) { return OOK_I64;
  } else if(strcasecmp(str, "u64") == 0) { return OOK_U64;
  } else if(strcasecmp(str, "f") == 0) { return OOK_FLOAT;
  } else if(strcasecmp(str, "d") == 0) { return OOK_DOUBLE;
  } else {
    fprintf(stderr, "Invalid type '%s'\n", str);
    exit(EXIT_FAILURE);
  }
  assert(false);
  return OOK_I8;
}

static size_t
bytewidth(const enum OOKTYPE basictype)
{
  switch(basictype) {
    case OOK_I8: case OOK_U8: return 1;
    case OOK_I16: case OOK_U16: return 2;
    case OOK_I32: case OOK_U32: return 4;
    case OOK_I64: case OOK_U64: return 8;
    case OOK_FLOAT: return 32;
    case OOK_DOUBLE: return 32;
  }
  assert(false);
  return 0;
}

static off_t
filesize(const char* fn)
{
  struct stat buf;
  if(stat(fn, &buf) != 0) {
    fprintf(stderr, "could not stat %s: %d\n", fn, errno);
    exit(EXIT_FAILURE);
  }
  return buf.st_size;
}
