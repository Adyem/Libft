#include "../test_internal.hpp"
#include "../../Modules/Debug/debug.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"

FT_TEST(test_debug_symbolize_address_reports_symbol_and_location)
{
    char symbol_buffer[256];
    char location_buffer[256];
    int32_t error_code;

    symbol_buffer[0] = '\0';
    location_buffer[0] = '\0';
    error_code = dbg_symbolize_address(
        reinterpret_cast<const void *>(&dbg_print_stack_trace),
        symbol_buffer, sizeof(symbol_buffer),
        location_buffer, sizeof(location_buffer));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, error_code);
    FT_ASSERT(ft_strstr(symbol_buffer, "dbg_print_stack_trace") != ft_nullptr);
    FT_ASSERT(ft_strstr(location_buffer, "debug.cpp") != ft_nullptr);
    return (1);
}
