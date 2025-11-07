# FullLibft

Libft is a collection of C and C++ utilities grouped into multiple small libraries.
It provides implementations of common libc functions, custom memory allocation helpers,
basic threading helpers, containers, string utilities, simple networking and more.
The top level `Makefile` builds every submodule and links them into `Full_Libft.a`.
The umbrella header `FullLibft.hpp` includes every component.
Internal code uses custom replacements such as `ft_strlen`, `ft_strchr`, `ft_strstr`, and `pf_snprintf` instead of the standard library equivalents. Move semantics now rely on the in-house `ft_move` helper implemented in the Template module.
All size counters rely on the `ft_size_t` typedef (aliasing `unsigned long long`) so modules share a consistent width for
buffer lengths and digit counts.
Header files now use class names or concise module names instead of module prefixes, except internal headers which retain their module prefix.

This document briefly lists the main headers and the interfaces they expose. The
summaries below only outline the available functions and classes. See the header
files for detailed information. High-level design notes for each subsystem now
live in `Docs/module_overviews.md` with per-module invariants and error-handling
conventions referenced by the summaries below.

All classes report errors through a mutable `_error_code` member with `get_error` and `get_error_str` accessors so `const` methods can update the error state. A thread-local `ft_errno` variable mirrors the last error for cross-module access.

## Building

```bash
make
```

## Testing

```bash
make tests
./Test/libft_tests
```

Functional tests reside in `Test/Test` and performance benchmarks in `Test/Efficiency`. The test suite is currently incomplete
and some files may fail to compile until they are updated for recent interface changes.

## API reference documentation

Libft ships with a Doxygen configuration that emits browsable API documentation
covering the public headers across every module. Install Doxygen locally and run
the helper script to validate the configuration or build the docs directly:

```bash
tools/run_doxygen.py --check   # verifies that referenced modules exist
tools/run_doxygen.py           # runs doxygen when the executable is available
```

Generated artifacts are written beneath `Docs/api/html`. The warning log lives at
`Docs/api/doxygen-warnings.log` so new diagnostics are easy to triage. The
configuration treats the module overview as the main page to connect the
high-level design notes with the generated reference.

The test runner prints `OK` or `KO` for each registered case and
summarizes the total. Detailed assertion failures are written to
`test_failures.log` with the source file and line number of the failing
check. Output is grouped by module so related tests appear together.

Each test uses the `FT_TEST` macro, which registers the case at program
startup so all linked test files run automatically.

The current suite exercises components across multiple modules:

- **Libft**: `ft_atoi`, `ft_atol`, `ft_bzero`, `ft_isdigit`, `ft_isalpha`, `ft_isalnum`, `ft_islower`, `ft_isupper`, `ft_isprint`, `ft_isspace`, `ft_memchr`,
  `ft_memcmp`, `ft_memcpy`, `ft_memdup`, `ft_memmove`, `ft_memset`, `ft_strchr`, `ft_strcmp`, `ft_strlcat`, `ft_strlcpy`, `ft_strncpy`, `ft_strlen`, `ft_strncmp`,
  `ft_strnstr`, `ft_strstr`, `ft_strrchr`, `ft_strmapi`, `ft_striteri`, `ft_strtok`, `ft_strtol`, `ft_strtoul`, `ft_setenv`, `ft_unsetenv`, `ft_getenv`, `ft_to_lower`, `ft_to_upper`,
`ft_fopen`, `ft_fclose`, `ft_fgets`, `ft_time_ms`, `ft_time_format`, `ft_to_string`
- **PThread**: `ft_task_scheduler` joins the existing `ft_thread` and `ft_this_thread` helpers. The scheduler clears success paths, surfaces queue allocation or empty-pop failures through its `_error_code` mirror, routes timed callbacks through the Time module's `time_monotonic_point_*` helpers instead of constructing `std::chrono::steady_clock` points directly, stores worker state in `ft_vector<ft_thread>` so thread management and futures stay on the library's error-reporting abstractions, and releases the scheduled-task mutex before executing fallbacks when cloning scheduled callbacks or pushing them into the work queue fails so recursive scheduling never deadlocks. Delayed and periodic submissions return cancellation handles that broadcast the scheduler condition variable and mirror `_error_code` into `ft_errno` when tasks are removed.
- **CrossProcess**: shared-memory descriptor send/receive helpers coordinate socket transport, platform-specific mapping, shared-mutex polling with bounded retries, payload acknowledgements that zero consumed buffers, and mirrored error-slot updates for both POSIX and Windows builds.
- **Networking**: IPv4 and IPv6 send/receive paths, UDP datagrams, a simple HTTP server, and WebSocket client/server handshake
  coverage using the RFC 6455 GUID alongside a regression that splits the client handshake response across multiple receives to
  validate incremental parsing
- **Logger**: color toggling, JSON sink, asynchronous logging, and the `ft_logger` façade propagates sink, file, syslog, and remote target failures through its `_error_code` mirror so configuration helpers always synchronize `ft_errno`.
- **Math**: vector, matrix, and quaternion helpers plus expression evaluation via `math_roll` (arithmetic, unary negatives, precedence, dice, lengthy expressions, and error handling) and tolerance-based floating-point helpers that avoid exact equality comparisons when validating modulus and cosine inputs
- **RNG**: normal, exponential, Poisson, binomial, and geometric distributions
- **String**: `ft_string_view`
- **CPP_class**: `ft_big_number` assignment, arithmetic, comparisons, error handling, and the `DataBuffer` utility now propagates allocator and stream failures through `_error_code` while clearing successful reads and writes. The `ft_file` wrapper routes all descriptor I/O through the `System_utils` helpers (`su_open`, `su_read`, `su_write`, `su_close`), defers closing the active descriptor until a new open succeeds, and reports failures through `_error_code` while leaving the previous handle untouched when replacements fail.
- **Template**: Iterators, pools, and object handles surface invalid dereferences, pool exhaustion, and container failures through `_error_code` so range helpers and pooled resources synchronize `ft_errno` after every operation.
- **JSon**: schema validation
- **HTML**: the `html_document` helper validates node, attribute, and selector inputs, mirrors allocation and serialization failures into `_error_code`, and clears successes before returning so DOM operations leave `ft_errno` authoritative.
- **API**: TLS clients and promise adapters validate inputs, propagate socket, allocation, and JSON parsing failures through `_error_code`, and clear success paths so synchronous and asynchronous requests leave `ft_errno` authoritative.
- **YAML**: round-trip parsing
- **Game**: `ft_game_state` centralizes worlds and character data with vectors of shared pointers for RAII cleanup, `ft_world` persistence and a shared-pointer-based `ft_event_scheduler` for timed actions via `ft_world::schedule_event` and `ft_world::update_events`, `ft_equipment`, `ft_inventory`, and `ft_quest` store items through shared pointers, `ft_crafting` consumes and produces shared items, `ft_world::plan_route`, `ft_pathfinding`, and copy/move constructors across game classes
- The combat system introduces `FT_DAMAGE_RULE_MAGIC_SHIELD`, which treats magic armor as a shield. Armor-based reduction is applied only to the damage that reaches health after the shield is exhausted and uses the `FT_PHYSICAL_DAMAGE_REDUCTION` and `FT_MAGIC_DAMAGE_REDUCTION` macros.
- Shared pointers expose their own `get_error` while managed objects may define their own; call `shared_ptr.get_error()` for allocation issues and `shared_ptr->get_error()` for object-specific errors.
- Game classes validate both the shared pointer and its managed object for errors before use so failures surface consistently.
- `ft_item` tracks its own error code so equipment, inventory, and crafting verify both the item pointer and the item itself before applying modifiers or stacking quantities.
- Buffs, debuffs, upgrades, events, and quests expose explicit `get_error` / `get_error_str` helpers, clear their state on success through their setters, and translate invalid inputs or container failures into `ft_errno` updates so callers can detect misconfiguration without stale success paths leaving the errno mirror untouched.
- Inventory, experience tables, map grids, reputations, skills, achievements, and world orchestration share the same error-clearing discipline, validating identifiers and container mutations before synchronizing `ft_errno` so downstream systems see authoritative status codes after each operation.
- JSON document helpers validate group and item mutations before committing them, surface parser and allocation failures through `_error_code`, and clear successful read/write operations so file serialization keeps `ft_errno` synchronized with the latest result.
- The `time_timer` utility now tracks `_error_code` as it starts, updates, and adjusts running timers, rejecting negative durations or paused states and mirroring the resulting status codes to `ft_errno` for callers.
- **Encryption**: key generation utilities and a SHA-1 hashing helper used to compute WebSocket accept keys

Additional cases verify whitespace parsing, overlapping ranges, truncating copies, partial zeroing, empty needles,
zero-length operations, null pointers, zero-size buffers, searches for the terminating character,
prefix comparisons, boundary digits, non-letter characters, embedded null bytes,
unsigned-byte comparisons, empty strings, repeated delimiters, index-based mapping, zero padding, empty haystacks,
negative fill values, overflow bytes and both-null comparisons.

To build the debug version use `make debug`. Individual sub-modules can be built by
entering their directory and running `make`.

## Modules

### Libft

Standard C utilities located in `Libft/`. Headers: `libft.hpp` and `limits.hpp`. Source files use the `libft_` prefix.

Embedded or size-constrained builds can opt out of optional helpers by defining feature
macros before including `libft.hpp`. Each macro defaults to `1` and can be set to `0`
to exclude the associated functionality:

- `LIBFT_ENABLE_ENVIRONMENT_HELPERS` removes `ft_getenv`, `ft_setenv`, and `ft_unsetenv`.
- `LIBFT_ENABLE_FILE_IO_HELPERS` removes `ft_fopen`, `ft_fclose`, and `ft_fgets`.
- `LIBFT_ENABLE_TIME_HELPERS` removes `ft_time_ms` and `ft_time_format`.
- `LIBFT_ENABLE_BOUNDS_CHECKED_HELPERS` removes the `_s` safety wrappers such as
  `ft_memcpy_s` and `ft_strncat_s`.
- `LIBFT_ENABLE_LOCALE_HELPERS` removes the locale-aware collation and case-folding
  helpers described below.

The convenience macros `LIBFT_HAS_*` mirror the effective configuration so callers can
branch on availability without repeating preprocessor checks.

#### Coordinating with CMA string helpers

Libft’s duplication helpers are implemented on top of the CMA allocator so projects can
mix the higher-level `ft_*` APIs with the lower-level `cma_*` entry points without
switching error-handling conventions. The overlapping functions now share the same
`ft_errno` outcomes for their core failure paths:

| Scenario | Libft helper | CMA helper | `ft_errno` |
| --- | --- | --- | --- |
| Null source with non-zero length | `ft_memdup`, `ft_span_dup` | `cma_memdup` | `FT_ERR_INVALID_ARGUMENT` |
| Zero-length span | `ft_memdup`, `ft_span_dup` | `cma_memdup` | `ER_SUCCESS` |
| Allocation failure | `ft_memdup`, `ft_span_dup` | `cma_memdup` | `FT_ERR_NO_MEMORY` |

Formatting wrappers follow the same pattern: `ft_to_string` mirrors allocator failures
as `FT_ERR_NO_MEMORY`, just like `cma_itoa`, while also reporting conversion issues as
`FT_ERR_INTERNAL`. CMA helpers that accept optional pointers, such as `cma_strdup`, leave
`ft_errno` untouched when invoked with `ft_nullptr`. Libft callers expecting
`FT_ERR_INVALID_ARGUMENT` for null inputs should validate their pointers before choosing
between the two surfaces, keeping mixed code paths predictable.

```
typedef uint32_t (*ft_utf8_case_hook)(uint32_t code_point);
size_t  ft_strlen_size_t(const char *string);
int     ft_strlen(const char *string);
char   *ft_strchr(const char *string, int char_to_find);
int     ft_atoi(const char *string);
int     ft_validate_int(const char *input);
void    ft_bzero(void *string, size_t size);
void   *ft_memchr(const void *ptr, int c, size_t size);
void   *ft_memcpy(void *dst, const void *src, size_t n);
void   *ft_memmove(void *dst, const void *src, size_t n);
size_t  ft_strlcat(char *dst, const char *src, size_t size);
size_t  ft_strlcpy(char *dst, const char *src, size_t size);
char   *ft_strrchr(const char *s, int c);
char   *ft_strnstr(const char *haystack, const char *needle, size_t len);
char   *ft_strstr(const char *haystack, const char *needle);
int     ft_strncmp(const char *s1, const char *s2, size_t n);
int     ft_memcmp(const void *s1, const void *s2, size_t n);
int     ft_isdigit(int c);
int     ft_isalpha(int c);
int     ft_isalnum(int c);
long    ft_atol(const char *string);
int     ft_strcmp(const char *s1, const char *s2);
void    ft_to_lower(char *string);
void    ft_to_upper(char *string);
char   *ft_strncpy(char *dst, const char *src, size_t n);
char   *ft_strtok(char *string, const char *delimiters);
long    ft_strtol(const char *input_string, char **end_pointer, int numeric_base);
unsigned long ft_strtoul(const char *nptr, char **endptr, int base);
int     ft_setenv(const char *name, const char *value, int overwrite);
int     ft_unsetenv(const char *name);
char   *ft_getenv(const char *name);
int     ft_locale_compare(const char *left, const char *right, const char *locale_name);
ft_string ft_locale_casefold(const char *input, const char *locale_name);
void   *ft_memset(void *dst, int value, size_t n);
int     ft_isspace(int c);
char   *ft_fgets(char *string, int size, FILE *stream);
FILE   *ft_fopen(const char *filename, const char *mode);
int     ft_fclose(FILE *stream);
int64_t ft_time_ms(void);
char   *ft_time_format(char *buffer, size_t buffer_size);
ft_string ft_to_string(long number);
int     ft_utf8_next(const char *string, size_t string_length, size_t *index_pointer,
            uint32_t *code_point_pointer, size_t *sequence_length_pointer);
int     ft_utf8_count(const char *string, size_t *code_point_count_pointer);
int     ft_utf8_encode(uint32_t code_point, char *buffer, size_t buffer_size,
            size_t *encoded_length_pointer);
int     ft_utf8_transform(const char *input, size_t input_length, char *output_buffer,
            size_t output_buffer_size, ft_utf8_case_hook case_hook);
int     ft_utf8_transform_alloc(const char *input, char **output_pointer,
            ft_utf8_case_hook case_hook);
uint32_t ft_utf8_case_ascii_lower(uint32_t code_point);
uint32_t ft_utf8_case_ascii_upper(uint32_t code_point);
int     ft_utf8_is_combining_code_point(uint32_t code_point);
int     ft_utf8_next_grapheme(const char *string, size_t string_length,
            size_t *index_pointer, size_t *grapheme_length_pointer);
int     ft_utf8_duplicate_grapheme(const char *string, size_t string_length,
            size_t *index_pointer, char **grapheme_pointer);
```

`ft_strtol` clamps values that exceed `FT_LONG_MAX` or `FT_LONG_MIN` and sets
`ft_errno` to `FT_ERR_OUT_OF_RANGE` when overflow is detected.

`ft_utf8_next` and `ft_utf8_count` decode UTF-8 sequences while mirroring invalid
byte sequences through `FT_ERR_INVALID_ARGUMENT`. The transformation helpers keep ASCII case
conversion on the existing hooks, set `FT_ERR_OUT_OF_RANGE` if the destination buffer is
too small, and surface allocation failures as `FT_ERR_NO_MEMORY`. Optional grapheme
wrappers reuse the CMA allocation utilities to duplicate composed characters
while leaving successful iterations with `ER_SUCCESS` so callers can detect the
end of input without spurious errors.

`ft_fgets` sets `ft_errno` to `FT_ERR_END_OF_FILE` when the stream reaches end of
file without an input error, allowing callers to differentiate EOF from other
failures.

`ft_setenv`, `ft_unsetenv`, and `ft_getenv` serialize every environment
mutation through an internal mutex so concurrent threads do not corrupt the
process-wide state. Because these helpers still operate on the shared
environment for the entire process, callers should coordinate updates and clean
up temporary overrides once the surrounding work completes.

`ft_locale_compare` and `ft_locale_casefold` wrap the new System_utils locale
helpers so callers can perform collation-aware comparisons and lowercasing
without constructing `std::locale` instances directly. When no locale name is
provided the helpers fall back to the process default and then to the classic C
locale if initialization fails, reporting configuration issues through
`ft_errno`.

`limits.hpp` exposes integer boundary constants:

```
FT_CHAR_BIT
FT_INT_MAX
FT_INT_MIN
FT_UINT_MAX
FT_LONG_MAX
FT_LONG_MIN
FT_ULONG_MAX
```

### Math

Located in `Math/`. Headers: `math.hpp` and `roll.hpp`. Provides basic math utilities.
`math.hpp` now uses the `MATH_MATH_HPP` include guard to avoid colliding with the template helper in `Template/math.hpp`, which switched to `FT_TEMPLATE_MATH_HPP`.

```
int         math_abs(int number);
long        math_abs(long number);
long long   math_abs(long long number);
double      math_fabs(double number);
double      math_fmod(double value, double modulus);
int         math_signbit(double number);
int         math_isnan(double number);
double      math_nan(void);
void        math_swap(int *first_number, int *second_number);
int         math_clamp(int value, int minimum, int maximum);
int         math_gcd(int first_number, int second_number);
long        math_gcd(long first_number, long second_number);
long long   math_gcd(long long first_number, long long second_number);
ft_big_number    math_big_gcd(const ft_big_number& first_number, const ft_big_number& second_number);
int         math_lcm(int first_number, int second_number);
long        math_lcm(long first_number, long second_number);
long long   math_lcm(long long first_number, long long second_number);
ft_big_number    math_big_lcm(const ft_big_number& first_number, const ft_big_number& second_number);
int         math_max(int first_number, int second_number);
long        math_max(long first_number, long second_number);
long long   math_max(long long first_number, long long second_number);
double      math_max(double first_number, double second_number);
int         math_min(int first_number, int second_number);
long        math_min(long first_number, long second_number);
long long   math_min(long long first_number, long long second_number);
double      math_min(double first_number, double second_number);
int         math_factorial(int number);
long        math_factorial(long number);
long long   math_factorial(long long number);
int         math_absdiff(int first_number, int second_number);
long        math_absdiff(long first_number, long second_number);
long long   math_absdiff(long long first_number, long long second_number);
double      math_absdiff(double first_number, double second_number);
int         math_average(int first_number, int second_number);
long        math_average(long first_number, long second_number);
long long   math_average(long long first_number, long long second_number);
double      math_average(double first_number, double second_number);
double      math_pow(double base_value, int exponent);
double      math_sqrt(double number);
double      math_exp(double value);
double      math_log(double value);
double      math_acos(double dot);
double      math_cos(double value);
double      ft_sin(double value);
double      ft_tan(double value);
double      math_deg2rad(double degrees);
int         math_validate_int(const char *input);
double      math_rad2deg(double radians);
double      ft_mean(const double *values, int array_size);
double      ft_median(const double *values, int array_size);
double      ft_mode(const double *values, int array_size);
double      ft_variance(const double *values, int array_size);
double      ft_stddev(const double *values, int array_size);
double      ft_geometric_mean(const double *values, int array_size);
double      ft_harmonic_mean(const double *values, int array_size);
```

`math_big_gcd` and `math_big_lcm` bridge the `Math` helpers with the
`ft_big_number` class so applications can compute number theory operations on
operands that exceed native integer ranges while retaining the existing big
integer error propagation.

Example usage:

```
double sine = ft_sin(math_deg2rad(90.0));
double tangent = ft_tan(math_deg2rad(45.0));
double numbers[] = {1.0, 2.0, 2.0, 3.0};
double mean = ft_mean(numbers, 4);
double median = ft_median(numbers, 4);
double mode = ft_mode(numbers, 4);
double variance = ft_variance(numbers, 4);
double standard_deviation = ft_stddev(numbers, 4);
double geometric_mean = ft_geometric_mean(numbers, 4);
double harmonic_mean = ft_harmonic_mean(numbers, 4);
```

Basic linear algebra types are provided in `linear_algebra.hpp`.
Implementations live in dedicated source files for each vector type with
shared constructors and destructors for clarity:

```
vector2(double x, double y);
vector2 add(const vector2 &other) const;
vector2 subtract(const vector2 &other) const;
double dot(const vector2 &other) const;
double length() const;
vector2 normalize() const;

vector3(double x, double y, double z);
vector3 cross(const vector3 &other) const;
vector3 add(const vector3 &other) const;
vector3 subtract(const vector3 &other) const;
double dot(const vector3 &other) const;
double length() const;
vector3 normalize() const;

vector4(double x, double y, double z, double w);
vector4 add(const vector4 &other) const;
vector4 subtract(const vector4 &other) const;
double dot(const vector4 &other) const;
double length() const;
vector4 normalize() const;

matrix2();
matrix2(double m00, double m01,
        double m10, double m11);
matrix2 multiply(const matrix2 &other) const;
matrix2 invert() const;
vector2 transform(const vector2 &vector) const;

matrix3();
matrix3(double m00, double m01, double m02,
        double m10, double m11, double m12,
        double m20, double m21, double m22);
matrix3 multiply(const matrix3 &other) const;
matrix3 invert() const;
vector3 transform(const vector3 &vector) const;

matrix4();
matrix4(double m00, double m01, double m02, double m03,
        double m10, double m11, double m12, double m13,
        double m20, double m21, double m22, double m23,
        double m30, double m31, double m32, double m33);
static matrix4 make_translation(double x, double y, double z);
static matrix4 make_scale(double x, double y, double z);
static matrix4 make_rotation_x(double angle);
static matrix4 make_rotation_y(double angle);
static matrix4 make_rotation_z(double angle);
matrix4 multiply(const matrix4 &other) const;
matrix4 invert() const;
vector4 transform(const vector4 &vector) const;

quaternion(double w, double x, double y, double z);
quaternion add(const quaternion &other) const;
quaternion multiply(const quaternion &other) const;
quaternion conjugate() const;
double length() const;
quaternion normalize() const;
```

Normalization routines treat near-zero lengths as invalid using an epsilon check.

Basic transformations can be built with the matrix factories and combined with
vector utilities:

```
vector3 position(1.0, 2.0, 3.0);
vector3 offset(0.5, 0.0, -1.0);
vector3 moved = position.add(offset);
matrix4 translation = matrix4::make_translation(0.5, 0.0, -1.0);
vector4 homogenous(moved.get_x(), moved.get_y(), moved.get_z(), 1.0);
vector4 transformed = translation.transform(homogenous);
```

Additional helpers for parsing expressions are available. They allocate a
single `int` holding the result, which the caller must release with
`cma_free`. On failure they return `ft_nullptr` and set `ft_errno`:

```
int        *math_roll(const char *expression);
int        *math_eval(const char *expression);
```

Internal parser helpers now use the `math_` prefix for consistency. Error
diagnostics set `ft_errno` but remain silent unless you compile with
`DEBUG` defined to `1` before including `roll.hpp`, which re-enables the
legacy debug logging.

### Geometry

Located in `Geometry/`. Header: `geometry.hpp`. Provides basic collision helpers:

```
bool    intersect_aabb(const aabb &first, const aabb &second);
bool    intersect_circle(const circle &first, const circle &second);
bool    intersect_sphere(const sphere &first, const sphere &second);
```

### Custom Memory Allocator (CMA)

Located in `CMA/`. Header: `CMA.hpp`. Provides memory helpers such as
`cma_malloc`, `cma_free`, aligned allocations, allocation-size queries,
and string helpers. Aligned allocations round the block size up to the
specified power-of-two (e.g., requesting 100 bytes with alignment 32
returns a 128-byte block) and are released with `cma_free`. Allocation
sizes can be inspected directly through `cma_block_size`, or safely with
`cma_checked_block_size`, which reports invalid pointers via `ft_errno`.
The `cma_atoi` helper validates its input before conversion and returns a
CMA-allocated integer on success or `ft_nullptr` on failure.
When allocation logging is enabled via the logger, the allocator emits debug messages for each `cma_malloc` and `cma_free`.
The allocator enforces an optional global allocation limit that can be
changed at runtime with `cma_set_alloc_limit`. A limit of `0` disables the
check. When thread safety is enabled, limit updates acquire the allocator
mutex so concurrent callers never observe torn writes, and they preserve the
incoming `ft_errno` value on both success and failure. Disabling thread
safety via `cma_set_thread_safety(false)` skips the mutex entirely, which
keeps single-threaded tests cheap but requires the caller to avoid mixing
limit updates with concurrent allocator traffic.

The limit check runs before the allocator touches shared state, letting
tests inject deterministic failures by setting a small cap and attempting an
oversized allocation. Pair the toggle with `cma_set_thread_safety(false)`
when failure injection must run without synchronization overhead, and
restore both settings during teardown so the allocator returns to its
default invariants. See
[`Docs/cma_allocation_controls.md`](Docs/cma_allocation_controls.md) for a
focused walkthrough and ready-to-copy helper routines.

The allocator also tracks allocation and free counts, accessible through
`cma_get_stats`, and exposes live plus peak byte usage via
`cma_get_extended_stats`.
`cma_calloc` now validates multiplication overflow by ensuring the element
count is zero or the product of `count` and `size` stays within `SIZE_MAX`.
Overflowing requests return `ft_nullptr` without incrementing the allocation
counter, and a regression test exercises the guard to ensure the allocator
never zero-initializes past the allocated buffer.
Thread safety can be enabled or disabled with `cma_set_thread_safety`.

```
void   *cma_malloc(ft_size_t size);
void    cma_free(void *ptr);
int     cma_checked_free(void *ptr);
char   *cma_strdup(const char *string);
void   *cma_memdup(const void *src, size_t size);
void   *cma_calloc(ft_size_t nmemb, ft_size_t size);
void   *cma_realloc(void *ptr, ft_size_t new_size);
void   *cma_aligned_alloc(ft_size_t alignment, ft_size_t size);
ft_size_t cma_alloc_size(const void *ptr);
ft_size_t cma_block_size(const void *memory_pointer);
int    cma_checked_block_size(const void *memory_pointer, ft_size_t *block_size);
int   *cma_atoi(const char *string);
char  **cma_split(const char *s, char c);
char   *cma_itoa(int n);
char   *cma_itoa_base(int n, int base);
char   *cma_strjoin(const char *s1, const char *s2);
char   *cma_strjoin_multiple(int count, ...);
char   *cma_substr(const char *s, unsigned int start, size_t len);
char   *cma_strtrim(const char *s1, const char *set);
void    cma_free_double(char **content);
void    cma_set_alloc_limit(ft_size_t limit);
void    cma_set_thread_safety(bool enable);
void    cma_get_stats(ft_size_t *allocation_count, ft_size_t *free_count);
void    cma_get_extended_stats(ft_size_t *allocation_count,
            ft_size_t *free_count,
            ft_size_t *current_bytes,
            ft_size_t *peak_bytes);
```

### Safe Compacted Memory Allocator (SCMA)

Located in `SCMA/`. Header: `SCMA.hpp`. SCMA manages relocatable heap
storage through opaque `scma_handle` identifiers instead of raw
pointers. Each handle stores a block index and generation counter so the
runtime can detect stale references when slots are recycled. Live bytes
reside in a single `ft_vector<unsigned char>` buffer that is compacted on
every allocation, after each free, and midway through resizes. The
compactor walks the metadata table, uses `std::memmove` to close gaps,
and updates offsets and the global used-size tracker before new blocks
are appended.

All public entry points validate the handle and synchronize through the
module mutex to keep relocations atomic. Allocation helpers grow the
backing heap as needed, reuse freed slots when available, and bump the
generation so callers holding an outdated handle see `FT_ERR_INVALID_HANDLE`
instead of mutating recycled storage. The API surface mirrors
`ft_errno` for error reporting and exposes convenience helpers for
reading, writing, resizing, snapshotting, and querying allocation sizes.

For structured access, the `scma_handle_accessor<T>` template wraps a
handle and requires trivially copyable types. It revalidates the handle
before each operation, offers `read_struct` / `write_struct`, indexed
`read_at` / `write_at` helpers, and reports how many `T` elements fit in
the block via `get_count`. The accessor exposes `get_error` /
`get_error_str` accessors so callers can diagnose failures while SCMA
continues compacting the heap behind the scenes, and the module exposes
manual `scma_mutex_lock` / `scma_mutex_unlock` helpers together with
`scma_mutex_lock_count` for nested synchronization.

### GetNextLine

`GetNextLine/get_next_line.hpp` implements a simple file reader that operates on POSIX file descriptors with a configurable buffer size. It stores per-descriptor leftovers in the custom `ft_unordered_map`, allowing error reporting through `ft_errno` without relying on the `CPP_class` stream wrappers.

```
char   *ft_strjoin_gnl(char *string_one, char *string_two);
char   *get_next_line(int fd, std::size_t buffer_size);
```

### Printf

`Printf/printf.hpp` contains minimal `printf` wrappers.

```
int pf_printf(const char *format, ...);
int pf_printf_fd(int fd, const char *format, ...);
int pf_snprintf(char *string, size_t size, const char *format, ...);
int pf_vsnprintf(char *string, size_t size, const char *format, va_list args);
```

`%f` supports precision using the `%.Nf` syntax.

### PThread Wrappers

`PThread/pthread.hpp` wraps a few `pthread` calls, condition variables, read-write locks, and provides basic atomic operations.

```
int pt_thread_join(pthread_t thread, void **retval);
int pt_thread_create(pthread_t *thread, const pthread_attr_t *attr,
                     void *(*start_routine)(void *), void *arg);
int pt_thread_detach(pthread_t thread);
int pt_thread_cancel(pthread_t thread);
int pt_thread_sleep(unsigned int milliseconds);
int pt_thread_yield();
int pt_thread_equal(pthread_t thread1, pthread_t thread2);
pt_thread_id_type pt_thread_self();
template <typename ValueType, typename Function>
int pt_async(ft_promise<ValueType>& promise, Function function);
int pt_atomic_load(const std::atomic<int>& atomic_variable);
void pt_atomic_store(std::atomic<int>& atomic_variable, int desired_value);
int pt_atomic_fetch_add(std::atomic<int>& atomic_variable, int increment_value);
bool pt_atomic_compare_exchange(std::atomic<int>& atomic_variable, int& expected_value, int desired_value);
int pt_cond_init(pthread_cond_t *condition, const pthread_condattr_t *attributes);
int pt_cond_destroy(pthread_cond_t *condition);
int pt_cond_wait(pthread_cond_t *condition, pthread_mutex_t *mutex);
int pt_cond_signal(pthread_cond_t *condition);
int pt_cond_broadcast(pthread_cond_t *condition);
int pt_rwlock_init(pthread_rwlock_t *rwlock, const pthread_rwlockattr_t *attributes);
int pt_rwlock_rdlock(pthread_rwlock_t *rwlock);
int pt_rwlock_wrlock(pthread_rwlock_t *rwlock);
int pt_rwlock_unlock(pthread_rwlock_t *rwlock);
int pt_rwlock_destroy(pthread_rwlock_t *rwlock);
```

### Thread

`PThread/thread.hpp` provides a lightweight `ft_thread` class built on POSIX threads:

```
ft_thread();
template <typename FunctionType, typename... Args>
ft_thread(FunctionType function, Args... args);
~ft_thread();
bool joinable() const;
void join();
void detach();
int get_error() const;
const char *get_error_str() const;
```

### C++ Classes (`CPP_class`)

Header files in `CPP_class/` define several helper classes.
Below is a brief list of the main classes and selected members.

#### `DataBuffer`
Uses the `ft_vector` container for internal byte storage, allowing allocation
failures to surface through `ft_errno`.
```
DataBuffer();
DataBuffer(const DataBuffer& other);
DataBuffer(DataBuffer&& other) noexcept;
DataBuffer& operator=(const DataBuffer& other);
DataBuffer& operator=(DataBuffer&& other) noexcept;
~DataBuffer();
void clear() noexcept;
size_t size() const noexcept;
const ft_vector<uint8_t>& data() const noexcept;
explicit operator bool() const noexcept;
bool good() const noexcept;
bool bad() const noexcept;
template<typename T> DataBuffer& operator<<(const T& value);
template<typename T> DataBuffer& operator>>(T& value);
DataBuffer& operator<<(size_t len);
DataBuffer& operator>>(size_t& len);
```

#### `ft_file`
```
ft_file() noexcept;
ft_file(const char* filename, int flags, mode_t mode) noexcept;
ft_file(const char* filename, int flags) noexcept;
ft_file(int fd) noexcept;
~ft_file() noexcept;
ft_file(ft_file&& other) noexcept;
ft_file& operator=(ft_file&& other) noexcept;
int     get_fd() const;
int     get_error() const noexcept;
const char *get_error_str() const noexcept;
int     open(const char* filename, int flags, mode_t mode) noexcept;
int     open(const char* filename, int flags) noexcept;
ssize_t write(const char *string) noexcept;
void    close() noexcept;
int     seek(off_t offset, int whence) noexcept;
ssize_t read(char *buffer, int count) noexcept;
int     printf(const char *format, ...) __attribute__((format(printf,2,3),hot));
operator int() const;
```

The `printf` helper forwards to the Printf module's `pf_printf_fd_v` to write formatted output directly to the file descriptor.
All descriptor operations delegate to `System_utils`'s `su_open`, `su_read`, `su_write`, and `su_close` wrappers so the class uses the same cross-platform behaviour as the rest of the library while surfacing failures through `_error_code`.

`ft_file` assumes exclusive ownership of the descriptor it wraps. Constructing an instance from an existing `int` transfers responsibility for closing that descriptor to the wrapper, so callers must not invoke `close`, `su_close`, or similar helpers on the raw handle once it has been handed to `ft_file`. When replacing an open descriptor through `open`, the existing handle remains active until the new open succeeds; failures leave the prior handle untouched so there is no window where both descriptors are invalid simultaneously. Ownership moves with the move constructor and move assignment operator, which reset the source instance to `-1` so only the destination closes the descriptor during teardown.

Use `close()` before passing the descriptor to other code that expects to assume ownership. The implicit `operator int()` exposes the underlying handle for integration with APIs that require a raw descriptor, but it does not transfer ownership; the wrapper will still close the descriptor in its destructor unless `close()` was called. When bridging to code paths that duplicate descriptors (for example with `dup` or `su_dup`), take the duplicate before `close()` so the wrapper can clean up its copy without affecting the external user.

#### `ft_istream`
```
ft_istream() noexcept;
~ft_istream() noexcept;
void read(char *buffer, std::size_t count);
std::size_t gcount() const noexcept;
bool bad() const noexcept;
int get_error() const noexcept;
const char *get_error_str() const noexcept;
```

#### `ft_stringbuf`
```
ft_stringbuf(const ft_string &string) noexcept;
~ft_stringbuf() noexcept;
std::size_t read(char *buffer, std::size_t count);
bool is_bad() const noexcept;
int get_error() const noexcept;
const char *get_error_str() const noexcept;
```

#### `ft_istringstream`
```
ft_istringstream(const ft_string &string) noexcept;
~ft_istringstream() noexcept;
```

#### `ft_ofstream`
```
ft_ofstream() noexcept;
~ft_ofstream() noexcept;
int open(const char *filename) noexcept;
ssize_t write(const char *string) noexcept;
void close() noexcept;
int get_error() const noexcept;
const char *get_error_str() const noexcept;
```

#### `ft_string`
```
ft_string() noexcept;
ft_string(const char *init_str) noexcept;
ft_string(const ft_string& other) noexcept;
ft_string(ft_string&& other) noexcept;
ft_string &operator=(const ft_string& other) noexcept;
ft_string &operator=(ft_string&& other) noexcept;
ft_string &operator=(const char *&other) noexcept;
ft_string& operator+=(const ft_string& other) noexcept;
ft_string& operator+=(const char* cstr) noexcept;
ft_string& operator+=(char c) noexcept;
~ft_string();
explicit ft_string(int error_code) noexcept;
static void* operator new(size_t size) noexcept;
static void operator delete(void* ptr) noexcept;
static void* operator new[](size_t size) noexcept;
static void operator delete[](void* ptr) noexcept;
void        append(char c) noexcept;
void        append(const char *string) noexcept;
void        append(const ft_string &string) noexcept;
void        clear() noexcept;
const char *at(size_t index) const noexcept;
const char *c_str() const noexcept;
char*       print() noexcept;
size_t      size() const noexcept;
bool        empty() const noexcept;
int         get_error() const noexcept;
const char *get_error_str() const noexcept;
void        move(ft_string& other) noexcept;
void        erase(std::size_t index, std::size_t count) noexcept;
operator const char*() const noexcept;
```

#### `ft_big_number`

**Feature summary**

- Stores arbitrarily large integers in a dynamically sized decimal buffer that
  grows and shrinks as digits are appended or trimmed.
- Tracks the sign separately from the digit storage so negative results are
  preserved and can be queried with `is_negative()`/`is_positive()` without
  altering the magnitude returned by `c_str()`.
- Accepts digits in any base from binary through hexadecimal via
  `assign_base()` and serializes back out with `to_string_base()` and the
  helper conversion functions.
- Supports modular arithmetic through `operator%()` and `mod_pow()` so the type
  fits cryptographic and number-theory workflows.
- Surfaces failures through `get_error()`/`get_error_str()` and mirrors the
  state in `ft_errno` so callers can propagate issues across module
  boundaries.

**Constructors and assignment**

- `ft_big_number()` – default-constructs zero with no sign.
- Copy/move construction and assignment operators clone or transfer the managed
  digit buffer while preserving the current sign and error state.
- `assign()`, `append_digit()`, `append()` and `append_unsigned()` accept
  decimal digits, while `assign_base()` parses alternate bases before normal
  form storage.

**Arithmetic operators**

- Addition, subtraction, multiplication, division, and modulo are available via
  the overloaded operators, each respecting the stored sign.
- Comparison operators mirror standard integer semantics for use in ordered
  containers or validation code paths.
- `mod_pow()` exposes an efficient modular exponentiation helper that reuses
  the existing arithmetic operators and error propagation.

**Error reporting**

- `FT_ERR_NO_MEMORY` – memory allocation failed while growing or
  shrinking the digit buffer.
- `FT_ERR_INVALID_ARGUMENT` – parsing encountered a non-numeric digit for the
  requested base.
- `FT_ERR_INVALID_STATE` – a magnitude-only subtraction detected the
  right operand was larger than the left, which would otherwise yield a
  negative magnitude.
- `FT_ERR_DIVIDE_BY_ZERO` – division or modulo detected a zero divisor.
- Query errors with `get_error()`/`get_error_str()` after each operation to
  guard against silent failures.

**API reference**

```
ft_big_number() noexcept;
ft_big_number(const ft_big_number& other) noexcept;
ft_big_number(ft_big_number&& other) noexcept;
ft_big_number& operator=(const ft_big_number& other) noexcept;
ft_big_number& operator=(ft_big_number&& other) noexcept;
ft_big_number operator+(const ft_big_number& other) const noexcept;
ft_big_number operator-(const ft_big_number& other) const noexcept;
ft_big_number operator*(const ft_big_number& other) const noexcept;
ft_big_number operator/(const ft_big_number& other) const noexcept;
ft_big_number operator%(const ft_big_number& other) const noexcept;
bool          operator==(const ft_big_number& other) const noexcept;
bool          operator!=(const ft_big_number& other) const noexcept;
bool          operator<(const ft_big_number& other) const noexcept;
bool          operator<=(const ft_big_number& other) const noexcept;
bool          operator>(const ft_big_number& other) const noexcept;
bool          operator>=(const ft_big_number& other) const noexcept;
void        assign(const char* number) noexcept;
void        assign_base(const char* digits, int base) noexcept;
void        append_digit(char digit) noexcept;
void        append(const char* digits) noexcept;
void        append_unsigned(unsigned long value) noexcept;
void        trim_leading_zeros() noexcept;
void        reduce_to(ft_size_t new_size) noexcept;
void        clear() noexcept;
const char *c_str() const noexcept;
ft_size_t   size() const noexcept;
bool        empty() const noexcept;
bool        is_negative() const noexcept;
bool        is_positive() const noexcept;
ft_string   to_string_base(int base) noexcept;
ft_big_number mod_pow(const ft_big_number& exponent, const ft_big_number& modulus) const noexcept;
int         get_error() const noexcept;
const char *get_error_str() const noexcept;
```

The standalone helpers below expose common serialization flows so callers can
emit uppercase hexadecimal digits or parse them (optionally prefixed with
`0x`/`0X` and an optional leading sign) without manually juggling intermediate
base conversions:

```
ft_string   big_number_to_hex_string(const ft_big_number& number) noexcept;
ft_big_number   big_number_from_hex_string(const char* hex_digits) noexcept;
```

**Usage examples**

```
ft_big_number balance;
balance.assign("2500");
ft_big_number withdrawal;
withdrawal.assign("2750");
ft_big_number delta = balance - withdrawal;
if (delta.is_negative())
{
    // delta.c_str() == "250" (magnitude) while the sign tracks the overdraft
}
```

```
ft_big_number value;
value.assign_base("FF", 16);      // 255 in decimal storage
ft_big_number modulus;
modulus.assign("97");             // prime modulus
ft_big_number remainder = value % modulus;
ft_string binary = value.to_string_base(2); // "11111111"
```

For additional scenarios, edge cases, and regression coverage, explore the
dedicated tests in [`Test/Test/test_big_number.cpp`](Test/Test/test_big_number.cpp).

#### `ft_nullptr`
```
struct ft_nullptr_t {
    template <typename PointerType>
    operator PointerType*() const { return nullptr; }
    template <typename ClassType, typename MemberType>
    operator MemberType ClassType::*() const { return nullptr; }
    void operator&() const; /* deleted */
};
extern const ft_nullptr_t ft_nullptr_instance;
#define ft_nullptr (ft_nullptr_instance)
```

### Networking

`Networking/networking.hpp` and `socket_class.hpp` implement a small
socket wrapper with IPv4 and IPv6 support.

`SocketConfig`, `ft_socket`, and `udp_socket` each maintain a mutable
`_error_code` and a private `set_error` helper so every configuration,
connection, or datagram operation clears successes and forwards system
errors through `ft_errno`.

Non-blocking client sockets leave the descriptor open when `connect`
returns `EINPROGRESS`, `EWOULDBLOCK`, or `WSAEWOULDBLOCK`. Callers must
wait for the socket to become writable using `nw_poll`, `select`, or the
event loop helpers before checking the connection result (for example by
querying `get_error` or `SO_ERROR`).

The accompanying tests exercise basic send and receive paths and invalid
configurations for both address families in `Test/test_networking.cpp`.

```
int nw_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int nw_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int nw_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int nw_listen(int sockfd, int backlog);
int nw_socket(int domain, int type, int protocol);
ssize_t nw_sendto(int sockfd, const void *buf, size_t len, int flags,
                  const struct sockaddr *dest_addr, socklen_t addrlen);
ssize_t nw_recvfrom(int sockfd, void *buf, size_t len, int flags,
                    struct sockaddr *src_addr, socklen_t *addrlen);
int nw_set_nonblocking(int socket_fd);
int nw_poll(int *read_file_descriptors, int read_count,
            int *write_file_descriptors, int write_count,
            int timeout_milliseconds);
void event_loop_init(event_loop *loop);
void event_loop_clear(event_loop *loop);
int  event_loop_add_socket(event_loop *loop, int socket_fd, bool is_write);
int  event_loop_remove_socket(event_loop *loop, int socket_fd, bool is_write);
int  event_loop_run(event_loop *loop, int timeout_milliseconds);
int  udp_event_loop_wait_read(event_loop *loop, udp_socket &socket, int timeout_milliseconds);
int  udp_event_loop_wait_write(event_loop *loop, udp_socket &socket, int timeout_milliseconds);
ssize_t udp_event_loop_receive(event_loop *loop, udp_socket &socket, void *buffer, size_t size,
                               int flags, struct sockaddr *source_address,
                               socklen_t *address_length, int timeout_milliseconds);
ssize_t udp_event_loop_send(event_loop *loop, udp_socket &socket, const void *data, size_t size,
                            int flags, const struct sockaddr *destination_address,
                            socklen_t address_length, int timeout_milliseconds);
```

The polling backend is chosen at compile time. Linux builds use `epoll`,
BSD and macOS use `kqueue`, and other systems including Windows fall back
to `select`. The event loop helpers call the appropriate backend through
`nw_poll`. The `epoll` and `kqueue` implementations allocate their event
buffers with `cma_malloc` and release them with `cma_free` to stay within
the custom allocator. The `select` backend is intended only for small
numbers of sockets and lacks the scalability of the other backends.

`udp_event_loop_wait_read` and `udp_event_loop_wait_write` wrap the common
registration and readiness checks for datagram sockets. They temporarily
add the socket to the loop, ensure it is non-blocking, and wait for the
requested edge before restoring the loop state. The higher-level
`udp_event_loop_receive` and `udp_event_loop_send` helpers build on top of
those primitives to perform `recvfrom`/`sendto` calls once readiness is
signaled, returning `0` on timeouts so callers can retry without blocking.

`wrapper.hpp` adds helpers for encrypted sockets:

```
ssize_t nw_ssl_write(SSL *ssl, const void *buf, size_t len);
ssize_t nw_ssl_read(SSL *ssl, void *buf, size_t len);
```

Both helpers validate the requested length against `INT_MAX` before handing the
operation to OpenSSL. Calls that exceed the library limit return `-1` without
invoking `SSL_write` or `SSL_read` to avoid truncating the request size.

Simple event loop example:

```
#include "Networking/networking.hpp"

int main()
{
    int server_socket;
    event_loop loop;
    int result;

    event_loop_init(&loop);
    server_socket = nw_socket(AF_INET, SOCK_STREAM, 0);
    nw_set_nonblocking(server_socket);
    event_loop_add_socket(&loop, server_socket, false);
    while (true)
    {
        result = event_loop_run(&loop, 1000);
        if (result > 0 && loop.read_file_descriptors[0] != -1)
        {
            /* handle events */
        }
    }
    event_loop_clear(&loop);
    return (0);
}
```

#### `SocketConfig`
```
SocketConfig();
~SocketConfig();
SocketConfig(const SocketConfig& other) noexcept;
SocketConfig(SocketConfig&& other) noexcept;
SocketConfig& operator=(const SocketConfig& other) noexcept;
SocketConfig& operator=(SocketConfig&& other) noexcept;
int  get_error();
const char *get_error_str();
```

#### `ft_socket`
```
ft_socket(const SocketConfig &config);
ft_socket();
~ft_socket();
ft_socket(ft_socket &&other) noexcept;
ft_socket &operator=(ft_socket &&other) noexcept;
int         initialize(const SocketConfig &config);
ssize_t     send_data(const void *data, size_t size, int flags = 0);
ssize_t     send_all(const void *data, size_t size, int flags = 0);
ssize_t     receive_data(void *buffer, size_t size, int flags = 0);
bool        close_socket();
int         get_error() const;
const char *get_error_str() const;
ssize_t     broadcast_data(const void *data, size_t size, int flags);
ssize_t     broadcast_data(const void *data, size_t size, int flags, int exception);
ssize_t     send_data(const void *data, size_t size, int flags, int fd);
bool        disconnect_client(int fd);
void        disconnect_all_clients();
size_t      get_client_count() const;
bool        is_client_connected(int fd) const;
int         get_fd() const;
const struct sockaddr_storage &get_address() const;
int         join_multicast_group(const SocketConfig &config);
```

Calling `send_all` now treats a zero-byte transmission as a peer disconnect and
returns `-1` with the `FT_ERR_SOCKET_SEND_FAILED` error code instead of retrying
indefinitely. This ensures callers can react promptly to closed connections.

The HTTP client exposes `http_client_send_plain_request` and
`http_client_send_ssl_request` helpers to retry partial transmissions and
surface `FT_ERR_SOCKET_SEND_FAILED` through `ft_errno` when the peer stops
accepting data.
The compatibility layer exposes `cmp_socket_send_all`, letting C callers invoke
`ft_socket::send_all` while preserving the same error propagation and return
values as the C++ method.

#### `udp_socket`
```c++
#include "Networking/udp_socket.hpp"

int main()
{
    SocketConfig server_configuration;
    server_configuration._type = SocketType::SERVER;
    server_configuration._protocol = IPPROTO_UDP;
    udp_socket server(server_configuration);

    SocketConfig client_configuration;
    client_configuration._type = SocketType::CLIENT;
    client_configuration._protocol = IPPROTO_UDP;
    udp_socket client(client_configuration);

    struct sockaddr_storage destination = server.get_address();
    const char *message = "hello";
    client.send_to(message, ft_strlen(message), 0,
                   reinterpret_cast<const struct sockaddr*>(&destination),
                   sizeof(struct sockaddr_in));
    char buffer[16];
    socklen_t addr_len = sizeof(destination);
    server.receive_from(buffer, sizeof(buffer) - 1, 0,
                        reinterpret_cast<struct sockaddr*>(&destination),
                        &addr_len);
    return (0);
}
```

#### HTTP client

`Networking/http_client.hpp` adds helpers for basic HTTP requests built on the socket layer.
Both `http_get` and `http_post` attempt connections against every address returned by the
system resolver, closing sockets between attempts until a connection succeeds. The
optional `custom_port` parameter lets callers override the default `80` or `443` values so
tests can bind ephemeral ports without elevated privileges.

```c++
#include "Networking/http_client.hpp"

int main()
{
    ft_string response;

    http_get("example.com", "/", response, true);
    return (0);
}
```

```c++
#include "Networking/http_client.hpp"

int main()
{
    ft_string body;
    ft_string response;

    body.append("name=libft");
    http_post("example.com", "/submit", body, response, false);
    return (0);
}
```

```c++
#include "Networking/http_client.hpp"

int main()
{
    ft_string response;

    http_get("localhost", "/status", response, false, "8080");
    return (0);
}
```

#### HTTP server

`Networking/http_server.hpp` implements a minimal synchronous server that uses
`ft_socket` for IPv4 and IPv6 support, optional non-blocking operation and
configurable timeouts. The server accepts a single connection, buffers data
until the headers and any declared body are fully received, and enforces a
64 KiB request limit to guard against oversized payloads. Basic GET or POST
requests, including POST bodies delivered across multiple TCP receives, are
parsed and echoed back in the response body.

Windows builds translate socket receive failures reported by `run_once` using
`WSAGetLastError()` into the library error namespace before exposing the value
through `get_error`, while POSIX platforms continue to rely on `errno`.

```c++
#include "Networking/http_server.hpp"

int main()
{
    ft_http_server server;

    server.start("127.0.0.1", 8080);
    server.run_once();
    return (0);
}
```

#### WebSocket client and server

`Networking/websocket_client.hpp` and `websocket_server.hpp` add helpers for
the WebSocket protocol including the opening handshake, frame parsing and
basic ping/pong handling. The client now accepts both masked and unmasked
server frames so responses from minimal implementations are decoded without
extra configuration.

```c++
#include "Networking/websocket_client.hpp"

int main()
{
    ft_websocket_client client;
    ft_string message;

    client.connect("example.com", 80, "/chat");
    client.send_text("hello");
    client.receive_text(message);
    client.close();
    return (0);
}
```

```c++
#include "Networking/websocket_server.hpp"

int main()
{
    ft_websocket_server server;
    ft_string message;
    int client_fd;

    server.start("127.0.0.1", 8080);
    server.run_once(client_fd, message);
    return (0);
}
```

### Logger

`Logger/logger.hpp` provides leveled logging with timestamps, formatted output
and optional file rotation. Logs are written to one or more configurable
destinations (sinks) and filtered according to the active log level.
Sink management relies on the thread-safe `ft_vector` container from the
Template module for internal storage.

Colorized terminal output can be toggled with `ft_log_set_color`, and a
predefined `ft_json_sink` helper emits each entry as a JSON object for
downstream processing. A syslog sink writes entries to the host's system
logger and `ft_log_set_remote_sink` forwards messages to a remote server over
UDP or TCP.

Syslog and asynchronous helpers are exposed directly from the single
`logger.hpp` header, removing the need for dedicated includes.

Asynchronous logging is enabled with `ft_log_enable_async(true)` and later
disabled with `ft_log_enable_async(false)` to flush pending messages. This
spawns a background thread that pulls messages from a thread-safe queue,
reducing latency in performance-critical paths at the cost of synchronization
overhead.

```
enum t_log_level {
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_NONE
};

void ft_log_set_level(t_log_level level);
int  ft_log_set_file(const char *path, size_t max_size);
int  ft_log_set_rotation(size_t max_size, size_t retention_count,
                         unsigned int max_age_seconds);
int  ft_log_get_rotation(size_t *max_size, size_t *retention_count,
                         unsigned int *max_age_seconds);
typedef void (*t_log_sink)(const char *message, void *user_data);
int  ft_log_add_sink(t_log_sink sink, void *user_data);
void ft_log_remove_sink(t_log_sink sink, void *user_data);
void ft_log_close();
void ft_log_set_alloc_logging(bool enable);
bool ft_log_get_alloc_logging();
void ft_log_set_color(bool enable);
bool ft_log_get_color();
void ft_json_sink(const char *message, void *user_data);
int  ft_log_set_syslog(const char *identifier);
int  ft_log_set_remote_sink(const char *host, unsigned short port,
                            bool use_tcp);

void ft_log_info(const char *fmt, ...);
void ft_log_warn(const char *fmt, ...);
void ft_log_error(const char *fmt, ...);
void ft_log_debug(const char *fmt, ...);
```

The logger automatically rotates file sinks when they exceed a configured size
or age. Call `ft_log_set_rotation` to update the active file sink thresholds,
retention count, and maximum age, or `ft_log_get_rotation` to query the current
policy.

For RAII-style usage, `ft_logger` wraps the C functions and automatically
closes the log when the object is destroyed:

```
{
    ft_logger log("app.log", 1024 * 1024, LOG_LEVEL_DEBUG);
    log.set_global();
    log.info("starting up");
    log.set_syslog("myapp");
    log.set_remote_sink("203.0.113.5", 9000, true);
}
```

Calling `set_global()` on an `ft_logger` instance makes it the library-wide
logger used internally by logging helpers. The pointer is reset to
`ft_nullptr` when that instance is destroyed. Allocation logging for the
custom memory allocator can be toggled with `set_alloc_logging` and
`get_alloc_logging`.

Additional guidance for the `_alloc_logging` and `_api_logging` toggles is
available in [`Docs/logger_logging_flags.md`](Docs/logger_logging_flags.md),
which explains the debug output each switch enables and when to enable them
during investigations.

### System Utils

`System_utils/system_utils.hpp` provides a simple assertion helper that logs failures using the
global logger before terminating the process. The header also offers direct helpers to abort or
raise common signals and wrappers around environment helpers. Each call locks a global mutex before
touching the process environment. It exposes portable helpers to query CPU count and total physical
memory and to retrieve the current user's home directory.

```
void    su_abort(void);
void    su_sigabrt(void);
void    su_sigfpe(void);
void    su_sigill(void);
void    su_sigint(void);
void    su_sigsegv(void);
void    su_sigterm(void);
void    su_assert(bool condition, const char *message);
char    *su_getenv(const char *name);
int     su_setenv(const char *name, const char *value, int overwrite);
int     su_putenv(char *string);
char    *su_get_home_directory(void);
int     su_open(const char *path_name);
int     su_open(const char *path_name, int flags);
int     su_open(const char *path_name, int flags, mode_t mode);
ssize_t su_read(int file_descriptor, void *buffer, size_t count);
ssize_t su_write(int file_descriptor, const void *buffer, size_t count);
unsigned int    su_get_cpu_count(void);
unsigned long long su_get_total_memory(void);
```

### Template Utilities

`Template/` contains a wide range of generic helpers and containers. Key
components include:

- Containers: `ft_vector`, `ft_deque`, `ft_stack`, `ft_queue`,
  `ft_priority_queue`, `ft_set`, `ft_map`, `ft_unordered_map`,
  `ft_trie`, `ft_circular_buffer`, `ft_graph` and `ft_matrix`.
 - Utility types: `ft_pair`, `ft_tuple`, `ft_optional`, `ft_variant`,
   `ft_bitset`, `ft_function` and `ft_string_view`.
- Smart pointers: `ft_shared_ptr` and `ft_unique_ptr`.
- Concurrency helpers: `ft_thread_pool`, `ft_future`, `ft_event_emitter`,
  `ft_promise`, plus direct use of `std::atomic`.
- `ft_thread_pool` stores workers in `ft_vector` and pending tasks in
  `ft_queue`, synchronizing via POSIX mutexes and condition variables.
- Additional helpers such as `algorithm.hpp`, `iterator.hpp` and `math.hpp`.

Refer to the header files for the full interface of these templates.

#### `ft_trie`

`Template/trie.hpp` implements a basic trie for storing values
associated with string keys.

```
ft_trie();
~ft_trie();
int insert(const char *key, ValueType *value_pointer, int unset_value = 0);
const node_value *search(const char *key) const;
int get_error() const;
```

#### `ft_promise`

`Template/promise.hpp` implements a minimal promise type for passing values
between threads. A promise stores a value set by a worker thread and reports
errors through the thread-local `ft_errno` system.

```
ft_promise();
void set_value(const ValueType& value);
void set_value(ValueType&& value);
ValueType get() const;
bool is_ready() const;
int  get_error() const;
const char *get_error_str() const;
```

#### Atomics

Standard `<atomic>` facilities are used throughout the threading helpers.
`PThread/pthread.hpp` exposes small wrappers such as `pt_atomic_load` and
`pt_atomic_compare_exchange` so the helper APIs continue to integrate with
the project's error handling strategy.

### Additional Modules


#### PThread Scheduler
`PThread/pthread.hpp` provides helpers for the current thread using the
custom `t_thread_id` and `t_duration_milliseconds` wrappers:

```
typedef struct s_thread_id
{
    pt_thread_id_type native_id;
}   t_thread_id;

t_thread_id ft_this_thread_get_id();
void ft_this_thread_sleep_for(t_duration_milliseconds duration);
void ft_this_thread_sleep_until(t_monotonic_time_point time_point);
void ft_this_thread_yield();
```

`PThread/task_scheduler.hpp` offers `ft_task_scheduler`, combining a
lock-free queue, thread pool and scheduler. Tasks may be submitted for
immediate execution, delayed execution or recurring intervals and each
submission returns a future when applicable.

```
ft_task_scheduler(size_t thread_count = 0);
template <typename FunctionType, typename... Args>
auto submit(FunctionType function, Args... args) -> ft_future<ReturnType>;
template <typename Rep, typename Period, typename FunctionType, typename... Args>
auto schedule_after(std::chrono::duration<Rep, Period> delay,
                    FunctionType function, Args... args)
    -> Pair<ft_future<ReturnType>, ft_scheduled_task_handle>;
template <typename Rep, typename Period, typename FunctionType, typename... Args>
ft_scheduled_task_handle schedule_every(std::chrono::duration<Rep, Period> interval,
                    FunctionType function, Args... args);
```

Recurring tasks preserve their callbacks across intervals, preventing
empty function executions when rescheduled.

Both delayed and recurring submissions now surface an `ft_scheduled_task_handle`.
Handles expose `cancel()` so callers can remove future runs from the scheduler’s
heap without racing the timer thread. Cancellation acquires the internal
mutex, removes the pending entry, updates `_error_code` (mirroring `ft_errno`)
and broadcasts the scheduler condition variable so sleeping threads re-check
their wake-up deadlines. Already queued callbacks are allowed to finish, so
callers should assume in-flight work may still run once.

Worker threads live in an `ft_vector<ft_thread>` and task submissions wrap
results in `ft_promise`/`ft_future` pairs so the scheduler exposes the
library's error reporting across thread management and futures instead of
falling back to the standard types.

Worker threads fetch jobs from a lock-free queue so producers and consumers do
not block each other. The scheduler thread manages delayed and recurring jobs.
When the queue reports an allocation failure, the scheduler releases its
internal mutex before running the callback, ensuring that tasks can reschedule
themselves without deadlocking, and then reschedules or removes the entry based
on its interval. `pt_mutex` and the templated `ft_unique_lock` keep the
scheduler on the pthread primitives without depending on `std::unique_lock`,
and callbacks are stored in `ft_function` instances to keep error reporting
within the library's conventions. The `ft_unique_lock` template keeps its
member definitions after the class declaration inside `PThread/unique_lock.hpp`
so including translation units see a consistent layout without relying on
in-class implementations.

#### Errno
`Errno/errno.hpp` defines a thread-local `ft_errno` variable and helpers for retrieving messages.

The [Libft error code registry](Errno/ERROR_CODE_REGISTRY.md) documents every value surfaced through `ft_errno`, the modules that emit them, and guidance for keeping success paths from leaking stale failures.

```
const char *ft_strerror(int err);
void        ft_perror(const char *msg);
void        ft_exit(const char *msg, int code);
```

#### CrossProcess
`CrossProcess/cross_process.hpp` exposes descriptor-driven helpers for sharing memory between producer and consumer processes on both POSIX and Windows.

```cpp
struct cross_process_message {
    uint64_t stack_base_address;
    uint64_t remote_memory_address;
    uint64_t remote_memory_size;
    uint64_t shared_mutex_address;
    uint64_t error_memory_address;
    char     shared_memory_name[256];
};

int cp_send_descriptor(int socket_fd, const cross_process_message &message);
int cp_receive_descriptor(int socket_fd, cross_process_message &message);
int cp_receive_memory(int socket_fd, cross_process_read_result &result);
int cp_write_memory(const cross_process_message &message,
                    const unsigned char *payload,
                    ft_size_t payload_length,
                    int error_code);
```

Descriptors travel over a control socket using `cp_send_descriptor`/`cp_receive_descriptor`, which delegate to the Compatebility layer so EINTR retries, Windows `WSASend` behaviour, and POSIX `write`/`read` loops stay centralized. `cp_receive_memory` maps the named shared memory segment, polls the exported mutex with up to five `trylock` attempts spaced 50 ms apart, copies the payload into an `ft_string`, and zeroes both the payload bytes and optional error-slot so the sender can detect that the data was consumed. `cp_write_memory` follows the same guarded mapping flow to zero-fill the buffer, write a fresh payload, and update the error slot when provided.

The Compatebility helpers wrap `shm_open`/`mmap` and `CreateFileMapping`/`MapViewOfFile`, ensuring every mapping is unmapped and each platform mutex handle is released even when locking or offset validation fails. Shared memory offsets are validated against the advertised size before any copy so mismatched layouts report `EINVAL`/`ERANGE` and leave the remote pages untouched.

#### RNG
Random helpers and containers in `RNG/`. `rng_secure_bytes` obtains
cryptographically secure random data from the operating system,
`rng_secure_bytes_with_fallback` exposes a clear path that retries with
`std::random_device` if the system source becomes unavailable,
`ft_random_uint32` wraps it to produce a single 32-bit value,
`rng_secure_uint64`/`rng_secure_uint32` surface explicit fallbacks for callers
that want integers, and `ft_generate_uuid` formats secure bytes as a version 4
UUID string. Deterministic helpers (`rng_stream_seed*`) use SplitMix64 mixing to
derive reproducible per-stream seeds from a base seed or user-provided string.

```
int   ft_random_int(void);
int   ft_dice_roll(int number, int faces);
float ft_random_float(void);
float ft_random_normal(void);
float ft_random_exponential(float lambda_value);
int   ft_random_poisson(double lambda_value);
int   ft_random_binomial(int trial_count, double success_probability);
int   ft_random_geometric(double success_probability);
uint32_t    ft_random_seed(const char *seed_str = ft_nullptr);
int   rng_secure_bytes(unsigned char *buffer, size_t length);
int   rng_secure_bytes_with_fallback(unsigned char *buffer, size_t length, int *fallback_used);
int   rng_secure_uint64(uint64_t *value, int *fallback_used = ft_nullptr);
int   rng_secure_uint32(uint32_t *value, int *fallback_used = ft_nullptr);
int   rng_stream_seed(uint64_t base_seed, uint64_t stream_identifier, uint64_t *stream_seed);
int   rng_stream_seed_sequence(uint64_t base_seed, uint64_t stream_identifier, uint32_t *buffer, size_t count);
int   rng_stream_seed_from_string(const char *seed_string, uint64_t stream_identifier, uint64_t *stream_seed);
int   rng_stream_seed_sequence_from_string(const char *seed_string, uint64_t stream_identifier, uint32_t *buffer, size_t count);
uint32_t ft_random_uint32(void);
void   ft_generate_uuid(char out[37]);
```

Example:

```
unsigned char buffer[16];
int fallback_used = 0;
if (rng_secure_bytes_with_fallback(buffer, 16, &fallback_used) == 0)
{
    /* use buffer; fallback_used reports whether std::random_device filled it */
}
uint64_t secure_number = 0;
if (rng_secure_uint64(&secure_number) == 0)
{
    /* secure_number now holds 64 random bits */
}
char uuid[37];
ft_generate_uuid(uuid);
uint64_t stream_seed = 0;
if (rng_stream_seed_from_string("simulation", 4, &stream_seed) == 0)
{
    /* stream_seed deterministically identifies stream #4 */
}
int occurrences = ft_random_poisson(4.0);
int successes = ft_random_binomial(10, 0.5);
int attempts = ft_random_geometric(0.25);
```

Secure randomness is required for secrets such as tokens, keys, and
identifiers that must not be predictable. For simulations or gameplay
where predictability is less critical, functions like `ft_random_int`
or `ft_random_float` are sufficient.

`RNG/deck.hpp` provides a simple deck container:

```
ElementType *popRandomElement();
ElementType *getRandomElement() const;
void         shuffle();
ElementType *drawTopElement();
ElementType *peekTopElement() const;
```

#### `ft_loot_table`
```
void addElement(ElementType *elem, int weight, int rarity);
ElementType *getRandomLoot() const;
ElementType *popRandomLoot();
```

Example:

```
ft_loot_table<ft_item> table;
table.addElement(&common_item, 80, 0);
table.addElement(&rare_item, 20, 1);
ft_item *drop = table.getRandomLoot();
```

#### Encryption
`basic_encryption.hpp` exposes minimal helpers and AES block operations:

```
int         be_saveGame(const char *filename, const char *data, const char *key);
char      **be_DecryptData(char **data, const char *key);
const char *be_getEncryptionKey();
void        aes_encrypt(uint8_t *block, const uint8_t *key);
void        aes_decrypt(uint8_t *block, const uint8_t *key);
int         rsa_generate_key_pair(uint64_t *public_key, uint64_t *private_key, uint64_t *modulus, int bit_size);
uint64_t    rsa_encrypt(uint64_t message, uint64_t public_key, uint64_t modulus);
uint64_t    rsa_decrypt(uint64_t cipher, uint64_t private_key, uint64_t modulus);
void        sha256_hash(const void *data, size_t length, unsigned char *digest);
void        hmac_sha256(const unsigned char *key, size_t key_len, const void *data, size_t len, unsigned char *digest);
```

RSA helpers operate on 64-bit integers and are intended for small demonstrations. Key generation with large sizes significantly impacts performance.

`be_getEncryptionKey` sources entropy from `/dev/urandom` on Unix or
`CryptGenRandom` on Windows with a fallback to `ft_random_seed`, ensuring
that generated keys vary across runs.

Example:

```
unsigned char block[16] = {0};
unsigned char key[16] = {0};
aes_encrypt(block, key);
aes_decrypt(block, key);
uint64_t public_key;
uint64_t private_key;
uint64_t modulus;
rsa_generate_key_pair(&public_key, &private_key, &modulus, 32);
uint64_t encrypted = rsa_encrypt(42, public_key, modulus);
uint64_t decrypted = rsa_decrypt(encrypted, private_key, modulus);
unsigned char digest[32];
const char *message = "hello";
sha256_hash(message, 5, digest);
const unsigned char key[] = "secret";
hmac_sha256(key, 6, message, 5, digest);
```

#### Compression
Provides zlib-based compression helpers using the Custom Memory Allocator (CMA).

`compression.hpp` offers buffer compression functions:

```
unsigned char *compress_buffer(const unsigned char *input_buffer, std::size_t input_size, std::size_t *compressed_size);
unsigned char *decompress_buffer(const unsigned char *input_buffer, std::size_t input_size, std::size_t *decompressed_size);
```

The helpers prefix each payload with its uncompressed size stored as an
unsigned 32-bit value. As a result, any request whose buffer exceeds
`compression_max_size` (`UINT32_MAX` bytes) is rejected before reaching zlib and
sets `ft_errno` to `FT_ERR_INVALID_ARGUMENT`.

The returned buffers are allocated with CMA and must be freed using `cma_free`.

High level helpers are also available:

```
unsigned char *ft_compress(const unsigned char *input_buffer, std::size_t input_size, std::size_t *compressed_size);
unsigned char *ft_decompress(const unsigned char *input_buffer, std::size_t input_size, std::size_t *decompressed_size);
int ft_compress_stream(int input_fd, int output_fd);
int ft_decompress_stream(int input_fd, int output_fd);
unsigned char *ft_base64_encode(const unsigned char *input_buffer, std::size_t input_size, std::size_t *encoded_size);
unsigned char *ft_base64_decode(const unsigned char *input_buffer, std::size_t input_size, std::size_t *decoded_size);
```

The streaming functions operate on file descriptors using `su_read` and `su_write`, and the Base64 helpers encode or decode buffers.
`ft_base64_decode` validates every character; when an invalid character is detected it frees any allocated memory, sets the decoded size to `0`, and returns `ft_nullptr` to signal the error.

#### JSon
Creation, reading and manipulation helpers in `JSon/json.hpp`:

```
json_item   *json_create_item(const char *key, const char *value);
json_item   *json_create_item(const char *key, const ft_big_number &value);
json_group  *json_create_json_group(const char *name);
void         json_add_item_to_group(json_group *group, json_item *item);
void         json_item_refresh_numeric_state(json_item *item);
int          json_write_to_file(const char *filename, json_group *groups);
char        *json_write_to_string(json_group *groups);
int          json_document_write_to_file(const char *file_path, const json_document &document);
char        *json_document_write_to_string(const json_document &document);
json_group  *json_read_from_file(const char *filename);
json_group  *json_read_from_string(const char *content);
json_group  *json_read_from_file_stream(FILE *file, size_t buffer_capacity);
json_group  *json_read_from_stream(json_stream_read_callback callback, void *user_data, size_t buffer_capacity);
json_group  *json_find_group(json_group *head, const char *name);
json_item   *json_find_item(json_group *group, const char *key);
void         json_remove_group(json_group **head, const char *name);
void         json_remove_item(json_group *group, const char *key);
void         json_update_item(json_group *group, const char *key, const char *value);
void         json_update_item(json_group *group, const char *key, const int value);
void         json_update_item(json_group *group, const char *key, const bool value);
void         json_update_item(json_group *group, const char *key, const ft_big_number &value);
bool         json_validate_schema(json_group *group, const json_schema &schema);
```
The `json_document` class wraps these helpers and manages a group list:

```
json_document();
~json_document();
void         append_group(json_group *group) noexcept;
json_group   *find_group(const char *name) const noexcept;
```

`json_item` now exposes an `is_big_number` flag and an `ft_big_number *big_number`
pointer so callers can detect when a numeric token overflowed a signed
64-bit range. `json_item_refresh_numeric_state` inspects the item's string
value, frees any previous big-number allocation, and attaches a fresh
`ft_big_number` when the digits exceed the 64-bit limit. Parsing and update
helpers call it automatically, and `json_write_to_file`/`json_write_to_string`
emit such values without quotes so round trips preserve large integers.

For large payloads the streaming helpers avoid building the entire document in
memory. `json_read_from_file_stream` accepts a `FILE *` alongside a
caller-provided buffer capacity (for example, 8 or 16 bytes) and tokenizes the
input incrementally while allocating only the keys and values that become part
of the resulting `json_group` list. When data arrives from sockets or other
chunked transports, `json_read_from_stream` reads from a
`json_stream_read_callback`, allowing callers to pipe received blocks directly
into the parser without concatenation. Choosing a buffer size that matches the
expected I/O chunk (such as the size used by `fread` or a network receive) keeps
memory growth bounded.

`json_document::read_from_file_streaming` wraps the same backend so existing
code that relies on the high-level document class can enable streaming parsing
by passing the desired buffer size. Prefer the streaming entry points when
processing multi-megabyte configuration files or continuous feeds that would be
expensive to duplicate into a single string before parsing.

Schemas describe expected fields and types using a minimal subset of the JSON Schema draft-07 specification:

```
typedef enum json_type
{
    JSON_STRING,
    JSON_NUMBER,
    JSON_BOOL
} json_type;

typedef struct json_schema_field
{
    const char *key;
    json_type   type;
    bool        required;
    json_schema_field *next;
} json_schema_field;

typedef struct json_schema
{
    json_schema_field *fields;
} json_schema;

json_schema_field name_field = { "name", JSON_STRING, true, ft_nullptr };
json_schema_field age_field = { "age", JSON_NUMBER, true, ft_nullptr };
name_field.next = &age_field;
json_schema schema = { &name_field };
bool valid = json_validate_schema(group, schema);
```

#### YAML
Basic YAML handling in `YAML/yaml.hpp` provides minimal parsing and serialization for scalars, lists and maps:

```
yaml_value    *yaml_read_from_string(const ft_string &content);
yaml_value    *yaml_read_from_file(const char *file_path);
ft_string      yaml_write_to_string(const yaml_value *value);
int           yaml_write_to_file(const char *file_path, const yaml_value *value);
void          yaml_free(yaml_value *value);
```

`yaml_value` tracks internal errors from `ft_string`, `ft_vector`, and `ft_map` and exposes `get_error` and `get_error_str` helpers for diagnostics.

The reader implementation is organized across `yaml_value.cpp`, `yaml_reader.cpp`, and helper functions declared directly in `yaml.hpp`.

#### File
Cross-platform file and directory utilities (`File/open_dir.hpp` and `File/file_utils.hpp`):

```
file_dir   *file_opendir(const char *directory_path);
int         file_closedir(file_dir *directory_stream);
file_dirent *file_readdir(file_dir *directory_stream);
int         file_dir_exists(const char *rel_path);
int         file_create_directory(const char *path, mode_t mode);
int         file_copy(const char *source_path, const char *destination_path);
int         file_move(const char *source_path, const char *destination_path);
int         file_exists(const char *path);
int         file_delete(const char *path);
ft_string   file_path_join(const char *path_left, const char *path_right);
ft_string   file_path_normalize(const char *path);
```

The `file_copy` and `file_move` helpers return (-1) on failure to allow error handling. `file_copy` uses `CopyFile` on Windows and `std::filesystem::copy_file` on POSIX systems. `file_move` wraps `MoveFile` or `rename` to provide portable file operations. `file_exists` returns (1) if the file exists and (0) otherwise. `file_delete` wraps `remove` or `unlink` to delete a file and returns (-1) on failure.

`file_dir_exists` follows the same convention as `file_exists`, returning (1) when the directory exists and (0) otherwise.

`file_path_join` combines two path fragments using the platform's separator, removing redundant separators. `file_path_normalize` converts mixed separators to the current platform and collapses duplicates:

```
ft_string joined = file_path_join("dir", "file.txt");
ft_string normalized = file_path_normalize("dir//sub\\file.txt");
```

On POSIX systems these evaluate to `dir/file.txt` and `dir/sub/file.txt` respectively.

`File/file_watch.hpp` offers a simple `ft_file_watch` class for monitoring
directories. It invokes a user supplied callback whenever files are created,
modified or deleted in the watched directory:

```
void on_file_event(const char *name, int event, void *data);

ft_file_watch watcher;
watcher.watch_directory("dir", on_file_event, ft_nullptr);
/* ... */
watcher.stop();
```

The implementation uses inotify on Linux, kqueue on BSD and macOS and
`ReadDirectoryChangesW` on Windows. These platform APIs must be available when
building the library.

`ft_file_watch::stop` closes the active watch handles before joining the worker
thread so the blocking read calls terminate cleanly. The event loop on each
platform now exits when its descriptor or handle is closed, preventing the
watcher from spinning on invalid resources during shutdown.

On Linux, `ft_file_watch::watch_directory` now closes the inotify descriptor if
`inotify_add_watch` fails so the setup path does not leak file descriptors.

`System_utils/system_utils.hpp` provides cross-platform file descriptor utilities:

```
void    cmp_initialize_standard_file_descriptors();
int     su_open(const char *pathname);
int     su_open(const char *pathname, int flags);
int     su_open(const char *pathname, int flags, mode_t mode);
ssize_t su_read(int fd, void *buf, size_t count);
ssize_t su_write(int fd, const void *buf, size_t count);
int     su_close(int fd);
```
A simple `su_file` struct wraps a file descriptor for stream-style I/O:

```
typedef struct su_file
{
    int _descriptor;
} su_file;

su_file *su_fopen(const char *path_name);
su_file *su_fopen(const char *path_name, int flags);
su_file *su_fopen(const char *path_name, int flags, mode_t mode);
int     su_fclose(su_file *stream);
size_t  su_fread(void *buffer, size_t size, size_t count, su_file *stream);
size_t  su_fwrite(const void *buffer, size_t size, size_t count, su_file *stream);
int     su_fseek(su_file *stream, long offset, int origin);
long    su_ftell(su_file *stream);
```
`System_utils_file_open.cpp`, `System_utils_file_io.cpp` and
`System_utils_file_stream.cpp` contain the implementations, keeping each source
file focused and small.
#### Storage
`Storage/kv_store.hpp` offers a lightweight JSON-backed key-value database:

```
int kv_set(const char *key_string, const char *value_string);
const char *kv_get(const char *key_string) const;
int kv_delete(const char *key_string);
int kv_flush() const;
```

Create an instance with the path to a JSON file. Values are kept in memory
until `kv_flush` writes them to disk. An optional AES-128 key enables
transparent encryption:

```
kv_store store("data.json");
store.kv_set("theme", "dark");
const char *theme = store.kv_get("theme");
store.kv_delete("unused");
store.kv_flush();
```

``` 
kv_store encrypted("secrets.json", "sixteen-byte-key", true);
encrypted.kv_set("token", "value");
encrypted.kv_flush();
```

Use `configure_encryption` to toggle or update the key at runtime:

```
store.configure_encryption("sixteen-byte-key", true);
store.kv_flush();
```

Errors set the thread-local `ft_errno` and are accessible through `get_error`
and `get_error_str`.

When encryption is enabled values are padded, AES-128 encrypted, Base64
encoded, and stored alongside a `__encryption__` metadata flag. The key must
be exactly 16 bytes. Loading encrypted data without a key, with a wrong key,
or with unsupported metadata fails with `FT_ERR_INVALID_ARGUMENT`. Existing plaintext files
can be encrypted by configuring a key and calling `kv_flush`, but mixing
encrypted and plaintext entries in the same file is not supported.

#### Config
`Config/config.hpp` parses simple configuration files:

```
cnfg_config *cnfg_parse(const char *filename);
char       *cnfg_parse_flags(int argument_count, char **argument_values);
void       cnfg_free(cnfg_config *config);
cnfg_config *config_load_env();
cnfg_config *config_load_file(const char *filename);
cnfg_config *config_merge_sources(int argument_count,
                                  char **argument_values,
                                  const char *filename);
int         config_write_file(const cnfg_config *config, const char *filename);
cnfg_config *config_merge(const cnfg_config *base_config,
                          const cnfg_config *override_config);
```

`cnfg_parse` gives precedence to environment variables. Before using a
value from the file, the parser checks `getenv` with the key name and
uses the environment value if it exists.

`flag_parser.hpp` wraps flag parsing in a class and `config_merge_sources`
combines command-line flags with environment variables and configuration
files. `config_write_file` serializes parsed data back to disk as INI or JSON
depending on the filename extension, and `config_merge` produces a new
configuration by layering an override set on top of a base:

```
cnfg_flag_parser parser(argument_count, argument_values);
parser.has_short_flag('a');
parser.has_long_flag("help");
parser.get_short_flag_count();
parser.get_long_flag_count();
parser.get_total_flag_count();
parser.get_error();
parser.get_error_str();
cnfg_config *config = config_merge_sources(argument_count, argument_values, "config.ini");
config_write_file(config, "config.json");
cnfg_config *combined = config_merge(default_config, override_config);
```

#### Time
`Time/time.hpp` exposes simple time helpers:

```
t_time  time_now(void);
long    time_now_ms(void);
long    time_monotonic(void);
t_monotonic_time_point   time_monotonic_point_now(void);
t_monotonic_time_point   time_monotonic_point_add_ms(t_monotonic_time_point time_point, long long milliseconds);
long long   time_monotonic_point_diff_ms(t_monotonic_time_point start_point, t_monotonic_time_point end_point);
int     time_monotonic_point_compare(t_monotonic_time_point first_point, t_monotonic_time_point second_point);
t_duration_milliseconds  time_duration_ms_create(long long milliseconds);
void    time_local(t_time time_value, t_time_info *out);
void    time_sleep(unsigned int seconds);
void    time_sleep_ms(unsigned int milliseconds);
size_t  time_strftime(char *buffer, size_t size, const char *format, const t_time_info *time_info);
ft_string    time_format_iso8601(t_time time_value);
bool    time_parse_iso8601(const char *string_input, std::tm *time_output, t_time *timestamp_output);
bool    time_parse_custom(const char *string_input, const char *format, std::tm *time_output, t_time *timestamp_output);
```

`t_time` stores seconds since the Unix epoch, `t_monotonic_time_point` wraps a monotonic millisecond counter derived from `std::chrono::steady_clock`, `t_duration_milliseconds` represents millisecond durations without pulling in `<chrono>`, and `t_time_info` holds the broken-down components. `time_monotonic` now forwards that millisecond counter through `static_cast<long>` so the helper satisfies the project's `-Wold-style-cast` policy without changing its return value semantics.

Example:

```
ft_string timestamp = time_format_iso8601(0);
// timestamp == "1970-01-01T00:00:00Z"

t_time parsed_epoch;
if (time_parse_iso8601("1970-01-01T00:00:00Z", NULL, &parsed_epoch))
{
    // parsed_epoch == 0
}

std::tm time_data;
time_parse_custom("1970/01/01", "%Y/%m/%d", &time_data, NULL);
```

`time_parse_iso8601` relies on the `cmp_timegm` compatibility helper, which wraps
`timegm` on POSIX platforms and `_mkgmtime` on Windows so `'Z'` timestamps are
translated using UTC even when the current timezone has a non-zero offset.

`timer.hpp` defines a small timer class:

```
time_timer();
~time_timer();
void    start(long duration_ms);
long    update();
long    add_time(long amount_ms);
long    remove_time(long amount_ms);
void    sleep_remaining();
```

`fps.hpp` provides a frame rate limiter with an adjustable target frame rate (minimum 24 FPS):

```
time_fps(long frames_per_second);
~time_fps();
long    get_frames_per_second();
int     set_frames_per_second(long frames_per_second);
void    sleep_to_next_frame();
int     get_error() const;
const char  *get_error_str() const;
```

#### ReadLine
`ReadLine/readline.hpp` implements a tiny interactive line reader:

```
void rl_clear_history();
void rl_add_suggestion(const char *word);
void rl_clear_suggestions();
char *rl_readline(const char *prompt);
```

Helper failures now short-circuit the input loop so `rl_readline` immediately
cleans up its buffer, restores terminal state, and returns `ft_nullptr`. The
regression exercise in `Test/Test/test_readline.cpp` simulates a backspace
error to ensure no code touches the released buffer after `rl_error` runs.

Refer to [`Docs/readline_terminal_capabilities.md`](Docs/readline_terminal_capabilities.md)
for details on how the module detects terminal capabilities, the error codes
returned by the Compatebility shims, and the fallbacks that keep ReadLine
operational in minimal environments.

#### API
HTTP client helpers in `API/api.hpp` and asynchronous wrappers. URL parsing
relies on the `ft_string` class instead of `std::string`, so any allocation
errors must be checked via `get_error`. The implementation is split across
multiple source files like `api_request.cpp` and `api_request_async.cpp` for
maintainability.

All HTTP helpers reuse connections through an internal pool keyed by host,
port, and transport security. Successful requests return sockets to the pool so
subsequent calls avoid a new TCP or TLS handshake, while protocol or I/O
failures automatically evict the underlying connection. Idle sockets are
pruned according to the configured timeout, and error states are surfaced
through `ft_socket::get_error()` so callers can detect failures without losing
context.

```
char       *api_request_string(const char *ip, uint16_t port,
                               const char *method, const char *path,
                               json_group *payload = ft_nullptr,
                               const char *headers = ft_nullptr, int *status = ft_nullptr,
                               int timeout = 60000);
char       *api_request_https(const char *ip, uint16_t port,
                               const char *method, const char *path,
                               json_group *payload = ft_nullptr,
                               const char *headers = ft_nullptr, int *status = ft_nullptr,
                               int timeout = 60000, const char *ca_certificate = ft_nullptr,
                               bool verify_peer = true);
char       *api_request_string_host(const char *host, uint16_t port,
                                    const char *method, const char *path,
                                    json_group *payload = ft_nullptr,
                                    const char *headers = ft_nullptr, int *status = ft_nullptr,
                                    int timeout = 60000);
json_group *api_request_json(const char *ip, uint16_t port,
                             const char *method, const char *path,
                             json_group *payload = ft_nullptr,
                             const char *headers = ft_nullptr, int *status = ft_nullptr,
                             int timeout = 60000);
json_group *api_request_json_host(const char *host, uint16_t port,
                                  const char *method, const char *path,
                                  json_group *payload = ft_nullptr,
                                  const char *headers = ft_nullptr, int *status = ft_nullptr,
                                  int timeout = 60000);
```

Example HTTPS requests:

```
char *secure_body = api_request_https("93.184.216.34", 443, "GET", "/",
        ft_nullptr, ft_nullptr, ft_nullptr, 60000,
        "/etc/ssl/certs/ca.pem", true);
cma_free(secure_body);

char *insecure_body = api_request_https("127.0.0.1", 8443, "GET", "/",
        ft_nullptr, ft_nullptr, ft_nullptr, 60000,
        ft_nullptr, false);
cma_free(insecure_body);
```

Callback based helpers run the request on a background thread and invoke
the user supplied callback with the body and status code. For string
responses the body is allocated with `cma_malloc` and must be freed by the
caller. For JSON responses the parsed `json_group*` must be freed by the
caller.

```
typedef void (*api_callback)(char *body, int status, void *user_data);
typedef void (*api_json_callback)(json_group *body, int status, void *user_data);

bool api_request_string_async(const char *ip, uint16_t port,
                              const char *method, const char *path,
                              api_callback callback, void *user_data,
                              json_group *payload = ft_nullptr,
                              const char *headers = ft_nullptr, int timeout = 60000);
bool api_request_string_tls_async(const char *host, uint16_t port,
                                  const char *method, const char *path,
                                  api_callback callback, void *user_data,
                                  json_group *payload = ft_nullptr,
                                  const char *headers = ft_nullptr, int timeout = 60000);
bool api_request_json_async(const char *ip, uint16_t port,
                            const char *method, const char *path,
                            api_json_callback callback, void *user_data,
                            json_group *payload = ft_nullptr,
                            const char *headers = ft_nullptr, int timeout = 60000);
bool api_request_json_tls_async(const char *host, uint16_t port,
                                const char *method, const char *path,
                                api_json_callback callback, void *user_data,
                                json_group *payload = ft_nullptr,
                                const char *headers = ft_nullptr, int timeout = 60000);
```

Connection pooling can be tuned or flushed via the following helpers:

```
void        api_connection_pool_flush();
void        api_connection_pool_set_max_idle(size_t max_idle);
size_t      api_connection_pool_get_max_idle();
void        api_connection_pool_set_idle_timeout(long long idle_timeout_ms);
long long   api_connection_pool_get_idle_timeout();
```

Reducing the idle limit forces older connections to close once the pool reaches
capacity, while lowering the timeout frees sockets that have been inactive for
too long. Call `api_connection_pool_flush` to close every cached connection—for
example before application shutdown or when rotating credentials.

Asynchronous helpers in `promise.hpp` return `ft_promise` objects:

```
class api_promise : public ft_promise<json_group*>;
bool request(const char *ip, uint16_t port,
             const char *method, const char *path,
             json_group *payload = ft_nullptr,
             const char *headers = ft_nullptr, int *status = ft_nullptr,
             int timeout = 60000);
class api_string_promise : public ft_promise<char*>;
bool request(const char *ip, uint16_t port,
             const char *method, const char *path,
             json_group *payload = ft_nullptr,
             const char *headers = ft_nullptr, int *status = ft_nullptr,
             int timeout = 60000);
class api_tls_promise : public ft_promise<json_group*>;
bool request(const char *host, uint16_t port,
             const char *method, const char *path,
             json_group *payload = ft_nullptr,
             const char *headers = ft_nullptr, int *status = ft_nullptr,
             int timeout = 60000);
class api_tls_string_promise : public ft_promise<char*>;
bool request(const char *host, uint16_t port,
             const char *method, const char *path,
             json_group *payload = ft_nullptr,
             const char *headers = ft_nullptr, int *status = ft_nullptr,
             int timeout = 60000);
```

#### `api_tls_client`
```
api_tls_client(const char *host, uint16_t port, int timeout = 60000);
~api_tls_client();
bool is_valid() const;
char *request(const char *method, const char *path, json_group *payload = ft_nullptr,
              const char *headers = ft_nullptr, int *status = ft_nullptr);
json_group *request_json(const char *method, const char *path,
                         json_group *payload = ft_nullptr,
                         const char *headers = ft_nullptr, int *status = ft_nullptr);

bool request_async(const char *method, const char *path,
                   json_group *payload = ft_nullptr,
                   const char *headers = ft_nullptr,
                   api_callback callback = ft_nullptr,
                   void *user_data = ft_nullptr);
```

#### HTML
Minimal node creation and searching utilities (`HTML/parser.hpp`):

```
html_node *html_create_node(const char *tagName, const char *textContent);
void       html_add_child(html_node *parentNode, html_node *childNode);
void       html_add_attr(html_node *targetNode, html_attr *newAttribute);
html_node *html_find_by_tag(html_node *nodeList, const char *tagName);
html_node *html_find_by_attr(html_node *nodeList, const char *key, const char *value);
html_node *html_find_by_selector(html_node *nodeList, const char *selector);
html_node *html_query_selector(html_node *nodeList, const char *selector);
```

The `html_document` class wraps these helpers and manages a root node list:

```
html_document();
~html_document();
void        append_node(html_node *new_node) noexcept;
html_node   *find_by_tag(const char *tag_name) const noexcept;
html_node   *find_by_selector(const char *selector) const noexcept;
```

Simple selectors allow searching by:

```
"tag"             - tag name
"#id"             - attribute id equals value
".class"          - attribute class equals value
"[key=value]"     - attribute key equals value
"[key]"           - presence of attribute key
```

Example:

```
html_node *highlight = html_query_selector(root, ".highlight");
```

`html_query_selector` supports only tag names, `.class`, and `#id` selectors.
Combinators like descendant or child selectors are not implemented.

#### XML
Basic XML parsing and writing utilities (`XML/xml.hpp`):

```
xml_document doc;
doc.load_from_string("<root><child>value</child></root>");
xml_node *root = doc.get_root();
char *xml = doc.write_to_string();
if (xml)
    cma_free(xml);
```

`xml_document` exposes methods:

```
xml_document() noexcept;
~xml_document() noexcept;
int load_from_string(const char *xml) noexcept;
int load_from_file(const char *file_path) noexcept;
char *write_to_string() const noexcept;
int write_to_file(const char *file_path) const noexcept;
xml_node *get_root() const noexcept;
```

#### Game
The Game module provides small building blocks for RPG-style mechanics. It includes world persistence, event queues, pathfinding helpers, equipment management, and crafting.

Equipment slots and quest rewards manage their items through `ft_sharedptr` for automatic cleanup.

Game headers are prefixed with `game_` to align with their source filenames.

All core classes define explicit copy and move constructors and assignments to manage resources safely.

Core classes include `ft_character`, `ft_item`, `ft_inventory`, `ft_equipment`, `ft_upgrade`, `ft_world`, `ft_game_state`, `ft_event`, `ft_event_scheduler`, `ft_map3d`, `ft_quest`, `ft_reputation`, `ft_buff`, `ft_debuff`, `ft_skill`, `ft_achievement`, `ft_experience_table`, and `ft_crafting`. Each class is summarized below. The `ft_character` implementation is divided across dedicated source files for constructors, accessors, mutation helpers, save/load logic, and other behavior.

The `ft_world` class can persist game state using JSON files and track timed events through a shared-pointer-owned scheduler.
Methods interacting with the scheduler verify the shared pointer and the scheduler itself for errors before proceeding.

```
ft_sharedptr<ft_world> world(new ft_world());
ft_character character;
ft_inventory inventory;
world->save_to_file("save.json", character, inventory);
world->load_from_file("save.json", character, inventory);
```

Timed events are scheduled through a priority queue of shared pointers.

```
ft_sharedptr<ft_event> spawn(new ft_event());
spawn->set_id(1);
spawn->set_duration(3);
world->schedule_event(spawn);
world->update_events(world, 1);
```

`ft_world::update_events` decrements durations and removes expired entries. `ft_world::plan_route` exposes grid pathfinding through the world object. Logging to a file or buffer is optional:

```
ft_string log_buffer;
world->update_events(world, 1, "combat.log", &log_buffer);
```

Events can be cancelled or rescheduled by identifier:

```
ft_sharedptr<ft_event_scheduler> scheduler = world->get_event_scheduler();
scheduler->reschedule_event(1, 5);
scheduler->cancel_event(1);
```

Both helpers search the scheduler's queue and set `FT_ERR_GAME_GENERAL_ERROR` if the event is not found.

Queued events can be saved and reloaded through `serialize_event_scheduler` and
`deserialize_event_scheduler`, which `ft_world::save_to_file` and
`ft_world::load_from_file` invoke to persist pending actions.
Callbacks attached to events are executed when their durations expire and are
reconstructed on load using the event's type identifier.

#### Game Server
`ft_game_server` exposes a small WebSocket endpoint that forwards client
messages into the world's event queue and broadcasts the serialized world to
all connected clients. A server is created with a shared pointer to an
`ft_world` instance and can optionally validate a shared token:

```
ft_sharedptr<ft_world> world(new ft_world());
ft_game_server server(world, "secret");
server.start("0.0.0.0", 8080);
server.run_once();
```

Clients join with a JSON message that includes their identifier and optional
token, and may later leave with a separate message:

```
{ "join": { "id": 1, "token": "secret" } }
{ "leave": { "id": 1 } }
```

Event messages schedule actions in the world. After each update the server
broadcasts the current state to every connected client:

```
{ "event": { "id": 1, "duration": 3 } }
```

Both helpers use the JSon module to read and write the `world`, `character`, `inventory`, and `equipment` groups. Skills stored on a character are serialized alongside these fields and restored on load.

Characters manage gear through an `ft_equipment` container. Slots such as head, chest, and weapon can be equipped or unequipped and the appropriate stat modifiers are applied automatically.

```
ft_item sword;
sword.set_modifier1_id(2);
sword.set_modifier1_value(5);
hero.equip_item(EQUIP_WEAPON, sword);
hero.unequip_item(EQUIP_WEAPON);
```

`ft_equipment` applies and removes modifiers as items are equipped or unequipped.

##### Pathfinding

`ft_pathfinding` computes routes on grids and graphs using an A* search where nonzero grid values block movement.

```
ft_map3d grid(3, 3, 1, 0);
ft_pathfinding finder;
ft_vector<ft_path_step> path;
finder.astar_grid(grid, 0, 0, 0, 2, 2, 0, path);
```

`ft_world::plan_route` wraps grid pathfinding for convenience.

```
ft_sharedptr<ft_world> world(new ft_world());
world->plan_route(grid, 0, 0, 0, 2, 2, 0, path);
```

Obstacles may change at runtime. `ft_map3d::toggle_obstacle` switches a cell
between free and blocked and notifies a listener. `ft_pathfinding` listens for
these updates with `update_obstacle` and re-plans the route on demand using
`recalculate_path`:

```
ft_map3d grid(3, 3, 1, 0);
ft_pathfinding finder;
ft_vector<ft_path_step> path;
finder.recalculate_path(grid, 0, 0, 0, 2, 2, 0, path);
grid.toggle_obstacle(1, 0, 0, &finder);
finder.recalculate_path(grid, 0, 0, 0, 2, 2, 0, path);
```

##### Crafting

`ft_crafting` stores crafting recipes and converts ingredients into new items. `craft_item` verifies ingredients, removes them from the inventory, and adds the crafted result through shared pointers.

```
ft_crafting crafting;
ft_vector<ft_crafting_ingredient> ingredients;
ft_crafting_ingredient ingredient_a(1, 2, -1);
ft_crafting_ingredient ingredient_b(2, 1, 1);
ingredients.push_back(ingredient_a);
ingredients.push_back(ingredient_b);
crafting.register_recipe(1, ft_move(ingredients));
ft_sharedptr<ft_item> sword(new ft_item());
crafting.craft_item(inventory, 1, sword);
```

#### `ft_game_state`

```
ft_vector<ft_sharedptr<ft_world> > &get_worlds() noexcept;
ft_vector<ft_sharedptr<ft_character> > &get_characters() noexcept;
int add_character(const ft_sharedptr<ft_character> &character) noexcept;
void remove_character(size_t index) noexcept;
int get_error() const noexcept;
const char *get_error_str() const noexcept;
```

#### `ft_character`
Use `FT_DAMAGE_PHYSICAL` or `FT_DAMAGE_MAGICAL` to tag incoming damage and
select the handling method with `FT_DAMAGE_RULE_FLAT`,
`FT_DAMAGE_RULE_SCALED`, or `FT_DAMAGE_RULE_BUFFER`.
```
int get_hit_points() const noexcept;
void set_hit_points(int hp) noexcept;
bool is_alive() const noexcept;
int get_physical_armor() const noexcept;
void set_physical_armor(int armor) noexcept;
int get_magic_armor() const noexcept;
void set_magic_armor(int armor) noexcept;
int get_current_physical_armor() const noexcept;
void set_current_physical_armor(int armor) noexcept;
int get_current_magic_armor() const noexcept;
void set_current_magic_armor(int armor) noexcept;
void restore_physical_armor() noexcept;
void restore_magic_armor() noexcept;
void restore_armor() noexcept;
void set_damage_rule(uint8_t rule) noexcept;
uint8_t get_damage_rule() const noexcept;
void take_damage(long long damage, uint8_t type) noexcept;
void take_damage_flat(long long damage, uint8_t type) noexcept;
void take_damage_scaled(long long damage, uint8_t type) noexcept;
void take_damage_buffer(long long damage, uint8_t type) noexcept;
int get_might() const noexcept;
void set_might(int might) noexcept;
int get_agility() const noexcept;
void set_agility(int agility) noexcept;
int get_endurance() const noexcept;
void set_endurance(int endurance) noexcept;
int get_reason() const noexcept;
void set_reason(int reason) noexcept;
int get_insigh() const noexcept;
void set_insigh(int insigh) noexcept;
int get_presence() const noexcept;
void set_presence(int presence) noexcept;
int get_coins() const noexcept;
void set_coins(int coins) noexcept;
void add_coins(int coins) noexcept;
void sub_coins(int coins) noexcept;
int get_valor() const noexcept;
void set_valor(int valor) noexcept;
void add_valor(int valor) noexcept;
void sub_valor(int valor) noexcept;
int get_experience() const noexcept;
void set_experience(int experience) noexcept;
void add_experience(int experience) noexcept;
void sub_experience(int experience) noexcept;
int get_x() const noexcept;
void set_x(int x) noexcept;
int get_y() const noexcept;
void set_y(int y) noexcept;
int get_z() const noexcept;
void set_z(int z) noexcept;
void move(int dx, int dy, int dz) noexcept;
ft_resistance get_fire_res() const noexcept;
void set_fire_res(int percent, int flat) noexcept;
ft_resistance get_frost_res() const noexcept;
void set_frost_res(int percent, int flat) noexcept;
ft_resistance get_lightning_res() const noexcept;
void set_lightning_res(int percent, int flat) noexcept;
ft_resistance get_air_res() const noexcept;
void set_air_res(int percent, int flat) noexcept;
ft_resistance get_earth_res() const noexcept;
void set_earth_res(int percent, int flat) noexcept;
ft_resistance get_chaos_res() const noexcept;
void set_chaos_res(int percent, int flat) noexcept;
ft_resistance get_physical_res() const noexcept;
void set_physical_res(int percent, int flat) noexcept;
ft_map<int, ft_skill>       &get_skills() noexcept;
const ft_map<int, ft_skill> &get_skills() const noexcept;
ft_skill *get_skill(int id) noexcept;
const ft_skill *get_skill(int id) const noexcept;
int add_skill(const ft_skill &skill) noexcept;
void remove_skill(int id) noexcept;
ft_map<int, ft_buff>       &get_buffs() noexcept;
const ft_map<int, ft_buff> &get_buffs() const noexcept;
ft_map<int, ft_debuff>       &get_debuffs() noexcept;
const ft_map<int, ft_debuff> &get_debuffs() const noexcept;
ft_map<int, ft_upgrade>       &get_upgrades() noexcept;
const ft_map<int, ft_upgrade> &get_upgrades() const noexcept;
ft_map<int, ft_quest>       &get_quests() noexcept;
const ft_map<int, ft_quest> &get_quests() const noexcept;
ft_map<int, ft_achievement>       &get_achievements() noexcept;
const ft_map<int, ft_achievement> &get_achievements() const noexcept;
ft_reputation       &get_reputation() noexcept;
const ft_reputation &get_reputation() const noexcept;
ft_experience_table       &get_experience_table() noexcept;
const ft_experience_table &get_experience_table() const noexcept;
int equip_item(int slot, const ft_item &item) noexcept;
void unequip_item(int slot) noexcept;
ft_sharedptr<ft_item> get_equipped_item(int slot) noexcept;
ft_sharedptr<ft_item> get_equipped_item(int slot) const noexcept;
int get_level() const noexcept;
int get_error() const noexcept;
const char *get_error_str() const noexcept;
```

#### `ft_item`
```
struct ft_item_modifier
{
    int id;
    int value;
};

int get_max_stack() const noexcept;
void set_max_stack(int max) noexcept;
int get_stack_size() const noexcept;
void set_stack_size(int amount) noexcept;
void add_to_stack(int amount) noexcept;
void sub_from_stack(int amount) noexcept;
int get_item_id() const noexcept;
void set_item_id(int id) noexcept;
int get_width() const noexcept;
void set_width(int width) noexcept;
int get_height() const noexcept;
void set_height(int height) noexcept;
int get_rarity() const noexcept;
void set_rarity(int rarity) noexcept;
ft_item_modifier get_modifier1() const noexcept;
void set_modifier1(const ft_item_modifier &mod) noexcept;
int get_modifier1_id() const noexcept;
void set_modifier1_id(int id) noexcept;
int get_modifier1_value() const noexcept;
void set_modifier1_value(int value) noexcept;
ft_item_modifier get_modifier2() const noexcept;
void set_modifier2(const ft_item_modifier &mod) noexcept;
int get_modifier2_id() const noexcept;
void set_modifier2_id(int id) noexcept;
int get_modifier2_value() const noexcept;
void set_modifier2_value(int value) noexcept;
ft_item_modifier get_modifier3() const noexcept;
void set_modifier3(const ft_item_modifier &mod) noexcept;
int get_modifier3_id() const noexcept;
void set_modifier3_id(int id) noexcept;
int get_modifier3_value() const noexcept;
void set_modifier3_value(int value) noexcept;
ft_item_modifier get_modifier4() const noexcept;
void set_modifier4(const ft_item_modifier &mod) noexcept;
int get_modifier4_id() const noexcept;
void set_modifier4_id(int id) noexcept;
int get_modifier4_value() const noexcept;
void set_modifier4_value(int value) noexcept;
```

#### `ft_inventory`
```
ft_map<int, ft_sharedptr<ft_item> >       &get_items() noexcept;
const ft_map<int, ft_sharedptr<ft_item> > &get_items() const noexcept;
size_t get_capacity() const noexcept;
void   resize(size_t capacity) noexcept;
size_t get_used() const noexcept;
void   set_used_slots(size_t used) noexcept;
bool   is_full() const noexcept;
int    get_weight_limit() const noexcept;
void   set_weight_limit(int limit) noexcept;
int    get_current_weight() const noexcept;
void   set_current_weight(int weight) noexcept;
int get_error() const noexcept;
const char *get_error_str() const noexcept;
int  add_item(const ft_sharedptr<ft_item> &item) noexcept;
void remove_item(int slot) noexcept;
int  count_item(int item_id) const noexcept;
bool has_item(int item_id) const noexcept;
int  count_rarity(int rarity) const noexcept;
bool has_rarity(int rarity) const noexcept;
```

Inventory behavior can be toggled with macros defined at the top of
`Game/game_inventory.hpp`.
Define `USE_INVENTORY_WEIGHT` or `USE_INVENTORY_SLOTS` to enable weight
limits or slot limits as desired.

#### `ft_upgrade`
```
int get_id() const noexcept;
void set_id(int id) noexcept;
uint16_t get_current_level() const noexcept;
void set_current_level(uint16_t level) noexcept;
void add_level(uint16_t level) noexcept;
void sub_level(uint16_t level) noexcept;
uint16_t get_max_level() const noexcept;
void set_max_level(uint16_t level) noexcept;
int get_modifier1() const noexcept;
void set_modifier1(int mod) noexcept;
void add_modifier1(int mod) noexcept;
void sub_modifier1(int mod) noexcept;
int get_modifier2() const noexcept;
void set_modifier2(int mod) noexcept;
void add_modifier2(int mod) noexcept;
void sub_modifier2(int mod) noexcept;
int get_modifier3() const noexcept;
void set_modifier3(int mod) noexcept;
void add_modifier3(int mod) noexcept;
void sub_modifier3(int mod) noexcept;
int get_modifier4() const noexcept;
void set_modifier4(int mod) noexcept;
void add_modifier4(int mod) noexcept;
void sub_modifier4(int mod) noexcept;
const ft_function<void(ft_world&, ft_event&)> &get_callback() const noexcept;
void set_callback(ft_function<void(ft_world&, ft_event&)> &&callback) noexcept;
```

#### `ft_world`
```
ft_map<int, ft_event>       &get_events() noexcept;
const ft_map<int, ft_event> &get_events() const noexcept;
void process_events(int ticks) noexcept;
int save_to_file(const char *file_path, const ft_character &character, const ft_inventory &inventory) const noexcept;
int load_from_file(const char *file_path, ft_character &character, ft_inventory &inventory) noexcept;
int plan_route(const ft_map3d &grid, size_t start_x, size_t start_y, size_t start_z, size_t goal_x, size_t goal_y, size_t goal_z, ft_vector<ft_path_step> &path) const noexcept;
int get_error() const noexcept;
const char *get_error_str() const noexcept;
```

#### `ft_event`
```
int get_id() const noexcept;
void set_id(int id) noexcept;
int get_duration() const noexcept;
void set_duration(int duration) noexcept;
void add_duration(int duration) noexcept;
void sub_duration(int duration) noexcept;
int get_modifier1() const noexcept;
void set_modifier1(int mod) noexcept;
void add_modifier1(int mod) noexcept;
void sub_modifier1(int mod) noexcept;
int get_modifier2() const noexcept;
void set_modifier2(int mod) noexcept;
void add_modifier2(int mod) noexcept;
void sub_modifier2(int mod) noexcept;
int get_modifier3() const noexcept;
void set_modifier3(int mod) noexcept;
void add_modifier3(int mod) noexcept;
void sub_modifier3(int mod) noexcept;
int get_modifier4() const noexcept;
void set_modifier4(int mod) noexcept;
void add_modifier4(int mod) noexcept;
void sub_modifier4(int mod) noexcept;
```

#### `ft_map3d`
```
ft_map3d(size_t width = 0, size_t height = 0, size_t depth = 0, int value = 0);
~ft_map3d();
void    resize(size_t width, size_t height, size_t depth, int value = 0);
int     get(size_t x, size_t y, size_t z) const;
void    set(size_t x, size_t y, size_t z, int value);
size_t  get_width() const;
size_t  get_height() const;
size_t  get_depth() const;
int     get_error() const;
const char *get_error_str() const;
```

#### `ft_pathfinding`
```
ft_pathfinding();
int astar_grid(const ft_map3d &grid, size_t start_x, size_t start_y, size_t start_z, size_t goal_x, size_t goal_y, size_t goal_z, ft_vector<ft_path_step> &out_path) const noexcept;
int dijkstra_graph(const ft_graph<int> &graph, size_t start_vertex, size_t goal_vertex, ft_vector<size_t> &out_path) const noexcept;
int get_error() const noexcept;
const char *get_error_str() const noexcept;
```
Included via `FullLibft.hpp`.

#### `ft_quest`
```
int get_id() const noexcept;
void set_id(int id) noexcept;
int get_phases() const noexcept;
void set_phases(int phases) noexcept;
int get_current_phase() const noexcept;
void set_current_phase(int phase) noexcept;
const ft_string &get_description() const noexcept;
void set_description(const ft_string &description) noexcept;
const ft_string &get_objective() const noexcept;
void set_objective(const ft_string &objective) noexcept;
int get_reward_experience() const noexcept;
void set_reward_experience(int experience) noexcept;
ft_vector<ft_item>       &get_reward_items() noexcept;
const ft_vector<ft_item> &get_reward_items() const noexcept;
void set_reward_items(const ft_vector<ft_item> &items) noexcept;
bool is_complete() const noexcept;
void advance_phase() noexcept;
```

#### `ft_reputation`
```
int get_total_rep() const noexcept;
void set_total_rep(int rep) noexcept;
void add_total_rep(int rep) noexcept;
void sub_total_rep(int rep) noexcept;
int get_current_rep() const noexcept;
void set_current_rep(int rep) noexcept;
void add_current_rep(int rep) noexcept;
void sub_current_rep(int rep) noexcept;
ft_map<int, int>       &get_milestones() noexcept;
const ft_map<int, int> &get_milestones() const noexcept;
void set_milestones(const ft_map<int, int> &milestones) noexcept;
int get_milestone(int id) const noexcept;
void set_milestone(int id, int value) noexcept;
ft_map<int, int>       &get_reps() noexcept;
const ft_map<int, int> &get_reps() const noexcept;
void set_reps(const ft_map<int, int> &reps) noexcept;
int get_rep(int id) const noexcept;
void set_rep(int id, int value) noexcept;
int get_error() const noexcept;
const char *get_error_str() const noexcept;
```

#### `ft_buff`
```
int get_id() const noexcept;
void set_id(int id) noexcept;
int get_duration() const noexcept;
void set_duration(int duration) noexcept;
void add_duration(int duration) noexcept;
void sub_duration(int duration) noexcept;
int get_modifier1() const noexcept;
void set_modifier1(int mod) noexcept;
void add_modifier1(int mod) noexcept;
void sub_modifier1(int mod) noexcept;
int get_modifier2() const noexcept;
void set_modifier2(int mod) noexcept;
void add_modifier2(int mod) noexcept;
void sub_modifier2(int mod) noexcept;
int get_modifier3() const noexcept;
void set_modifier3(int mod) noexcept;
void add_modifier3(int mod) noexcept;
void sub_modifier3(int mod) noexcept;
int get_modifier4() const noexcept;
void set_modifier4(int mod) noexcept;
void add_modifier4(int mod) noexcept;
void sub_modifier4(int mod) noexcept;
```

#### `ft_debuff`
```
int get_id() const noexcept;
void set_id(int id) noexcept;
int get_duration() const noexcept;
void set_duration(int duration) noexcept;
void add_duration(int duration) noexcept;
void sub_duration(int duration) noexcept;
int get_modifier1() const noexcept;
void set_modifier1(int mod) noexcept;
void add_modifier1(int mod) noexcept;
void sub_modifier1(int mod) noexcept;
int get_modifier2() const noexcept;
void set_modifier2(int mod) noexcept;
void add_modifier2(int mod) noexcept;
void sub_modifier2(int mod) noexcept;
int get_modifier3() const noexcept;
void set_modifier3(int mod) noexcept;
void add_modifier3(int mod) noexcept;
void sub_modifier3(int mod) noexcept;
int get_modifier4() const noexcept;
void set_modifier4(int mod) noexcept;
void add_modifier4(int mod) noexcept;
void sub_modifier4(int mod) noexcept;
```
#### `ft_skill`
```
int get_id() const noexcept;
void set_id(int id) noexcept;
int get_level() const noexcept;
void set_level(int level) noexcept;
int get_cooldown() const noexcept;
void set_cooldown(int cooldown) noexcept;
void add_cooldown(int cooldown) noexcept;
void sub_cooldown(int cooldown) noexcept;
int get_modifier1() const noexcept;
void set_modifier1(int mod) noexcept;
void add_modifier1(int mod) noexcept;
void sub_modifier1(int mod) noexcept;
int get_modifier2() const noexcept;
void set_modifier2(int mod) noexcept;
void add_modifier2(int mod) noexcept;
void sub_modifier2(int mod) noexcept;
int get_modifier3() const noexcept;
void set_modifier3(int mod) noexcept;
void add_modifier3(int mod) noexcept;
void sub_modifier3(int mod) noexcept;
int get_modifier4() const noexcept;
void set_modifier4(int mod) noexcept;
void add_modifier4(int mod) noexcept;
void sub_modifier4(int mod) noexcept;
int get_error() const noexcept;
const char *get_error_str() const noexcept;
```


#### `ft_achievement`
```
int  get_id() const noexcept;
void set_id(int id) noexcept;
int  get_goal(int id) const noexcept;
void set_goal(int id, int goal) noexcept;
int  get_progress(int id) const noexcept;
void set_progress(int id, int progress) noexcept;
void add_progress(int id, int value) noexcept;
bool is_goal_complete(int id) const noexcept;
bool is_complete() const noexcept;
```

#### `ft_crafting`
```
ft_map<int, ft_vector<ft_crafting_ingredient>>       &get_recipes() noexcept;
const ft_map<int, ft_vector<ft_crafting_ingredient>> &get_recipes() const noexcept;
int register_recipe(int recipe_id, ft_vector<ft_crafting_ingredient> &&ingredients) noexcept;
int craft_item(ft_inventory &inventory, int recipe_id, const ft_sharedptr<ft_item> &result) noexcept;
int get_error() const noexcept;
const char *get_error_str() const noexcept;
```

#### `ft_experience_table`
```
int  get_count() const noexcept;
int  get_level(int experience) const noexcept;
int  get_value(int index) const noexcept;
void set_value(int index, int value) noexcept;
int  set_levels(const int *levels, int count) noexcept;
int  generate_levels_total(int count, int base, double multiplier) noexcept;
int  generate_levels_scaled(int count, int base, double multiplier) noexcept;
int  resize(int new_count) noexcept;
```

This project is a work in progress, and basic functionality can heavily
change over time, potentially breaking code that relies on the library.
