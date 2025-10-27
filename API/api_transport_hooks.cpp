#include "api.hpp"
#include "../CPP_class/class_nullptr.hpp"

struct api_transport_hooks_state
{
    bool enabled;
    api_transport_hooks hooks;
};

static void api_transport_reset_hooks(api_transport_hooks &hooks)
{
    hooks.request_stream = ft_nullptr;
    hooks.request_stream_http2 = ft_nullptr;
    hooks.request_stream_host = ft_nullptr;
    hooks.request_stream_tls = ft_nullptr;
    hooks.request_stream_tls_http2 = ft_nullptr;
    hooks.request_string = ft_nullptr;
    hooks.request_string_http2 = ft_nullptr;
    hooks.request_string_host = ft_nullptr;
    hooks.request_string_tls = ft_nullptr;
    hooks.request_string_tls_http2 = ft_nullptr;
    hooks.request_https = ft_nullptr;
    hooks.request_https_http2 = ft_nullptr;
    hooks.request_string_async = ft_nullptr;
    hooks.request_string_tls_async = ft_nullptr;
    hooks.user_data = ft_nullptr;
    return ;
}

static api_transport_hooks_state g_api_transport_state = { false, {} };

void    api_set_transport_hooks(const api_transport_hooks *hooks)
{
    if (!hooks)
    {
        g_api_transport_state.enabled = false;
        api_transport_reset_hooks(g_api_transport_state.hooks);
        return ;
    }
    g_api_transport_state.hooks = *hooks;
    g_api_transport_state.enabled = true;
    return ;
}

void    api_clear_transport_hooks(void)
{
    g_api_transport_state.enabled = false;
    api_transport_reset_hooks(g_api_transport_state.hooks);
    return ;
}

const api_transport_hooks    *api_get_transport_hooks(void)
{
    if (!g_api_transport_state.enabled)
        return (ft_nullptr);
    return (&g_api_transport_state.hooks);
}
