#ifndef LOGGER_INTERNAL_HPP
#define LOGGER_INTERNAL_HPP

#include <cstdarg>
#include <pthread.h>
#include "../Template/vector.hpp"
#include "../CPP_class/class_string.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include "logger.hpp"

extern ft_logger *g_logger;
extern t_log_level g_level;
extern ft_bool g_async_running;
extern ft_bool g_use_color;

typedef void (*t_log_sink)(const char *message, void *user_data);

typedef ssize_t (*t_network_send_function)(int32_t socket_fd, const void *buffer, ft_size_t length, int32_t flags);

struct s_log_sink
{
    pt_mutex   *mutex;
    ft_bool        thread_safe_enabled;
    t_log_sink  function;
    void       *user_data;

    s_log_sink()
        : mutex(ft_nullptr), thread_safe_enabled(FT_FALSE), function(ft_nullptr), user_data(ft_nullptr)
    {
        return ;
    }
};

struct s_file_sink
{
    pt_mutex   *mutex;
    ft_bool        thread_safe_enabled;
    int32_t       file_descriptor;
    ft_string path;
    ft_size_t    max_size;
    ft_size_t    retention_count;
    uint32_t max_age_seconds;

    s_file_sink()
        : mutex(ft_nullptr), thread_safe_enabled(FT_FALSE), file_descriptor(-1), path(),
          max_size(0), retention_count(1), max_age_seconds(0)
    {
        return ;
    }
};

struct s_network_sink
{
    pt_mutex                   *mutex;
    ft_bool                        thread_safe_enabled;
    int32_t                         socket_fd;
    t_network_send_function     send_function;
    ft_string                   host;
    uint16_t              port;
    ft_bool                        use_tcp;

    s_network_sink()
        : mutex(ft_nullptr), thread_safe_enabled(FT_FALSE), socket_fd(-1),
          send_function(ft_nullptr), host(), port(0), use_tcp(FT_FALSE)
    {
        return ;
    }
};

struct s_redaction_rule
{
    ft_string   pattern;
    ft_string   replacement;

    s_redaction_rule()
        : pattern(), replacement()
    {
        return ;
    }
};

struct s_log_context_entry
{
    ft_string   key;
    ft_string   value;
    ft_bool        has_value;

    s_log_context_entry()
        : key(), value(), has_value(FT_FALSE)
    {
        return ;
    }
};

struct s_log_context_view
{
    const char  *key;
    const char  *value;
    ft_bool        has_value;

    s_log_context_view()
        : key(ft_nullptr), value(ft_nullptr), has_value(FT_FALSE)
    {
        return ;
    }
};

extern ft_vector<s_log_sink> g_sinks;
extern pthread_mutex_t g_sinks_mutex;
extern ft_vector<s_redaction_rule> g_redaction_rules;

int32_t logger_lock_sinks();
int32_t logger_unlock_sinks();
int32_t logger_copy_redaction_rules(ft_vector<s_redaction_rule> &destination);
int32_t logger_apply_redactions(ft_string &text,
        const ft_vector<s_redaction_rule> &rules);
int32_t logger_context_push(const s_log_field *fields, ft_size_t field_count,
        ft_size_t *pushed_count);
void logger_context_pop(ft_size_t entry_count);
int32_t logger_context_apply_plain(ft_string &text);
int32_t logger_context_snapshot(ft_vector<s_log_context_view> &snapshot);
void logger_context_clear();
int32_t logger_context_format_flat(ft_string &output);
int32_t logger_build_standard_message(t_log_level level, const ft_string &message_text,
        const ft_string &context_fragment, ft_string &formatted_message);

void ft_log_rotate(s_file_sink *sink);
int32_t logger_prepare_rotation(s_file_sink *sink, ft_bool *rotate_for_size, ft_bool *rotate_for_age);
void logger_execute_rotation(s_file_sink *sink);
void ft_file_sink(const char *message, void *user_data);
void ft_network_sink(const char *message, void *user_data);
const char *ft_level_to_str(t_log_level level);
int32_t ft_log_level_to_severity(t_log_level level);
void ft_log_vwrite(t_log_level level, const char *format_string, va_list argument_list);

int32_t log_sink_prepare_thread_safety(s_log_sink *sink);
void log_sink_teardown_thread_safety(s_log_sink *sink);
int32_t log_sink_lock(const s_log_sink *sink, ft_bool *lock_acquired);
void log_sink_unlock(const s_log_sink *sink, ft_bool lock_acquired);

int32_t file_sink_prepare_thread_safety(s_file_sink *sink);
void file_sink_teardown_thread_safety(s_file_sink *sink);
int32_t file_sink_lock(const s_file_sink *sink, ft_bool *lock_acquired);
void file_sink_unlock(const s_file_sink *sink, ft_bool lock_acquired);

int32_t network_sink_prepare_thread_safety(s_network_sink *sink);
void network_sink_teardown_thread_safety(s_network_sink *sink);
int32_t network_sink_lock(const s_network_sink *sink, ft_bool *lock_acquired);
void network_sink_unlock(const s_network_sink *sink, ft_bool lock_acquired);

#endif
