#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <check.h>
#include "ook.h"

/* older versions of check don't have pointer tests, so we hack it with integer
 * tests.  but I guess type checking is stricter in newer check; the int tests
 * don't work for pointers anymore. */
#if CHECK_MINOR_VERSION == 9 && CHECK_MICRO_VERSION >= 10
# define tjf_ck_ptr_eq(a,b) ck_assert_ptr_eq(a,b)
# define tjf_ck_ptr_ne(a,b) ck_assert_ptr_ne(a,b)
#else
# define tjf_ck_ptr_eq(a,b) ck_assert_int_eq(a,b)
# define tjf_ck_ptr_ne(a,b) ck_assert_int_ne(a,b)
#endif

static struct ookfile* zeroes;
static const char* simplefile = ".testing";
static const char* multifile = ".testing-multicomponent";
static const char* towrite = ".simple-writetest";
static const char* thirty = ".30";
static struct ookfile* of;

static void
print_arru32(const uint32_t* d, size_t n)
{
  printf("{ ");
  for(size_t i=0; i < n; ++i) {
    printf("%u ", d[i]);
  }
  printf("}\n");
}

void
print_arru16(const uint16_t* d, const size_t n)
{
  printf("{ ");
  for(size_t i=0; i < n; ++i) {
    printf("%hu ", d[i]);
  }
  printf("}\n");
}

void
print_slice(const uint32_t* d, size_t slice, const size_t nx,const size_t ny)
{
  const size_t z = slice;
  for(size_t y=0; y < ny; ++y) {
    const size_t coord = z*ny*nx + y*nx;
    print_arru32(d+coord, nx);
  }
}

static uint32_t
value(size_t x, size_t y, size_t z)
{
  return 25700U + z/16 + (y*(16/2)) + x;
}

void
print_all(const uint16_t* d, const size_t bsize[3], const size_t components)
{
  const size_t c=components; /* convenience. */
  for(size_t z=0; z < bsize[2]; ++z) {
    printf("+++++++++++++++++++++++++++\n");
    for(size_t y=0; y < bsize[1]; ++y) {
      printf("| ");
      for(size_t x=0; x < bsize[0]*c; ++x) {
        printf("%5hu ", d[z*bsize[1]*bsize[0]*c + y*bsize[0]*c + x]);
      }
      printf("|\n");
    }
  }
}

void
print_function(const size_t bsize[3])
{
  for(size_t z=0; z < bsize[2]; ++z) {
    printf("+++++++++++++++++++++++++++\n");
    for(size_t y=0; y < bsize[1]; ++y) {
      printf(": ");
      for(size_t x=0; x < bsize[0]*2; ++x) {
        if((x % 2) == 1) {
          printf("%5u ", value(x,y,z) % 16);
        } else {
          printf("%5u ", value(x,y,z));
        }
      }
      printf(":\n");
    }
  }
}

static void
setup_multicomp()
{
  const size_t dims[3] = { 4, 4, 4 };

  FILE* fp = fopen(multifile, "wb");
  tjf_ck_ptr_ne(fp, NULL);
  uint16_t data[dims[0]*2];
  for(size_t z=0; z < dims[2]; ++z) {
    for(size_t y=0; y < dims[1]; ++y) {
      for(size_t x=0; x < dims[0]*2; x+=2) {
        data[x+0] = value(x,y,z);
        data[x+1] = value(x,y,z) % 16;
      }
      fwrite(data, sizeof(uint16_t), dims[0]*2, fp);
    }
  }
  fclose(fp);
  ck_assert(ookinit());

  const uint64_t sz[3] = { dims[0], dims[1], dims[2] };
  const size_t bsize[3] = { dims[0]/2, dims[1],  dims[2]/2 };
  of = ookread(StdCIO, multifile, sz, bsize, OOK_U16, 2);
  tjf_ck_ptr_ne(of, NULL);
}

static void
teardown_multicomp()
{
  ck_assert(ookclose(of) == 0);
  of = NULL; /* force memory being unreachable (for leak checking) */
  remove(multifile);
}

START_TEST(multicomp_read)
{
  size_t bsize[3];
  ookmaxbricksize(of, bsize);
  /* bricksize is stored correctly? */
  ck_assert_int_eq(bsize[0], 2U);
  ck_assert_int_eq(bsize[1], 4U);
  ck_assert_int_eq(bsize[2], 2U);

  const size_t components = 2;
  const size_t bytes_brick = sizeof(uint16_t) * components *
                             bsize[0]*bsize[1]*bsize[2];
  uint16_t* data = malloc(bytes_brick);
  memset(data, 0, bytes_brick);
  ookbrick(of, 0, data);

  for(size_t z=0; z < bsize[2]; ++z) {
    for(size_t y=0; y < bsize[1]; ++y) {
      for(size_t x=0; x < bsize[0]*2; x+=2) {
        ck_assert_int_eq(
          data[z*bsize[1]*bsize[0]*2 + y*bsize[0]*2 + x + 0],
          value(x,y,z)
        );
        ck_assert_int_eq(
          data[z*bsize[1]*bsize[0]*2 + y*bsize[0]*2 + x + 1],
          value(x,y,z) % 16
        );
      }
    }
  }
  free(data);
}
END_TEST

static void
setup_zero()
{
  ck_assert(ookinit());
  const uint64_t sz[3] = { 32, 32, 32 };
  const size_t bsize[3] = { 16, 16, 32 };
  zeroes = ookread(StdCIO, "/dev/zero", sz, bsize, OOK_U16, 1);
  tjf_ck_ptr_ne(zeroes, NULL);
}

static void
teardown_zero()
{
  ck_assert(ookclose(zeroes) == 0);
  zeroes = NULL; /* force memory being unreachable (for leak checking) */
}

START_TEST(zero_simple)
{
  size_t bsize[3];
  ookmaxbricksize(zeroes, bsize);
  const size_t components = 1;
  const size_t bytes_brick = sizeof(uint16_t) * components *
                             bsize[0]*bsize[1]*bsize[2];
  void* data = malloc(bytes_brick);
  ookbrick(zeroes, 0, data);
  free(data);
}
END_TEST

/* tests reading all bricks from a dataset.  we should always get zeroes from
 * this dataset, so we verify that all the data are 0 after the read. */
START_TEST(zero_all_bricks)
{
  size_t bsize[3];
  ookmaxbricksize(zeroes, bsize);

  const size_t components = 1;
  const size_t bytes_brick = sizeof(uint16_t) * components *
                             bsize[0]*bsize[1]*bsize[2];
  void* data = malloc(bytes_brick);

  memset(data, 1, bytes_brick);

  ck_assert_int_eq(ookbricks(zeroes), 4U);

  for(size_t i=0; i < ookbricks(zeroes); ++i) {
    memset(data, 1, bytes_brick); /* force the read to overwrite data */
    ookbrick(zeroes, i, data);
    /* verify every element is 0. */
    for(size_t j=0; j < bsize[0]*bsize[1]*bsize[2]; ++j) {
      ck_assert_int_eq(*((uint16_t*)data), 0U);
    }
  }
  free(data);
}
END_TEST

START_TEST(zero_rw)
{
  size_t bsize[3];
  ookmaxbricksize(zeroes, bsize);

  const size_t components = 1;
  const size_t bytes_brick = sizeof(uint16_t) * components *
                             bsize[0]*bsize[1]*bsize[2];
  void* data = malloc(bytes_brick);
  ookbrick(zeroes, 0, data);
  ookwrite(zeroes, 1, data);
  free(data);
}
END_TEST

static void
setup_simple()
{
  FILE* fp = fopen(simplefile, "wb");
  tjf_ck_ptr_ne(fp, NULL);
  uint32_t data[16];
  for(size_t z=0; z < 16; ++z) {
    for(size_t y=0; y < 16; ++y) {
      for(size_t x=0; x < 16; ++x) {
        data[x] = value(x,y,z);
      }
      fwrite(data, sizeof(uint32_t), 16, fp);
    }
  }
  fclose(fp);
  ck_assert(ookinit());

  const uint64_t sz[3] = { 16, 16, 16 };
  const size_t bsize[3] = { 8, 8, 16 };
  of = ookread(StdCIO, simplefile, sz, bsize, OOK_U32, 1);
  tjf_ck_ptr_ne(of, NULL);
}

static void
teardown_simple()
{
  ck_assert(ookclose(of) == 0);
  of = NULL; /* force memory being unreachable (for leak checking) */
  remove(simplefile);
}

START_TEST(simple_verify)
{
  size_t bsize[3];
  ookmaxbricksize(of, bsize);
  /* bricksize is stored correctly? */
  ck_assert_int_eq(bsize[0], 8U);
  ck_assert_int_eq(bsize[1], 8U);
  ck_assert_int_eq(bsize[2], 16U);

  const size_t components = 1;
  const size_t bytes_brick = sizeof(uint32_t) * components *
                             bsize[0]*bsize[1]*bsize[2];
  uint32_t* data = malloc(bytes_brick);
  memset(data, 0, bytes_brick);
  ookbrick(of, 0, data);
  for(size_t z=0; z < bsize[2]; ++z) {
    for(size_t y=0; y < bsize[1]; ++y) {
      for(size_t x=0; x < bsize[0]; ++x) {
        ck_assert_int_eq(
          data[z*bsize[1]*bsize[0] + y*bsize[0] + x],
          value(x,y,z)
        );
      }
    }
  }
  free(data);
}
END_TEST

START_TEST(simple_layout)
{
  size_t layout[3];
  ooklayout(of, layout);
  ck_assert_int_eq(layout[0], 2);
  ck_assert_int_eq(layout[1], 2);
  ck_assert_int_eq(layout[2], 1);
}
END_TEST

static void
setup_writer()
{
  srand(0xdeadbeef); /* always use same seed, for reproducibility */
  ck_assert(ookinit());

  const uint64_t vol[3] = { 4, 8, 12 };
  const size_t bsize[3] = { 2, 4, 6 };
  const size_t components = 1;
  of = ookcreate(StdCIO, towrite, vol, bsize, OOK_FLOAT, components);
  tjf_ck_ptr_ne(of, NULL);
}

static void
teardown_writer()
{
  ck_assert(ookclose(of) == 0);
  of = NULL; /* force memory being unreachable (for leak checking) */
  remove(towrite);
}

START_TEST(writer_nothing)
{
  /* nothing.  basically just test ookcreate/ookclose. */
}
END_TEST

static uint64_t
filesize(const char* filename)
{
  FILE* fp = fopen(filename, "rb");
  if(!fp) { return 0; }
  ck_assert(fseek(fp, 0, SEEK_END) == 0);
  long sz = ftell(fp);
  ck_assert_int_ne(sz, -1);
  fclose(fp);
  return (uint64_t)sz;
}

START_TEST(writer_threshold)
{
  const uint64_t vol[3] = { 4, 8, 12 };
  const size_t bsize[3] = { 2, 4, 6 };
  const size_t components = 1;
  ck_assert_int_eq(ookbricks(of), 8);

  float* data = malloc(sizeof(float) * bsize[0]*bsize[1]*bsize[2] * components);
  tjf_ck_ptr_ne(data, NULL);
  for(size_t brick=0; brick < ookbricks(of); ++brick) {
    for(size_t z=0; z < bsize[2]; ++z) {
      for(size_t y=0; y < bsize[1]; ++y) {
        for(size_t x=0; x < bsize[0]; ++x) {
          data[z*bsize[1]*bsize[0] + y*bsize[0] + x] = (float)rand() /
                                                       (RAND_MAX/4.0);
        }
      }
    }
    errno=0;
    ookwrite(of, brick, data);
    ck_assert_int_eq(errno, 0);
  }
  ck_assert(ookclose(of) == 0);
  of = ookread(StdCIO, towrite, vol, bsize, OOK_FLOAT, components);
  tjf_ck_ptr_ne(of, NULL);
  const char* thfile = ".threshold-test";
  struct ookfile* fout = ookcreate(StdCIO, thfile, vol, bsize, OOK_U8,
                                   components);
  ck_assert_int_eq(ookbricks(fout), 8);
  uint8_t* thresh = malloc(sizeof(uint8_t) * bsize[0]*bsize[1]*bsize[2] *
                           components);
  tjf_ck_ptr_ne(thresh, NULL);
  for(size_t b=0; b < ookbricks(fout); ++b) {
    ookbrick(of, b, data);
    for(size_t i=0; i < bsize[0]*bsize[1]*bsize[2]; ++i) {
      thresh[i] = 0.25f <= data[i] && data[i] <= 0.7f;
    }
    ookwrite(fout, b, thresh);
  }
  ck_assert(ookclose(fout) == 0);
  free(data); data = NULL;
  free(thresh); thresh = NULL;
  ck_assert_int_eq(filesize(thfile), vol[0]*vol[1]*vol[2]*components);
}
END_TEST

static void
is_value(const float* data, const size_t bsize[3])
{
  for(size_t z=0; z < bsize[2]; ++z) {
    for(size_t y=0; y < bsize[1]; ++y) {
      for(size_t x=0; x < bsize[0]; ++x) {
        assert(data[z*bsize[1]*bsize[0] + y*bsize[0] + x] ==
               (float)value(x,y,z));
        ck_assert(data[z*bsize[1]*bsize[0] + y*bsize[0] + x] ==
                  (float)value(x,y,z));
      }
    }
  }
}

START_TEST(writer_basic)
{
  const uint64_t vol[3] = { 4, 8, 12 };
  const size_t bsize[3] = { 2, 4, 6 };
  const size_t components = 1;

  float* data = malloc(sizeof(float) * bsize[0]*bsize[1]*bsize[2] * components);
  for(size_t z=0; z < bsize[2]; ++z) {
    for(size_t y=0; y < bsize[1]; ++y) {
      for(size_t x=0; x < bsize[0]; ++x) {
        data[z*bsize[1]*bsize[0] + y*bsize[0] + x] = (float)value(x,y,z);
      }
    }
  }
  is_value(data, bsize);
  errno = 0;
  ookwrite(of, 0, data); ck_assert_int_eq(errno, 0);
  ookwrite(of, 1, data); ck_assert_int_eq(errno, 0);
  ookwrite(of, 2, data); ck_assert_int_eq(errno, 0);
  ookwrite(of, 3, data); ck_assert_int_eq(errno, 0);
  ck_assert(ookclose(of) == 0);

  of = ookread(StdCIO, towrite, vol, bsize, OOK_FLOAT, components);
  tjf_ck_ptr_ne(of, NULL);
  for(size_t i=0; i < 4; ++i) {
    memset(data, 0, sizeof(float) * bsize[0]*bsize[1]*bsize[2] * components);
    ookbrick(of, i, data);
    is_value(data, bsize);
  }

  free(data); data = NULL;
}
END_TEST

static void
setup_30()
{
  ck_assert(ookinit());
  FILE* fp = fopen(thirty, "wb");
  ck_assert(fp != NULL);
  for(size_t i=0; i < 30; ++i) {
    float f = (float)i;
    fwrite(&f, sizeof(float), 1, fp);
  }
  fclose(fp);

  const uint64_t vol[3] = {15, 2, 1};
  const size_t bsize[3] = {8, 2, 1};
  const size_t components = 1;
  of = ookread(StdCIO, thirty, vol, bsize, OOK_FLOAT, components);
  tjf_ck_ptr_ne(of, NULL);
}

static void
teardown_30()
{
  ookclose(of);
  of = NULL; /* force memory being unreachable (for leak checking) */
  remove(thirty);
}

START_TEST(lbrick_size)
{
  size_t bsize[3];
  ookbricksize(of, 1, bsize);
  ck_assert_int_eq(bsize[0], 7);
  ck_assert_int_eq(bsize[1], 2);
  ck_assert_int_eq(bsize[2], 1);
  size_t bid[3] = {1, 0, 0};
  ookbricksize3(of, bid, bsize);
  ck_assert_int_eq(bsize[0], 7);
  ck_assert_int_eq(bsize[1], 2);
  ck_assert_int_eq(bsize[2], 1);
}
END_TEST

Suite*
rwop_suite()
{
  Suite* s = suite_create("rwop");
  TCase* zero = tcase_create("zero");
  tcase_add_test(zero, zero_simple);
  tcase_add_test(zero, zero_all_bricks);
  tcase_add_test(zero, zero_rw);
  TCase* simple = tcase_create("simple");
  tcase_add_test(simple, simple_verify);
  tcase_add_test(simple, simple_layout);
  TCase* writer = tcase_create("writer");
  tcase_add_test(writer, writer_nothing);
  tcase_add_test(writer, writer_basic);
  tcase_add_test(writer, writer_threshold);
  TCase* multicomp = tcase_create("multicomp");
  tcase_add_test(multicomp, multicomp_read);
  TCase* lastbrick = tcase_create("lastbrick");
  tcase_add_test(lastbrick, lbrick_size);

  tcase_add_checked_fixture(zero, setup_zero, teardown_zero);
  tcase_add_checked_fixture(simple, setup_simple, teardown_simple);
  tcase_add_checked_fixture(multicomp, setup_multicomp, teardown_multicomp);
  tcase_add_checked_fixture(writer, setup_writer, teardown_writer);
  tcase_add_checked_fixture(lastbrick, setup_30, teardown_30);
  suite_add_tcase(s, zero);
  suite_add_tcase(s, simple);
  suite_add_tcase(s, multicomp);
  suite_add_tcase(s, writer);
  suite_add_tcase(s, lastbrick);
  return s;
}
