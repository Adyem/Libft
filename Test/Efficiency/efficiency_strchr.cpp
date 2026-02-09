#include "../../Basic/basic.hpp"
#include "utils.hpp"

#include <cstring>
#include <string>

int test_efficiency_strchr(void)
{
    const size_t iterations = 100000;
    std::string string(1000, 'a');
    string[500] = 'b';
    volatile const char *result = nullptr;
    auto standard_strchr = static_cast<const char *(*)(const char *, int)>(std::strchr);

    auto start_std = clock_type::now();
    for (size_t index = 0; index < iterations; ++index)
    {
        prevent_optimization(const_cast<char*>(string.data()));
        result = standard_strchr(string.c_str(), 'b');
        prevent_optimization(const_cast<char*>(result));
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t index = 0; index < iterations; ++index)
    {
        prevent_optimization(const_cast<char*>(string.data()));
        result = ft_strchr(string.c_str(), 'b');
        prevent_optimization(const_cast<char*>(result));
    }
    auto end_ft = clock_type::now();
    print_comparison("strchr", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

