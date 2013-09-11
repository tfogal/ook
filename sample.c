#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
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
compatible(FILE* f1, FILE* f2)
{
  if(ook_bricks(f1) != ook_bricks(f2) ||
     ook_components(f1) != ook_components(f2) ||
     ook_signed(f1) != ook_signed(f2)) {
    return false;
  }

  size_t bsize[2][3];
  ook_max_brick_size(f1, bsize[0]);
	ook_max_brick_size(f2, bsize[1]);
  if(bsize[0][0] != bsize[1][0] ||
     bsize[0][1] != bsize[1][1] ||
     bsize[0][2] != bsize[1][2]) {
    return false;
	}

  const enum OOKTYPE type[2] = { ook_type(f1), ook_type(f2) };
  if(type[0] != type[1]) {
    return false;
  }

  /* our processing functions are currently broken, assume 1-component data */
  if(ook_components(f1) != 1) {
    return false;
  }

  uint64_t dims[2][3];
  ook_dimensions(f1, dims[0]);
  ook_dimensions(f2, dims[1]);
  if(dims[0][0] != dims[1][0] ||
     dims[0][1] != dims[1][1] ||
     dims[0][2] != dims[1][2]) {
    return false;
  }

  /* we should remove this restriction at some point, but again our processing
   * functions are nonsense. */
  const bool sgned[2] = { ook_signed(f1), ook_signed(f2) };
  if(sgned[0] != sgned[1]) {
    return false;
  }

  return true;
}

int
main(int argc, char* argv[])
{
  if(argc != 3) {
    return 0;
  }
  FILE* f1 = fopen(argv[1], "r");
  if(!f1) { perror("open"); exit(EXIT_FAILURE); }
  FILE* f2 = fopen(argv[2], "r");
  if(!f2) { perror("open"); exit(EXIT_FAILURE); }

  if(!ook_init()) {
    fclose(f1); fclose(f2);
    fprintf(stderr, "Initialization failed.\n");
    exit(EXIT_FAILURE);
  }

  if(!compatible(f1, f2)) {
    fprintf(stderr, "Data not registered.\n");
    fclose(f1); fclose(f2);
    exit(EXIT_FAILURE);
  }

  size_t bsize[3];
  ook_max_brick_size(f1, bsize);

  const enum OOKTYPE type[2] = { ook_type(f1), ook_type(f2) };

  const size_t bytes_brick[2] = {
    ook_width(f1) * ook_components(f1) * bsize[0]*bsize[1]*bsize[2],
    ook_width(f2) * ook_components(f2) * bsize[0]*bsize[1]*bsize[2]
  };
  void* data[2] = { xmalloc(bytes_brick[0]), xmalloc(bytes_brick[1]) };
  float* output = xmalloc(sizeof(float) * ook_components(f1) *
                          bsize[0]*bsize[1]*bsize[2]);

  uint64_t dims[3];
  ook_dimensions(f1, dims);

  struct OokFile* fout = ook_new("output", bsize, dims, OOK_FLOAT,
                                 ook_components(f1));
  if(!fout) {
    perror("open");
    free(output);
    free(data[0]); free(data[1]);
    fclose(f1); fclose(f2);
    return EXIT_FAILURE;
  }

  typedef void (t_func_apply)(const void*, const void*, void*, const size_t);
  t_func_apply* fqn;
  switch(type[0]) {
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

  for(size_t brick=0; brick < ook_bricks(f1); ++brick) {
    size_t bs[3];
    ook_brick_size(f1, brick, bs);
    ook_brick(f1, brick, &data[0]);
    ook_brick(f2, brick, &data[1]);
    fqn(data[0], data[1], output, bs[0]*bs[1]*bs[2]);
    ook_brick_out(fout, brick, output, bsize[0]*bsize[1]*bsize[2]);
  }

  free(data[0]);
  free(data[1]);
  free(output);
}
