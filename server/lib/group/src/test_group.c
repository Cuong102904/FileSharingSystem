#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "group.h"

// Helpers for test setup
static void reset_groups_file(void) {
    FILE *fp = fopen("database/groups.txt", "w"); // truncate or create
    assert(fp != NULL && "Cannot prepare database/groups.txt");
    fclose(fp);
}

static char *slurp_file(const char *path, char *buf, size_t buf_sz) {
    FILE *fp = fopen(path, "r");
    if (!fp) return NULL;
    size_t n = fread(buf, 1, buf_sz - 1, fp);
    buf[n] = '\0';
    fclose(fp);
    return buf;
}

int main(void) {
    // Prepare clean state
    reset_groups_file();

    // Append tests
    Group g1 = create_group(101, 1001, "alpha");
    Group g2 = create_group(102, 1002, "beta");
    Group g3 = create_group(103, 1003, "gamma");

    assert(append_group_to_file(g1) == 0);
    assert(append_group_to_file(g2) == 0);
    assert(append_group_to_file(g3) == 0);

    // Verify file contents
    char file_buf[1024];
    assert(slurp_file("database/groups.txt", file_buf, sizeof(file_buf)) != NULL);

    // Expected lines format: "<group_id> <admin_id> <name>\n"
    const char *expected1 = "101 1001 alpha\n";
    const char *expected2 = "102 1002 beta\n";
    const char *expected3 = "103 1003 gamma\n";

    // Check each expected line appears once and in order
    const char *p = file_buf;
    assert(strstr(p, expected1) != NULL);
    p = strstr(p, expected1) + strlen(expected1);
    assert(strstr(p, expected2) != NULL);
    p = strstr(p, expected2) + strlen(expected2);
    assert(strstr(p, expected3) != NULL);

    // list_groups output capture
    // Redirect stdout to a temp file
    FILE *out = freopen("database/list_out.txt", "w", stdout);
    assert(out != NULL && "Failed to redirect stdout");
    assert(list_groups() == 0);
    fflush(stdout);

    // Restore stdout to terminal
    freopen("/dev/tty", "w", stdout);

    // Read captured output and validate
    char out_buf[2048];
    assert(slurp_file("database/list_out.txt", out_buf, sizeof(out_buf)) != NULL);

    // Each line is: "Group ID: <id>, Admin ID: <id>, Name: <name>\n"
    assert(strstr(out_buf, "Group ID: 101, Admin ID: 1001, Name: alpha") != NULL);
    assert(strstr(out_buf, "Group ID: 102, Admin ID: 1002, Name: beta") != NULL);
    assert(strstr(out_buf, "Group ID: 103, Admin ID: 1003, Name: gamma") != NULL);

    printf("IO tests PASSED ✅\n");
    return 0;
}