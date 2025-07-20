# FullLibft

FullLibft is a collection of C and C++ utilities grouped into multiple small libraries.
It provides implementations of common libc functions, custom memory allocation helpers,
basic threading helpers, containers, string utilities, simple networking and more.
The top level `Makefile` builds every submodule and links them into `Full_Libft.a`.

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

Standard C utilities located in `Libft/`. Header: `libft.hpp`.

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

### Custom Memory Allocator (CMA)

Located in `CMA/`. Header: `CMA.hpp`. Provides memory helpers such as
`cma_malloc`, `cma_free` and string helpers.

```
void   *cma_malloc(std::size_t size);
void    cma_free(void *ptr);
int     cma_checked_free(void *ptr);
char   *cma_strdup(const char *string);
void   *cma_memdup(const void *src, size_t size);
void   *cma_calloc(std::size_t nmemb, std::size_t size);
void   *cma_realloc(void *ptr, std::size_t new_size);
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

### Template Utilities

`Template/` contains several generic helpers such as `ft_vector`, `ft_map`,
`ft_pair`, smart pointers and mathematical helpers. Refer to the header files
for the full interface of these templates.

### Additional Modules

* **Errno** – `Errno/errno.hpp` defines error codes and functions `ft_strerror` and `ft_perror`.
* **RNG** – random helpers like `ft_dice_roll` and a `ft_deck` container.
* **Encryption** – very small API (`be_saveGame`, `be_DecryptData`, `be_getEncryptionKey`).
* **JSon** – simple JSON serialization helpers (`json_create_item`, `json_read_from_file`, etc.).
* **File** – directory handling wrappers such as `ft_opendir` and `ft_readdir`.
* **HTML** – minimal HTML node creation and searching utilities.
* **Game** – basic game related classes (`ft_character`, `ft_item`, `ft_inventory`, `ft_upgrade`, `ft_world`, `ft_event`, `ft_map3d`, `ft_quest`, `ft_reputation`, `ft_buff`, `ft_debuff`). `ft_buff` and `ft_debuff` each store four independent modifiers and expose getters, setters, and adders (including for duration). `ft_event`, `ft_upgrade`, `ft_item`, and `ft_reputation` also expose adders, and now each of these classes provides matching subtract helpers. `ft_inventory` manages stacked items and can query item counts with `has_item` and `count_item`. `ft_character` keeps track of coins and a `valor` attribute with helpers to add or subtract these values. The character's current level can be retrieved with `get_level()` which relies on an internal experience table.
`ft_quest` objects can report completion with `is_complete()` and progress phases via `advance_phase()`.

The project is a work in progress and not every component is documented here.
Consult the individual header files for precise behavior and additional
functionality.
