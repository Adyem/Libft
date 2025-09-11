# FullLibft

Libft is a collection of C and C++ utilities grouped into multiple small libraries.
It provides implementations of common libc functions, custom memory allocation helpers,
basic threading helpers, containers, string utilities, simple networking and more.
The top level `Makefile` builds every submodule and links them into `Full_Libft.a`.
The umbrella header `FullLibft.hpp` includes every component.
Header files now use class names or concise module names instead of module prefixes, except internal headers which retain their module prefix.

This document briefly lists the main headers and the interfaces they expose. The
summaries below only outline the available functions and classes. See the header
files for detailed information.

All classes report errors through a mutable `_error_code` member with `get_error` and `get_error_str` accessors so `const` methods can update the error state.

## Building

```bash
make
```

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
void    ft_bzero(void *string, size_t size);
void   *ft_memchr(const void *ptr, int c, size_t size);
void   *ft_memcpy(void *dst, const void *src, size_t n);
void   *ft_memmove(void *dst, const void *src, size_t n);
size_t  ft_strlcat(char *dst, const char *src, size_t size);
size_t  ft_strlcpy(char *dst, const char *src, size_t size);
char   *ft_strrchr(const char *s, int c);
char   *ft_strnstr(const char *haystack, const char *needle, size_t len);
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
void   *ft_memset(void *dst, int value, size_t n);
int     ft_isspace(int c);
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
double      math_deg2rad(double degrees);
double      math_rad2deg(double radians);
```

Additional helpers for parsing expressions are available. They allocate a
single `int` holding the result, which the caller must release with
`cma_free`. On failure they return `ft_nullptr` and set `ft_errno`:

```
int        *math_roll(const char *expression);
int        *math_eval(const char *expression);
```

Internal parser helpers now use the `math_` prefix for consistency.

### Custom Memory Allocator (CMA)

Located in `CMA/`. Header: `CMA.hpp`. Provides memory helpers such as
`cma_malloc`, `cma_free`, aligned allocations, allocation-size queries,
and string helpers. Aligned allocations round the block size up to the
specified power-of-two (e.g., requesting 100 bytes with alignment 32
returns a 128-byte block) and are released with `cma_free`.
When allocation logging is enabled via the logger, the allocator emits debug messages for each `cma_malloc` and `cma_free`.
The allocator enforces an optional global allocation limit that can be
changed at runtime with `cma_set_alloc_limit`. A limit of `0` disables the
check. The allocator also tracks allocation and free counts, accessible
through `cma_get_stats`. Internally, `cma_realloc` has been simplified by
removing redundant braces.

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
void    cma_get_stats(std::size_t *allocation_count, std::size_t *free_count);
```

### GetNextLine

`GetNextLine/get_next_line.hpp` implements a simple file reader that works with `ft_istream` and a configurable buffer size. It stores per-stream leftovers in the custom `ft_unord_map`, allowing error reporting through `ft_errno`. The `CPP_class` module provides `ft_istream`, `ft_stringbuf`, and `ft_istringstream` as lightweight replacements for the standard stream classes.

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
```

### PThread Wrappers

`PThread/pthread.hpp` wraps a few `pthread` calls, condition variables, and provides basic atomic operations.

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
```

### C++ Classes (`CPP_class`)

Header files in `CPP_class/` define several helper classes.
Below is a brief list of the main classes and selected members.

#### `DataBuffer`
```
DataBuffer();
DataBuffer(const DataBuffer& other);
DataBuffer(DataBuffer&& other) noexcept;
DataBuffer& operator=(const DataBuffer& other);
DataBuffer& operator=(DataBuffer&& other) noexcept;
~DataBuffer();
void clear() noexcept;
size_t size() const noexcept;
const std::vector<uint8_t>& data() const noexcept;
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
socket wrapper.

```
int nw_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int nw_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int nw_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int nw_listen(int sockfd, int backlog);
int nw_socket(int domain, int type, int protocol);
```

`wrapper.hpp` adds helpers for encrypted sockets:

```
ssize_t nw_ssl_write(SSL *ssl, const void *buf, size_t len);
ssize_t nw_ssl_read(SSL *ssl, void *buf, size_t len);
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

### Logger

`Logger/logger.hpp` provides leveled logging with timestamps, formatted output
and optional file rotation. Logs are written to one or more configurable
destinations (sinks) and filtered according to the active log level.

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
touching the process environment. It also exposes portable helpers to query CPU count and total
physical memory.

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
- Utility types: `ft_pair`, `ft_tuple`, `ft_optional`, `ft_variant` and
  `ft_bitset`.
- Smart pointers: `ft_shared_ptr` and `ft_unique_ptr`.
- Concurrency helpers: `ft_thread_pool`, `ft_future`, `ft_event_emitter` and
  `ft_promise`.
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
errors through the shared `ft_errno` system.

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


#### Errno
`Errno/errno.hpp` defines a thread-local `_error_code` accessed through the `ft_errno` macro and helpers for retrieving messages.

```
const char *ft_strerror(int err);
void        ft_perror(const char *msg);
void        ft_exit(const char *msg, int code);
```

#### RNG
Random helpers and containers in `RNG/`.

```
int   ft_random_int(void);
int   ft_dice_roll(int number, int faces);
float ft_random_float(void);
float ft_random_normal(void);
float ft_random_exponential(float lambda_value);
int   ft_random_seed(const char *seed_str = ft_nullptr);
```

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
void addElement(ElementType *elem, int weight);
ElementType *getRandomLoot() const;
ElementType *popRandomLoot();
```

#### Encryption
`basic_encryption.hpp` exposes minimal helpers and AES block operations:

```
int         be_saveGame(const char *filename, const char *data, const char *key);
char      **be_DecryptData(char **data, const char *key);
const char *be_getEncryptionKey();
void        aes_encrypt(uint8_t *block, const uint8_t *key);
void        aes_decrypt(uint8_t *block, const uint8_t *key);
```

Example:

```
unsigned char block[16] = {0};
unsigned char key[16] = {0};
aes_encrypt(block, key);
aes_decrypt(block, key);
```

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
```

The `json_document` class wraps these helpers and manages a group list:

```
json_document();
~json_document();
void         append_group(json_group *group) noexcept;
json_group   *find_group(const char *name) const noexcept;
```

#### File
Cross-platform file and directory utilities (`File/open_dir.hpp`):

```
file_dir   *file_opendir(const char *directory_path);
int         file_closedir(file_dir *directory_stream);
file_dirent *file_readdir(file_dir *directory_stream);
int         file_dir_exists(const char *rel_path);
int         file_create_directory(const char *path, mode_t mode);
```

`System_utils/system_utils.hpp` provides cross-platform file descriptor utilities:

```
void    ft_initialize_standard_file_descriptors();
int     su_open(const char *pathname);
int     su_open(const char *pathname, int flags);
int     su_open(const char *pathname, int flags, mode_t mode);
ssize_t su_read(int fd, void *buf, size_t count);
ssize_t su_write(int fd, const void *buf, size_t count);
int     ft_close(int fd);
```
#### Config
`Config/config.hpp` parses simple configuration files:

```
cnfg_config *cnfg_parse(const char *filename);
char       *cnfg_parse_flags(int argument_count, char **argument_values);
void       cnfg_free(cnfg_config *config);
```

`cnfg_parse` gives precedence to environment variables. Before using a
value from the file, the parser checks `getenv` with the key name and
uses the environment value if it exists.

`flag_parser.hpp` wraps flag parsing in a class:

```
cnfg_flag_parser parser(argument_count, argument_values);
parser.has_short_flag('a');
parser.has_long_flag("help");
parser.get_short_flag_count();
parser.get_long_flag_count();
parser.get_total_flag_count();
parser.get_error();
parser.get_error_str();
```

#### Time
`Time/time.hpp` exposes simple time helpers:

```
time_t  time_now(void);
long    time_now_ms(void);
long    time_monotonic(void);
void    time_local(time_t time_value, struct tm *out);
void    time_sleep(unsigned int seconds);
void    time_sleep_ms(unsigned int milliseconds);
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
HTTP client helpers in `API/api.hpp` and asynchronous wrappers:

```
char       *api_request_string(const char *ip, uint16_t port,
                               const char *method, const char *path,
                               json_group *payload = ft_nullptr,
                               const char *headers = ft_nullptr, int *status = ft_nullptr,
                               int timeout = 60000);
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

Callback based helpers run the request on a background thread and invoke
the user supplied callback with the body and status code. The response body
is allocated with `cma_malloc` and must be freed by the caller.

```
typedef void (*api_callback)(char *body, int status, void *user_data);

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

#### Game
Basic game related classes include `ft_character`, `ft_item`, `ft_inventory`,
`ft_upgrade`, `ft_world`, `ft_event`, `ft_map3d`, `ft_quest`, `ft_reputation`,
`ft_buff`, `ft_debuff`, `ft_achievement`, and `ft_experience_table`. Each class
is summarized below.

#### `ft_character`
```
int get_hit_points() const noexcept;
void set_hit_points(int hp) noexcept;
bool is_alive() const noexcept;
int get_armor() const noexcept;
void set_armor(int armor) noexcept;
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
int get_current_stack() const noexcept;
void set_current_stack(int amount) noexcept;
void add_to_stack(int amount) noexcept;
void sub_from_stack(int amount) noexcept;
int get_item_id() const noexcept;
void set_item_id(int id) noexcept;
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
ft_map<int, ft_item>       &get_items() noexcept;
const ft_map<int, ft_item> &get_items() const noexcept;
size_t get_capacity() const noexcept;
void   resize(size_t capacity) noexcept;
size_t get_used() const noexcept;
bool   is_full() const noexcept;
int get_error() const noexcept;
const char *get_error_str() const noexcept;
int  add_item(const ft_item &item) noexcept;
void remove_item(int slot) noexcept;
int  count_item(int item_id) const noexcept;
bool has_item(int item_id) const noexcept;
```

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
```

#### `ft_world`
```
ft_map<int, ft_event>       &get_events() noexcept;
const ft_map<int, ft_event> &get_events() const noexcept;
int save_game(const char *file_path, const ft_character &character) const noexcept;
int load_game(const char *file_path, ft_character &character) noexcept;
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

#### `ft_quest`
```
int get_id() const noexcept;
void set_id(int id) noexcept;
int get_phases() const noexcept;
void set_phases(int phases) noexcept;
int get_current_phase() const noexcept;
void set_current_phase(int phase) noexcept;
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
