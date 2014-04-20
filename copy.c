/* File copy example using ook.  Usage (e.g.):
 *    ./ookcopy -i volume.raw -o out.raw -t i16 -x 128 -y 96 -z 84
 * reads: 'volume.raw'
 * outputs: 'out.raw'
 * assumes: single-component data.
 * This doesn't really have a purpose other than testing the library.  If you
 * wanted to quickly get your own processing inserted, you could hack
 * something in between 'ookbrick' and 'ookwrite'. */
#define _POSIX_C_SOURCE 200112L
#include <assert.h>
#include <float.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include "ook.h"

/* dimensions of the input volume. */
static uint64_t vol[3] = {0};
/* filename given as input */
static char* input = NULL;
/* filename to create/generate */
static char* output = NULL;
/* input type to assume */
static enum OOKTYPE itype = OOK_I8;
/* verbosity of output.  0 (the default) is terse. */
static uint16_t verbose = 0U;
/* threshold to utilize */
static double threshold[2] = { -FLT_MAX, FLT_MAX };

/* allocation that succeeds or dies. */
static void* xmalloc(const size_t bytes);
/* duplicates a string.  caller must free! */
static char* tjfstrdup(const char* str);
/* identifies the appropriate ook type from a string representation of it. */
static enum OOKTYPE strtotype(const char*);
static size_t bytewidth(const enum OOKTYPE);

static void
usage(const char* progname)
{
  printf(
"Usage: %s -i input.raw -t type -x <uint> -y <uint> -z <uint> -o out.raw\n\n"
"\t-i  input volume to read.  only raw data are supported.\n"
"\t-t  type of input volume. one of: i8,u8,i16,u16,i32,u32,i64,u64,f,d\n"
"\t-x  number of voxels in input (and output) volume, in X dimension.\n"
"\t-y  ditto, for Y dimension\n"
"\t-z  ditto, for Z dimension\n"
"\t-m  minimum value to threshold with [default=%f]\n"
"\t-M  maximum value to threshold with [default=%f]\n"
"\t-o  output volume to create.  always creates a raw uint8 volume.\n\n"
"Type names are generally 'i' for integer, 'u' for unsigned integer, "
"followed by the byte width of the type.  The special types 'f' and 'd' "
"stand for 'float' and 'double', respectively.\n",
  progname, threshold[0], threshold[1]);
}

/* sets global variables (options) based on command line options.
 * allocates 'input' and 'output'. */
static void
parseopt(int argc, char* const argv[])
{
  int opt;
  while((opt = getopt(argc, argv, "i:o:t:x:y:z:m:M:vh")) != -1) {
    switch(opt) {
      case 'i':
        if(input != NULL) { free(input); input = NULL; }
        input = tjfstrdup(optarg);
        break;
      case 'o':
        if(output != NULL) { free(output); output = NULL; }
        output = tjfstrdup(optarg);
        break;
      case 't':
        itype = strtotype(optarg);
        break;
      case 'x': vol[0] = (uint64_t)atoll(optarg); break;
      case 'y': vol[1] = (uint64_t)atoll(optarg); break;
      case 'z': vol[2] = (uint64_t)atoll(optarg); break;
      case 'm': threshold[0] = (double)atof(optarg); break;
      case 'M': threshold[1] = (double)atof(optarg); break;
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
  if(NULL == output) {
    fprintf(stderr, "Output file needed.\n");
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
  const size_t bricksize[3] = { 64, 64, 64 };

  struct ookfile* fin = ookread(StdCIO, input, vol, bricksize, itype, 1);
  if(!fin) { perror("open"); exit(EXIT_FAILURE); }

  size_t bsize[3];
  ookmaxbricksize(fin, bsize);

  const size_t components = 1; /* our program assumes this.. */

  const size_t bytes_brick =
    bytewidth(itype) * components * bsize[0]*bsize[1]*bsize[2];
  void* data = xmalloc(bytes_brick);
  float* outdata = xmalloc(bytes_brick);

  struct ookfile* fout = ookcreate(StdCIO, output, vol, bsize, itype,
                                   components);
  if(!fout) {
    perror("open");
    free(outdata);
    free(data);
    ookclose(fin);
    return EXIT_FAILURE;
  }

  printf("\n");
  for(size_t brick=0; brick < ookbricks(fin); ++brick) {
    size_t bs[3];
    ookbricksize(fin, brick, bs);
    ookbrick(fin, brick, data);
    ookwrite(fout, brick, data);
    printf("\rProcessed brick %5zu / %5zu...", brick, ookbricks(fin));
  }
  printf("\n");

  free(data);
  free(outdata);
  free(input);
  free(output);
  ookclose(fin);
  ookclose(fout);
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
