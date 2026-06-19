# Targeted Test Runner

The `tools/test_runner.py` helper allows building and executing only the
selected test translation units instead of compiling the entire suite.  The
tool searches for the requested sources, triggers the `Test/Makefile` with the
`FILES` override, and propagates the requested test names to the runtime
filter.

## Usage

```bash
# Build and run every FT_TEST contained in Test/Test/test_strlen.cpp
python3 tools/test_runner.py Test/Test/test_strlen.cpp

# Compile the file that owns `test_strlen_basic` and execute only that test
python3 tools/test_runner.py test_strlen_basic

# Combine filters or build the debug variant
python3 tools/test_runner.py test_strlen.cpp test_strlen_basic --debug

# Target a heavy multi-test class such as API request
python3 tools/test_runner.py test_api_request_invalid_ip_sets_socket_error
```

The script automatically scans every test translation unit for `FT_TEST`
definitions and keeps a lookup table in memory.  Supplying either
`--test name` or the positional shorthand `name` is enough to find the owning
source file and to compile only that object.  When specific test names are
provided the helper prints which translation unit will be built so you can
confirm the resolved targets before invoking `make`.  At runtime the
`FT_TEST_NAME_FILTER` environment variable is set so the updated test runner
only executes the requested `FT_TEST` functions.

The runtime filter also works directly when invoking `Test/libft_tests`.
Filter values are comma-separated substring matches, so this runs every test
whose name contains `serializer`:

```bash
FT_TEST_NAME_FILTER=serializer ./Test/libft_tests
```

Set `FT_TEST_HIDE_SUCCESSFUL=1` to keep successful tests out of the final
output. The runner still prints the currently executing test on one transient
line; that line is cleared when the test passes and replaced with a persistent
`FAIL` line only when the test fails.

Abort diagnostics written to `stderr` are left visible so `su_abort()` stack
traces can reach the console.

```bash
FT_TEST_NAME_FILTER=serializer FT_TEST_HIDE_SUCCESSFUL=1 ./Test/libft_tests
```

Targeting the more complex API request suite demonstrates that the helper can
still quickly map a single FT_TEST to the sprawling `Test/test_api_request.cpp`
translation unit.  Only that object is recompiled while the Full_Libft archive
is reused, letting you iterate on classes with large dependencies without
triggering a full test rebuild.
