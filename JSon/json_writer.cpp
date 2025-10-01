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
#include "../CPP_class/class_nullptr.hpp"
#include "../CPP_class/class_big_number.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"

static char *json_writer_return_failure(void)
{
    if (ft_errno == ER_SUCCESS)
        ft_errno = FT_EALLOC;
    return (ft_nullptr);
}

int json_write_to_file(const char *file_path, json_group *groups)
{
    if (!file_path)
    {
        ft_errno = FT_EINVAL;
        return (-1);
    }
    int file_descriptor = su_open(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_descriptor < 0)
        return (-1);
    pf_printf_fd(file_descriptor, "{\n");
    json_group *group_iterator = groups;
    while (group_iterator)
    {
        pf_printf_fd(file_descriptor, "  \"%s\": {\n", group_iterator->name);
        json_item *item_iterator = group_iterator->items;
        while (item_iterator)
        {
            const char *value_text = item_iterator->value;
            bool is_big_number_value = false;
            if (item_iterator->is_big_number == true && item_iterator->big_number)
            {
                value_text = item_iterator->big_number->c_str();
                is_big_number_value = true;
            }
            if (item_iterator->next)
            {
                if (is_big_number_value == true)
                    pf_printf_fd(file_descriptor,
                        "    \"%s\": %s,\n", item_iterator->key, value_text);
                else
                    pf_printf_fd(file_descriptor,
                        "    \"%s\": \"%s\",\n", item_iterator->key, value_text);
            }
            else
            {
                if (is_big_number_value == true)
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
    if (cmp_close(file_descriptor) != 0)
        return (-1);
    ft_errno = ER_SUCCESS;
    return (0);
}

char *json_write_to_string(json_group *groups)
{
    char *result = cma_strdup("{\n");
    if (!result)
        return (json_writer_return_failure());
    json_group *group_iterator = groups;
    while (group_iterator)
    {
        char *line = cma_strjoin_multiple(3, "  \"", group_iterator->name, "\": {\n");
        if (!line)
        {
            cma_free(result);
            return (json_writer_return_failure());
        }
        char *temporary = cma_strjoin(result, line);
        cma_free(result);
        cma_free(line);
        if (!temporary)
            return (json_writer_return_failure());
        result = temporary;
        json_item *item_iterator = group_iterator->items;
        while (item_iterator)
        {
            const char *value_text = item_iterator->value;
            bool is_big_number_value = false;
            if (item_iterator->is_big_number == true && item_iterator->big_number)
            {
                value_text = item_iterator->big_number->c_str();
                is_big_number_value = true;
            }
            if (item_iterator->next)
            {
                if (is_big_number_value == true)
                    line = cma_strjoin_multiple(5, "    \"", item_iterator->key, "\": ", value_text, ",\n");
                else
                    line = cma_strjoin_multiple(5, "    \"", item_iterator->key, "\": \"", value_text, "\",\n");
            }
            else
            {
                if (is_big_number_value == true)
                    line = cma_strjoin_multiple(5, "    \"", item_iterator->key, "\": ", value_text, "\n");
                else
                    line = cma_strjoin_multiple(5, "    \"", item_iterator->key, "\": \"", value_text, "\"\n");
            }
            if (!line)
            {
                cma_free(result);
                return (json_writer_return_failure());
            }
            temporary = cma_strjoin(result, line);
            cma_free(result);
            cma_free(line);
            if (!temporary)
                return (json_writer_return_failure());
            result = temporary;
            item_iterator = item_iterator->next;
        }
        if (group_iterator->next)
            line = cma_strdup("  },\n");
        else
            line = cma_strdup("  }\n");
        if (!line)
        {
            cma_free(result);
            return (json_writer_return_failure());
        }
        temporary = cma_strjoin(result, line);
        cma_free(result);
        cma_free(line);
        if (!temporary)
            return (json_writer_return_failure());
        result = temporary;
        group_iterator = group_iterator->next;
    }
    char *end_line = cma_strdup("}\n");
    if (!end_line)
    {
        cma_free(result);
        return (json_writer_return_failure());
    }
    char *temporary = cma_strjoin(result, end_line);
    cma_free(result);
    cma_free(end_line);
    if (!temporary)
        return (json_writer_return_failure());
    result = temporary;
    ft_errno = ER_SUCCESS;
    return (result);
}

int json_document_write_to_file(const char *file_path, const json_document &document)
{
    return (document.write_to_file(file_path));
}

char *json_document_write_to_string(const json_document &document)
{
    return (document.write_to_string());
}

