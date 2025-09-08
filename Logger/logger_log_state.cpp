#include "logger_internal.hpp"

// Global logger state

t_log_level g_level = LOG_LEVEL_DEBUG;
int g_fd = 1;
std::string g_path;
size_t g_max_size = 0;
