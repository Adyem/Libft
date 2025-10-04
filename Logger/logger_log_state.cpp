#include "logger_internal.hpp"

t_log_level g_level = LOG_LEVEL_INFO;
ft_vector<s_log_sink> g_sinks;
pt_mutex g_sinks_mutex;
bool g_use_color = true;
