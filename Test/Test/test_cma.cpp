#include "../../CMA/CMA.hpp"
#include "../../Errno/errno.hpp"
#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"
#include <cstdint>
#include <climits>
#include <thread>

static void cma_free_split_result(char **strings)
{
    ft_size_t index;

    if (!strings)
        return ;
    index = 0;
    while (strings[index])
    {
        cma_free(strings[index]);
        index++;
    }
    cma_free(strings);
    return ;
}

int test_cma_checked_free_basic(void)
{
    ft_errno = 0;
    void *p = cma_malloc(32);
    if (!p)
        return (0);
    int r = cma_checked_free(p);
    return (r == 0 && ft_errno == ER_SUCCESS);
}

int test_cma_checked_free_offset(void)
{
    ft_errno = 0;
    char *p = static_cast<char*>(cma_malloc(32));
    if (!p)
        return (0);
    int r = cma_checked_free(p + 10);
    return (r == 0 && ft_errno == ER_SUCCESS);
}

int test_cma_checked_free_invalid(void)
{
    int local;
    ft_errno = 0;
    int r = cma_checked_free(&local);
    return (r == -1 && ft_errno == CMA_INVALID_PTR);
}

FT_TEST(test_cma_calloc_overflow_guard, "cma_calloc rejects overflowed sizes")
{
    ft_size_t allocation_count_before;
    ft_size_t allocation_count_after;
    void *allocated_pointer;

    cma_get_stats(&allocation_count_before, ft_nullptr);
    ft_errno = ER_SUCCESS;
    allocated_pointer = cma_calloc(SIZE_MAX, 2);
    int allocation_errno = ft_errno;
    cma_get_stats(&allocation_count_after, ft_nullptr);
    ft_errno = allocation_errno;
    FT_ASSERT(allocated_pointer == ft_nullptr);
    FT_ASSERT_EQ(allocation_count_before, allocation_count_after);
    FT_ASSERT_EQ(allocation_errno, FT_EINVAL);
    return (1);
}

FT_TEST(test_cma_malloc_zero_size_allocates, "cma_malloc returns a block for zero-size requests")
{
    ft_size_t allocation_count_before;
    ft_size_t allocation_count_after;
    void *allocation_pointer;

    cma_set_alloc_limit(0);
    cma_get_stats(&allocation_count_before, ft_nullptr);
    ft_errno = ER_SUCCESS;
    allocation_pointer = cma_malloc(0);
    if (!allocation_pointer)
        return (0);
    cma_get_stats(&allocation_count_after, ft_nullptr);
    cma_free(allocation_pointer);
    ft_size_t expected_allocation_count = allocation_count_before + 1;
    FT_ASSERT_EQ(expected_allocation_count, allocation_count_after);
    FT_ASSERT_EQ(ft_errno, ER_SUCCESS);
    return (1);
}

FT_TEST(test_cma_realloc_failure_preserves_original_buffer, "cma_realloc keeps the original buffer when growth fails")
{
    char *original_buffer;
    void *realloc_result;
    int byte_index;

    cma_set_alloc_limit(0);
    original_buffer = static_cast<char*>(cma_malloc(16));
    if (!original_buffer)
        return (0);
    ft_memset(original_buffer, 'X', 16);
    cma_set_alloc_limit(8);
    ft_errno = ER_SUCCESS;
    realloc_result = cma_realloc(original_buffer, 32);
    int realloc_errno = ft_errno;
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(ft_nullptr, realloc_result);
    FT_ASSERT_EQ(realloc_errno, FT_EALLOC);
    byte_index = 0;
    while (byte_index < 16)
    {
        if (original_buffer[byte_index] != 'X')
        {
            cma_free(original_buffer);
            return (0);
        }
        byte_index++;
    }
    cma_free(original_buffer);
    return (1);
}

FT_TEST(test_cma_malloc_limit_sets_errno, "cma_malloc reports allocation failures")
{
    void *allocation_pointer;

    cma_set_alloc_limit(8);
    ft_errno = ER_SUCCESS;
    allocation_pointer = cma_malloc(16);
    int allocation_errno = ft_errno;
    cma_set_alloc_limit(0);
    FT_ASSERT(allocation_pointer == ft_nullptr);
    FT_ASSERT_EQ(allocation_errno, FT_EALLOC);
    FT_ASSERT_EQ(ft_errno, FT_EALLOC);
    return (1);
}

FT_TEST(test_cma_malloc_success_sets_errno, "cma_malloc reports success on allocation")
{
    void *allocation_pointer;

    cma_set_alloc_limit(0);
    ft_errno = FT_EALLOC;
    allocation_pointer = cma_malloc(32);
    if (!allocation_pointer)
        return (0);
    FT_ASSERT_EQ(ft_errno, ER_SUCCESS);
    cma_free(allocation_pointer);
    return (1);
}

FT_TEST(test_cma_realloc_success_sets_errno, "cma_realloc reports success on growth")
{
    char *original_buffer;
    void *reallocation_pointer;
    int byte_index;

    cma_set_alloc_limit(0);
    original_buffer = static_cast<char*>(cma_malloc(16));
    if (!original_buffer)
        return (0);
    ft_memset(original_buffer, 'Z', 16);
    ft_errno = FT_EALLOC;
    reallocation_pointer = cma_realloc(original_buffer, 64);
    if (!reallocation_pointer)
    {
        cma_free(original_buffer);
        return (0);
    }
    byte_index = 0;
    while (byte_index < 16)
    {
        if (static_cast<char*>(reallocation_pointer)[byte_index] != 'Z')
        {
            cma_free(reallocation_pointer);
            return (0);
        }
        byte_index++;
    }
    FT_ASSERT_EQ(ft_errno, ER_SUCCESS);
    cma_free(reallocation_pointer);
    return (1);
}

FT_TEST(test_cma_itoa_positive_number, "cma_itoa converts positive numbers")
{
    char    *converted_string;

    cma_set_alloc_limit(0);
    ft_errno = FT_EALLOC;
    converted_string = cma_itoa(12345);
    if (!converted_string)
        return (0);
    FT_ASSERT_EQ(0, ft_strcmp(converted_string, "12345"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free(converted_string);
    return (1);
}

FT_TEST(test_cma_itoa_handles_negative_and_zero,
        "cma_itoa preserves sign and handles zero")
{
    char    *negative_string;
    char    *zero_string;

    cma_set_alloc_limit(0);
    ft_errno = FT_EALLOC;
    negative_string = cma_itoa(-2048);
    if (!negative_string)
        return (0);
    zero_string = cma_itoa(0);
    if (!zero_string)
    {
        cma_free(negative_string);
        return (0);
    }
    FT_ASSERT_EQ(0, ft_strcmp(negative_string, "-2048"));
    FT_ASSERT_EQ(0, ft_strcmp(zero_string, "0"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free(negative_string);
    cma_free(zero_string);
    return (1);
}

FT_TEST(test_cma_itoa_handles_int_min, "cma_itoa duplicates INT_MIN literal")
{
    char    *minimum_string;

    cma_set_alloc_limit(0);
    ft_errno = FT_EALLOC;
    minimum_string = cma_itoa(FT_INT_MIN);
    if (!minimum_string)
        return (0);
    FT_ASSERT_EQ(0, ft_strcmp(minimum_string, "-2147483648"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free(minimum_string);
    return (1);
}

FT_TEST(test_cma_itoa_allocation_failure_sets_errno,
        "cma_itoa reports allocation failures")
{
    char    *converted_string;

    cma_set_alloc_limit(2);
    ft_errno = ER_SUCCESS;
    converted_string = cma_itoa(9999);
    FT_ASSERT_EQ(ft_nullptr, converted_string);
    FT_ASSERT_EQ(FT_EALLOC, ft_errno);
    cma_set_alloc_limit(0);
    return (1);
}

FT_TEST(test_cma_itoa_base_hexadecimal,
        "cma_itoa_base converts numbers in hexadecimal")
{
    char    *converted_string;

    cma_set_alloc_limit(0);
    ft_errno = FT_EALLOC;
    converted_string = cma_itoa_base(305441741, 16);
    if (!converted_string)
        return (0);
    FT_ASSERT_EQ(0, ft_strcmp(converted_string, "1234ABCD"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free(converted_string);
    return (1);
}

FT_TEST(test_cma_itoa_base_negative_decimal,
        "cma_itoa_base prefixes negatives when base is decimal")
{
    char    *converted_string;

    cma_set_alloc_limit(0);
    ft_errno = FT_EALLOC;
    converted_string = cma_itoa_base(-256, 10);
    if (!converted_string)
        return (0);
    FT_ASSERT_EQ(0, ft_strcmp(converted_string, "-256"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free(converted_string);
    return (1);
}

FT_TEST(test_cma_itoa_base_rejects_invalid_base,
        "cma_itoa_base validates base range")
{
    char    *converted_string;

    cma_set_alloc_limit(0);
    ft_errno = ER_SUCCESS;
    converted_string = cma_itoa_base(42, 1);
    FT_ASSERT_EQ(ft_nullptr, converted_string);
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_cma_strtrim_trims_prefix_and_suffix,
        "cma_strtrim removes leading and trailing characters")
{
    char    *trimmed_string;

    cma_set_alloc_limit(0);
    ft_errno = FT_EALLOC;
    trimmed_string = cma_strtrim("***hello***", "*");
    if (!trimmed_string)
        return (0);
    FT_ASSERT_EQ(0, ft_strcmp(trimmed_string, "hello"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free(trimmed_string);
    return (1);
}

FT_TEST(test_cma_strtrim_returns_empty_when_all_trimmed,
        "cma_strtrim returns an empty string when every character is trimmed")
{
    char    *trimmed_string;

    cma_set_alloc_limit(0);
    ft_errno = FT_EALLOC;
    trimmed_string = cma_strtrim("     ", " ");
    if (!trimmed_string)
        return (0);
    FT_ASSERT_EQ(0, ft_strcmp(trimmed_string, ""));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free(trimmed_string);
    return (1);
}

FT_TEST(test_cma_strtrim_rejects_null_input,
        "cma_strtrim returns nullptr when input or set is null")
{
    char    *trimmed_string;

    cma_set_alloc_limit(0);
    ft_errno = ER_SUCCESS;
    trimmed_string = cma_strtrim(ft_nullptr, " ");
    FT_ASSERT_EQ(ft_nullptr, trimmed_string);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    trimmed_string = cma_strtrim("example", ft_nullptr);
    FT_ASSERT_EQ(ft_nullptr, trimmed_string);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_cma_substr_within_bounds,
        "cma_substr extracts segments within the source bounds")
{
    char    *substring;

    cma_set_alloc_limit(0);
    ft_errno = FT_EALLOC;
    substring = cma_substr("abcdef", 2, 3);
    if (!substring)
        return (0);
    FT_ASSERT_EQ(0, ft_strcmp(substring, "cde"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free(substring);
    return (1);
}

FT_TEST(test_cma_substr_truncates_when_length_exceeds,
        "cma_substr truncates when requested length exceeds remaining characters")
{
    char    *substring;

    cma_set_alloc_limit(0);
    ft_errno = FT_EALLOC;
    substring = cma_substr("libft", 3, 10);
    if (!substring)
        return (0);
    FT_ASSERT_EQ(0, ft_strcmp(substring, "ft"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free(substring);
    return (1);
}

FT_TEST(test_cma_substr_handles_out_of_range_start,
        "cma_substr returns an empty string when start is outside the source")
{
    char    *substring;

    cma_set_alloc_limit(0);
    ft_errno = FT_EALLOC;
    substring = cma_substr("short", 42, 3);
    if (!substring)
        return (0);
    FT_ASSERT_EQ(0, ft_strcmp(substring, ""));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free(substring);
    return (1);
}

FT_TEST(test_cma_substr_rejects_null_source,
        "cma_substr returns nullptr when source is null")
{
    char    *substring;

    cma_set_alloc_limit(0);
    ft_errno = ER_SUCCESS;
    substring = cma_substr(ft_nullptr, 0, 1);
    FT_ASSERT_EQ(ft_nullptr, substring);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_cma_split_basic_tokens, "cma_split separates tokens and null-terminates the array")
{
    char        **parts;
    ft_size_t    index;

    cma_set_alloc_limit(0);
    ft_errno = FT_EALLOC;
    parts = cma_split("alpha,beta,gamma", ',');
    if (!parts)
        return (0);
    FT_ASSERT_EQ(0, ft_strcmp(parts[0], "alpha"));
    FT_ASSERT_EQ(0, ft_strcmp(parts[1], "beta"));
    FT_ASSERT_EQ(0, ft_strcmp(parts[2], "gamma"));
    FT_ASSERT_EQ(ft_nullptr, parts[3]);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    index = 0;
    while (parts[index])
    {
        FT_ASSERT(parts[index] != ft_nullptr);
        index++;
    }
    cma_free_split_result(parts);
    return (1);
}

FT_TEST(test_cma_split_skips_repeated_delimiters, "cma_split ignores empty segments between delimiters")
{
    char    **parts;

    cma_set_alloc_limit(0);
    ft_errno = FT_EALLOC;
    parts = cma_split("::left::right:", ':');
    if (!parts)
        return (0);
    FT_ASSERT_EQ(0, ft_strcmp(parts[0], "left"));
    FT_ASSERT_EQ(0, ft_strcmp(parts[1], "right"));
    FT_ASSERT_EQ(ft_nullptr, parts[2]);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free_split_result(parts);
    return (1);
}

FT_TEST(test_cma_split_null_string_returns_empty_array, "cma_split returns an empty array when input is null")
{
    char    **parts;

    cma_set_alloc_limit(0);
    ft_errno = FT_EALLOC;
    parts = cma_split(ft_nullptr, ',');
    if (!parts)
        return (0);
    FT_ASSERT_EQ(ft_nullptr, parts[0]);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free(parts);
    return (1);
}

FT_TEST(test_cma_split_allocation_failure_sets_errno, "cma_split propagates allocation failures")
{
    char    **parts;

    cma_set_alloc_limit(1);
    ft_errno = ER_SUCCESS;
    parts = cma_split("a,b", ',');
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(ft_nullptr, parts);
    FT_ASSERT_EQ(FT_EALLOC, ft_errno);
    return (1);
}

FT_TEST(test_cma_strdup_copies_string, "cma_strdup duplicates the input string")
{
    const char  *source;
    char        *duplicate;

    source = "duplicate target";
    cma_set_alloc_limit(0);
    ft_errno = FT_EALLOC;
    duplicate = cma_strdup(source);
    if (!duplicate)
        return (0);
    FT_ASSERT(duplicate != source);
    FT_ASSERT_EQ(0, ft_strcmp(source, duplicate));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free(duplicate);
    return (1);
}

FT_TEST(test_cma_strdup_null_input_preserves_errno, "cma_strdup returns nullptr without touching errno when input is null")
{
    ft_errno = FT_EINVAL;
    FT_ASSERT_EQ(ft_nullptr, cma_strdup(ft_nullptr));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_cma_strdup_allocation_failure_sets_errno, "cma_strdup surfaces allocation errors")
{
    char    *duplicate;

    cma_set_alloc_limit(1);
    ft_errno = ER_SUCCESS;
    duplicate = cma_strdup("needs space");
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(ft_nullptr, duplicate);
    FT_ASSERT_EQ(FT_EALLOC, ft_errno);
    return (1);
}

FT_TEST(test_cma_memdup_copies_buffer, "cma_memdup duplicates raw bytes")
{
    unsigned char       source[5];
    unsigned char       *duplicate;

    source[0] = 0x10;
    source[1] = 0x20;
    source[2] = 0x30;
    source[3] = 0x40;
    source[4] = 0x50;
    cma_set_alloc_limit(0);
    ft_errno = FT_EALLOC;
    duplicate = static_cast<unsigned char *>(cma_memdup(source, sizeof(source)));
    if (!duplicate)
        return (0);
    FT_ASSERT_EQ(0, ft_memcmp(source, duplicate, sizeof(source)));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free(duplicate);
    return (1);
}

FT_TEST(test_cma_memdup_zero_size_returns_valid_block, "cma_memdup returns a block when size is zero")
{
    unsigned char   source;
    void            *duplicate;

    source = 0xAB;
    cma_set_alloc_limit(0);
    ft_errno = FT_EALLOC;
    duplicate = cma_memdup(&source, 0);
    if (!duplicate)
        return (0);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free(duplicate);
    return (1);
}

FT_TEST(test_cma_memdup_null_source_sets_errno, "cma_memdup rejects null source pointers when size is non-zero")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(ft_nullptr, cma_memdup(ft_nullptr, 4));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_cma_memdup_allocation_failure_sets_errno, "cma_memdup propagates allocation failures")
{
    void    *duplicate;

    cma_set_alloc_limit(1);
    ft_errno = ER_SUCCESS;
    duplicate = cma_memdup("buffer", 16);
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(ft_nullptr, duplicate);
    FT_ASSERT_EQ(FT_EALLOC, ft_errno);
    return (1);
}

FT_TEST(test_cma_alloc_size_reports_block_size, "cma_alloc_size returns the stored allocation size")
{
    void        *buffer;
    ft_size_t   reported_size;

    cma_set_alloc_limit(0);
    buffer = cma_malloc(64);
    if (!buffer)
        return (0);
    ft_errno = FT_EINVAL;
    reported_size = cma_alloc_size(buffer);
    FT_ASSERT_EQ(static_cast<ft_size_t>(64), reported_size);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free(buffer);
    return (1);
}

FT_TEST(test_cma_alloc_size_null_pointer_sets_errno, "cma_alloc_size sets FT_EINVAL for null pointers")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), cma_alloc_size(ft_nullptr));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_cma_alloc_size_rejects_non_cma_pointer, "cma_alloc_size detects pointers outside the allocator")
{
    char        *buffer;
    ft_size_t   reported_size;

    cma_set_alloc_limit(0);
    buffer = static_cast<char *>(cma_malloc(32));
    if (!buffer)
        return (0);
    ft_errno = ER_SUCCESS;
    reported_size = cma_alloc_size(buffer + 1);
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), reported_size);
    FT_ASSERT_EQ(CMA_INVALID_PTR, ft_errno);
    cma_free(buffer);
    return (1);
}

static void cma_thread_success(bool *thread_result)
{
    ft_errno = ER_SUCCESS;
    void *memory_pointer = cma_malloc(32);
    if (!memory_pointer)
    {
        *thread_result = false;
        return ;
    }
    int free_result = cma_checked_free(memory_pointer);
    if (free_result != 0)
    {
        *thread_result = false;
        return ;
    }
    *thread_result = (ft_errno == ER_SUCCESS);
    return ;
}

static void cma_thread_failure(bool *thread_result)
{
    int local_variable;
    ft_errno = ER_SUCCESS;
    int free_result = cma_checked_free(&local_variable);
    *thread_result = (free_result == -1 && ft_errno == CMA_INVALID_PTR);
    return ;
}

int test_cma_thread_local(void)
{
    bool success_result = false;
    bool failure_result = false;
    std::thread success_thread(cma_thread_success, &success_result);
    std::thread failure_thread(cma_thread_failure, &failure_result);
    success_thread.join();
    failure_thread.join();
    return (success_result && failure_result);
}

static void allocation_thread(bool *thread_result)
{
    int allocation_index;

    allocation_index = 0;
    while (allocation_index < 100)
    {
        void *memory_pointer = cma_malloc(64);
        if (!memory_pointer)
        {
            *thread_result = false;
            return ;
        }
        cma_free(memory_pointer);
        allocation_index++;
    }
    *thread_result = true;
    return ;
}

int test_cma_thread_alloc(void)
{
    static const int thread_count = 4;
    bool thread_results[thread_count];
    std::thread threads[thread_count];
    int thread_index;

    cma_set_thread_safety(true);
    thread_index = 0;
    while (thread_index < thread_count)
    {
        thread_results[thread_index] = false;
        threads[thread_index] = std::thread(allocation_thread, &thread_results[thread_index]);
        thread_index++;
    }
    thread_index = 0;
    while (thread_index < thread_count)
    {
        threads[thread_index].join();
        thread_index++;
    }
    thread_index = 0;
    while (thread_index < thread_count)
    {
        if (thread_results[thread_index] == false)
            return (0);
        thread_index++;
    }
    return (1);
}
