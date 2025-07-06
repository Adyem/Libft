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
        { test_network_send_uninitialized, "network send uninitialized" }
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
