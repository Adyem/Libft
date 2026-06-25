#include "../Basic/class_nullptr.hpp"
#include "../Printf/printf.hpp"
#include "../Sink/sink.hpp"
#include "system_utils.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

void su_internal_set_abort_reason(const char *reason);

void su_assert(ft_bool condition, const char *message)
{
    char message_buffer[512];
    const char *message_text;

    if (condition)
        return ;
    message_text = "unknown";
    if (message != ft_nullptr)
        message_text = message;
    (void)pf_snprintf(message_buffer, sizeof(message_buffer),
        "Assertion failed: %s", message_text);
    (void)sink_record_message(3, message_buffer);
    su_internal_set_abort_reason(message);
    su_abort();
}
