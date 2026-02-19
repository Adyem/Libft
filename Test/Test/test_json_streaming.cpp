#include "../test_internal.hpp"
#include "../../JSon/json_stream_reader.hpp"
#include "../../JSon/json_stream_writer.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../CPP_class/class_string.hpp"
#include "../../Template/vector.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Basic/basic.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

typedef struct memory_stream
{
    const char *data;
    size_t length;
    size_t offset;
} memory_stream;

typedef struct captured_event
{
    json_stream_event_type type;
    ft_string value;
    bool bool_value;
} captured_event;

static size_t memory_stream_reader(void *user_data, char *buffer, size_t max_size)
{
    memory_stream *stream;
    size_t remaining;
    size_t index;

    if (!user_data || !buffer || max_size == 0)
        return (0);
    stream = static_cast<memory_stream *>(user_data);
    if (stream->offset >= stream->length)
        return (0);
    remaining = stream->length - stream->offset;
    if (remaining > max_size)
        remaining = max_size;
    index = 0;
    while (index < remaining)
    {
        buffer[index] = stream->data[stream->offset + index];
        index += 1;
    }
    stream->offset += remaining;
    return (remaining);
}

static int capture_event(void *user_data, const json_stream_event *event)
{
    ft_vector<captured_event> *events;
    captured_event record;

    if (!user_data || !event)
        return (-1);
    events = static_cast<ft_vector<captured_event> *>(user_data);
    record.type = event->type;
    record.bool_value = event->bool_value;
    if (event->value.data != ft_nullptr && event->value.length > 0)
        record.value.assign(event->value.data, event->value.length);
    else
        record.value.clear();
    events->push_back(record);
    return (0);
}

static int gather_events(const char *json_text, ft_vector<captured_event> &events)
{
    memory_stream stream;
    json_stream_reader reader = json_stream_reader();
    int status;

    events.clear();
    if (!json_text)
        return (-1);
    stream.data = json_text;
    stream.length = ft_strlen_size_t(json_text);
    stream.offset = 0;
    if (json_stream_reader_init_callback(&reader, memory_stream_reader, &stream, 16) != 0)
        return (-1);
    status = json_stream_reader_traverse(&reader, capture_event, &events);
    json_stream_reader_destroy(&reader);
    return (status);
}

static size_t string_sink_write(void *user_data, const char *buffer, size_t size)
{
    ft_string *output;

    if (!user_data || !buffer)
        return (0);
    output = static_cast<ft_string *>(user_data);
    output->append(buffer, size);
    return (size);
}

FT_TEST(test_json_stream_reader_emits_events, "json_stream_reader_traverse produces callback sequence")
{
    const char *json_text;
    ft_vector<captured_event> events;
    int status;
    size_t expected_count;
    size_t index;
    static const json_stream_event_type expected_types[] = {
        JSON_STREAM_EVENT_BEGIN_DOCUMENT,
        JSON_STREAM_EVENT_BEGIN_OBJECT,
        JSON_STREAM_EVENT_KEY,
        JSON_STREAM_EVENT_BEGIN_ARRAY,
        JSON_STREAM_EVENT_NUMBER,
        JSON_STREAM_EVENT_NUMBER,
        JSON_STREAM_EVENT_NUMBER,
        JSON_STREAM_EVENT_END_ARRAY,
        JSON_STREAM_EVENT_KEY,
        JSON_STREAM_EVENT_BEGIN_OBJECT,
        JSON_STREAM_EVENT_KEY,
        JSON_STREAM_EVENT_BOOLEAN,
        JSON_STREAM_EVENT_KEY,
        JSON_STREAM_EVENT_STRING,
        JSON_STREAM_EVENT_END_OBJECT,
        JSON_STREAM_EVENT_END_OBJECT,
        JSON_STREAM_EVENT_END_DOCUMENT
    };

    json_text = "{\n  \"numbers\": [1, 2, 3],\n  \"nested\": {\n    \"flag\": true,\n    \"text\": \"hello\"\n  }\n}\n";
    status = gather_events(json_text, events);
    FT_ASSERT_EQ(0, status);
    expected_count = sizeof(expected_types) / sizeof(expected_types[0]);
    FT_ASSERT_EQ(expected_count, events.size());
    index = 0;
    while (index < expected_count)
    {
        FT_ASSERT_EQ(static_cast<int>(expected_types[index]), static_cast<int>(events[index].type));
        index += 1;
    }
    FT_ASSERT_EQ(0, ft_strcmp(events[2].value.c_str(), "numbers"));
    FT_ASSERT_EQ(0, ft_strcmp(events[4].value.c_str(), "1"));
    FT_ASSERT_EQ(0, ft_strcmp(events[5].value.c_str(), "2"));
    FT_ASSERT_EQ(0, ft_strcmp(events[6].value.c_str(), "3"));
    FT_ASSERT_EQ(0, ft_strcmp(events[8].value.c_str(), "nested"));
    FT_ASSERT_EQ(0, ft_strcmp(events[10].value.c_str(), "flag"));
    FT_ASSERT_EQ(true, events[11].bool_value);
    FT_ASSERT_EQ(0, ft_strcmp(events[12].value.c_str(), "text"));
    FT_ASSERT_EQ(0, ft_strcmp(events[13].value.c_str(), "hello"));
    return (1);
}

FT_TEST(test_json_stream_writer_round_trip, "json_stream_writer_process reproduces input structure")
{
    const char *json_text;
    ft_vector<captured_event> events;
    json_stream_writer writer;
    ft_string encoded;
    size_t index;
    size_t count;

    json_text = "{\n  \"numbers\": [1, 2, 3],\n  \"nested\": {\n    \"flag\": true,\n    \"text\": \"hello\"\n  }\n}\n";
    FT_ASSERT_EQ(0, gather_events(json_text, events));
    FT_ASSERT_EQ(0, json_stream_writer_init(&writer, string_sink_write, &encoded));
    index = 0;
    count = events.size();
    while (index < count)
    {
        json_stream_event event;

        ft_bzero(&event, sizeof(event));
        event.type = events[index].type;
        event.bool_value = events[index].bool_value;
        if (events[index].value.size() > 0)
        {
            event.value.data = events[index].value.c_str();
            event.value.length = events[index].value.size();
        }
        else
        {
            event.value.data = ft_nullptr;
            event.value.length = 0;
        }
        FT_ASSERT_EQ(0, json_stream_writer_process(&writer, &event));
        index += 1;
    }
    FT_ASSERT_EQ(0, json_stream_writer_finish(&writer));
    json_stream_writer_destroy(&writer);
    FT_ASSERT_EQ(0, ft_strcmp(encoded.c_str(), "{\"numbers\":[1,2,3],\"nested\":{\"flag\":true,\"text\":\"hello\"}}"));
    return (1);
}
