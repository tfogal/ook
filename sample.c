#define _POSIX_C_SOURCE 200112L
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ook.h"

static void*
xmalloc(const size_t bytes)
{
  void* rv = malloc(bytes);
  if(rv == NULL) {
    exit(EXIT_FAILURE);
  }
  return rv;
}

static void
addi8(const void* v1, const void* v2, void* out, const size_t n)
{
  const int8_t* i1 = (const int8_t*)v1;
  const int8_t* i2 = (const int8_t*)v2;
  float* o = (float*)out;

  for(size_t i=0; i < n; ++i) {
    o[i] = (float)i1[i] + i2[i];
  }
}
static void
addu8(const void* v1, const void* v2, void* out, const size_t n)
{
  const uint8_t* i1 = (const uint8_t*)v1;
  const uint8_t* i2 = (const uint8_t*)v2;
  float* o = (float*)out;

  for(size_t i=0; i < n; ++i) {
    o[i] = (float)i1[i] + i2[i];
  }
}
static void
addi16(const void* v1, const void* v2, void* out, const size_t n)
{
  const int16_t* i1 = (const int16_t*)v1;
  const int16_t* i2 = (const int16_t*)v2;
  float* o = (float*)out;

  for(size_t i=0; i < n; ++i) {
    o[i] = (float)i1[i] + i2[i];
  }
}
static void
addu16(const void* v1, const void* v2, void* out, const size_t n)
{
  const uint16_t* i1 = (const uint16_t*)v1;
  const uint16_t* i2 = (const uint16_t*)v2;
  float* o = (float*)out;

  for(size_t i=0; i < n; ++i) {
    o[i] = (float)i1[i] + i2[i];
  }
}
static void
addi32(const void* v1, const void* v2, void* out, const size_t n)
{
  const int32_t* i1 = (const int32_t*)v1;
  const int32_t* i2 = (const int32_t*)v2;
  float* o = (float*)out;

  for(size_t i=0; i < n; ++i) {
    o[i] = (float)i1[i] + i2[i];
  }
}
static void
addu32(const void* v1, const void* v2, void* out, const size_t n)
{
  const uint32_t* i1 = (const uint32_t*)v1;
  const uint32_t* i2 = (const uint32_t*)v2;
  float* o = (float*)out;

  for(size_t i=0; i < n; ++i) {
    o[i] = (float)i1[i] + i2[i];
  }
}
static void
addi64(const void* v1, const void* v2, void* out, const size_t n)
{
  const int64_t* i1 = (const int64_t*)v1;
  const int64_t* i2 = (const int64_t*)v2;
  float* o = (float*)out;

  for(size_t i=0; i < n; ++i) {
    o[i] = (float)i1[i] + i2[i];
  }
}
static void
addu64(const void* v1, const void* v2, void* out, const size_t n)
{
  const uint64_t* i1 = (const uint64_t*)v1;
  const uint64_t* i2 = (const uint64_t*)v2;
  float* o = (float*)out;

  for(size_t i=0; i < n; ++i) {
    o[i] = (float)i1[i] + i2[i];
  }
}
static void
addfloat(const void* v1, const void* v2, void* out, const size_t n)
{
  const float* i1 = (const float*)v1;
  const float* i2 = (const float*)v2;
  float* o = (float*)out;

  for(size_t i=0; i < n; ++i) {
    o[i] = (float)i1[i] + i2[i];
  }
}
static void
adddouble(const void* v1, const void* v2, void* out, const size_t n)
{
  const double* i1 = (const double*)v1;
  const double* i2 = (const double*)v2;
  float* o = (float*)out;

  for(size_t i=0; i < n; ++i) {
    o[i] = (float)i1[i] + i2[i];
  }
}

/* ensures the two files given can be combined, as per the rules of this
 * program.  this generally means that the data are registered. */
static bool
compatible(struct ookfile* f1, struct ookfile* f2)
{
  if(ookbricks(f1) != ookbricks(f2)) {
    return false;
  }

  size_t bsize[2][3];
  ookmaxbricksize(f1, bsize[0]);
	ookmaxbricksize(f2, bsize[1]);
  if(bsize[0][0] != bsize[1][0] ||
     bsize[0][1] != bsize[1][1] ||
     bsize[0][2] != bsize[1][2]) {
    return false;
	}

  uint64_t dims[2][3];
  ookdimensions(f1, dims[0]);
  ookdimensions(f2, dims[1]);
  if(dims[0][0] != dims[1][0] ||
     dims[0][1] != dims[1][1] ||
     dims[0][2] != dims[1][2]) {
    return false;
  }

  return true;
}

/* filenames which will give the input to the program. */
static char* input[2] = {NULL, NULL};
/* filename of the output file we will generate. */
static char* output = NULL;
/* verbosity of output.  0 (the default) is terse. */
static uint16_t verbose = 0U;

static char*
tjfstrdup(const char* str)
{
  const size_t n = strlen(str);
  char* rv = xmalloc(n + 1);
  return strncpy(rv, str, n+1);
}

/* sets global variables (options) based on command line options.
 * allocates 'input' and 'output'. */
static void
parseopt(int argc, char* const argv[])
{
  int opt;
  while((opt = getopt(argc, argv, "i:o:v")) != -1) {
    switch(opt) {
      case 'i': {
        const size_t idx = input[0] == NULL ? 0 : 1;
        if(input[idx] != NULL) {
          fprintf(stderr, "Max two inputs! '%s' is too many.\n", optarg);
          exit(EXIT_FAILURE);
        }
        input[idx] = tjfstrdup(optarg);
      } break;
      case 'o':
        if(output != NULL) { free(output); output = NULL; }
        output = tjfstrdup(optarg);
        break;
      case 'v':
        verbose++;
        break;
      default:
				fprintf(stderr, "Usage: %s -i <filename> -i <filename> -o <filename>"
                " [-v]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
  }
  if(NULL == input[0]) {
    fprintf(stderr, "No input files given!\n");
    exit(EXIT_FAILURE);
  }
  if(NULL == input[1]) {
    fprintf(stderr, "Need two input files!\n");
    exit(EXIT_FAILURE);
  }
  if(NULL == output) {
    fprintf(stderr, "Output file needed.\n");
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
  const uint64_t volumesize[3] = { 2025, 1600, 400 };
  const uint64_t bricksize[3] = { 405, 320, 80 };

  struct ookfile* f1 = ookread(StdCIO, input[0], volumesize, bricksize,
                               OOK_U16, 1);
  if(!f1) { perror("open"); exit(EXIT_FAILURE); }
  struct ookfile* f2 = ookread(StdCIO, input[1], volumesize, bricksize,
                               OOK_U8, 1);
  if(!f2) { perror("open"); exit(EXIT_FAILURE); }

  if(!compatible(f1, f2)) {
    fprintf(stderr, "Data not registered.\n");
    ookclose(f1); ookclose(f2);
    exit(EXIT_FAILURE);
  }

  size_t bsize[3];
  ookmaxbricksize(f1, bsize);

  const size_t components = 1;

  const size_t bytes_brick[2] = {
    sizeof(uint16_t) * components * bsize[0]*bsize[1]*bsize[2],
    sizeof(uint16_t) * components * bsize[0]*bsize[1]*bsize[2],
  };
  void* data[2] = { xmalloc(bytes_brick[0]), xmalloc(bytes_brick[1]) };
  float* outdata = xmalloc(sizeof(float) * components *
                           bsize[0]*bsize[1]*bsize[2]);
  uint64_t dims[3];
  ookdimensions(f1, dims);

  struct ookfile* fout = ookcreate(StdCIO, output, dims, bsize, OOK_FLOAT,
                                   components);
  if(!fout) {
    perror("open");
    free(outdata);
    free(data[0]); free(data[1]);
    ookclose(f1); ookclose(f2);
    return EXIT_FAILURE;
  }
  const enum OOKTYPE type = OOK_U16;

  typedef void (t_func_apply)(const void*, const void*, void*, const size_t);
  t_func_apply* fqn;
  switch(type) {
    case OOK_I8: fqn = addi8; break;
    case OOK_U8: fqn = addu8; break;
    case OOK_I16: fqn = addi16; break;
    case OOK_U16: fqn = addu16; break;
    case OOK_I32: fqn = addi32; break;
    case OOK_U32: fqn = addu32; break;
    case OOK_I64: fqn = addi64; break;
    case OOK_U64: fqn = addu64; break;
    case OOK_FLOAT: fqn = addfloat; break;
    case OOK_DOUBLE: fqn = adddouble; break;
  }

  for(size_t brick=0; brick < ookbricks(f1); ++brick) {
    size_t bs[3];
    ookbricksize(f1, brick, bs);
    ookbrick(f1, brick, &data[0]);
    ookbrick(f2, brick, &data[1]);
    fqn(data[0], data[1], outdata, bs[0]*bs[1]*bs[2]);
    ookwrite(fout, brick, outdata);
  }

  free(data[0]);
  free(data[1]);
  free(outdata);
  free(input[0]);
  free(input[1]);
  free(output);
  ookclose(f1);
  ookclose(f2);
}
