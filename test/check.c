#include <stdlib.h>
#include <check.h>

extern Suite* bricksize_suite();
extern Suite* rwop_suite();

int
main(void)
{
  int failed;
  Suite* s = suite_create("ook");
  SRunner* sr = srunner_create(s);
  srunner_add_suite(sr, bricksize_suite());
  srunner_add_suite(sr, rwop_suite());
  srunner_run_all(sr, CK_NORMAL);
  failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
