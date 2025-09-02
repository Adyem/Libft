#include "logger_internal.hpp"
#include "../CPP_class/nullptr.hpp"

ft_logger *g_logger = ft_nullptr;

ft_logger::ft_logger(const char *path, size_t max_size, t_log_level level) noexcept
    : _alloc_logging(false)
{
    ft_log_set_level(level);
    if (path)
        ft_log_set_file(path, max_size);
}

ft_logger::~ft_logger() noexcept
{
    ft_log_close();
    if (g_logger == this)
        g_logger = ft_nullptr;
}

void ft_logger::set_global() noexcept
{
    g_logger = this;
}

void ft_logger::set_level(t_log_level level) noexcept
{
    ft_log_set_level(level);
}

int ft_logger::set_file(const char *path, size_t max_size) noexcept
{
    return ft_log_set_file(path, max_size);
}

void ft_logger::set_alloc_logging(bool enable) noexcept
{
    this->_alloc_logging = enable;
}

bool ft_logger::get_alloc_logging() const noexcept
{
    return (this->_alloc_logging);
}

void ft_logger::close() noexcept
{
    ft_log_close();
}

void ft_logger::debug(const char *fmt, ...) noexcept
{
    va_list args;
    va_start(args, fmt);
    ft_log_vwrite(LOG_LEVEL_DEBUG, fmt, args);
    va_end(args);
}

void ft_logger::info(const char *fmt, ...) noexcept
{
    va_list args;
    va_start(args, fmt);
    ft_log_vwrite(LOG_LEVEL_INFO, fmt, args);
    va_end(args);
}

void ft_logger::warn(const char *fmt, ...) noexcept
{
    va_list args;
    va_start(args, fmt);
    ft_log_vwrite(LOG_LEVEL_WARN, fmt, args);
    va_end(args);
}

void ft_logger::error(const char *fmt, ...) noexcept
{
    va_list args;
    va_start(args, fmt);
    ft_log_vwrite(LOG_LEVEL_ERROR, fmt, args);
    va_end(args);
}

