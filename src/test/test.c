#include "../prima.h"

#include <check.h>
#include <stdlib.h>

Suite *Prima_suite_create(void);

START_TEST(Prima_power_valid_test)
{
    uint32_t length = 1;
    uint8_t value[] = {(uint8_t)1};
    int result = isValidPowerControl(length, value);
    ck_assert_int_eq(1, result);
}
END_TEST

Suite *Prima_suite_create(void)
{
    Suite *suite = suite_create("Prima_suite");
    TCase *tcase_core = tcase_create("Prima_tcase");
    tcase_add_test(tcase_core, Prima_power_valid_test);
    suite_add_tcase(suite, tcase_core);
    return suite;
}

int main() {
  int failed_count = 0;
  Suite* suite1 = Prima_suite_create();
  SRunner* suite_runner1 = srunner_create(suite1);
  srunner_run_all(suite_runner1, CK_NORMAL);
  failed_count += srunner_ntests_failed(suite_runner1);
  srunner_free(suite_runner1);

  printf("Fail = %d\n", failed_count);

  return 0;
}