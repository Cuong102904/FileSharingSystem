#include "../../utils/testing.h"
#include "../include/membership.h"
#include <stdio.h>
#include <stdlib.h>

int failed = 0;

// Helper to create test data
void setup_test_data() {
  FILE *fp = fopen("database/group_members.txt", "w");
  if (!fp) {
    printf("Failed to create test database\n");
    return;
  }
  fprintf(fp, "Group1 duongtt\n");
  fprintf(fp, "Group1 alice\n");
  fprintf(fp, "Group2 duongtt\n");
  fprintf(fp, "Group2 bob\n");
  fclose(fp);
}

void cleanup_test_data() { remove("database/group_members.txt"); }

TEST(test_membership_valid_member) {
  setup_test_data();

  // duongtt is member of Group1
  ASSERT(membership_check("Group1", "duongtt") == true);

  cleanup_test_data();
}

TEST(test_membership_not_member) {
  setup_test_data();

  // bob is NOT member of Group1
  ASSERT(membership_check("Group1", "bob") == false);

  cleanup_test_data();
}

TEST(test_membership_group_not_exists) {
  setup_test_data();

  // Group999 doesn't exist
  ASSERT(membership_check("Group999", "duongtt") == false);

  cleanup_test_data();
}

TEST(test_membership_file_not_exists) {
  cleanup_test_data(); // Ensure file doesn't exist

  // Should return false when file missing
  ASSERT(membership_check("Group1", "duongtt") == false);
}

int main() {
  printf("Running Group Management Unit Tests\n");
  printf("===================================\n");

  RUN_TEST(test_membership_valid_member);
  RUN_TEST(test_membership_not_member);
  RUN_TEST(test_membership_group_not_exists);
  RUN_TEST(test_membership_file_not_exists);

  printf("\n===================================\n");

  if (failed) {
    printf("Some tests FAILED\n");
    return 1;
  } else {
    printf("All tests PASSED\n");
    return 0;
  }
}
