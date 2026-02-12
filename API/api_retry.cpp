#include "api_http_internal.hpp"
#include "../CPP_class/class_string.hpp"
#include "../Template/vector.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"
#include "../Basic/basic.hpp"
#include "../Time/time.hpp"
#include "../Errno/errno.hpp"
#include <climits>

struct api_circuit_state
{
    ft_string key;
    int failure_count;
    long long open_until_ms;
    bool half_open;
    int half_open_success_count;
};

static ft_vector<api_circuit_state> &api_retry_circuit_get_states(void)
{
    static ft_vector<api_circuit_state> states;

    return (states);
}

static pt_mutex &api_retry_circuit_get_mutex(void)
{
    static pt_mutex circuit_mutex;

    return (circuit_mutex);
}

static long long api_retry_circuit_now(void)
{
    long long now_value;

    now_value = time_monotonic();
    return (now_value);
}

static long long api_retry_circuit_compute_deadline(long long now_ms,
    int cooldown_ms)
{
    long long cooldown_value;

    cooldown_value = static_cast<long long>(cooldown_ms);
    if (cooldown_value <= 0)
        return (now_ms);
    if (cooldown_value > (LLONG_MAX - now_ms))
        return (LLONG_MAX);
    return (now_ms + cooldown_value);
}

static bool api_retry_circuit_keys_match(const ft_string &lhs,
    const char *rhs)
{
    const char *lhs_cstr;

    lhs_cstr = lhs.c_str();
    if (!lhs_cstr || !rhs)
        return (false);
    if (ft_strcmp(lhs_cstr, rhs) == 0)
        return (true);
    return (false);
}

static api_circuit_state *api_retry_circuit_find_state(
    ft_vector<api_circuit_state> &states, const ft_string &key)
{
    const char *key_cstr;
    size_t index;

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

    state = api_retry_circuit_find_state(states, key);
    if (state)
        return (state);
    api_circuit_state new_state;

    new_state.key = key;
    new_state.failure_count = 0;
    new_state.open_until_ms = 0;
    new_state.half_open = false;
    new_state.half_open_success_count = 0;
    states.push_back(new_state);
    if (states.get_error() != FT_ERR_SUCCESS)
        return (ft_nullptr);
    size_t last_index;

    last_index = states.size();
    if (last_index == 0)
        return (ft_nullptr);
    return (&states[last_index - 1]);
}

bool api_retry_circuit_allow(const api_connection_pool_handle &handle,
    const api_retry_policy *retry_policy, int &error_code)
{
    int threshold;
    const char *key_cstr;

    threshold = api_retry_get_circuit_threshold(retry_policy);
    if (threshold <= 0)
        return (true);
    key_cstr = handle.key.c_str();
    if (!key_cstr || key_cstr[0] == '\0')
        return (true);
    ft_unique_lock<pt_mutex> guard(api_retry_circuit_get_mutex());

    if (guard.get_error() != FT_ERR_SUCCESS)
        return (true);
    ft_vector<api_circuit_state> &states = api_retry_circuit_get_states();
    api_circuit_state *state;

    state = api_retry_circuit_get_state(states, handle.key);
    if (!state)
        return (true);
    long long now_ms;

    now_ms = api_retry_circuit_now();
    if (state->open_until_ms > now_ms)
    {
        error_code = FT_ERR_API_CIRCUIT_OPEN;
        return (false);
    }
    if (state->open_until_ms != 0 && state->open_until_ms <= now_ms)
    {
        state->open_until_ms = 0;
        state->half_open = true;
        state->half_open_success_count = 0;
        state->failure_count = 0;
    }
    return (true);
}

void api_retry_circuit_record_success(const api_connection_pool_handle &handle,
    const api_retry_policy *retry_policy)
{
    int threshold;
    const char *key_cstr;

    threshold = api_retry_get_circuit_threshold(retry_policy);
    if (threshold <= 0)
        return ;
    key_cstr = handle.key.c_str();
    if (!key_cstr || key_cstr[0] == '\0')
        return ;
    ft_unique_lock<pt_mutex> guard(api_retry_circuit_get_mutex());

    if (guard.get_error() != FT_ERR_SUCCESS)
        return ;
    ft_vector<api_circuit_state> &states = api_retry_circuit_get_states();
    api_circuit_state *state;

    state = api_retry_circuit_get_state(states, handle.key);
    if (!state)
        return ;
    state->failure_count = 0;
    state->open_until_ms = 0;
    if (state->half_open)
    {
        int required_successes;

        required_successes = api_retry_get_half_open_successes(retry_policy);
        if (required_successes <= 0)
            required_successes = 1;
        state->half_open_success_count += 1;
        if (state->half_open_success_count >= required_successes)
        {
            state->half_open = false;
            state->half_open_success_count = 0;
        }
    }
    else
        state->half_open_success_count = 0;
    return ;
}

void api_retry_circuit_record_failure(const api_connection_pool_handle &handle,
    const api_retry_policy *retry_policy)
{
    int threshold;
    const char *key_cstr;

    threshold = api_retry_get_circuit_threshold(retry_policy);
    if (threshold <= 0)
        return ;
    key_cstr = handle.key.c_str();
    if (!key_cstr || key_cstr[0] == '\0')
        return ;
    ft_unique_lock<pt_mutex> guard(api_retry_circuit_get_mutex());

    if (guard.get_error() != FT_ERR_SUCCESS)
        return ;
    ft_vector<api_circuit_state> &states = api_retry_circuit_get_states();
    api_circuit_state *state;

    state = api_retry_circuit_get_state(states, handle.key);
    if (!state)
        return ;
    long long now_ms;
    int cooldown_ms;

    now_ms = api_retry_circuit_now();
    cooldown_ms = api_retry_get_circuit_cooldown(retry_policy);
    if (state->half_open)
    {
        state->half_open = false;
        state->half_open_success_count = 0;
        state->failure_count = threshold;
        state->open_until_ms = api_retry_circuit_compute_deadline(now_ms,
                cooldown_ms);
        return ;
    }
    if (state->failure_count < threshold)
        state->failure_count += 1;
    if (state->failure_count >= threshold)
    {
        state->failure_count = threshold;
        state->half_open = false;
        state->half_open_success_count = 0;
        state->open_until_ms = api_retry_circuit_compute_deadline(now_ms,
                cooldown_ms);
    }
    return ;
}

void api_retry_circuit_reset(void)
{
    ft_unique_lock<pt_mutex> guard(api_retry_circuit_get_mutex());

    if (guard.get_error() != FT_ERR_SUCCESS)
        return ;
    ft_vector<api_circuit_state> &states = api_retry_circuit_get_states();

    states.clear();
    return ;
}
