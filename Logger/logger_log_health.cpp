#include "logger_internal.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/pthread.hpp"
#include "../Time/time.hpp"
#include "../Networking/networking.hpp"

struct s_log_remote_health_state
{
    s_network_sink *sink;
    ft_string       host;
    unsigned short  port;
    bool            use_tcp;
    bool            reachable;
    t_time          last_check;
    int             last_error;

    s_log_remote_health_state()
        : sink(ft_nullptr), host(), port(0), use_tcp(false), reachable(false),
          last_check(0), last_error(FT_ERR_INVALID_STATE)
    {
        return ;
    }
};

struct s_network_sink_snapshot
{
    s_network_sink         *sink;
    int                     socket_fd;
    t_network_send_function send_function;
    ft_string               host;
    unsigned short          port;
    bool                    use_tcp;

    s_network_sink_snapshot()
        : sink(ft_nullptr), socket_fd(-1), send_function(ft_nullptr), host(),
          port(0), use_tcp(false)
    {
        return ;
    }
};

static pthread_mutex_t g_health_mutex;
static pthread_once_t g_health_mutex_once = PTHREAD_ONCE_INIT;
static int g_health_mutex_init_error = 0;
static bool g_health_thread_running = false;
static pthread_t g_health_thread;
static unsigned int g_health_interval_seconds = 30;
static ft_vector<s_log_remote_health_state> g_health_states;

static void logger_health_initialize_mutex()
{
    int init_result;

    init_result = pthread_mutex_init(&g_health_mutex, nullptr);
    if (init_result != 0)
    {
        g_health_mutex_init_error = init_result;
        return ;
    }
    g_health_mutex_init_error = 0;
    return ;
}

static int logger_health_lock()
{
    int once_result;
    int lock_result;

    once_result = pthread_once(&g_health_mutex_once, logger_health_initialize_mutex);
    if (once_result != 0)
    {
        ft_errno = ft_map_system_error(once_result);
        return (-1);
    }
    if (g_health_mutex_init_error != 0)
    {
        ft_errno = ft_map_system_error(g_health_mutex_init_error);
        return (-1);
    }
    lock_result = pthread_mutex_lock(&g_health_mutex);
    if (lock_result != 0)
    {
        ft_errno = ft_map_system_error(lock_result);
        return (-1);
    }
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

static int logger_health_unlock()
{
    int once_result;
    int unlock_result;

    once_result = pthread_once(&g_health_mutex_once, logger_health_initialize_mutex);
    if (once_result != 0)
    {
        ft_errno = ft_map_system_error(once_result);
        return (-1);
    }
    if (g_health_mutex_init_error != 0)
    {
        ft_errno = ft_map_system_error(g_health_mutex_init_error);
        return (-1);
    }
    unlock_result = pthread_mutex_unlock(&g_health_mutex);
    if (unlock_result != 0)
    {
        ft_errno = ft_map_system_error(unlock_result);
        return (-1);
    }
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

static int logger_health_fetch_config(bool *running, unsigned int *interval_seconds)
{
    int lock_result;
    int unlock_result;

    lock_result = logger_health_lock();
    if (lock_result != 0)
        return (-1);
    if (running)
        *running = g_health_thread_running;
    if (interval_seconds)
        *interval_seconds = g_health_interval_seconds;
    unlock_result = logger_health_unlock();
    if (unlock_result != 0)
        return (-1);
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

static int logger_health_wait_loop(unsigned int interval_seconds)
{
    unsigned int wait_seconds;
    unsigned int elapsed_seconds;
    bool running;

    wait_seconds = interval_seconds;
    if (wait_seconds == 0)
        wait_seconds = 30;
    elapsed_seconds = 0;
    while (elapsed_seconds < wait_seconds)
    {
        if (logger_health_fetch_config(&running, ft_nullptr) != 0)
            return (-1);
        if (!running)
            return (0);
        time_sleep(1);
        elapsed_seconds += 1;
    }
    return (0);
}

static void logger_health_probe_sink(const s_network_sink_snapshot &snapshot_entry,
        bool *reachable, int *error_code)
{
    const char *ping_message;
    size_t message_length;
    size_t total_bytes_sent;
    int previous_errno;
    int send_errno;
    t_network_send_function send_function;
    int socket_fd;

    if (!reachable || !error_code)
    {
        if (error_code)
            *error_code = FT_ERR_INVALID_ARGUMENT;
        if (reachable)
            *reachable = false;
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return ;
    }
    *reachable = false;
    *error_code = FT_ERR_INVALID_ARGUMENT;
    if (!snapshot_entry.sink)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return ;
    }
    socket_fd = snapshot_entry.socket_fd;
    send_function = snapshot_entry.send_function;
    if (socket_fd < 0)
    {
        *error_code = FT_ERR_INVALID_HANDLE;
        ft_errno = FT_ERR_INVALID_HANDLE;
        return ;
    }
    if (!send_function)
        send_function = nw_send;
    ping_message = "[ft_logger] health probe\n";
    message_length = ft_strlen(ping_message);
    total_bytes_sent = 0;
    previous_errno = ft_errno;
    while (total_bytes_sent < message_length)
    {
        ssize_t send_result;

        send_result = send_function(socket_fd, ping_message + total_bytes_sent,
                message_length - total_bytes_sent, 0);
        if (send_result <= 0)
        {
            send_errno = ft_errno;
            if (send_errno == FT_ERR_SUCCESSS || send_errno == previous_errno)
                send_errno = FT_ERR_SOCKET_SEND_FAILED;
            ft_errno = send_errno;
            *error_code = ft_errno;
            return ;
        }
        total_bytes_sent += static_cast<size_t>(send_result);
    }
    *reachable = true;
    *error_code = FT_ERR_SUCCESSS;
    ft_errno = previous_errno;
    return ;
}

static int logger_snapshot_network_sinks(ft_vector<s_network_sink_snapshot> &snapshot)
{
    size_t sink_count;
    size_t index;
    int final_error;

    if (logger_lock_sinks() != 0)
        return (-1);
    sink_count = g_sinks.size();
    if (g_sinks.get_error() != FT_ERR_SUCCESSS)
    {
        final_error = g_sinks.get_error();
        if (logger_unlock_sinks() != 0)
            return (-1);
        ft_errno = final_error;
        return (-1);
    }
    index = 0;
    while (index < sink_count)
    {
        s_log_sink entry;
        s_log_sink &stored_entry = g_sinks[index];
        bool        log_sink_lock_acquired;

        entry = g_sinks[index];
        if (g_sinks.get_error() != FT_ERR_SUCCESSS)
        {
            final_error = g_sinks.get_error();
            if (logger_unlock_sinks() != 0)
                return (-1);
            ft_errno = final_error;
            return (-1);
        }
        log_sink_lock_acquired = false;
        if (log_sink_lock(&stored_entry, &log_sink_lock_acquired) != 0)
        {
            final_error = ft_errno;
            if (logger_unlock_sinks() != 0)
                return (-1);
            ft_errno = final_error;
            return (-1);
        }
        entry = stored_entry;
        if (entry.function == ft_network_sink)
        {
            s_network_sink *network_sink;
            s_network_sink_snapshot snapshot_entry;
            bool                   network_lock_acquired;

            network_sink = static_cast<s_network_sink *>(entry.user_data);
            snapshot_entry.sink = network_sink;
            snapshot_entry.socket_fd = -1;
            snapshot_entry.send_function = ft_nullptr;
            network_lock_acquired = false;
            if (network_sink)
            {
                if (network_sink_lock(network_sink, &network_lock_acquired) != 0)
                {
                    final_error = ft_errno;
                    if (network_lock_acquired)
                        network_sink_unlock(network_sink, network_lock_acquired);
                    if (log_sink_lock_acquired)
                        log_sink_unlock(&stored_entry, log_sink_lock_acquired);
                    if (logger_unlock_sinks() != 0)
                        return (-1);
                    ft_errno = final_error;
                    return (-1);
                }
            }
            if (network_sink)
            {
                snapshot_entry.socket_fd = network_sink->socket_fd;
                snapshot_entry.send_function = network_sink->send_function;
                snapshot_entry.host = network_sink->host;
                if (snapshot_entry.host.get_error() != FT_ERR_SUCCESSS)
                {
                    final_error = snapshot_entry.host.get_error();
                    if (network_lock_acquired)
                        network_sink_unlock(network_sink, network_lock_acquired);
                    if (log_sink_lock_acquired)
                        log_sink_unlock(&stored_entry, log_sink_lock_acquired);
                    if (logger_unlock_sinks() != 0)
                        return (-1);
                    ft_errno = final_error;
                    return (-1);
                }
                snapshot_entry.port = network_sink->port;
                snapshot_entry.use_tcp = network_sink->use_tcp;
            }
            if (network_lock_acquired)
                network_sink_unlock(network_sink, network_lock_acquired);
            snapshot.push_back(snapshot_entry);
            if (snapshot.get_error() != FT_ERR_SUCCESSS)
            {
                final_error = snapshot.get_error();
                if (log_sink_lock_acquired)
                    log_sink_unlock(&stored_entry, log_sink_lock_acquired);
                if (logger_unlock_sinks() != 0)
                    return (-1);
                ft_errno = final_error;
                return (-1);
            }
        }
        if (log_sink_lock_acquired)
            log_sink_unlock(&stored_entry, log_sink_lock_acquired);
        index += 1;
    }
    if (logger_unlock_sinks() != 0)
        return (-1);
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

static int logger_health_sync_states(const ft_vector<s_network_sink_snapshot> &snapshot)
{
    size_t state_count;
    size_t snapshot_count;
    size_t index;
    int lock_result;
    int unlock_result;

    snapshot_count = snapshot.size();
    if (snapshot.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = snapshot.get_error();
        return (-1);
    }
    lock_result = logger_health_lock();
    if (lock_result != 0)
        return (-1);
    state_count = g_health_states.size();
    if (g_health_states.get_error() != FT_ERR_SUCCESSS)
    {
        int vector_error;

        vector_error = g_health_states.get_error();
        unlock_result = logger_health_unlock();
        if (unlock_result != 0)
            return (-1);
        ft_errno = vector_error;
        return (-1);
    }
    index = 0;
    while (index < state_count)
    {
        s_log_remote_health_state &state = g_health_states[index];
        bool found;
        size_t snapshot_index;

        if (g_health_states.get_error() != FT_ERR_SUCCESSS)
        {
            int vector_error;

            vector_error = g_health_states.get_error();
            unlock_result = logger_health_unlock();
            if (unlock_result != 0)
                return (-1);
            ft_errno = vector_error;
            return (-1);
        }
        
        found = false;
        snapshot_index = 0;
        while (snapshot_index < snapshot_count)
        {
            const s_network_sink_snapshot &snapshot_entry = snapshot[snapshot_index];

            if (snapshot.get_error() != FT_ERR_SUCCESSS)
            {
                unlock_result = logger_health_unlock();
                if (unlock_result != 0)
                    return (-1);
                ft_errno = snapshot.get_error();
                return (-1);
            }
            if (snapshot_entry.sink == state.sink)
            {
                state.host = snapshot_entry.host;
                if (state.host.get_error() != FT_ERR_SUCCESSS)
                {
                    int host_error;

                    host_error = state.host.get_error();
                    unlock_result = logger_health_unlock();
                    if (unlock_result != 0)
                        return (-1);
                    ft_errno = host_error;
                    return (-1);
                }
                state.port = snapshot_entry.port;
                state.use_tcp = snapshot_entry.use_tcp;
                found = true;
                break;
            }
            snapshot_index += 1;
        }
        if (!found)
        {
            g_health_states.erase(g_health_states.begin() + index);
            if (g_health_states.get_error() != FT_ERR_SUCCESSS)
            {
                int vector_error;

                vector_error = g_health_states.get_error();
                unlock_result = logger_health_unlock();
                if (unlock_result != 0)
                    return (-1);
                ft_errno = vector_error;
                return (-1);
            }
            state_count = g_health_states.size();
            if (g_health_states.get_error() != FT_ERR_SUCCESSS)
            {
                int vector_error;

                vector_error = g_health_states.get_error();
                unlock_result = logger_health_unlock();
                if (unlock_result != 0)
                    return (-1);
                ft_errno = vector_error;
                return (-1);
            }
            continue;
        }
        index += 1;
    }
    index = 0;
    state_count = g_health_states.size();
    if (g_health_states.get_error() != FT_ERR_SUCCESSS)
    {
        int vector_error;

        vector_error = g_health_states.get_error();
        unlock_result = logger_health_unlock();
        if (unlock_result != 0)
            return (-1);
        ft_errno = vector_error;
        return (-1);
    }
    while (index < snapshot_count)
    {
        const s_network_sink_snapshot &snapshot_entry = snapshot[index];
        size_t state_index;
        bool exists;

        if (snapshot.get_error() != FT_ERR_SUCCESSS)
        {
            unlock_result = logger_health_unlock();
            if (unlock_result != 0)
                return (-1);
            ft_errno = snapshot.get_error();
            return (-1);
        }
        exists = false;
        state_index = 0;
        while (state_index < state_count)
        {
            s_log_remote_health_state &state = g_health_states[state_index];

            if (g_health_states.get_error() != FT_ERR_SUCCESSS)
            {
                int vector_error;

                vector_error = g_health_states.get_error();
                unlock_result = logger_health_unlock();
                if (unlock_result != 0)
                    return (-1);
                ft_errno = vector_error;
                return (-1);
            }
            if (state.sink == snapshot_entry.sink)
            {
                exists = true;
                break;
            }
            state_index += 1;
        }
        if (!exists)
        {
            s_log_remote_health_state new_state;

            new_state.sink = snapshot_entry.sink;
            new_state.host = snapshot_entry.host;
            if (new_state.host.get_error() != FT_ERR_SUCCESSS)
            {
                int host_error;

                host_error = new_state.host.get_error();
                unlock_result = logger_health_unlock();
                if (unlock_result != 0)
                    return (-1);
                ft_errno = host_error;
                return (-1);
            }
            new_state.port = snapshot_entry.port;
            new_state.use_tcp = snapshot_entry.use_tcp;
            new_state.reachable = false;
            new_state.last_check = 0;
            new_state.last_error = FT_ERR_INVALID_STATE;
            g_health_states.push_back(new_state);
            if (g_health_states.get_error() != FT_ERR_SUCCESSS)
            {
                int vector_error;

                vector_error = g_health_states.get_error();
                unlock_result = logger_health_unlock();
                if (unlock_result != 0)
                    return (-1);
                ft_errno = vector_error;
                return (-1);
            }
            state_count = g_health_states.size();
            if (g_health_states.get_error() != FT_ERR_SUCCESSS)
            {
                int vector_error;

                vector_error = g_health_states.get_error();
                unlock_result = logger_health_unlock();
                if (unlock_result != 0)
                    return (-1);
                ft_errno = vector_error;
                return (-1);
            }
        }
        index += 1;
    }
    unlock_result = logger_health_unlock();
    if (unlock_result != 0)
        return (-1);
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

static int logger_health_record_result(s_network_sink *sink, bool reachable, int error_code)
{
    size_t state_count;
    size_t index;
    int lock_result;
    int unlock_result;
    t_time current_time;

    if (!sink)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    lock_result = logger_health_lock();
    if (lock_result != 0)
        return (-1);
    state_count = g_health_states.size();
    if (g_health_states.get_error() != FT_ERR_SUCCESSS)
    {
        int vector_error;

        vector_error = g_health_states.get_error();
        unlock_result = logger_health_unlock();
        if (unlock_result != 0)
            return (-1);
        ft_errno = vector_error;
        return (-1);
    }
    current_time = time_now();
    index = 0;
    while (index < state_count)
    {
        s_log_remote_health_state &state = g_health_states[index];

        if (g_health_states.get_error() != FT_ERR_SUCCESSS)
        {
            int vector_error;

            vector_error = g_health_states.get_error();
            unlock_result = logger_health_unlock();
            if (unlock_result != 0)
                return (-1);
            ft_errno = vector_error;
            return (-1);
        }
        if (state.sink == sink)
        {
            state.reachable = reachable;
            state.last_check = current_time;
            state.last_error = error_code;
            unlock_result = logger_health_unlock();
            if (unlock_result != 0)
                return (-1);
            ft_errno = FT_ERR_SUCCESSS;
            return (0);
        }
        index += 1;
    }
    unlock_result = logger_health_unlock();
    if (unlock_result != 0)
        return (-1);
    ft_errno = FT_ERR_NOT_FOUND;
    return (-1);
}

static void *logger_health_thread(void *argument)
{
    (void)argument;
    while (1)
    {
        bool running;
        unsigned int interval_seconds;

        if (logger_health_fetch_config(&running, &interval_seconds) != 0)
            break;
        if (!running)
            break;
        ft_log_probe_remote_health();
        if (logger_health_wait_loop(interval_seconds) != 0)
            break;
    }
    return (ft_nullptr);
}

void ft_log_enable_remote_health(bool enable)
{
    int lock_result;
    int unlock_result;

    lock_result = logger_health_lock();
    if (lock_result != 0)
        return ;
    if (enable)
    {
        if (g_health_thread_running)
        {
            unlock_result = logger_health_unlock();
            if (unlock_result != 0)
                return ;
            ft_errno = FT_ERR_SUCCESSS;
            return ;
        }
        g_health_thread_running = true;
        unlock_result = logger_health_unlock();
        if (unlock_result != 0)
            return ;
        if (pt_thread_create(&g_health_thread, ft_nullptr, logger_health_thread, ft_nullptr) != 0)
        {
            lock_result = logger_health_lock();
            if (lock_result == 0)
            {
                g_health_thread_running = false;
                logger_health_unlock();
            }
            return ;
        }
        ft_errno = FT_ERR_SUCCESSS;
        return ;
    }
    if (!g_health_thread_running)
    {
        unlock_result = logger_health_unlock();
        if (unlock_result != 0)
            return ;
        ft_errno = FT_ERR_SUCCESSS;
        return ;
    }
    g_health_thread_running = false;
    unlock_result = logger_health_unlock();
    if (unlock_result != 0)
        return ;
    void *thread_result;

    thread_result = ft_nullptr;
    if (pt_thread_join(g_health_thread, &thread_result) != 0)
        return ;
    ft_errno = FT_ERR_SUCCESSS;
    return ;
}

void ft_log_set_remote_health_interval(unsigned int interval_seconds)
{
    int lock_result;
    int unlock_result;

    lock_result = logger_health_lock();
    if (lock_result != 0)
        return ;
    g_health_interval_seconds = interval_seconds;
    unlock_result = logger_health_unlock();
    if (unlock_result != 0)
        return ;
    ft_errno = FT_ERR_SUCCESSS;
    return ;
}

int ft_log_probe_remote_health()
{
    int final_result;
    int final_errno;

    final_result = 0;
    final_errno = FT_ERR_SUCCESSS;
    {
        ft_vector<s_network_sink_snapshot> snapshot;
        size_t snapshot_count;
        size_t index;
        bool failure_detected;
        int failure_code;

        if (logger_snapshot_network_sinks(snapshot) != 0)
        {
            final_result = -1;
            final_errno = ft_errno;
            goto cleanup_snapshot;
        }
        if (logger_health_sync_states(snapshot) != 0)
        {
            final_result = -1;
            final_errno = ft_errno;
            goto cleanup_snapshot;
        }
        snapshot_count = snapshot.size();
        if (snapshot.get_error() != FT_ERR_SUCCESSS)
        {
            final_result = -1;
            final_errno = snapshot.get_error();
            goto cleanup_snapshot;
        }
        index = 0;
        failure_detected = false;
        failure_code = FT_ERR_SUCCESSS;
        while (index < snapshot_count)
        {
            const s_network_sink_snapshot &snapshot_entry = snapshot[index];
            bool reachable;
            int probe_error;

            if (snapshot.get_error() != FT_ERR_SUCCESSS)
            {
                final_result = -1;
                final_errno = snapshot.get_error();
                goto cleanup_snapshot;
            }
            if (!snapshot_entry.sink)
            {
                index += 1;
                continue;
            }
            reachable = false;
            probe_error = FT_ERR_INVALID_STATE;
            logger_health_probe_sink(snapshot_entry, &reachable, &probe_error);
            if (logger_health_record_result(snapshot_entry.sink, reachable, probe_error) != 0)
            {
                if (ft_errno != FT_ERR_NOT_FOUND)
                {
                    final_result = -1;
                    final_errno = ft_errno;
                    goto cleanup_snapshot;
                }
                ft_errno = FT_ERR_SUCCESSS;
            }
            if (!reachable && !failure_detected)
            {
                failure_detected = true;
                failure_code = probe_error;
            }
            index += 1;
        }
        if (failure_detected)
        {
            final_result = -1;
            final_errno = failure_code;
            goto cleanup_snapshot;
        }
        final_result = 0;
        final_errno = FT_ERR_SUCCESSS;
    cleanup_snapshot:
        ;
    }
    ft_errno = final_errno;
    return (final_result);
}

int ft_log_get_remote_health(s_log_remote_health *statuses, size_t capacity, size_t *count)
{
    ft_vector<s_network_sink_snapshot> snapshot;
    size_t states_count;
    size_t index;
    int lock_result;
    int unlock_result;

    if (!count)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (logger_snapshot_network_sinks(snapshot) != 0)
        return (-1);
    if (logger_health_sync_states(snapshot) != 0)
        return (-1);
    lock_result = logger_health_lock();
    if (lock_result != 0)
        return (-1);
    states_count = g_health_states.size();
    if (g_health_states.get_error() != FT_ERR_SUCCESSS)
    {
        int vector_error;

        vector_error = g_health_states.get_error();
        unlock_result = logger_health_unlock();
        if (unlock_result != 0)
            return (-1);
        ft_errno = vector_error;
        return (-1);
    }
    if (statuses && capacity < states_count)
    {
        *count = states_count;
        unlock_result = logger_health_unlock();
        if (unlock_result != 0)
            return (-1);
        ft_errno = FT_ERR_OUT_OF_RANGE;
        return (-1);
    }
    index = 0;
    while (index < states_count)
    {
        s_log_remote_health_state &state = g_health_states[index];

        if (g_health_states.get_error() != FT_ERR_SUCCESSS)
        {
            int vector_error;

            vector_error = g_health_states.get_error();
            unlock_result = logger_health_unlock();
            if (unlock_result != 0)
                return (-1);
            ft_errno = vector_error;
            return (-1);
        }
        if (statuses && index < capacity)
        {
            statuses[index].host = ft_nullptr;
            statuses[index].port = 0;
            statuses[index].use_tcp = false;
            statuses[index].reachable = state.reachable;
            statuses[index].last_check = state.last_check;
            statuses[index].last_error = state.last_error;
            if (state.host.c_str())
            {
                statuses[index].host = state.host.c_str();
                statuses[index].port = state.port;
                statuses[index].use_tcp = state.use_tcp;
            }
        }
        index += 1;
    }
    *count = states_count;
    unlock_result = logger_health_unlock();
    if (unlock_result != 0)
        return (-1);
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}
