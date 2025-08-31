# FullLibft

Libft is a collection of C and C++ utilities grouped into multiple small libraries.
It provides implementations of common libc functions, custom memory allocation helpers,
basic threading helpers, containers, string utilities, simple networking and more.
The top level `Makefile` builds every submodule and links them into `Full_Libft.a`.
The umbrella header `FullLibft.hpp` includes every component.

This document briefly lists the main headers and the interfaces they expose. The
summaries below only outline the available functions and classes. See the header
files for detailed information.

## Building

```bash
make
```

To build the debug version use `make debug`. Individual sub-modules can be built by
entering their directory and running `make`.

## Modules

### Libft

Standard C utilities located in `Libft/`. Headers: `libft.hpp` and `ft_limits.hpp`.

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
int     ft_abs(int number);
```

`ft_limits.hpp` exposes integer boundary constants:

```
FT_CHAR_BIT
FT_INT_MAX
FT_INT_MIN
FT_UINT_MAX
FT_LONG_MAX
FT_LONG_MIN
FT_ULONG_MAX
```

### Custom Memory Allocator (CMA)

Located in `CMA/`. Header: `CMA.hpp`. Provides memory helpers such as
`cma_malloc`, `cma_free`, aligned allocations, allocation-size queries,
and string helpers. Aligned allocations round the block size up to the
specified power-of-two (e.g., requesting 100 bytes with alignment 32
returns a 128-byte block) and are released with `cma_free`.
When allocation logging is enabled via the logger, the allocator emits debug messages for each `cma_malloc` and `cma_free`.

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
```

### GetNextLine

`GetNextLine/get_next_line.hpp` implements a simple file reader.

```
char   *ft_strjoin_gnl(char *s1, char *s2);
char   *get_next_line(ft_file &file);
char  **ft_read_file_lines(ft_file &file);
char  **ft_open_and_read_file(const char *file_name);
```

### Printf

`Printf/printf.hpp` contains minimal `printf` wrappers.

```
int pf_printf(const char *format, ...);
int pf_printf_fd(int fd, const char *format, ...);
```

### PThread Wrappers

`PThread/PThread.hpp` wraps a few `pthread` calls.

```
int pt_thread_join(pthread_t thread, void **retval);
int pt_thread_create(pthread_t *thread, const pthread_attr_t *attr,
                     void *(*start_routine)(void *), void *arg);
int pt_thread_detach(pthread_t thread);
template <typename ValueType, typename Function>
int pt_async(ft_promise<ValueType>& promise, Function function);
```

### C++ Classes (`CPP_class`)

Header files in `CPP_class/` define several helper classes.
Below is a brief list of the main classes and selected members.

#### `DataBuffer`
```
DataBuffer();
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
explicit ft_string(int errorCode) noexcept;
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

`ssl_wrapper.hpp` adds helpers for encrypted sockets:

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
and optional file rotation. Logs are written to a configurable destination and
filtered according to the active log level.

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

A global pointer `g_logger` is provided for optional shared access. Call
`set_global()` on an `ft_logger` instance to populate it; the pointer is reset
to `ft_nullptr` when that instance is destroyed. Allocation logging for the
custom memory allocator can be toggled with `set_alloc_logging` and
`get_alloc_logging`.

### Template Utilities

`Template/` contains several generic helpers such as `ft_vector`, `ft_map`,
`ft_pair`, smart pointers and mathematical helpers. Refer to the header files
for the full interface of these templates.

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
`Errno/errno.hpp` defines error codes and helpers for retrieving messages.

```
const char *ft_strerror(int err);
void        ft_perror(const char *msg);
```

#### RNG
Random helpers and containers in `RNG/`.

```
int   ft_random_int(void);
int   ft_dice_roll(int number, int faces);
float ft_random_float(void);
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
`BasicEncryption.hpp` exposes minimal helpers:

```
int         be_saveGame(const char *filename, const char *data, const char *key);
char      **be_DecryptData(char **data, const char *key);
const char *be_getEncryptionKey();
```

#### JSon
Creation, reading and manipulation helpers in `JSon/json.hpp`:

```
json_item   *json_create_item(const char *key, const char *value);
json_group  *json_create_json_group(const char *name);
void         json_add_item_to_group(json_group *group, json_item *item);
int          json_write_to_file(const char *filename, json_group *groups);
char        *json_write_to_string(json_group *groups);
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

#### File
Cross-platform file and directory utilities (`file/open_dir.hpp`):

```
FT_DIR   *ft_opendir(const char *directoryPath);
int       ft_closedir(FT_DIR *directoryStream);
ft_dirent *ft_readdir(FT_DIR *directoryStream);
int       dir_exists(const char *rel_path);
int       file_create_directory(const char *path, mode_t mode);
```

#### Config
`Config/config.hpp` parses simple configuration files:

```
ft_config *ft_config_parse(const char *filename);
void       ft_config_free(ft_config *config);
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

`api_promise.hpp` adds asynchronous versions returning `ft_promise` objects for
string and JSON responses, and TLS variants for HTTPS.

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
```

#### HTML
Minimal node creation and searching utilities (`HTML/html_parser.hpp`):

```
html_node *html_create_node(const char *tagName, const char *textContent);
void       html_add_child(html_node *parentNode, html_node *childNode);
void       html_add_attr(html_node *targetNode, html_attr *newAttribute);
html_node *html_find_by_tag(html_node *nodeList, const char *tagName);
html_node *html_find_by_attr(html_node *nodeList, const char *key, const char *value);
```

#### Game
Basic game related classes (`ft_character`, `ft_item`, `ft_inventory`,
`ft_upgrade`, `ft_world`, `ft_event`, `ft_map3d`, `ft_quest`, `ft_reputation`,
`ft_buff`, `ft_debuff`, `ft_achievement`, `ft_experience_table`). `ft_buff` and
`ft_debuff` each store four independent modifiers and expose getters, setters,
and adders (including for duration). `ft_event`, `ft_upgrade`, `ft_item`, and
`ft_reputation` also expose adders, and now each of these classes provides
matching subtract helpers. `ft_inventory` manages stacked items and can query
item counts with `has_item` and `count_item`. `ft_character` keeps track of
coins and a `valor` attribute with helpers to add or subtract these values.
`ft_experience_table` maps experience points to levels and can generate level
progressions. `ft_achievement` tracks goal progress with `add_progress`,
`is_goal_complete`, and `is_complete`. The character's current level can be
retrieved with `get_level()` which relies on an internal experience table.
`ft_quest` objects can report completion with `is_complete()` and progress
phases via `advance_phase()`.

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

The project is a work in progress and not every component is documented here.
Consult the individual header files for precise behavior and additional
functionality.
