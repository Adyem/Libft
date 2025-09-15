# FullLibft

Libft is a collection of C and C++ utilities grouped into multiple small libraries.
It provides implementations of common libc functions, custom memory allocation helpers,
basic threading helpers, containers, string utilities, simple networking and more.
The top level `Makefile` builds every submodule and links them into `Full_Libft.a`.
The umbrella header `FullLibft.hpp` includes every component.
Internal code uses custom replacements such as `ft_strlen`, `ft_strchr`, `ft_strstr`, and `pf_snprintf` instead of the standard library equivalents. It also provides `ft_move` as a drop-in replacement for `std::move`.
Header files now use class names or concise module names instead of module prefixes, except internal headers which retain their module prefix.

This document briefly lists the main headers and the interfaces they expose. The
summaries below only outline the available functions and classes. See the header
files for detailed information.

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

Functional tests reside in `Test/Test` and performance benchmarks in `Test/Efficiency`.

The test runner prints `OK` or `KO` for each registered case and
summarizes the total. Detailed assertion failures are written to
`test_failures.log` with the source file and line number of the failing
check. Output is grouped by module so related tests appear together.

Each test uses the `FT_TEST` macro, which registers the case at program
startup so all linked test files run automatically.

The current suite exercises components across multiple modules:

- **Libft**: `ft_atoi`, `ft_atol`, `ft_bzero`, `ft_isdigit`, `ft_isalpha`, `ft_isalnum`, `ft_islower`, `ft_isupper`, `ft_isprint`, `ft_isspace`, `ft_memchr`,
  `ft_memcmp`, `ft_memcpy`, `ft_memdup`, `ft_memmove`, `ft_memset`, `ft_strchr`, `ft_strcmp`, `ft_strjoin_multiple`, `ft_strlcat`, `ft_strlcpy`, `ft_strncpy`, `ft_strlen`, `ft_strncmp`,
  `ft_strnstr`, `ft_strstr`, `ft_strrchr`, `ft_strmapi`, `ft_striteri`, `ft_strtok`, `ft_strtol`, `ft_strtoul`, `ft_setenv`, `ft_unsetenv`, `ft_getenv`, `ft_to_lower`, `ft_to_upper`,
`ft_fopen`, `ft_fclose`, `ft_fgets`, `ft_time_ms`, `ft_time_format`, `ft_to_string`
- **Concurrency**: `ft_promise`, `ft_task_scheduler`, `ft_this_thread`
- **Networking**: IPv4 and IPv6 send/receive paths, UDP datagrams, and a simple HTTP server
- **Logger**: color toggling, JSON sink, asynchronous logging
- **Math**: vector, matrix, and quaternion helpers
- **RNG**: normal, exponential, Poisson, binomial, and geometric distributions
- **String**: `ft_string_view`
- **JSon**: schema validation
- **YAML**: round-trip parsing
- **Game**: `ft_game_state` centralizes worlds and character data with vectors of shared pointers for RAII cleanup, `ft_world` persistence and a shared-pointer-based `ft_event_scheduler` for timed actions via `ft_world::schedule_event` and `ft_world::update_events`, `ft_equipment`, `ft_inventory`, and `ft_quest` store items through shared pointers, `ft_crafting` consumes and produces shared items, `ft_world::plan_route`, `ft_pathfinding`, and copy/move constructors across game classes
- Shared pointers expose their own `get_error` while managed objects may define their own; call `shared_ptr.get_error()` for allocation issues and `shared_ptr->get_error()` for object-specific errors.
- Game classes validate both the shared pointer and its managed object for errors before use so failures surface consistently.
- `ft_item` tracks its own error code so equipment, inventory, and crafting verify both the item pointer and the item itself before applying modifiers or stacking quantities.
- **Encryption**: key generation utilities

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

```
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
char   *ft_strjoin_multiple(int count, ...);
long    ft_strtol(const char *input_string, char **end_pointer, int numeric_base);
unsigned long ft_strtoul(const char *nptr, char **endptr, int base);
int     ft_setenv(const char *name, const char *value, int overwrite);
int     ft_unsetenv(const char *name);
char   *ft_getenv(const char *name);
void   *ft_memset(void *dst, int value, size_t n);
int     ft_isspace(int c);
char   *ft_fgets(char *string, int size, FILE *stream);
FILE   *ft_fopen(const char *filename, const char *mode);
int     ft_fclose(FILE *stream);
long    ft_time_ms(void);
char   *ft_time_format(char *buffer, size_t buffer_size);
ft_string ft_to_string(long number);
```

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

Located in `Math/`. Headers: `math.hpp` and `roll.hpp`. Provides basic math utilities:

```
int         math_abs(int number);
long        math_abs(long number);
long long   math_abs(long long number);
double      math_fabs(double number);
int         math_signbit(double number);
int         math_isnan(double number);
double      math_nan(void);
void        math_swap(int *first_number, int *second_number);
int         math_clamp(int value, int minimum, int maximum);
int         math_gcd(int first_number, int second_number);
long        math_gcd(long first_number, long second_number);
long long   math_gcd(long long first_number, long long second_number);
int         math_lcm(int first_number, int second_number);
long        math_lcm(long first_number, long second_number);
long long   math_lcm(long long first_number, long long second_number);
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
```

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

Internal parser helpers now use the `math_` prefix for consistency.

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
returns a 128-byte block) and are released with `cma_free`. A safe
`atoi` helper first validates the input and returns an allocated integer
or `ft_nullptr` on failure.
When allocation logging is enabled via the logger, the allocator emits debug messages for each `cma_malloc` and `cma_free`.
The allocator enforces an optional global allocation limit that can be
changed at runtime with `cma_set_alloc_limit`. A limit of `0` disables the
check. The allocator also tracks allocation and free counts, accessible
through `cma_get_stats`. Internally, `cma_realloc` has been simplified by
removing redundant braces.
Thread safety can be enabled or disabled with `cma_set_thread_safety`.

```
void   *cma_malloc(std::size_t size);
void    cma_free(void *ptr);
int     cma_checked_free(void *ptr);
char   *cma_strdup(const char *string);
void   *cma_memdup(const void *src, size_t size);
void   *cma_calloc(std::size_t nmemb, std::size_t size);
void   *cma_realloc(void *ptr, std::size_t new_size);
void   *cma_aligned_alloc(std::size_t alignment, std::size_t size);
std::size_t cma_alloc_size(const void *ptr);
int   *cma_atoi(const char *string);
char  **cma_split(const char *s, char c);
char   *cma_itoa(int n);
char   *cma_itoa_base(int n, int base);
char   *cma_strjoin(const char *s1, const char *s2);
char   *cma_strjoin_multiple(int count, ...);
char   *cma_substr(const char *s, unsigned int start, size_t len);
char   *cma_strtrim(const char *s1, const char *set);
void    cma_free_double(char **content);
void    cma_cleanup();
void    cma_set_alloc_limit(std::size_t limit);
void    cma_set_thread_safety(bool enable);
void    cma_get_stats(std::size_t *allocation_count, std::size_t *free_count);
```

### GetNextLine

`GetNextLine/get_next_line.hpp` implements a simple file reader that works with `ft_istream` and a configurable buffer size. It stores per-stream leftovers in the custom `ft_unord_map`, allowing error reporting through `ft_errno`. The `CPP_class` module provides `ft_istream`, `ft_stringbuf`, `ft_istringstream`, and `ft_ofstream` as lightweight replacements for the standard stream classes.

```
char   *ft_strjoin_gnl(char *string_one, char *string_two);
char   *get_next_line(ft_istream &input, std::size_t buffer_size);
char  **ft_read_file_lines(ft_istream &input, std::size_t buffer_size);
char  **ft_open_and_read_file(const char *file_name, std::size_t buffer_size);
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

#### `ft_nullptr`
```
namespace ft {
    struct nullptr_t {
        template <typename PointerType>
        operator PointerType*() const { return nullptr; }
        template <typename ClassType, typename MemberType>
        operator MemberType ClassType::*() const { return nullptr; }
        void operator&() const; /* deleted */
    };
    extern const nullptr_t ft_nullptr_instance;
}
#define ft_nullptr (ft::ft_nullptr_instance)
```

### Networking

`Networking/networking.hpp` and `socket_class.hpp` implement a small
socket wrapper with IPv4 and IPv6 support.

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
```

The polling backend is chosen at compile time. Linux builds use `epoll`,
BSD and macOS use `kqueue`, and other systems including Windows fall back
to `select`. The event loop helpers call the appropriate backend through
`nw_poll`. The `epoll` and `kqueue` implementations allocate their event
buffers with `cma_malloc` and release them with `cma_free` to stay within
the custom allocator. The `select` backend is intended only for small
numbers of sockets and lacks the scalability of the other backends.

`wrapper.hpp` adds helpers for encrypted sockets:

```
ssize_t nw_ssl_write(SSL *ssl, const void *buf, size_t len);
ssize_t nw_ssl_read(SSL *ssl, void *buf, size_t len);
```

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

#### HTTP server

`Networking/http_server.hpp` implements a minimal synchronous server that uses
`ft_socket` for IPv4 and IPv6 support, optional non-blocking operation and
configurable timeouts. The server accepts a single connection and parses basic
GET or POST requests.

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
basic ping/pong handling.

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
  `ft_priority_queue`, `ft_set`, `ft_map`, `ft_unordened_map`,
  `ft_trie`, `ft_circular_buffer`, `ft_graph` and `ft_matrix`.
 - Utility types: `ft_pair`, `ft_tuple`, `ft_optional`, `ft_variant`,
   `ft_bitset`, `ft_function` and `ft_string_view`.
- Smart pointers: `ft_shared_ptr` and `ft_unique_ptr`.
- Concurrency helpers: `ft_thread_pool`, `ft_future`, `ft_event_emitter` and
  `ft_promise`.
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

### Additional Modules


#### Concurrency
`Concurrency/this_thread.hpp` provides helpers for the current thread:

```
std::thread::id ft_this_thread_get_id();
void ft_this_thread_sleep_for(std::chrono::milliseconds duration);
void ft_this_thread_sleep_until(std::chrono::steady_clock::time_point time_point);
void ft_this_thread_yield();
```

`Concurrency/task_scheduler.hpp` offers `ft_task_scheduler`, combining a
lock-free queue, thread pool and scheduler. Tasks may be submitted for
immediate execution, delayed execution or recurring intervals and each
submission returns a future when applicable.

```
ft_task_scheduler(size_t thread_count = 0);
template <typename FunctionType, typename... Args>
auto submit(FunctionType function, Args... args) -> std::future<ReturnType>;
template <typename Rep, typename Period, typename FunctionType, typename... Args>
auto schedule_after(std::chrono::duration<Rep, Period> delay,
                    FunctionType function, Args... args) -> std::future<ReturnType>;
template <typename Rep, typename Period, typename FunctionType, typename... Args>
void schedule_every(std::chrono::duration<Rep, Period> interval,
                    FunctionType function, Args... args);
```

Recurring tasks preserve their callbacks across intervals, preventing
empty function executions when rescheduled.

Worker threads fetch jobs from a lock-free queue so producers and consumers do
not block each other. The scheduler thread manages delayed and recurring jobs.

#### Errno
`Errno/errno.hpp` defines a thread-local `ft_errno` variable and helpers for retrieving messages.

```
const char *ft_strerror(int err);
void        ft_perror(const char *msg);
void        ft_exit(const char *msg, int code);
```

#### RNG
Random helpers and containers in `RNG/`. `rng_secure_bytes` obtains
cryptographically secure random data from the operating system,
`ft_random_uint32` wraps it to produce a single 32-bit value,
and `ft_generate_uuid` formats secure bytes as a version 4 UUID string.

```
int   ft_random_int(void);
int   ft_dice_roll(int number, int faces);
float ft_random_float(void);
float ft_random_normal(void);
float ft_random_exponential(float lambda_value);
int   ft_random_poisson(double lambda_value);
int   ft_random_binomial(int trial_count, double success_probability);
int   ft_random_geometric(double success_probability);
int   ft_random_seed(const char *seed_str = ft_nullptr);
int   rng_secure_bytes(unsigned char *buffer, size_t length);
uint32_t ft_random_uint32(void);
void   ft_generate_uuid(char out[37]);
```

Example:

```
unsigned char buffer[16];
if (rng_secure_bytes(buffer, 16) == 0)
{
    /* use buffer */
}
uint32_t secure_value = ft_random_uint32();
char uuid[37];
ft_generate_uuid(uuid);
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

#### JSon
Creation, reading and manipulation helpers in `JSon/json.hpp`:

```
json_item   *json_create_item(const char *key, const char *value);
json_group  *json_create_json_group(const char *name);
void         json_add_item_to_group(json_group *group, json_item *item);
int          json_write_to_file(const char *filename, json_group *groups);
char        *json_write_to_string(json_group *groups);
int          json_document_write_to_file(const char *file_path, const json_document &document);
char        *json_document_write_to_string(const json_document &document);
json_group  *json_read_from_file(const char *filename);
json_group  *json_read_from_string(const char *content);
json_group  *json_find_group(json_group *head, const char *name);
json_item   *json_find_item(json_group *group, const char *key);
void         json_remove_group(json_group **head, const char *name);
void         json_remove_item(json_group *group, const char *key);
void         json_update_item(json_group *group, const char *key, const char *value);
void         json_update_item(json_group *group, const char *key, const int value);
void         json_update_item(json_group *group, const char *key, const bool value);
bool         json_validate_schema(json_group *group, const json_schema &schema);
```
The `json_document` class wraps these helpers and manages a group list:

```
json_document();
~json_document();
void         append_group(json_group *group) noexcept;
json_group   *find_group(const char *name) const noexcept;
```

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

`System_utils/system_utils.hpp` provides cross-platform file descriptor utilities:

```
void    cmp_initialize_standard_file_descriptors();
int     su_open(const char *pathname);
int     su_open(const char *pathname, int flags);
int     su_open(const char *pathname, int flags, mode_t mode);
ssize_t su_read(int fd, void *buf, size_t count);
ssize_t su_write(int fd, const void *buf, size_t count);
int     cmp_close(int fd);
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
until `kv_flush` writes them to disk.

```
kv_store store("data.json");
store.kv_set("theme", "dark");
const char *theme = store.kv_get("theme");
store.kv_delete("unused");
store.kv_flush();
```

Errors set the thread-local `ft_errno` and are accessible through `get_error`
and `get_error_str`.

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
```

`cnfg_parse` gives precedence to environment variables. Before using a
value from the file, the parser checks `getenv` with the key name and
uses the environment value if it exists.

`flag_parser.hpp` wraps flag parsing in a class and `config_merge_sources`
combines command-line flags with environment variables and configuration
files:

```
cnfg_flag_parser parser(argument_count, argument_values);
parser.has_short_flag('a');
parser.has_long_flag("help");
parser.get_short_flag_count();
parser.get_long_flag_count();
parser.get_total_flag_count();
parser.get_error();
parser.get_error_str();
config_merge_sources(argument_count, argument_values, "config.ini");
```

#### Time
`Time/time.hpp` exposes simple time helpers:

```
t_time  time_now(void);
long    time_now_ms(void);
long    time_monotonic(void);
void    time_local(t_time time_value, t_time_info *out);
void    time_sleep(unsigned int seconds);
void    time_sleep_ms(unsigned int milliseconds);
size_t  time_strftime(char *buffer, size_t size, const char *format, const t_time_info *time_info);
ft_string    time_format_iso8601(t_time time_value);
bool    time_parse_iso8601(const char *string_input, std::tm *time_output, t_time *timestamp_output);
bool    time_parse_custom(const char *string_input, const char *format, std::tm *time_output, t_time *timestamp_output);
```

`t_time` stores seconds since the Unix epoch and `t_time_info` holds the broken-down components.

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

#### API
HTTP client helpers in `API/api.hpp` and asynchronous wrappers. URL parsing
relies on the `ft_string` class instead of `std::string`, so any allocation
errors must be checked via `get_error`. The implementation is split across
multiple source files like `api_request.cpp` and `api_request_async.cpp` for
maintainability.

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

Both helpers search the scheduler's queue and set `GAME_GENERAL_ERROR` if the event is not found.

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
ft_crafting_ingredient ingredient_a = {1, 2, -1};
ft_crafting_ingredient ingredient_b = {2, 1, 1};
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
ft_item *get_equipped_item(int slot) noexcept;
const ft_item *get_equipped_item(int slot) const noexcept;
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

Inventory behavior can be toggled with macros in `Game/game_rules.hpp`.
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
