#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cinttypes>
#include "../../Modules/CMA/CMA.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "utils.hpp"
#include "../../Modules/Basic/limits.hpp"

struct s_efficiency_test_case
{
    int32_t (*function_pointer)(void);
    const char *name;
};

struct s_benchmark_allocator_snapshot
{
    ft_size_t allocation_count;
    ft_size_t free_count;
    ft_size_t current_bytes;
    ft_size_t peak_bytes;
    ft_bool available;
};

struct s_benchmark_result
{
    const char *name;
    int64_t elapsed_microseconds;
    ft_size_t allocation_delta;
    ft_size_t free_delta;
    ft_size_t current_bytes_delta;
    ft_size_t peak_bytes_delta;
    ft_bool allocator_stats_available;
    int32_t passed;
};

int test_efficiency_abs(void);
int test_efficiency_atoi(void);
int test_efficiency_atol(void);
int test_efficiency_bzero(void);
int test_efficiency_clamp(void);
int test_efficiency_cma_calloc(void);
int test_efficiency_cma_malloc(void);
int test_efficiency_cma_memdup(void);
int test_efficiency_cma_realloc(void);
int test_efficiency_cma_strdup(void);
int test_efficiency_exp(void);
int test_efficiency_isalnum(void);
int test_efficiency_isalpha(void);
int test_efficiency_isdigit(void);
int test_efficiency_isspace(void);
int test_efficiency_map_insert_find(void);
int test_efficiency_map_insert_remove(void);
int test_efficiency_map_iterate(void);
int test_efficiency_map_copy(void);
int test_efficiency_map_move(void);
int test_efficiency_map_swap(void);
int test_efficiency_map_clear(void);
int test_efficiency_memchr(void);
int test_efficiency_memcmp(void);
int test_efficiency_memcpy(void);
int test_efficiency_memmove(void);
int test_efficiency_memset(void);
int test_efficiency_pair_create_copy(void);
int test_efficiency_pair_move(void);
int test_efficiency_pair_swap(void);
int test_efficiency_pool_acquire_release(void);
int test_efficiency_pow(void);
int test_efficiency_printf(void);
int test_efficiency_promise_set_get(void);
int test_efficiency_queue_enqueue_dequeue(void);
int test_efficiency_queue_interleaved(void);
int test_efficiency_queue_move(void);
int test_efficiency_queue_swap(void);
int test_efficiency_rwlock_reader_priority(void);
int test_efficiency_rwlock_writer_priority(void);
int test_efficiency_shared_ptr_create(void);
int test_efficiency_shared_ptr_copy(void);
int test_efficiency_shared_ptr_move(void);
int test_efficiency_shared_ptr_reset(void);
int test_efficiency_shared_ptr_use_count(void);
int test_efficiency_shared_ptr_swap(void);
int test_efficiency_sqrt(void);
int test_efficiency_stack_push_pop(void);
int test_efficiency_stack_interleaved(void);
int test_efficiency_stack_move(void);
int test_efficiency_stack_swap(void);
int test_efficiency_strchr(void);
int test_efficiency_strcmp(void);
int test_efficiency_string_create(void);
int test_efficiency_string_copy(void);
int test_efficiency_string_move(void);
int test_efficiency_string_append(void);
int test_efficiency_strlen(void);
int test_efficiency_strncmp(void);
int test_efficiency_strrchr(void);
int test_efficiency_swap_large(void);
int test_efficiency_unique_ptr_create(void);
int test_efficiency_unique_ptr_move(void);
int test_efficiency_unordered_map_insert_find(void);
int test_efficiency_unordered_map_insert_erase(void);
int test_efficiency_unordered_map_iterate(void);
int test_efficiency_unordered_map_copy(void);
int test_efficiency_unordered_map_move(void);
int test_efficiency_unordered_map_clear(void);
int test_efficiency_unordered_map_swap(void);
int test_efficiency_vector_push_back(void);
int test_efficiency_vector_insert_erase(void);
int test_efficiency_vector_reserve_resize(void);
int test_efficiency_vector_clear(void);
int test_efficiency_vector_iterate(void);
int test_efficiency_vector_move(void);
int test_efficiency_vector_swap(void);

static const s_efficiency_test_case g_efficiency_tests[] =
{
    {&test_efficiency_abs, "test_efficiency_abs"},
    {&test_efficiency_atoi, "test_efficiency_atoi"},
    {&test_efficiency_atol, "test_efficiency_atol"},
    {&test_efficiency_bzero, "test_efficiency_bzero"},
    {&test_efficiency_clamp, "test_efficiency_clamp"},
    {&test_efficiency_cma_calloc, "test_efficiency_cma_calloc"},
    {&test_efficiency_cma_malloc, "test_efficiency_cma_malloc"},
    {&test_efficiency_cma_memdup, "test_efficiency_cma_memdup"},
    {&test_efficiency_cma_realloc, "test_efficiency_cma_realloc"},
    {&test_efficiency_cma_strdup, "test_efficiency_cma_strdup"},
    {&test_efficiency_exp, "test_efficiency_exp"},
    {&test_efficiency_isalnum, "test_efficiency_isalnum"},
    {&test_efficiency_isalpha, "test_efficiency_isalpha"},
    {&test_efficiency_isdigit, "test_efficiency_isdigit"},
    {&test_efficiency_isspace, "test_efficiency_isspace"},
    {&test_efficiency_map_insert_find, "test_efficiency_map_insert_find"},
    {&test_efficiency_map_insert_remove, "test_efficiency_map_insert_remove"},
    {&test_efficiency_map_iterate, "test_efficiency_map_iterate"},
    {&test_efficiency_map_copy, "test_efficiency_map_copy"},
    {&test_efficiency_map_move, "test_efficiency_map_move"},
    {&test_efficiency_map_swap, "test_efficiency_map_swap"},
    {&test_efficiency_map_clear, "test_efficiency_map_clear"},
    {&test_efficiency_memchr, "test_efficiency_memchr"},
    {&test_efficiency_memcmp, "test_efficiency_memcmp"},
    {&test_efficiency_memcpy, "test_efficiency_memcpy"},
    {&test_efficiency_memmove, "test_efficiency_memmove"},
    {&test_efficiency_memset, "test_efficiency_memset"},
    {&test_efficiency_pair_create_copy, "test_efficiency_pair_create_copy"},
    {&test_efficiency_pair_move, "test_efficiency_pair_move"},
    {&test_efficiency_pair_swap, "test_efficiency_pair_swap"},
    {&test_efficiency_pool_acquire_release, "test_efficiency_pool_acquire_release"},
    {&test_efficiency_pow, "test_efficiency_pow"},
    {&test_efficiency_printf, "test_efficiency_printf"},
    {&test_efficiency_promise_set_get, "test_efficiency_promise_set_get"},
    {&test_efficiency_queue_enqueue_dequeue, "test_efficiency_queue_enqueue_dequeue"},
    {&test_efficiency_queue_interleaved, "test_efficiency_queue_interleaved"},
    {&test_efficiency_queue_move, "test_efficiency_queue_move"},
    {&test_efficiency_queue_swap, "test_efficiency_queue_swap"},
    {&test_efficiency_rwlock_reader_priority, "test_efficiency_rwlock_reader_priority"},
    {&test_efficiency_rwlock_writer_priority, "test_efficiency_rwlock_writer_priority"},
    {&test_efficiency_shared_ptr_create, "test_efficiency_shared_ptr_create"},
    {&test_efficiency_shared_ptr_copy, "test_efficiency_shared_ptr_copy"},
    {&test_efficiency_shared_ptr_move, "test_efficiency_shared_ptr_move"},
    {&test_efficiency_shared_ptr_reset, "test_efficiency_shared_ptr_reset"},
    {&test_efficiency_shared_ptr_use_count, "test_efficiency_shared_ptr_use_count"},
    {&test_efficiency_shared_ptr_swap, "test_efficiency_shared_ptr_swap"},
    {&test_efficiency_sqrt, "test_efficiency_sqrt"},
    {&test_efficiency_stack_push_pop, "test_efficiency_stack_push_pop"},
    {&test_efficiency_stack_interleaved, "test_efficiency_stack_interleaved"},
    {&test_efficiency_stack_move, "test_efficiency_stack_move"},
    {&test_efficiency_stack_swap, "test_efficiency_stack_swap"},
    {&test_efficiency_strchr, "test_efficiency_strchr"},
    {&test_efficiency_strcmp, "test_efficiency_strcmp"},
    {&test_efficiency_string_create, "test_efficiency_string_create"},
    {&test_efficiency_string_copy, "test_efficiency_string_copy"},
    {&test_efficiency_string_move, "test_efficiency_string_move"},
    {&test_efficiency_string_append, "test_efficiency_string_append"},
    {&test_efficiency_strlen, "test_efficiency_strlen"},
    {&test_efficiency_strncmp, "test_efficiency_strncmp"},
    {&test_efficiency_strrchr, "test_efficiency_strrchr"},
    {&test_efficiency_swap_large, "test_efficiency_swap_large"},
    {&test_efficiency_unique_ptr_create, "test_efficiency_unique_ptr_create"},
    {&test_efficiency_unique_ptr_move, "test_efficiency_unique_ptr_move"},
    {&test_efficiency_unordered_map_insert_find, "test_efficiency_unordered_map_insert_find"},
    {&test_efficiency_unordered_map_insert_erase, "test_efficiency_unordered_map_insert_erase"},
    {&test_efficiency_unordered_map_iterate, "test_efficiency_unordered_map_iterate"},
    {&test_efficiency_unordered_map_copy, "test_efficiency_unordered_map_copy"},
    {&test_efficiency_unordered_map_move, "test_efficiency_unordered_map_move"},
    {&test_efficiency_unordered_map_clear, "test_efficiency_unordered_map_clear"},
    {&test_efficiency_unordered_map_swap, "test_efficiency_unordered_map_swap"},
    {&test_efficiency_vector_push_back, "test_efficiency_vector_push_back"},
    {&test_efficiency_vector_insert_erase, "test_efficiency_vector_insert_erase"},
    {&test_efficiency_vector_reserve_resize, "test_efficiency_vector_reserve_resize"},
    {&test_efficiency_vector_clear, "test_efficiency_vector_clear"},
    {&test_efficiency_vector_iterate, "test_efficiency_vector_iterate"},
    {&test_efficiency_vector_move, "test_efficiency_vector_move"},
    {&test_efficiency_vector_swap, "test_efficiency_vector_swap"}
};

static const char *get_name_filter(void)
{
    const char *filter;

    filter = std::getenv("FT_TEST_NAME_FILTER");
    if (filter == NULL || filter[0] == '\0')
        return (NULL);
    return (filter);
}

static int32_t name_matches_filter(const char *filter, const char *name)
{
    const char *cursor;
    const char *start;
    size_t      length;

    cursor = filter;
    while (*cursor != '\0')
    {
        while (*cursor == ' ' || *cursor == ',')
            cursor++;
        start = cursor;
        while (*cursor != '\0' && *cursor != ',')
            cursor++;
        length = static_cast<size_t>(cursor - start);
        if (length > 0
            && std::strncmp(start, name, length) == 0
            && name[length] == '\0')
            return (1);
        if (*cursor == ',')
            cursor++;
    }
    return (0);
}

static s_benchmark_allocator_snapshot benchmark_capture_allocator_snapshot(void)
{
    s_benchmark_allocator_snapshot snapshot;
    int32_t error_code;

    snapshot.allocation_count = 0;
    snapshot.free_count = 0;
    snapshot.current_bytes = 0;
    snapshot.peak_bytes = 0;
    snapshot.available = FT_FALSE;
    error_code = cma_get_extended_stats(&snapshot.allocation_count,
            &snapshot.free_count, &snapshot.current_bytes,
            &snapshot.peak_bytes);
    if (error_code == FT_ERR_SUCCESS)
        snapshot.available = FT_TRUE;
    return (snapshot);
}

static ft_size_t benchmark_delta(ft_size_t before, ft_size_t after)
{
    if (after >= before)
        return (after - before);
    return (0);
}

static s_benchmark_result run_benchmark_case(
        const s_efficiency_test_case *test_case)
{
    s_benchmark_result result;
    s_benchmark_allocator_snapshot before_snapshot;
    s_benchmark_allocator_snapshot after_snapshot;
    clock_type::time_point start_time;
    clock_type::time_point end_time;
    int32_t test_result;

    result.name = test_case->name;
    result.elapsed_microseconds = 0;
    result.allocation_delta = 0;
    result.free_delta = 0;
    result.current_bytes_delta = 0;
    result.peak_bytes_delta = 0;
    result.allocator_stats_available = FT_FALSE;
    result.passed = 0;
    before_snapshot = benchmark_capture_allocator_snapshot();
    start_time = clock_type::now();
    test_result = test_case->function_pointer();
    end_time = clock_type::now();
    after_snapshot = benchmark_capture_allocator_snapshot();
    result.elapsed_microseconds = elapsed_us(start_time, end_time);
    result.passed = test_result;
    if (before_snapshot.available == FT_TRUE
        && after_snapshot.available == FT_TRUE)
    {
        result.allocator_stats_available = FT_TRUE;
        result.allocation_delta = benchmark_delta(
                before_snapshot.allocation_count,
                after_snapshot.allocation_count);
        result.free_delta = benchmark_delta(before_snapshot.free_count,
                after_snapshot.free_count);
        result.current_bytes_delta = benchmark_delta(
                before_snapshot.current_bytes, after_snapshot.current_bytes);
        result.peak_bytes_delta = benchmark_delta(before_snapshot.peak_bytes,
                after_snapshot.peak_bytes);
    }
    return (result);
}

static void print_benchmark_result(const s_benchmark_result *result)
{
    const char *status;

    status = "FAIL";
    if (result->passed != 0)
        status = "PASS";
    if (result->allocator_stats_available == FT_TRUE)
    {
        std::printf("[BENCHMARK] %s status=%s elapsed=%" PRId64
            " us allocs=%" PRIu64 " frees=%" PRIu64
            " current_bytes_delta=%" PRIu64 " peak_bytes_delta=%" PRIu64 "\n",
            result->name, status, result->elapsed_microseconds,
            result->allocation_delta, result->free_delta,
            result->current_bytes_delta, result->peak_bytes_delta);
        return ;
    }
    std::printf("[BENCHMARK] %s status=%s elapsed=%" PRId64
        " us allocator_stats=unavailable\n", result->name, status,
        result->elapsed_microseconds);
    return ;
}

static int32_t run_efficiency_tests(void)
{
    const char              *name_filter;
    size_t                   index;
    size_t                   selected_count;
    int32_t                  overall_result;
    s_benchmark_result       benchmark_result;
    int64_t                  total_elapsed_microseconds;
    ft_size_t                total_allocation_delta;
    ft_size_t                total_free_delta;

    name_filter = get_name_filter();
    index = 0;
    selected_count = 0;
    overall_result = 0;
    total_elapsed_microseconds = 0;
    total_allocation_delta = 0;
    total_free_delta = 0;
    while (index < (sizeof(g_efficiency_tests) / sizeof(g_efficiency_tests[0])))
    {
        if (name_filter != NULL
            && name_matches_filter(name_filter, g_efficiency_tests[index].name) == 0)
        {
            index++;
            continue ;
        }
        selected_count++;
        std::printf("[EFFICIENCY] %s\n", g_efficiency_tests[index].name);
        benchmark_result = run_benchmark_case(&g_efficiency_tests[index]);
        if (benchmark_result.passed == 0)
            overall_result = 1;
        print_benchmark_result(&benchmark_result);
        total_elapsed_microseconds += benchmark_result.elapsed_microseconds;
        if (benchmark_result.allocator_stats_available == FT_TRUE)
        {
            total_allocation_delta += benchmark_result.allocation_delta;
            total_free_delta += benchmark_result.free_delta;
        }
        std::printf("\n");
        index++;
    }
    if (selected_count == 0)
    {
        std::fprintf(stderr, "No efficiency tests matched FT_TEST_NAME_FILTER.\n");
        return (1);
    }
    std::printf("[BENCHMARK] summary selected=%zu"
        " elapsed=%" PRId64 " us allocs=%" PRIu64 " frees=%" PRIu64 "\n",
        selected_count, total_elapsed_microseconds, total_allocation_delta,
        total_free_delta);
    return (overall_result);
}

int main(void)
{
    return (run_efficiency_tests());
}
