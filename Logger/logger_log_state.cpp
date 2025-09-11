#include "logger_internal.hpp"

// Global logger state

t_log_level g_level = LOG_LEVEL_DEBUG;
ft_vector<s_log_sink> g_sinks;
