# Errno

The `Errno` module defines shared error codes, lifecycle-state constants, and basic error reporting helpers used across the library.

## Types and Constants

- `ft_size_t` - Project size type, currently `uint64_t`.
- `ft_bool` - Project boolean storage type, currently `uint8_t`.
- `FT_FALSE` / `FT_TRUE` - Project boolean constants.
- `PTErrorCode` - Signed project error-code enum. Negative values represent critical failures, zero is `FT_ERR_SUCCESS`, and positive values represent recoverable errors.
- `FT_CLASS_STATE_UNINITIALISED` - Lifecycle state value for objects that have not been initialized.
- `FT_CLASS_STATE_DESTROYED` - Lifecycle state value for objects that were destroyed or failed initialization.
- `FT_CLASS_STATE_INITIALISED` - Lifecycle state value for initialized objects.
- `FT_ERR_IS_SUCCESS(error_code)` - Tests whether an error code is success.
- `FT_ERR_HAS_ERROR(error_code)` - Tests whether an error code is non-zero.
- `FT_ERR_IS_CRITICAL(error_code)` - Tests whether an error code is a critical negative failure.
- `FT_ERR_IS_RECOVERABLE(error_code)` - Tests whether an error code is a positive recoverable outcome.

## Public API

- `ft_strerror(int32_t error_code)` - Returns a readable string for a project error code.
- `ft_perror(const char *error_msg, int32_t error_code)` - Prints a caller-provided message and readable error text.
- `ft_exit(const char *error_msg, int32_t exit_code)` - Prints an error message and exits the process.
