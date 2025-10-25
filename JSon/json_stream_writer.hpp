#ifndef JSON_STREAM_WRITER_HPP
#define JSON_STREAM_WRITER_HPP

#include <cstddef>
#include <stdbool.h>

#include "json_stream_events.hpp"

#ifdef __cplusplus
extern "C" {
#endif

typedef size_t (*json_stream_write_callback)(void *user_data, const char *buffer, size_t size);

typedef struct json_stream_writer_context
{
    char type;
    bool need_comma;
    bool expecting_key;
} json_stream_writer_context;

typedef struct json_stream_writer
{
    json_stream_write_callback write_callback;
    void *user_data;
    json_stream_writer_context *context_stack;
    size_t context_size;
    size_t context_capacity;
    bool began_document;
    bool finished_document;
    bool root_value_emitted;
    int error_code;
} json_stream_writer;

int     json_stream_writer_init(json_stream_writer *writer, json_stream_write_callback callback, void *user_data);
void    json_stream_writer_destroy(json_stream_writer *writer);
int     json_stream_writer_process(json_stream_writer *writer, const json_stream_event *event);
int     json_stream_writer_finish(json_stream_writer *writer);

#ifdef __cplusplus
}
#endif

#endif
