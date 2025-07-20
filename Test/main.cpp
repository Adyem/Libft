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
int test_atol_basic(void);
int test_atol_whitespace(void);
int test_atol_longmax(void);
int test_atol_longmin(void);
int test_ft_string_append(void);
int test_ft_string_concat(void);
int test_data_buffer_io(void);
int test_ft_file_write_read(void);
int test_ft_vector_push_back(void);
int test_ft_vector_insert_erase(void);
int test_ft_vector_reserve_resize(void);
int test_ft_vector_clear(void);
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

int main(void)
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
        { test_atol_basic, "atol basic" },
        { test_atol_whitespace, "atol whitespace" },
        { test_atol_longmax, "atol longmax" },
        { test_atol_longmin, "atol longmin" },
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
        { test_quest_progress, "quest progress" }
    };
    const int total = sizeof(tests) / sizeof(tests[0]);

    while (true)
    {
        std::printf("Available tests:\n");
        for (int i = 0; i < total; ++i)
            std::printf("%2d) %s\n", i + 1, tests[i].description);

        char *line = rl_readline("Select tests to run (or 'all'): ");
        if (!line)
            break;
        std::string input(line);
        cma_free(line);
        std::transform(input.begin(), input.end(), input.begin(),
                       [](unsigned char c){ return std::tolower(c); });

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
        std::printf("%d/%zu tests passed\n", passed, to_run.size());

        char *again = rl_readline("Run more tests? (y/n): ");
        if (!again)
            break;
        std::string answer(again);
        cma_free(again);
        std::transform(answer.begin(), answer.end(), answer.begin(),
                       [](unsigned char c){ return std::tolower(c); });
        if (answer.empty() || answer[0] != 'y')
            break;
    }
    return (0);
}
