# Refactoring Summary

### Refactor: Protocol Command Structure
- **Change**: Replaced generic `arg1`, `arg2`, `arg3` in `ParsedCommand` struct with a **Tagged Union**.
- **Reason**: To improve code readability ("Which argument is the username?") and type safety.
- **Workflow**:
    - **Before**: `cmd.type = LOGIN; strcpy(cmd.arg1, "user");`
    - **After**: `cmd.type = LOGIN; strcpy(cmd.payload.auth.username, "user");`

### Unit Testing Code Pattern
When writing tests for the refactored structure, access the specific union member corresponding to the command type.

**Example for UPLOAD:**
```c
TEST(test_parse_upload_command) {
    ParsedCommand cmd;
    const char *input = "UPLOAD mygroup file.txt docs/";
    
    protocol_parse_command(input, &cmd);
    
    // Check type
    ASSERT(cmd.type == CMD_UPLOAD);
    
    // Check specific payload fields (Union Members)
    ASSERT_STR_EQ(cmd.payload.upload.group, "mygroup");
    ASSERT_STR_EQ(cmd.payload.upload.local_path, "file.txt");
    ASSERT_STR_EQ(cmd.payload.upload.remote_path, "docs/");
}
```

### Testing Framework (`server/lib/utils/testing.h`)
Minimal macro-based testing utility.
- **`TEST(name)`**: Defines a test function.
- **`ASSERT(expr)`**: Checks condition. Prints PASS (Green) or FAIL (Red). Sets global `failed` flag.
- **`ASSERT_STR_EQ(s1, s2)`**: Helper to check string equality.
- **`RUN_TEST(name)`**: Executes the test function and prints its name.

### How to Run Tests
1.  **Compile & Run**: The `Makefile` handles everything.
    ```bash
    make test
    ```
    **Flow**: `make test` -> Compiles `test_protocol_parser` binary -> Runs it immediately.
2.  **Output**: You will see a list of PASS/FAIL for each assertion, followed by a summary "All tests PASSED" or "Some tests FAILED".
