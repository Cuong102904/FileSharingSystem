#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "../include/group_repo.h"
#include "../include/group.h"

// Forward-declared in group.c
extern void init_mutex();

// Helper: write test data in CSV format "id,admin_id,name"
static void write_groups_db(void) {
    FILE* fp = fopen(GROUP_DB, "w");
    assert(fp && "Failed to open GROUP_DB for writing");
    
    fputs("1,100,Alpha\n", fp);
    fputs("2,200,Beta Group\n", fp);
    fputs("3,300,Gamma\n", fp);
    fclose(fp);
}

static void test_find() {
    printf("[TEST] group_find_by_id\n");
    assert(group_find_by_id(1) == 1);
    assert(group_find_by_id(3) == 1);
    assert(group_find_by_id(999) == 0);

    printf("[TEST] group_find_by_name\n");
    assert(group_find_by_name("Alpha") == 1);
    assert(group_find_by_name("Beta Group") == 1);
    assert(group_find_by_name("Unknown") == 0);
}

static void test_save_and_delete() {
    printf("[TEST] group_save\n");
    Group g = new_group(4, 400, "Delta");
    int save_rc = group_save(&g);
    assert(save_rc == 0);
    assert(group_find_by_id(4) == 1);
    assert(group_find_by_name("Delta") == 1);

    printf("[TEST] group_delete\n");
    int del_rc = group_delete(2); // delete "Beta Group"
    assert(del_rc == 1);
    assert(group_find_by_id(2) == 0);
    assert(group_find_by_name("Beta Group") == 0);
}

static void test_list_all() {
    printf("[TEST] group_list_all\n");
    group_list_all(); // visually verify output
}

int main(void) {
    // Use the shared mutex
    init_mutex();

    // Prepare test DB (CSV format expected by the parser)
    write_groups_db();

    test_find();
    test_save_and_delete();
    test_list_all();

    printf("All group tests passed.\n");
    return 0;
}