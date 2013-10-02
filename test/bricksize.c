#include <check.h>
#include "ook.h"

/* type is opaque; make it transparent. */
struct oofile {
  void* blank_;
  struct io op;
  size_t bricksize[3];
  uint64_t volsize[3];
  enum FAKE { F_ONE, F_TWO } f;
  size_t blank2_;
};

/* verifies brick sizes when the divisor is even */
START_TEST(test_bsize_even)
{
	struct oofile of;
	of.volsize[0] = of.volsize[1] = of.volsize[2] = 1000;
	of.bricksize[0] = of.bricksize[1] = of.bricksize[2] = 100;
	size_t bs[3];
	for(size_t i=0; i < 10*10*10; ++i) {
		ookbricksize((struct ookfile*)&of, i, bs);
    ck_assert_int_eq(bs[0], 100);
    ck_assert_int_eq(bs[1], 100);
    ck_assert_int_eq(bs[2], 100);
	}
}
END_TEST

START_TEST(test_bsize_uneven)
{
	struct oofile of;
	of.volsize[0] = of.volsize[1] = of.volsize[2] = 40;
	of.bricksize[0] = of.bricksize[1] = of.bricksize[2] = 16;
	size_t bs[3];
	ookbricksize((struct ookfile*)&of, 0, bs);
	ck_assert(bs[0] == 16 && bs[1] == 16 && bs[2] == 16);
	ookbricksize((struct ookfile*)&of, 1, bs);
	ck_assert(bs[0] == 16 && bs[1] == 16 && bs[2] == 16);
	ookbricksize((struct ookfile*)&of, 2, bs);
	ck_assert(bs[0] == 8 && bs[1] == 16 && bs[2] == 16);
	ookbricksize((struct ookfile*)&of, 3, bs);
	ck_assert(bs[0] == 16 && bs[1] == 16 && bs[2] == 16);
	ookbricksize((struct ookfile*)&of, 4, bs);
	ck_assert(bs[0] == 16 && bs[1] == 16 && bs[2] == 16);
	ookbricksize((struct ookfile*)&of, 5, bs);
	ck_assert(bs[0] == 8 && bs[1] == 16 && bs[2] == 16);
	ookbricksize((struct ookfile*)&of, 6, bs);
	ck_assert(bs[0] == 16 && bs[1] == 8 && bs[2] == 16);
	ookbricksize((struct ookfile*)&of, 7, bs);
	ck_assert(bs[0] == 16 && bs[1] == 8 && bs[2] == 16);
	ookbricksize((struct ookfile*)&of, 8, bs);
	ck_assert(bs[0] == 8 && bs[1] == 8 && bs[2] == 16);

	ookbricksize((struct ookfile*)&of, 17, bs);
	ck_assert(bs[0] == 8 && bs[1] == 8 && bs[2] == 16);
	ookbricksize((struct ookfile*)&of, 18, bs);
	ck_assert(bs[0] == 16 && bs[1] == 16 && bs[2] == 8);
	ookbricksize((struct ookfile*)&of, 19, bs);
	ck_assert(bs[0] == 16 && bs[1] == 16 && bs[2] == 8);
	ookbricksize((struct ookfile*)&of, 20, bs);
	ck_assert(bs[0] == 8 && bs[1] == 16 && bs[2] == 8);
	ookbricksize((struct ookfile*)&of, 21, bs);
	ck_assert(bs[0] == 16 && bs[1] == 16 && bs[2] == 8);
	ookbricksize((struct ookfile*)&of, 22, bs);
	ck_assert(bs[0] == 16 && bs[1] == 16 && bs[2] == 8);
	ookbricksize((struct ookfile*)&of, 23, bs);
	ck_assert(bs[0] == 8 && bs[1] == 16 && bs[2] == 8);
	ookbricksize((struct ookfile*)&of, 24, bs);
	ck_assert(bs[0] == 16 && bs[1] == 8 && bs[2] == 8);
	ookbricksize((struct ookfile*)&of, 25, bs);
	ck_assert(bs[0] == 16 && bs[1] == 8 && bs[2] == 8);
	ookbricksize((struct ookfile*)&of, 26, bs);
	ck_assert(bs[0] == 8 && bs[1] == 8 && bs[2] == 8);
}
END_TEST

Suite*
bricksize_suite()
{
  Suite* s = suite_create("bsize");
  TCase* tc = tcase_create("bsize-case");
  tcase_add_test(tc, test_bsize_even);
  tcase_add_test(tc, test_bsize_uneven);
  suite_add_tcase(s, tc);
  return s;
}
