#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "group.h"

int main() {
    printf("Testing create_group...\n");

    // Test 1: normal input
    Group g1 = create_group(1, 10, "study_group");
    assert(g1.group_id == 1);
    assert(g1.admin_id == 10);
    assert(strcmp(g1.name, "study_group") == 0);
    printf("Test 1 PASSED\n");

    // Test 2: NULL name
    Group g2 = create_group(2, 20, NULL);
    assert(g2.group_id == 2);
    assert(g2.admin_id == 20);
    assert(strcmp(g2.name, "") == 0);
    printf("Test 2 PASSED\n");

    // Test 3: long name (overflow safety)
    char long_name[300];
    memset(long_name, 'A', sizeof(long_name) - 1);
    long_name[299] = '\0';

    Group g3 = create_group(3, 30, long_name);
    assert(g3.group_id == 3);
    assert(g3.admin_id == 30);
    assert(strlen(g3.name) < sizeof(g3.name));
    printf("Test 3 PASSED\n");

    printf("\nAll tests PASSED ✅\n");
    return 0;
}
