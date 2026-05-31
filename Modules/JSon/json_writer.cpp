#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <new>
#include "json.hpp"
#include "document.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../CPP_class/class_big_number.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"

static thread_local int32_t g_json_writer_last_error = FT_ERR_SUCCESS;

static void json_writer_set_error(int32_t error_code)
{
    g_json_writer_last_error = error_code;
    return ;
}

#define JSON_WRITER_ERROR_RETURN(code, value) \
    do { json_writer_set_error(code); return (value); } while (0)

#define JSON_WRITER_SUCCESS_RETURN(value) \
    do { json_writer_set_error(FT_ERR_SUCCESS); return (value); } while (0)

static char *json_writer_return_failure(void)
{
    if (g_json_writer_last_error == FT_ERR_SUCCESS)
        json_writer_set_error(FT_ERR_NO_MEMORY);
    return (ft_nullptr);
}

int32_t json_write_to_file(const char *file_path, json_group *groups)
{
    if (!file_path)
    {
        JSON_WRITER_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, -1);
    }
    int32_t file_descriptor = su_open(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_descriptor < 0)
        return (FT_ERR_INVALID_STATE);
    pf_printf_fd(file_descriptor, "{\n");
    json_group *group_iterator = groups;
    while (group_iterator)
    {
        pf_printf_fd(file_descriptor, "  \"%s\": {\n", group_iterator->name);
        json_item *item_iterator = group_iterator->items;
        while (item_iterator)
        {
            const char *value_text = item_iterator->value;
            ft_bool is_big_number_value = FT_FALSE;
            if (item_iterator->is_big_number == FT_TRUE && item_iterator->big_number)
            {
                value_text = item_iterator->big_number->c_str();
                is_big_number_value = FT_TRUE;
            }
            if (item_iterator->next)
            {
                if (is_big_number_value == FT_TRUE)
                    pf_printf_fd(file_descriptor,
                        "    \"%s\": %s,\n", item_iterator->key, value_text);
                else
                    pf_printf_fd(file_descriptor,
                        "    \"%s\": \"%s\",\n", item_iterator->key, value_text);
            }
            else
            {
                if (is_big_number_value == FT_TRUE)
                    pf_printf_fd(file_descriptor,
                        "    \"%s\": %s\n", item_iterator->key, value_text);
                else
                    pf_printf_fd(file_descriptor,
                        "    \"%s\": \"%s\"\n", item_iterator->key, value_text);
            }
            item_iterator = item_iterator->next;
        }
        if (group_iterator->next)
            pf_printf_fd(file_descriptor, "  },\n");
        else
            pf_printf_fd(file_descriptor, "  }\n");
        group_iterator = group_iterator->next;
    }
    pf_printf_fd(file_descriptor, "}\n");
    if (cmp_close(file_descriptor) != FT_ERR_SUCCESS)
        return (FT_ERR_INVALID_STATE);
    JSON_WRITER_SUCCESS_RETURN(0);
}

int32_t json_write_to_backend(ft_document_sink &sink, json_group *groups)
{
    char *serialized_content;
    ft_size_t serialized_length;
    int32_t write_result;

    serialized_content = json_write_to_string(groups);
    if (!serialized_content)
        return (FT_ERR_INVALID_STATE);
    serialized_length = ft_strlen(serialized_content);
    write_result = sink.write_all(serialized_content, serialized_length);
    cma_free(serialized_content);
    if (write_result != FT_ERR_SUCCESS)
    {
        json_writer_set_error(write_result);
        return (FT_ERR_INVALID_STATE);
    }
    JSON_WRITER_SUCCESS_RETURN(0);
}

char *json_write_to_string(json_group *groups)
{
    char *result = adv_strdup("{\n");
    if (!result)
        return (json_writer_return_failure());
    json_group *group_iterator = groups;
    while (group_iterator)
    {
        char *line = adv_strjoin_multiple(3, "  \"", group_iterator->name, "\": {\n");
        if (!line)
        {
            cma_free(result);
            return (json_writer_return_failure());
        }
        char *temporary = adv_strjoin(result, line);
        cma_free(result);
        cma_free(line);
        if (!temporary)
            return (json_writer_return_failure());
        result = temporary;
        json_item *item_iterator = group_iterator->items;
        while (item_iterator)
        {
            const char *value_text = item_iterator->value;
            ft_bool is_big_number_value = FT_FALSE;
            if (item_iterator->is_big_number == FT_TRUE && item_iterator->big_number)
            {
                value_text = item_iterator->big_number->c_str();
                is_big_number_value = FT_TRUE;
            }
            if (item_iterator->next)
            {
                if (is_big_number_value == FT_TRUE)
                    line = adv_strjoin_multiple(5, "    \"", item_iterator->key, "\": ", value_text, ",\n");
                else
                    line = adv_strjoin_multiple(5, "    \"", item_iterator->key, "\": \"", value_text, "\",\n");
            }
            else
            {
                if (is_big_number_value == FT_TRUE)
                    line = adv_strjoin_multiple(5, "    \"", item_iterator->key, "\": ", value_text, "\n");
                else
                    line = adv_strjoin_multiple(5, "    \"", item_iterator->key, "\": \"", value_text, "\"\n");
            }
            if (!line)
            {
                cma_free(result);
                return (json_writer_return_failure());
            }
            temporary = adv_strjoin(result, line);
            cma_free(result);
            cma_free(line);
            if (!temporary)
                return (json_writer_return_failure());
            result = temporary;
            item_iterator = item_iterator->next;
        }
        if (group_iterator->next)
            line = adv_strdup("  },\n");
        else
            line = adv_strdup("  }\n");
        if (!line)
        {
            cma_free(result);
            return (json_writer_return_failure());
        }
        temporary = adv_strjoin(result, line);
        cma_free(result);
        cma_free(line);
        if (!temporary)
            return (json_writer_return_failure());
        result = temporary;
        group_iterator = group_iterator->next;
    }
    char *end_line = adv_strdup("}\n");
    if (!end_line)
    {
        cma_free(result);
        return (json_writer_return_failure());
    }
    char *temporary = adv_strjoin(result, end_line);
    cma_free(result);
    cma_free(end_line);
    if (!temporary)
        return (json_writer_return_failure());
    result = temporary;
    JSON_WRITER_SUCCESS_RETURN(result);
}

int32_t json_document_write_to_file(const char *file_path, const json_document &document)
{
    return (document.write_to_file(file_path));
}

char *json_document_write_to_string(const json_document &document)
{
    return (document.write_to_string());
}

int32_t json_document_write_to_backend(ft_document_sink &sink, const json_document &document)
{
    return (document.write_to_backend(sink));
}
