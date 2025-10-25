#ifndef JSON_STREAM_EVENTS_HPP
#define JSON_STREAM_EVENTS_HPP

#include <cstddef>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum json_stream_event_type
{
    JSON_STREAM_EVENT_BEGIN_DOCUMENT = 0,
    JSON_STREAM_EVENT_END_DOCUMENT = 1,
    JSON_STREAM_EVENT_BEGIN_OBJECT = 2,
    JSON_STREAM_EVENT_END_OBJECT = 3,
    JSON_STREAM_EVENT_BEGIN_ARRAY = 4,
    JSON_STREAM_EVENT_END_ARRAY = 5,
    JSON_STREAM_EVENT_KEY = 6,
    JSON_STREAM_EVENT_STRING = 7,
    JSON_STREAM_EVENT_NUMBER = 8,
    JSON_STREAM_EVENT_BOOLEAN = 9,
    JSON_STREAM_EVENT_NULL = 10
} json_stream_event_type;

typedef struct json_stream_scalar
{
    const char *data;
    size_t length;
} json_stream_scalar;

typedef struct json_stream_event
{
    json_stream_event_type type;
    json_stream_scalar value;
    bool bool_value;
} json_stream_event;

typedef int (*json_stream_event_callback)(void *user_data, const json_stream_event *event);

#ifdef __cplusplus
}
#endif

#endif
