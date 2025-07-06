#include "../Libft/libft.hpp"
#include "../Printf/printf.hpp"

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
int test_strcmp_equal(void);
int test_strcmp_null(void);
int test_isdigit_true(void);
int test_isdigit_false(void);
int test_memset_null(void);
int test_memset_basic(void);
int test_toupper_basic(void);
int test_atoi_simple(void);
int test_atoi_negative(void);
int test_html_create_node(void);
int test_html_find_by_tag(void);
int test_html_write_to_string(void);
int test_html_find_by_attr(void);
int test_network_send_receive(void);
int test_network_invalid_ip(void);
int test_network_send_uninitialized(void);
int test_strlen_size_t_null(void);
int test_strlen_size_t_basic(void);
int test_strlen_size_t_empty(void);
int test_bzero_basic(void);
int test_bzero_zero(void);
int test_memcpy_basic(void);
int test_memcpy_null(void);
int test_memcpy_partial(void);
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

int main(void)
{
    const s_test tests[] = {
        { test_strlen_nullptr, "strlen nullptr" },
        { test_strlen_simple, "strlen simple" },
        { test_strcmp_equal, "strcmp equal" },
        { test_strcmp_null, "strcmp null" },
        { test_isdigit_true, "isdigit true" },
        { test_isdigit_false, "isdigit false" },
        { test_memset_null, "memset null" },
        { test_memset_basic, "memset basic" },
        { test_toupper_basic, "toupper basic" },
        { test_atoi_simple, "atoi simple" },
        { test_atoi_negative, "atoi negative" },
        { test_html_create_node, "html create node" },
        { test_html_find_by_tag, "html find by tag" },
        { test_html_write_to_string, "html write to string" },
        { test_html_find_by_attr, "html find by attr" },
        { test_network_send_receive, "network send/receive" },
        { test_network_invalid_ip, "network invalid ip" },
        { test_network_send_uninitialized, "network send uninitialized" },
        { test_strlen_size_t_null, "strlen_size_t null" },
        { test_strlen_size_t_basic, "strlen_size_t basic" },
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
        { test_strlen_size_t_empty, "strlen_size_t empty" },
        { test_bzero_zero, "bzero zero" },
        { test_memcpy_partial, "memcpy partial" },
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
        { test_isspace_false, "isspace false" }
    };
    const int total = sizeof(tests) / sizeof(tests[0]);
    int index = 0;
    int passed = 0;

    while (index < total)
    {
        run_test(index + 1, &tests[index], &passed);
        index++;
    }
    pf_printf("%d/%d tests passed\n", passed, total);
    return (0);
}
