#ifndef LOGGER_INTERNAL_HPP
#define LOGGER_INTERNAL_HPP

#include <cstdarg>
#include <pthread.h>
#include "../Template/vector.hpp"
#include "../CPP_class/class_string_class.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include "logger.hpp"

extern ft_logger *g_logger;
extern t_log_level g_level;
extern bool g_async_running;
extern bool g_use_color;

typedef void (*t_log_sink)(const char *message, void *user_data);

typedef ssize_t (*t_network_send_function)(int socket_fd, const void *buffer, size_t length, int flags);

struct s_log_sink
{
    pt_mutex   *mutex;
    bool        thread_safe_enabled;
    t_log_sink  function;
    void       *user_data;

    s_log_sink()
        : mutex(ft_nullptr), thread_safe_enabled(false), function(ft_nullptr), user_data(ft_nullptr)
    {
        return ;
    }
};

struct s_file_sink
{
    pt_mutex   *mutex;
    bool        thread_safe_enabled;
    int       fd;
    ft_string path;
    size_t    max_size;
    size_t    retention_count;
    unsigned int max_age_seconds;

    s_file_sink()
        : mutex(ft_nullptr), thread_safe_enabled(false), fd(-1), path(),
          max_size(0), retention_count(1), max_age_seconds(0)
    {
        return ;
    }
};

struct s_network_sink
{
    pt_mutex                   *mutex;
    bool                        thread_safe_enabled;
    int                         socket_fd;
    t_network_send_function     send_function;
    ft_string                   host;
    unsigned short              port;
    bool                        use_tcp;

    s_network_sink()
        : mutex(ft_nullptr), thread_safe_enabled(false), socket_fd(-1),
          send_function(ft_nullptr), host(), port(0), use_tcp(false)
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
    bool        has_value;

    s_log_context_entry()
        : key(), value(), has_value(false)
    {
        return ;
    }
};

struct s_log_context_view
{
    const char  *key;
    const char  *value;
    bool        has_value;

    s_log_context_view()
        : key(ft_nullptr), value(ft_nullptr), has_value(false)
    {
        return ;
    }
};

extern ft_vector<s_log_sink> g_sinks;
extern pthread_mutex_t g_sinks_mutex;
extern ft_vector<s_redaction_rule> g_redaction_rules;

int logger_lock_sinks();
int logger_unlock_sinks();
int logger_copy_redaction_rules(ft_vector<s_redaction_rule> &destination);
int logger_apply_redactions(ft_string &text,
        const ft_vector<s_redaction_rule> &rules);
int logger_context_push(const s_log_field *fields, size_t field_count,
        size_t *pushed_count);
void logger_context_pop(size_t count);
int logger_context_apply_plain(ft_string &text);
int logger_context_snapshot(ft_vector<s_log_context_view> &snapshot);
void logger_context_clear();
int logger_context_format_flat(ft_string &output);
int logger_build_standard_message(t_log_level level, const ft_string &message_text,
        const ft_string &context_fragment, ft_string &formatted_message);

void ft_log_rotate(s_file_sink *sink);
int logger_prepare_rotation(s_file_sink *sink, bool *rotate_for_size, bool *rotate_for_age);
void logger_execute_rotation(s_file_sink *sink);
void ft_file_sink(const char *message, void *user_data);
void ft_network_sink(const char *message, void *user_data);
const char *ft_level_to_str(t_log_level level);
int ft_log_level_to_severity(t_log_level level);
void ft_log_vwrite(t_log_level level, const char *fmt, va_list args);

int log_sink_prepare_thread_safety(s_log_sink *sink);
void log_sink_teardown_thread_safety(s_log_sink *sink);
int log_sink_lock(const s_log_sink *sink, bool *lock_acquired);
void log_sink_unlock(const s_log_sink *sink, bool lock_acquired);

int file_sink_prepare_thread_safety(s_file_sink *sink);
void file_sink_teardown_thread_safety(s_file_sink *sink);
int file_sink_lock(const s_file_sink *sink, bool *lock_acquired);
void file_sink_unlock(const s_file_sink *sink, bool lock_acquired);

int network_sink_prepare_thread_safety(s_network_sink *sink);
void network_sink_teardown_thread_safety(s_network_sink *sink);
int network_sink_lock(const s_network_sink *sink, bool *lock_acquired);
void network_sink_unlock(const s_network_sink *sink, bool lock_acquired);

#endif
