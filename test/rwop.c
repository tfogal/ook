#include <stdio.h>
#include <stdlib.h>
#include <check.h>
#include "ook.h"

static struct ookfile* zeroes;

static void
setup()
{
  ck_assert(ookinit(StdCIO));
  const uint64_t sz[3] = { 32, 32, 32 };
  const size_t bsize[3] = { 16, 16, 32 };
  zeroes = ookread("/dev/zero", sz, bsize, OOK_U16, 1);
  ck_assert_int_ne(zeroes, NULL);
}

static void
teardown()
{
  ck_assert(ookclose(zeroes) == 0);
  zeroes = NULL; /* force memory being unreachable (for leak checking) */
}

START_TEST(test_rwop_simple)
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
START_TEST(test_rwop_all_bricks)
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

Suite*
rwop_suite()
{
  Suite* s = suite_create("rwop");
  TCase* tc = tcase_create("rwop-case");
  tcase_add_test(tc, test_rwop_simple);
  tcase_add_test(tc, test_rwop_all_bricks);
  tcase_add_checked_fixture(tc, setup, teardown);
  suite_add_tcase(s, tc);
  return s;
}
