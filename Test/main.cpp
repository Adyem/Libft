#include "../Libft/libft.hpp"
#include "../Printf/printf.hpp"
#include "../CMA/CMA.hpp"
#include "../ReadLine/readline.hpp"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <sstream>
#include <string>
#include <vector>

struct s_test
{
    int (*func)(void);
    const char *description;
};

static void run_test(int index, const s_test *test, int *passed)
{
    if (test->func())
    {
        pf_printf("OK %d %s\n", index, test->description);
        (*passed)++;
        return ;
    }
    pf_printf("KO %d %s\n", index, test->description);
    return ;
}

struct s_perf_test
{
    int (*func)(void);
    const char *description;
};

static void run_efficiency_test(int index, const s_perf_test *test)
{
    pf_printf("%d) %s\n", index, test->description);
    test->func();
    return ;
}

int test_strlen_nullptr(void);
int test_strlen_simple(void);
int test_strlen_long(void);
int test_strcmp_equal(void);
int test_strcmp_null(void);
int test_isdigit_true(void);
int test_isdigit_false(void);
int test_memset_null(void);
int test_memset_basic(void);
int test_memset_large(void);
int test_toupper_basic(void);
int test_atoi_simple(void);
int test_atoi_negative(void);
int test_atoi_intmax(void);
int test_atoi_intmin(void);
int test_html_create_node(void);
int test_html_find_by_tag(void);
int test_html_write_to_string(void);
int test_html_find_by_attr(void);
int test_network_send_receive(void);
int test_network_invalid_ip(void);
int test_network_send_uninitialized(void);
int test_strlen_size_t_null(void);
int test_strlen_size_t_basic(void);
int test_strlen_size_t_long(void);
int test_strlen_size_t_empty(void);
int test_bzero_basic(void);
int test_bzero_zero(void);
int test_memcpy_basic(void);
int test_memcpy_null(void);
int test_memcpy_partial(void);
int test_memcpy_large(void);
int test_memmove_overlap(void);
int test_memmove_no_overlap(void);
int test_memchr_found(void);
int test_memchr_not_found(void);
int test_memcmp_basic(void);
int test_memcmp_diff(void);
int test_strchr_basic(void);
int test_strchr_not_found(void);
int test_strchr_first_occurrence(void);
int test_strrchr_basic(void);
int test_strrchr_not_found(void);
int test_strnstr_basic(void);
int test_strnstr_not_found(void);
int test_strlcpy_basic(void);
int test_strlcpy_truncate(void);
int test_strlcat_basic(void);
int test_strlcat_truncate(void);
int test_strncmp_basic(void);
int test_strncmp_diff(void);
int test_strncpy_basic(void);
int test_strncpy_padding(void);
int test_isalpha_true(void);
int test_isalpha_lower_true(void);
int test_isalpha_false(void);
int test_isalnum_true(void);
int test_isalnum_false(void);
int test_isspace_true(void);
int test_isspace_false(void);
int test_tolower_basic(void);
int test_tolower_no_change(void);
int test_abs_basic(void);
int test_abs_zero(void);
int test_abs_positive(void);
int test_abs_int_min(void);
int test_abs_llong_min(void);
int test_fabs_negative_zero(void);
int test_fabs_nan(void);
int test_atol_basic(void);
int test_atol_whitespace(void);
int test_atol_longmax(void);
int test_atol_longmin(void);
int test_setenv_getenv_basic(void);
int test_setenv_no_overwrite(void);
int test_ft_string_append(void);
int test_ft_string_concat(void);
int test_data_buffer_io(void);
int test_ft_file_write_read(void);
int test_ft_vector_push_back(void);
int test_ft_vector_insert_erase(void);
int test_ft_vector_reserve_resize(void);
int test_ft_vector_clear(void);
int test_ft_vector_vs_std_push_back(void);
int test_ft_vector_vs_std_insert_erase(void);
int test_ft_vector_vs_std_reserve_resize(void);
int test_ft_map_insert_find(void);
int test_ft_map_remove(void);
int test_ft_map_at(void);
int test_ft_map_at_missing(void);
int test_ft_map_clear_empty(void);
int test_ft_shared_ptr_basic(void);
int test_ft_shared_ptr_array(void);
int test_ft_shared_ptr_reset(void);
int test_ft_unique_ptr_basic(void);
int test_ft_unique_ptr_array(void);
int test_ft_unique_ptr_release(void);
int test_ft_unique_ptr_swap(void);
int test_pf_printf_basic(void);
int test_pf_printf_misc(void);
int test_pf_printf_bool(void);
int test_pf_printf_nullptr(void);
int test_pf_printf_modifiers(void);
int test_get_next_line_basic(void);
int test_get_next_line_empty(void);
int test_ft_open_and_read_file(void);
int test_cma_checked_free_basic(void);
int test_cma_checked_free_offset(void);
int test_cma_checked_free_invalid(void);
int test_game_simulation(void);
int test_item_basic(void);
int test_inventory_count(void);
int test_inventory_full(void);
int test_character_valor(void);
int test_character_add_sub_coins(void);
int test_character_add_sub_valor(void);
int test_buff_subtracters(void);
int test_debuff_subtracters(void);
int test_event_subtracters(void);
int test_upgrade_subtracters(void);
int test_item_stack_subtract(void);
int test_reputation_subtracters(void);
int test_character_level(void);
int test_quest_progress(void);
int test_queue_basic(void);
int test_ft_queue_class_basic(void);
int test_ft_promise_set_get(void);
int test_ft_promise_not_ready(void);
int test_config_basic(void);
int test_config_missing_value(void);
int test_pt_async_basic(void);

int test_efficiency_strlen(void);
int test_efficiency_memcpy(void);
int test_efficiency_memmove(void);
int test_efficiency_memset(void);
int test_efficiency_strcmp(void);
int test_efficiency_isdigit(void);
int test_efficiency_bzero(void);
int test_efficiency_memcmp(void);
int test_efficiency_strchr(void);
int test_efficiency_strncmp(void);
int test_efficiency_isalpha(void);
int test_efficiency_isalnum(void);
int test_efficiency_memchr(void);
int test_efficiency_strrchr(void);
int test_efficiency_isspace(void);
int test_efficiency_abs(void);
int test_efficiency_atoi(void);
int test_efficiency_atol(void);
int test_efficiency_cma_malloc(void);
int test_efficiency_cma_calloc(void);
int test_efficiency_cma_strdup(void);
int test_efficiency_cma_memdup(void);
int test_efficiency_cma_realloc(void);
int test_efficiency_vector_push_back(void);
int test_efficiency_vector_insert_erase(void);
int test_efficiency_vector_reserve_resize(void);
int test_efficiency_vector_clear(void);
int test_efficiency_vector_iterate(void);
int test_efficiency_vector_move(void);
int test_efficiency_vector_swap(void);
int test_efficiency_map_insert_find(void);
int test_efficiency_map_insert_remove(void);
int test_efficiency_map_iterate(void);
int test_efficiency_map_copy(void);
int test_efficiency_map_move(void);
int test_efficiency_map_swap(void);
int test_efficiency_map_clear(void);
int test_efficiency_shared_ptr_create(void);
int test_efficiency_shared_ptr_copy(void);
int test_efficiency_shared_ptr_move(void);
int test_efficiency_shared_ptr_reset(void);
int test_efficiency_shared_ptr_use_count(void);
int test_efficiency_shared_ptr_swap(void);
int test_efficiency_string_create(void);
int test_efficiency_string_copy(void);
int test_efficiency_string_move(void);
int test_efficiency_string_append(void);
int test_efficiency_unique_ptr_create(void);
int test_efficiency_unique_ptr_move(void);
int test_efficiency_stack_push_pop(void);
int test_efficiency_stack_interleaved(void);
int test_efficiency_stack_move(void);
int test_efficiency_stack_swap(void);
int test_efficiency_queue_enqueue_dequeue(void);
int test_efficiency_queue_interleaved(void);
int test_efficiency_queue_move(void);
int test_efficiency_queue_swap(void);
int test_efficiency_unord_map_insert_find(void);
int test_efficiency_unord_map_insert_remove(void);
int test_efficiency_unord_map_iterate(void);
int test_efficiency_unord_map_copy(void);
int test_efficiency_unord_map_move(void);
int test_efficiency_unord_map_swap(void);
int test_efficiency_unord_map_clear(void);
int test_efficiency_pair_create_copy(void);
int test_efficiency_pair_move(void);
int test_efficiency_pair_swap(void);
int test_efficiency_promise_set_get(void);
int test_efficiency_pow(void);
int test_efficiency_sqrt(void);
int test_efficiency_exp(void);
int test_efficiency_clamp(void);
int test_efficiency_pool_acquire_release(void);
int test_efficiency_swap_large(void);
int test_efficiency_mutex_lock(void);
int test_efficiency_printf(void);

int main(int argc, char **argv)
{
    const s_test tests[] = {
        { test_strlen_nullptr, "strlen nullptr" },
        { test_strlen_simple, "strlen simple" },
        { test_strlen_long, "strlen long" },
        { test_strcmp_equal, "strcmp equal" },
        { test_strcmp_null, "strcmp null" },
        { test_isdigit_true, "isdigit true" },
        { test_isdigit_false, "isdigit false" },
        { test_memset_null, "memset null" },
        { test_memset_basic, "memset basic" },
        { test_memset_large, "memset large" },
        { test_toupper_basic, "toupper basic" },
        { test_atoi_simple, "atoi simple" },
        { test_atoi_negative, "atoi negative" },
        { test_atoi_intmax, "atoi intmax" },
        { test_atoi_intmin, "atoi intmin" },
        { test_html_create_node, "html create node" },
        { test_html_find_by_tag, "html find by tag" },
        { test_html_write_to_string, "html write to string" },
        { test_html_find_by_attr, "html find by attr" },
        { test_network_send_receive, "network send/receive" },
        { test_network_invalid_ip, "network invalid ip" },
        { test_network_send_uninitialized, "network send uninitialized" },
        { test_strlen_size_t_null, "strlen_size_t null" },
        { test_strlen_size_t_basic, "strlen_size_t basic" },
        { test_strlen_size_t_long, "strlen_size_t long" },
        { test_bzero_basic, "bzero basic" },
        { test_memcpy_basic, "memcpy basic" },
        { test_memcpy_null, "memcpy null" },
        { test_memmove_overlap, "memmove overlap" },
        { test_memchr_found, "memchr found" },
        { test_memcmp_basic, "memcmp basic" },
        { test_strchr_basic, "strchr basic" },
        { test_strchr_first_occurrence, "strchr first occurrence" },
        { test_strrchr_basic, "strrchr basic" },
        { test_strnstr_basic, "strnstr basic" },
        { test_strlcpy_basic, "strlcpy basic" },
        { test_strlcat_basic, "strlcat basic" },
        { test_strncmp_basic, "strncmp basic" },
        { test_strncpy_basic, "strncpy basic" },
        { test_isalpha_true, "isalpha true" },
        { test_isalpha_false, "isalpha false" },
        { test_tolower_basic, "tolower basic" },
        { test_tolower_no_change, "tolower no change" },
        { test_abs_basic, "abs basic" },
        { test_abs_zero, "abs zero" },
        { test_abs_positive, "abs positive" },
        { test_abs_int_min, "abs INT_MIN" },
        { test_abs_llong_min, "abs LLONG_MIN" },
        { test_fabs_negative_zero, "fabs negative zero" },
        { test_fabs_nan, "fabs NaN" },
        { test_atol_basic, "atol basic" },
        { test_atol_whitespace, "atol whitespace" },
        { test_atol_longmax, "atol longmax" },
        { test_atol_longmin, "atol longmin" },
        { test_setenv_getenv_basic, "setenv/getenv basic" },
        { test_setenv_no_overwrite, "setenv no overwrite" },
        { test_strlen_size_t_empty, "strlen_size_t empty" },
        { test_bzero_zero, "bzero zero" },
        { test_memcpy_partial, "memcpy partial" },
        { test_memcpy_large, "memcpy large" },
        { test_memmove_no_overlap, "memmove no overlap" },
        { test_memchr_not_found, "memchr not found" },
        { test_memcmp_diff, "memcmp diff" },
        { test_strchr_not_found, "strchr not found" },
        { test_strrchr_not_found, "strrchr not found" },
        { test_strnstr_not_found, "strnstr not found" },
        { test_strlcpy_truncate, "strlcpy truncate" },
        { test_strlcat_truncate, "strlcat truncate" },
        { test_strncmp_diff, "strncmp diff" },
        { test_strncpy_padding, "strncpy padding" },
        { test_isalpha_lower_true, "isalpha lower" },
        { test_isalnum_true, "isalnum true" },
        { test_isalnum_false, "isalnum false" },
        { test_isspace_true, "isspace true" },
        { test_isspace_false, "isspace false" },
        { test_ft_string_append, "ft_string append" },
        { test_ft_string_concat, "ft_string concat" },
        { test_data_buffer_io, "DataBuffer io" },
        { test_ft_file_write_read, "ft_file write/read" },
        { test_ft_vector_push_back, "ft_vector push_back" },
        { test_ft_vector_insert_erase, "ft_vector insert/erase" },
        { test_ft_vector_reserve_resize, "ft_vector reserve/resize" },
        { test_ft_vector_clear, "ft_vector clear" },
        { test_ft_vector_vs_std_push_back, "ft_vector vs std::vector push_back" },
        { test_ft_vector_vs_std_insert_erase, "ft_vector vs std::vector insert/erase" },
        { test_ft_vector_vs_std_reserve_resize, "ft_vector vs std::vector reserve/resize" },
        { test_ft_map_insert_find, "ft_map insert/find" },
        { test_ft_map_remove, "ft_map remove" },
        { test_ft_map_at, "ft_map at" },
        { test_ft_map_at_missing, "ft_map at missing" },
        { test_ft_map_clear_empty, "ft_map clear/empty" },
        { test_ft_shared_ptr_basic, "ft_sharedptr basic" },
        { test_ft_shared_ptr_array, "ft_sharedptr array" },
        { test_ft_shared_ptr_reset, "ft_sharedptr reset" },
        { test_ft_unique_ptr_basic, "ft_uniqueptr basic" },
        { test_ft_unique_ptr_array, "ft_uniqueptr array" },
        { test_ft_unique_ptr_release, "ft_uniqueptr release" },
        { test_ft_unique_ptr_swap, "ft_uniqueptr swap" },
        { test_pf_printf_basic, "pf_printf basic" },
        { test_pf_printf_misc, "pf_printf misc" },
        { test_pf_printf_bool, "pf_printf bool" },
        { test_pf_printf_nullptr, "pf_printf nullptr" },
        { test_pf_printf_modifiers, "pf_printf modifiers" },
        { test_get_next_line_basic, "get_next_line basic" },
        { test_get_next_line_empty, "get_next_line empty" },
        { test_ft_open_and_read_file, "open_and_read_file" },
        { test_cma_checked_free_basic, "cma_checked_free basic" },
        { test_cma_checked_free_offset, "cma_checked_free offset" },
        { test_cma_checked_free_invalid, "cma_checked_free invalid" },
        { test_game_simulation, "game simulation" },
        { test_item_basic, "item basic" },
        { test_inventory_count, "inventory count" },
        { test_inventory_full, "inventory full" },
        { test_character_valor, "character valor" },
        { test_character_add_sub_coins, "character coin add/sub" },
        { test_character_add_sub_valor, "character valor add/sub" },
        { test_buff_subtracters, "buff subtracters" },
        { test_debuff_subtracters, "debuff subtracters" },
        { test_event_subtracters, "event subtracters" },
        { test_upgrade_subtracters, "upgrade subtracters" },
        { test_item_stack_subtract, "item stack subtract" },
        { test_reputation_subtracters, "reputation subtracters" },
        { test_character_level, "character level" },
        { test_quest_progress, "quest progress" },
        { test_queue_basic, "queue basic" },
        { test_ft_queue_class_basic, "queue class basic" },
        { test_ft_promise_set_get, "ft_promise set/get" },
        { test_ft_promise_not_ready, "ft_promise not ready" },
        { test_config_basic, "config basic" },
        { test_config_missing_value, "config missing value" },
        { test_pt_async_basic, "pt_async basic" }
    };

    const s_perf_test perf_tests[] = {
        { test_efficiency_strlen, "strlen" },
        { test_efficiency_memcpy, "memcpy" },
        { test_efficiency_memmove, "memmove" },
        { test_efficiency_memset, "memset" },
        { test_efficiency_strcmp, "strcmp" },
        { test_efficiency_isdigit, "isdigit" },
        { test_efficiency_bzero, "bzero" },
        { test_efficiency_memcmp, "memcmp" },
        { test_efficiency_strchr, "strchr" },
        { test_efficiency_strncmp, "strncmp" },
        { test_efficiency_isalpha, "isalpha" },
        { test_efficiency_isalnum, "isalnum" },
        { test_efficiency_memchr, "memchr" },
        { test_efficiency_strrchr, "strrchr" },
        { test_efficiency_isspace, "isspace" },
        { test_efficiency_abs, "abs" },
        { test_efficiency_atoi, "atoi" },
        { test_efficiency_atol, "atol" },
        { test_efficiency_cma_malloc, "cma_malloc" },
        { test_efficiency_cma_calloc, "cma_calloc" },
        { test_efficiency_cma_strdup, "cma_strdup" },
        { test_efficiency_cma_memdup, "cma_memdup" },
        { test_efficiency_cma_realloc, "cma_realloc" },
        { test_efficiency_vector_push_back, "ft_vector push_back" },
        { test_efficiency_vector_insert_erase, "ft_vector insert/erase" },
        { test_efficiency_vector_reserve_resize, "ft_vector reserve/resize" },
        { test_efficiency_vector_clear, "ft_vector clear" },
        { test_efficiency_vector_iterate, "ft_vector iterate" },
        { test_efficiency_vector_move, "ft_vector move" },
        { test_efficiency_vector_swap, "ft_vector swap" },
        { test_efficiency_map_insert_find, "ft_map insert/find" },
        { test_efficiency_map_insert_remove, "ft_map insert/remove" },
        { test_efficiency_map_iterate, "ft_map iterate" },
        { test_efficiency_map_copy, "ft_map copy" },
        { test_efficiency_map_move, "ft_map move" },
        { test_efficiency_map_swap, "ft_map swap" },
        { test_efficiency_map_clear, "ft_map clear" },
        { test_efficiency_shared_ptr_create, "ft_sharedptr create" },
        { test_efficiency_shared_ptr_copy, "ft_sharedptr copy" },
        { test_efficiency_shared_ptr_move, "ft_sharedptr move" },
        { test_efficiency_shared_ptr_reset, "ft_sharedptr reset" },
        { test_efficiency_shared_ptr_use_count, "ft_sharedptr use_count" },
        { test_efficiency_shared_ptr_swap, "ft_sharedptr swap" },
        { test_efficiency_string_create, "ft_string create" },
        { test_efficiency_string_copy, "ft_string copy" },
        { test_efficiency_string_move, "ft_string move" },
        { test_efficiency_string_append, "ft_string append" },
        { test_efficiency_unique_ptr_create, "ft_uniqueptr create" },
        { test_efficiency_unique_ptr_move, "ft_uniqueptr move" },
        { test_efficiency_stack_push_pop, "ft_stack push/pop" },
        { test_efficiency_stack_interleaved, "ft_stack interleaved" },
        { test_efficiency_stack_move, "ft_stack move" },
        { test_efficiency_stack_swap, "ft_stack swap" },
        { test_efficiency_queue_enqueue_dequeue, "ft_queue enqueue/dequeue" },
        { test_efficiency_queue_interleaved, "ft_queue interleaved" },
        { test_efficiency_queue_move, "ft_queue move" },
        { test_efficiency_queue_swap, "ft_queue swap" },
        { test_efficiency_unord_map_insert_find, "ft_unord_map insert/find" },
        { test_efficiency_unord_map_insert_remove, "ft_unord_map insert/remove" },
        { test_efficiency_unord_map_iterate, "ft_unord_map iterate" },
        { test_efficiency_unord_map_copy, "ft_unord_map copy" },
        { test_efficiency_unord_map_move, "ft_unord_map move" },
        { test_efficiency_unord_map_swap, "ft_unord_map swap" },
        { test_efficiency_unord_map_clear, "ft_unord_map clear" },
        { test_efficiency_pair_create_copy, "ft_pair create/copy" },
        { test_efficiency_pair_move, "ft_pair move" },
        { test_efficiency_pair_swap, "ft_pair swap" },
        { test_efficiency_promise_set_get, "ft_promise set/get" },
        { test_efficiency_pow, "ft_pow" },
        { test_efficiency_sqrt, "ft_sqrt" },
        { test_efficiency_exp, "ft_exp" },
        { test_efficiency_clamp, "ft_clamp" },
        { test_efficiency_pool_acquire_release, "ft_pool acquire/release" },
        { test_efficiency_swap_large, "ft_swap large struct" },
        { test_efficiency_mutex_lock, "ft_mutex lock/unlock" },
        { test_efficiency_printf, "ft_printf" },
    };

    const int total = sizeof(tests) / sizeof(tests[0]);
    const int perf_total = sizeof(perf_tests) / sizeof(perf_tests[0]);

    if (argc > 1)
    {
        std::string arg(argv[1]);
        if (arg == "--all")
        {
            int passed = 0;
            for (int i = 0; i < total; ++i)
                run_test(i + 1, &tests[i], &passed);
            printf("%d/%d tests passed\n", passed, total);
            return (0);
        }
        else if (arg == "--perf-all")
        {
            for (int i = 0; i < perf_total; ++i)
                run_efficiency_test(i + 1, &perf_tests[i]);
            return (0);
        }
    }

    while (true)
    {
        char *mode_line = rl_readline("Type 'run' to run tests, 'perf' for efficiency, or 'exit': ");
        if (!mode_line)
            break ;
        std::string mode(mode_line);
        cma_free(mode_line);
        std::transform(mode.begin(), mode.end(), mode.begin(),
                       [](unsigned char c){ return (std::tolower(c)); });

        if (mode == "exit")
            break ;
        else if (mode == "run")
        {
            while (true)
            {
                printf("Available tests:\n");
                for (int i = 0; i < total; ++i)
                    printf("%2d) %s\n", i + 1, tests[i].description);
                char *line = rl_readline("Select tests to run ('all', 'return', 'exit'): ");
                if (!line)
                    return (0);
                std::string input(line);
                cma_free(line);
                std::transform(input.begin(), input.end(), input.begin(),
                               [](unsigned char c){ return (std::tolower(c)); });

                if (input == "exit")
                    return (0);
                if (input == "return")
                    break ;

                std::vector<int> to_run;
                if (input == "all")
                {
                    for (int i = 0; i < total; ++i)
                        to_run.push_back(i + 1);
                }
                else
                {
                    std::stringstream ss(input);
                    int value;
                    while (ss >> value)
                        if (value >= 1 && value <= total)
                            to_run.push_back(value);
                }

                int passed = 0;
                for (int idx : to_run)
                    run_test(idx, &tests[idx - 1], &passed);
                printf("%d/%zu tests passed\n", passed, to_run.size());
            }
        }
        else if (mode == "perf")
        {
            while (true)
            {
                printf("Efficiency tests:\n");
                for (int i = 0; i < perf_total; ++i)
                    printf("%d) %s\n", i + 1, perf_tests[i].description);
                char *line = rl_readline("Select efficiency tests ('all', 'return', 'exit'): ");
                if (!line)
                    return (0);
                std::string input(line);
                cma_free(line);
                std::transform(input.begin(), input.end(), input.begin(),
                               [](unsigned char c){ return (std::tolower(c)); });

                if (input == "exit")
                    return (0);
                if (input == "return")
                    break ;

                if (input == "all")
                {
                    for (int i = 0; i < perf_total; ++i)
                        run_efficiency_test(i + 1, &perf_tests[i]);
                }
                else
                {
                    std::stringstream ss(input);
                    int value;
                    while (ss >> value)
                        if (value >= 1 && value <= perf_total)
                            run_efficiency_test(value, &perf_tests[value - 1]);
                }
            }
        }
    }
    return (0);
}
