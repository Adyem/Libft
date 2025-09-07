#include "CMA.hpp"
#include "../Libft/libft.hpp"
#include "../CPP_class/nullptr.hpp"
#include <type_traits>

template <typename... Args>
char *cma_strjoin_multiple(const Args&... args)
{
    static_assert((std::is_convertible_v<Args, const char*> && ...),
                  "All arguments must be const char*");
    constexpr int count = sizeof...(Args);
    if (count == 0)
        return ft_nullptr;
    size_t total_length = (ft_strlen(args) + ...);
    char *result = static_cast<char*>(cma_malloc(total_length + 1));
    if (!result)
        return ft_nullptr;
    size_t result_index = 0;
    auto append = [&](const char *s) {
        if (s) {
            size_t len = ft_strlen(s);
            ft_memcpy(result + result_index, s, len);
            result_index += len;
        }
    };
    (append(args), ...);
    result[result_index] = '\0';
    return result;
}
