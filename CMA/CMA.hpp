#ifndef CMA_HPP
# define CMA_HPP

#include <cstddef>
#include <type_traits>
#include "../Libft/libft.hpp"
#include "../CPP_class/class_nullptr.hpp"

void    *cma_malloc(std::size_t size) __attribute__ ((warn_unused_result, hot));
void    cma_free(void* ptr) __attribute__ ((hot));
int     cma_checked_free(void* ptr) __attribute__ ((warn_unused_result, hot));
char    *cma_strdup(const char *string) __attribute__ ((warn_unused_result));
void    *cma_memdup(const void *source, size_t size) __attribute__ ((warn_unused_result));
void    *cma_calloc(std::size_t, std::size_t size) __attribute__ ((warn_unused_result));
void    *cma_realloc(void* ptr, std::size_t new_size) __attribute__ ((warn_unused_result));
void    *cma_aligned_alloc(std::size_t alignment, std::size_t size)
            __attribute__ ((warn_unused_result, hot));
std::size_t    cma_alloc_size(const void* ptr)
            __attribute__ ((warn_unused_result, hot));
int    *cma_atoi(const char *string) __attribute__ ((warn_unused_result));
char    **cma_split(char const *string, char delimiter) __attribute__ ((warn_unused_result));
char    *cma_itoa(int number) __attribute__ ((warn_unused_result));
char    *cma_itoa_base(int number, int base) __attribute__ ((warn_unused_result));
char    *cma_strjoin(char const *string_1, char const *string_2)
            __attribute__ ((warn_unused_result));
template <int total_argument_count, typename... argument_types>
char    *cma_strjoin_multiple_checked(const argument_types &... strings)
            __attribute__ ((warn_unused_result));
char    *cma_substr(const char *source, unsigned int start, size_t length)
            __attribute__ ((warn_unused_result));
char    *cma_strtrim(const char *string, const char *set)
            __attribute__ ((warn_unused_result));
void    cma_free_double(char **content);
void    cma_cleanup();
void    cma_set_alloc_limit(std::size_t limit);
void    cma_set_thread_safety(bool enable);
void    cma_get_stats(std::size_t *allocation_count, std::size_t *free_count);

template <int total_argument_count, typename... argument_types>
char    *cma_strjoin_multiple_checked(const argument_types &... strings)
{
    static_assert(total_argument_count > 1,
        "argument count must be positive");
    static_assert((std::is_convertible_v<argument_types, const char *> && ...),
        "arguments must be const char *");
    static_assert(sizeof...(argument_types) == total_argument_count - 1,
        "argument count mismatch");
    static constexpr int string_count = total_argument_count - 1;
    const char *string_array[string_count] = {strings...};
    int argument_index;
    int result_index;
    int string_length;
    int total_length;
    char *result;

    argument_index = 0;
    total_length = 0;
    while (argument_index < string_count)
    {
        if (string_array[argument_index])
            total_length += ft_strlen(string_array[argument_index]);
        argument_index++;
    }
    result = static_cast<char *>(cma_malloc(total_length + 1));
    if (!result)
        return (ft_nullptr);
    argument_index = 0;
    result_index = 0;
    while (argument_index < string_count)
    {
        if (string_array[argument_index])
        {
            string_length = ft_strlen(string_array[argument_index]);
            ft_memcpy(result + result_index, string_array[argument_index],
                string_length);
            result_index += string_length;
        }
        argument_index++;
    }
    result[result_index] = '\0';
    return (result);
}

#define cma_strjoin_multiple(count, ...) \
    cma_strjoin_multiple_checked<(count) + 1>(__VA_ARGS__)

#endif
