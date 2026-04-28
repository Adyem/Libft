#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cstdlib>

struct s_efficiency_test_case
{
    int32_t (*function_pointer)(void);
    const char *name;
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

static int32_t run_efficiency_tests(void)
{
    const char              *name_filter;
    size_t                   index;
    size_t                   selected_count;
    int32_t                  overall_result;
    int32_t                  test_result;

    name_filter = get_name_filter();
    index = 0;
    selected_count = 0;
    overall_result = 0;
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
        test_result = g_efficiency_tests[index].function_pointer();
        if (test_result == 0)
            overall_result = 1;
        std::printf("\n");
        index++;
    }
    if (selected_count == 0)
    {
        std::fprintf(stderr, "No efficiency tests matched FT_TEST_NAME_FILTER.\n");
        return (1);
    }
    return (overall_result);
}

int main(void)
{
    return (run_efficiency_tests());
}
