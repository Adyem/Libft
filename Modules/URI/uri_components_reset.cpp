#include "uri_internal.hpp"

void uri_components_reset(uri_components *components) noexcept
{
    if (components == ft_nullptr)
        return ;
    components->scheme = ft_nullptr;
    components->userinfo = ft_nullptr;
    components->host = ft_nullptr;
    components->port = ft_nullptr;
    components->path = ft_nullptr;
    components->query = ft_nullptr;
    components->fragment = ft_nullptr;
    return ;
}
