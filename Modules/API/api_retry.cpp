#include "api_http_internal.hpp"
#include "../CPP_class/class_string.hpp"
#include "../Template/vector.hpp"
#include "../PThread/mutex.hpp"
#include "../CMA/CMA.hpp"
#include "../Basic/basic.hpp"
#include "../Time/time.hpp"
#include "../Errno/errno.hpp"
#include "../System_utils/system_utils.hpp"
#include "api_internal.hpp"
#include <new>
#include <climits>
#include <cstdio>
#include "../Basic/limits.hpp"
#include "../Errno/errno_internal.hpp"
#include "../PThread/recursive_mutex.hpp"

struct api_circuit_state
{
    ft_string key;
    int32_t failure_count;
    int64_t open_until_ms;
    ft_bool half_open;
    ft_bool probe_in_flight;
    int32_t half_open_success_count;
};

#ifdef DEBUG
static void api_retry_circuit_debug_log(const char *message)
{
    std::FILE *file_pointer;

    file_pointer = std::fopen("api_retry_debug.log", "a");
    if (!file_pointer)
        return ;
    std::fprintf(file_pointer, "%s\n", message);
    std::fclose(file_pointer);
    return ;
}
#else
#define api_retry_circuit_debug_log(message) do { } while (0)
#endif

static ft_vector<api_circuit_state> &api_retry_circuit_get_states(void)
{
    static ft_vector<api_circuit_state> states;

    return (states);
}

static int32_t api_retry_circuit_ensure_states_initialised(
    ft_vector<api_circuit_state> &states)
{
    if (states.is_initialised() == FT_CLASS_STATE_INITIALISED)
        return (FT_ERR_SUCCESS);
    return (states.initialize());
}

static pt_mutex *api_retry_circuit_get_mutex(void)
{
    static pt_mutex *circuit_mutex = ft_nullptr;

    if (circuit_mutex == ft_nullptr)
    {
        circuit_mutex = new (std::nothrow) pt_mutex();
        if (circuit_mutex == ft_nullptr)
            su_abort();
        if (circuit_mutex->initialize() != FT_ERR_SUCCESS)
            su_abort();
#ifdef LIBFT_TEST_BUILD
        (void)cma_untrack_leak(circuit_mutex);
        if (circuit_mutex->_native_mutex != ft_nullptr)
            (void)cma_untrack_leak(circuit_mutex->_native_mutex);
#endif
    }

    return (circuit_mutex);
}

static int64_t api_retry_circuit_now(void)
{
    int64_t now_value;

    now_value = time_monotonic();
    return (now_value);
}

static int64_t api_retry_circuit_compute_deadline(int64_t now_ms,
    int32_t cooldown_ms)
{
    int64_t cooldown_value;

    cooldown_value = static_cast<int64_t>(cooldown_ms);
    if (cooldown_value <= 0)
        return (now_ms);
    if (cooldown_value > (LLONG_MAX - now_ms))
        return (LLONG_MAX);
    return (now_ms + cooldown_value);
}

static ft_bool api_retry_circuit_keys_match(const ft_string &lhs,
    const char *rhs)
{
    const char *lhs_cstr;

    lhs_cstr = lhs.c_str();
    if (!lhs_cstr || !rhs)
        return (FT_FALSE);
    if (ft_strcmp(lhs_cstr, rhs) == 0)
        return (FT_TRUE);
    return (FT_FALSE);
}

static api_circuit_state *api_retry_circuit_find_state(
    ft_vector<api_circuit_state> &states, const ft_string &key)
{
    const char *key_cstr;
    ft_size_t index;

    key_cstr = key.c_str();
    if (!key_cstr)
        return (ft_nullptr);
    index = 0;
    while (index < states.size())
    {
        api_circuit_state &state = states[index];

        if (api_retry_circuit_keys_match(state.key, key_cstr))
            return (&state);
        index++;
    }
    return (ft_nullptr);
}

static api_circuit_state *api_retry_circuit_get_state(
    ft_vector<api_circuit_state> &states, const ft_string &key)
{
    api_circuit_state *state;
    int32_t key_initialization_error;
    int32_t push_back_error;

    state = api_retry_circuit_find_state(states, key);
    if (state)
        return (state);
    api_circuit_state new_state;

    key_initialization_error = new_state.key.initialize();
    if (key_initialization_error != FT_ERR_SUCCESS)
        return (ft_nullptr);
    new_state.key = key;
    new_state.failure_count = 0;
    new_state.open_until_ms = 0;
    new_state.half_open = FT_FALSE;
    new_state.probe_in_flight = FT_FALSE;
    new_state.half_open_success_count = 0;
    push_back_error = states.push_back(new_state);
    if (push_back_error != FT_ERR_SUCCESS)
        return (ft_nullptr);
    ft_size_t last_index;

    last_index = states.size();
    if (last_index == 0)
        return (ft_nullptr);
    return (&states[last_index - 1]);
}

ft_bool api_retry_circuit_allow(const api_connection_pool_handle &handle,
    const api_retry_policy *retry_policy, int32_t &error_code)
{
    pt_mutex *circuit_mutex;
    int32_t threshold;
    const char *key_cstr;

    circuit_mutex = api_retry_circuit_get_mutex();
    threshold = api_retry_get_circuit_threshold(retry_policy);
    if (threshold <= 0)
        return (FT_TRUE);
    key_cstr = handle.key.c_str();
    if (!key_cstr || key_cstr[0] == '\0')
        return (FT_TRUE);
    if (circuit_mutex->lock() != FT_ERR_SUCCESS)
        return (FT_TRUE);
    ft_vector<api_circuit_state> &states = api_retry_circuit_get_states();
    int32_t states_initialization_error;
    api_circuit_state *state;

    states_initialization_error = api_retry_circuit_ensure_states_initialised(states);
    if (states_initialization_error != FT_ERR_SUCCESS)
    {
        (void)circuit_mutex->unlock();
        return (FT_TRUE);
    }
    state = api_retry_circuit_get_state(states, handle.key);
    if (!state)
    {
        (void)circuit_mutex->unlock();
        return (FT_TRUE);
    }
    int64_t now_ms;

    now_ms = api_retry_circuit_now();
    if (state->open_until_ms > now_ms)
    {
        error_code = FT_ERR_API_CIRCUIT_OPEN;
        api_retry_circuit_debug_log("allow: open");
        (void)circuit_mutex->unlock();
        return (FT_FALSE);
    }
    if (state->open_until_ms != 0 && state->open_until_ms <= now_ms)
    {
        state->open_until_ms = 0;
        state->half_open = FT_TRUE;
        state->probe_in_flight = FT_FALSE;
        state->half_open_success_count = 0;
        state->failure_count = 0;
        api_retry_circuit_debug_log("allow: half_open");
    }
    if (state->half_open)
    {
        if (state->probe_in_flight)
        {
            error_code = FT_ERR_API_CIRCUIT_OPEN;
            api_retry_circuit_debug_log("allow: probe_in_flight");
            (void)circuit_mutex->unlock();
            return (FT_FALSE);
        }
        state->probe_in_flight = FT_TRUE;
        api_retry_circuit_debug_log("allow: reserve_probe");
    }
    api_retry_circuit_debug_log("allow: permit");
    (void)circuit_mutex->unlock();
    return (FT_TRUE);
}

void api_retry_circuit_record_success(const api_connection_pool_handle &handle,
    const api_retry_policy *retry_policy)
{
    pt_mutex *circuit_mutex;
    int32_t threshold;
    const char *key_cstr;

    circuit_mutex = api_retry_circuit_get_mutex();
    threshold = api_retry_get_circuit_threshold(retry_policy);
    if (threshold <= 0)
        return ;
    key_cstr = handle.key.c_str();
    if (!key_cstr || key_cstr[0] == '\0')
        return ;
    if (circuit_mutex->lock() != FT_ERR_SUCCESS)
        return ;
    ft_vector<api_circuit_state> &states = api_retry_circuit_get_states();
    int32_t states_initialization_error;
    api_circuit_state *state;

    states_initialization_error = api_retry_circuit_ensure_states_initialised(states);
    if (states_initialization_error != FT_ERR_SUCCESS)
    {
        (void)circuit_mutex->unlock();
        return ;
    }
    state = api_retry_circuit_get_state(states, handle.key);
    if (!state)
    {
        (void)circuit_mutex->unlock();
        return ;
    }
    state->failure_count = 0;
    state->open_until_ms = 0;
    if (state->half_open)
    {
        int32_t required_successes;

        required_successes = api_retry_get_half_open_successes(retry_policy);
        if (required_successes <= 0)
            required_successes = 1;
        state->half_open_success_count += 1;
        if (state->half_open_success_count >= required_successes)
        {
            state->half_open = FT_FALSE;
            state->probe_in_flight = FT_FALSE;
            state->half_open_success_count = 0;
        }
    }
    else
    {
        state->half_open_success_count = 0;
        state->probe_in_flight = FT_FALSE;
    }
    (void)circuit_mutex->unlock();
    return ;
}

void api_retry_circuit_record_failure(const api_connection_pool_handle &handle,
    const api_retry_policy *retry_policy)
{
    pt_mutex *circuit_mutex;
    int32_t threshold;
    const char *key_cstr;

    circuit_mutex = api_retry_circuit_get_mutex();
    threshold = api_retry_get_circuit_threshold(retry_policy);
    if (threshold <= 0)
        return ;
    key_cstr = handle.key.c_str();
    if (!key_cstr || key_cstr[0] == '\0')
        return ;
    if (circuit_mutex->lock() != FT_ERR_SUCCESS)
        return ;
    ft_vector<api_circuit_state> &states = api_retry_circuit_get_states();
    int32_t states_initialization_error;
    api_circuit_state *state;

    states_initialization_error = api_retry_circuit_ensure_states_initialised(states);
    if (states_initialization_error != FT_ERR_SUCCESS)
    {
        (void)circuit_mutex->unlock();
        return ;
    }
    state = api_retry_circuit_get_state(states, handle.key);
    if (!state)
    {
        (void)circuit_mutex->unlock();
        return ;
    }
    int64_t now_ms;
    int32_t cooldown_ms;

    now_ms = api_retry_circuit_now();
    cooldown_ms = api_retry_get_circuit_cooldown(retry_policy);
    if (state->half_open)
    {
        state->half_open = FT_FALSE;
        state->probe_in_flight = FT_FALSE;
        state->half_open_success_count = 0;
        state->failure_count = threshold;
        state->open_until_ms = api_retry_circuit_compute_deadline(now_ms,
                cooldown_ms);
        (void)circuit_mutex->unlock();
        return ;
    }
    if (state->failure_count < threshold)
        state->failure_count += 1;
    state->probe_in_flight = FT_FALSE;
    if (state->failure_count >= threshold)
    {
        state->failure_count = threshold;
        state->half_open = FT_FALSE;
        state->probe_in_flight = FT_FALSE;
        state->half_open_success_count = 0;
        state->open_until_ms = api_retry_circuit_compute_deadline(now_ms,
                cooldown_ms);
    }
    (void)circuit_mutex->unlock();
    return ;
}

void api_retry_circuit_reset(void)
{
    pt_mutex *circuit_mutex;

    circuit_mutex = api_retry_circuit_get_mutex();
    if (circuit_mutex->lock() != FT_ERR_SUCCESS)
        return ;
    ft_vector<api_circuit_state> &states = api_retry_circuit_get_states();
    int32_t states_initialization_error;

    states_initialization_error = api_retry_circuit_ensure_states_initialised(states);
    if (states_initialization_error != FT_ERR_SUCCESS)
    {
        (void)circuit_mutex->unlock();
        return ;
    }
    states.clear();
    (void)circuit_mutex->unlock();
    api_retry_circuit_debug_log("reset");
    api_connection_pool_flush();
    return ;
}

#ifdef LIBFT_TEST_BUILD
void api_retry_circuit_destroy_for_tests(void)
{
    ft_vector<api_circuit_state> &states = api_retry_circuit_get_states();

    (void)states.destroy();
    return ;
}
#endif
