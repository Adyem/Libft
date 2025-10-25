#ifndef JSON_STREAM_READER_HPP
#define JSON_STREAM_READER_HPP

#include <cstddef>
#include <cstdio>

#include "json_stream_events.hpp"

struct json_group;

typedef size_t (*json_stream_read_callback)(void *user_data, char *buffer, size_t max_size);

typedef struct json_stream_reader
{
    json_stream_read_callback read_callback;
    void *user_data;
    char *buffer;
    size_t buffer_capacity;
    size_t buffer_size;
    size_t buffer_index;
    bool end_of_stream;
    int error_code;
} json_stream_reader;

int         json_stream_reader_init_file(json_stream_reader *reader, FILE *file, size_t buffer_capacity);
int         json_stream_reader_init_callback(json_stream_reader *reader, json_stream_read_callback callback, void *user_data, size_t buffer_capacity);
void        json_stream_reader_destroy(json_stream_reader *reader);
json_group  *json_stream_reader_parse(json_stream_reader *reader);
json_group  *json_read_from_file_stream(FILE *file, size_t buffer_capacity);
json_group  *json_read_from_stream(json_stream_read_callback callback, void *user_data, size_t buffer_capacity);
int         json_stream_reader_traverse(json_stream_reader *reader, json_stream_event_callback callback, void *user_data);
int         json_stream_read_from_file_events(FILE *file,
                size_t buffer_capacity,
                json_stream_event_callback callback,
                void *user_data);
int         json_stream_read_from_stream_events(json_stream_read_callback callback,
                void *user_data,
                size_t buffer_capacity,
                json_stream_event_callback event_callback,
                void *event_user_data);

#endif
