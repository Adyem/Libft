#include "xml.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/basic.hpp"

static int32_t xml_serializer_store_string(
    ft_string &output, const char *serialized_content) noexcept
{
    int32_t operation_error;

    if (serialized_content == ft_nullptr)
        return (FT_ERR_INVALID_STATE);
    if (output.is_initialised() == FT_FALSE)
    {
        operation_error = output.initialize();
        if (operation_error != FT_ERR_SUCCESS)
            return (operation_error);
    }
    operation_error = output.assign(serialized_content,
            static_cast<ft_size_t>(ft_strlen(serialized_content)));
    if (operation_error != FT_ERR_SUCCESS)
        return (operation_error);
    return (FT_ERR_SUCCESS);
}

int32_t xml_serialize_to_string(xml_serialize_callback serialize_callback,
    void *user_data, ft_string &output) noexcept
{
    xml_document document;
    char *serialized_content;
    int32_t operation_error;

    if (serialize_callback == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    operation_error = document.initialize();
    if (operation_error != FT_ERR_SUCCESS)
        return (operation_error);
    operation_error = serialize_callback(document, user_data);
    if (operation_error != FT_ERR_SUCCESS)
        return (operation_error);
    serialized_content = document.write_to_string();
    if (serialized_content == ft_nullptr)
    {
        operation_error = document.get_error();
        if (operation_error == FT_ERR_SUCCESS)
            operation_error = FT_ERR_INVALID_STATE;
        return (operation_error);
    }
    operation_error = xml_serializer_store_string(output, serialized_content);
    cma_free(serialized_content);
    return (operation_error);
}

int32_t xml_serialize_to_backend(xml_serialize_callback serialize_callback,
    void *user_data, ft_document_sink &sink) noexcept
{
    xml_document document;
    int32_t operation_error;

    if (serialize_callback == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    operation_error = document.initialize();
    if (operation_error != FT_ERR_SUCCESS)
        return (operation_error);
    operation_error = serialize_callback(document, user_data);
    if (operation_error != FT_ERR_SUCCESS)
        return (operation_error);
    operation_error = document.write_to_backend(sink);
    return (operation_error);
}

int32_t xml_deserialize_from_string(const char *content,
    xml_deserialize_callback deserialize_callback, void *user_data) noexcept
{
    xml_document document;
    int32_t operation_error;

    if (content == ft_nullptr || deserialize_callback == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    operation_error = document.initialize();
    if (operation_error != FT_ERR_SUCCESS)
        return (operation_error);
    operation_error = document.load_from_string(content);
    if (operation_error != FT_ERR_SUCCESS)
        return (operation_error);
    operation_error = deserialize_callback(document, user_data);
    return (operation_error);
}

int32_t xml_deserialize_from_backend(ft_document_source &source,
    xml_deserialize_callback deserialize_callback, void *user_data) noexcept
{
    xml_document document;
    int32_t operation_error;

    if (deserialize_callback == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    operation_error = document.initialize();
    if (operation_error != FT_ERR_SUCCESS)
        return (operation_error);
    operation_error = document.load_from_backend(source);
    if (operation_error != FT_ERR_SUCCESS)
        return (operation_error);
    operation_error = deserialize_callback(document, user_data);
    return (operation_error);
}
