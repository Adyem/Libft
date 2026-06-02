#include "uri_internal.hpp"
#include "../CMA/CMA.hpp"
#include "../Basic/limits.hpp"
#include "uri.hpp"

void uri_components_destroy(uri_components *components) noexcept
{
    if (components == ft_nullptr)
        return ;
    cma_free(components->scheme);
    cma_free(components->userinfo);
    cma_free(components->host);
    cma_free(components->port);
    cma_free(components->path);
    cma_free(components->query);
    cma_free(components->fragment);
    uri_components_reset(components);
    return ;
}
