#include "../../Modules/Buffer/byte_buffer.hpp"
#include "../../Modules/CMA/CMA.hpp"
#include "../../Modules/Config/config.hpp"
#include "../../Modules/Encoding/encoding.hpp"
#include "../../Modules/File/file_utils.hpp"
#include "../../Modules/Filesystem/filesystem.hpp"
#include "../../Modules/Basic/limits.hpp"
#include <cstring>

int32_t ft_byte_buffer::prepend_buffer(const ft_byte_buffer &other) noexcept
{
    ft_byte_buffer snapshot;
    const ft_byte_buffer *first_buffer;
    const ft_byte_buffer *second_buffer;
    uintptr_t this_address;
    uintptr_t other_address;
    ft_size_t destination_size;
    ft_size_t source_size;
    int32_t lock_error;
    int32_t prepend_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "ft_byte_buffer::prepend_buffer");
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(other._initialised_state,
            "ft_byte_buffer::prepend_buffer",
            "called with uninitialised source object");
    if (&other == this)
    {
        prepend_error = snapshot.initialize(*this);
        if (prepend_error != FT_ERR_SUCCESS)
            return (ft_byte_buffer::set_error(prepend_error));
        prepend_error = this->prepend_buffer(snapshot);
        (void)snapshot.destroy();
        return (ft_byte_buffer::set_error(prepend_error));
    }
    this_address = reinterpret_cast<uintptr_t>(this);
    other_address = reinterpret_cast<uintptr_t>(&other);
    if (this_address < other_address)
    {
        first_buffer = this;
        second_buffer = &other;
    }
    else
    {
        first_buffer = &other;
        second_buffer = this;
    }
    lock_error = first_buffer->lock_internal();
    if (lock_error != FT_ERR_SUCCESS)
        return (ft_byte_buffer::set_error(lock_error));
    if (second_buffer != first_buffer)
    {
        lock_error = second_buffer->lock_internal();
        if (lock_error != FT_ERR_SUCCESS)
        {
            first_buffer->unlock_internal();
            return (ft_byte_buffer::set_error(lock_error));
        }
    }
    destination_size = this->_size;
    source_size = other._size;
    prepend_error = FT_ERR_SUCCESS;
    if (destination_size + source_size < destination_size)
        prepend_error = FT_ERR_OUT_OF_RANGE;
    else
        prepend_error = this->reserve(destination_size + source_size);
    if (prepend_error == FT_ERR_SUCCESS)
    {
        if (source_size > 0)
        {
            if (other._data == ft_nullptr)
                prepend_error = FT_ERR_INVALID_STATE;
            else
            {
                if (destination_size > 0)
                    std::memmove(this->_data + source_size, this->_data,
                        destination_size);
                std::memcpy(this->_data, other._data, source_size);
                this->_size = destination_size + source_size;
                this->_read_position += source_size;
            }
        }
        else
            this->_size = destination_size;
    }
    if (second_buffer != first_buffer)
        second_buffer->unlock_internal();
    first_buffer->unlock_internal();
    return (ft_byte_buffer::set_error(prepend_error));
}

config_data *config_reload_file(const char *filename)
{
    return (config_load_file(filename));
}

int32_t config_save_file(const config_data *config, const char *filename)
{
    return (config_write_file(config, filename));
}

ft_file_watch *config_watch_file(const char *filename,
    file_watch_callback callback, void *user_data)
{
    ft_file_watch *file_watch;
    char *directory_name;
    int32_t initialize_error;
    int32_t watch_error;

    if (filename == ft_nullptr || callback == ft_nullptr)
        return (ft_nullptr);
    directory_name = file_path_dirname(filename);
    if (directory_name == ft_nullptr)
        return (ft_nullptr);
    file_watch = new (std::nothrow) ft_file_watch();
    if (file_watch == ft_nullptr)
    {
        cma_free(directory_name);
        return (ft_nullptr);
    }
    initialize_error = file_watch->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete file_watch;
        cma_free(directory_name);
        return (ft_nullptr);
    }
    watch_error = file_watch->watch_directory(directory_name, callback, user_data);
    cma_free(directory_name);
    if (watch_error != FT_ERR_SUCCESS)
    {
        (void)file_watch->destroy();
        delete file_watch;
        return (ft_nullptr);
    }
    return (file_watch);
}

static ft_bool filesystem_ascii_case_equal_span(const char *left,
    ft_size_t length, const char *right) noexcept
{
    ft_size_t index;
    char left_character;
    char right_character;

    if (left == ft_nullptr || right == ft_nullptr)
        return (FT_FALSE);
    index = 0;
    while (index < length && right[index] != '\0')
    {
        left_character = left[index];
        right_character = right[index];
        if (left_character >= 'A' && left_character <= 'Z')
            left_character = static_cast<char>(left_character - 'A' + 'a');
        if (right_character >= 'A' && right_character <= 'Z')
            right_character = static_cast<char>(right_character - 'A' + 'a');
        if (left_character != right_character)
            return (FT_FALSE);
        index++;
    }
    if (index != length || right[index] != '\0')
        return (FT_FALSE);
    return (FT_TRUE);
}

int32_t filesystem_split_path(const char *path, ft_string *directory_out,
    ft_string *basename_out)
{
    ft_string *directory_value;
    ft_string *basename_value;
    int32_t error_code;

    if (directory_out == ft_nullptr || basename_out == ft_nullptr
        || directory_out == basename_out)
    {
        return (FT_ERR_INVALID_ARGUMENT);
    }
    directory_value = filesystem_dirname(path);
    if (directory_value == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    basename_value = filesystem_basename(path);
    if (basename_value == ft_nullptr)
    {
        (void)directory_value->destroy();
        delete directory_value;
        return (FT_ERR_NO_MEMORY);
    }
    (void)directory_out->destroy();
    (void)basename_out->destroy();
    error_code = directory_out->initialize(directory_value->c_str());
    if (error_code == FT_ERR_SUCCESS)
        error_code = basename_out->initialize(basename_value->c_str());
    if (error_code != FT_ERR_SUCCESS)
    {
        (void)directory_out->destroy();
        (void)basename_out->destroy();
    }
    (void)directory_value->destroy();
    delete directory_value;
    (void)basename_value->destroy();
    delete basename_value;
    return (error_code);
}

ft_bool filesystem_is_hidden(const char *path) noexcept
{
    char *basename;
    ft_bool result;

    basename = file_path_basename(path);
    if (basename == ft_nullptr)
        return (FT_FALSE);
    result = FT_FALSE;
    if (basename[0] == '.'
        && basename[1] != '\0'
        && !(basename[1] == '.' && basename[2] == '\0'))
        result = FT_TRUE;
    cma_free(basename);
    return (result);
}

ft_bool filesystem_is_reserved_name(const char *path) noexcept
{
    char *basename;
    ft_size_t basename_length;
    ft_size_t trimmed_length;
    ft_size_t stem_length;
    ft_bool result;

    basename = file_path_basename(path);
    if (basename == ft_nullptr)
        return (FT_FALSE);
    basename_length = 0;
    while (basename[basename_length] != '\0')
        basename_length++;
    trimmed_length = basename_length;
    while (trimmed_length > 0
        && (basename[trimmed_length - 1] == '.'
            || basename[trimmed_length - 1] == ' '))
    {
        trimmed_length--;
    }
    stem_length = 0;
    while (stem_length < trimmed_length && basename[stem_length] != '.')
        stem_length++;
    result = FT_FALSE;
    if (stem_length == 3 && filesystem_ascii_case_equal_span(basename, 3,
            "CON") == FT_TRUE)
        result = FT_TRUE;
    else if (stem_length == 3 && filesystem_ascii_case_equal_span(basename, 3,
            "PRN") == FT_TRUE)
        result = FT_TRUE;
    else if (stem_length == 3 && filesystem_ascii_case_equal_span(basename, 3,
            "AUX") == FT_TRUE)
        result = FT_TRUE;
    else if (stem_length == 3 && filesystem_ascii_case_equal_span(basename, 3,
            "NUL") == FT_TRUE)
        result = FT_TRUE;
    else if (stem_length == 4 && filesystem_ascii_case_equal_span(basename, 3,
            "COM") == FT_TRUE
        && basename[3] >= '1' && basename[3] <= '9')
    {
        result = FT_TRUE;
    }
    else if (stem_length == 4 && filesystem_ascii_case_equal_span(basename, 3,
            "LPT") == FT_TRUE
        && basename[3] >= '1' && basename[3] <= '9')
    {
        result = FT_TRUE;
    }
    cma_free(basename);
    return (result);
}

ft_bool filesystem_is_rooted(const char *path) noexcept
{
    return (filesystem_is_absolute(path));
}

static char encoding_base32_character(uint8_t value) noexcept
{
    static const char base32_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";

    return (base32_table[value & 0x1FU]);
}

static int32_t encoding_base32_value(char character) noexcept
{
    if (character >= 'A' && character <= 'Z')
        return (character - 'A');
    if (character >= 'a' && character <= 'z')
        return (character - 'a');
    if (character >= '2' && character <= '7')
        return (character - '2' + 26);
    return (-1);
}

static int32_t link_shim_set_encoding_success(void)
{
    char *dummy_output;

    dummy_output = encoding_base64_encode(ft_nullptr, 0);
    if (dummy_output != ft_nullptr)
        cma_free(dummy_output);
    return (FT_ERR_SUCCESS);
}

static int32_t link_shim_set_encoding_invalid_argument(void)
{
    ft_size_t dummy_size;

    dummy_size = 0;
    (void)encoding_base64_decode("A", 1, &dummy_size);
    return (FT_ERR_INVALID_ARGUMENT);
}

char *encoding_base32_encode(const uint8_t *input, ft_size_t input_size)
{
    ft_size_t output_size;
    ft_size_t output_index;
    ft_size_t input_index;
    ft_size_t remaining_size;
    uint8_t byte_one;
    uint8_t byte_two;
    uint8_t byte_three;
    uint8_t byte_four;
    uint8_t byte_five;
    char *output;

    if (input == ft_nullptr && input_size != 0)
    {
        (void)link_shim_set_encoding_invalid_argument();
        return (ft_nullptr);
    }
    if (input_size > (FT_SYSTEM_SIZE_MAX / 8U) * 5U)
    {
        (void)link_shim_set_encoding_invalid_argument();
        return (ft_nullptr);
    }
    output_size = ((input_size + 4) / 5) * 8;
    output = static_cast<char *>(cma_malloc(output_size + 1));
    if (output == ft_nullptr)
    {
        (void)link_shim_set_encoding_invalid_argument();
        return (ft_nullptr);
    }
    input_index = 0;
    output_index = 0;
    while (input_index < input_size)
    {
        remaining_size = input_size - input_index;
        byte_one = input[input_index];
        byte_two = 0U;
        byte_three = 0U;
        byte_four = 0U;
        byte_five = 0U;
        if (remaining_size >= 2)
            byte_two = input[input_index + 1];
        if (remaining_size >= 3)
            byte_three = input[input_index + 2];
        if (remaining_size >= 4)
            byte_four = input[input_index + 3];
        if (remaining_size >= 5)
            byte_five = input[input_index + 4];
        output[output_index++] = encoding_base32_character(
            static_cast<uint8_t>(byte_one >> 3));
        output[output_index++] = encoding_base32_character(
            static_cast<uint8_t>(((byte_one & 0x07U) << 2)
                | (byte_two >> 6)));
        if (remaining_size >= 2)
        {
            output[output_index++] = encoding_base32_character(
                static_cast<uint8_t>((byte_two >> 1) & 0x1FU));
            output[output_index++] = encoding_base32_character(
                static_cast<uint8_t>(((byte_two & 0x01U) << 4)
                    | (byte_three >> 4)));
        }
        else
        {
            output[output_index++] = '=';
            output[output_index++] = '=';
        }
        if (remaining_size >= 3)
        {
            output[output_index++] = encoding_base32_character(
                static_cast<uint8_t>(((byte_three & 0x0FU) << 1)
                    | (byte_four >> 7)));
        }
        else
        {
            output[output_index++] = '=';
        }
        if (remaining_size >= 4)
        {
            output[output_index++] = encoding_base32_character(
                static_cast<uint8_t>((byte_four >> 2) & 0x1FU));
            output[output_index++] = encoding_base32_character(
                static_cast<uint8_t>(((byte_four & 0x03U) << 3)
                    | (byte_five >> 5)));
        }
        else
        {
            output[output_index++] = '=';
            output[output_index++] = '=';
        }
        if (remaining_size >= 5)
        {
            output[output_index++] = encoding_base32_character(
                static_cast<uint8_t>(byte_five & 0x1FU));
        }
        else
            output[output_index++] = '=';
        input_index += 5;
    }
    output[output_index] = '\0';
    (void)link_shim_set_encoding_success();
    return (output);
}

static int32_t encoding_base32_decode_block(const uint8_t values[8],
    ft_size_t data_length, uint8_t *output, ft_size_t *output_size)
{
    if (data_length == 2)
    {
        if ((values[1] & 0x03U) != 0U)
            return (FT_ERR_INVALID_ARGUMENT);
        output[0] = static_cast<uint8_t>((values[0] << 3)
            | (values[1] >> 2));
        *output_size = 1;
        return (FT_ERR_SUCCESS);
    }
    if (data_length == 4)
    {
        if ((values[3] & 0x0FU) != 0U)
            return (FT_ERR_INVALID_ARGUMENT);
        output[0] = static_cast<uint8_t>((values[0] << 3)
            | (values[1] >> 2));
        output[1] = static_cast<uint8_t>(((values[1] & 0x03U) << 6)
            | (values[2] << 1) | (values[3] >> 4));
        *output_size = 2;
        return (FT_ERR_SUCCESS);
    }
    if (data_length == 5)
    {
        if ((values[4] & 0x01U) != 0U)
            return (FT_ERR_INVALID_ARGUMENT);
        output[0] = static_cast<uint8_t>((values[0] << 3)
            | (values[1] >> 2));
        output[1] = static_cast<uint8_t>(((values[1] & 0x03U) << 6)
            | (values[2] << 1) | (values[3] >> 4));
        output[2] = static_cast<uint8_t>(((values[3] & 0x0FU) << 4)
            | (values[4] >> 1));
        *output_size = 3;
        return (FT_ERR_SUCCESS);
    }
    if (data_length == 7)
    {
        if ((values[6] & 0x07U) != 0U)
            return (FT_ERR_INVALID_ARGUMENT);
        output[0] = static_cast<uint8_t>((values[0] << 3)
            | (values[1] >> 2));
        output[1] = static_cast<uint8_t>(((values[1] & 0x03U) << 6)
            | (values[2] << 1) | (values[3] >> 4));
        output[2] = static_cast<uint8_t>(((values[3] & 0x0FU) << 4)
            | (values[4] >> 1));
        output[3] = static_cast<uint8_t>(((values[4] & 0x01U) << 7)
            | (values[5] << 2) | (values[6] >> 3));
        *output_size = 4;
        return (FT_ERR_SUCCESS);
    }
    if (data_length == 8)
    {
        output[0] = static_cast<uint8_t>((values[0] << 3)
            | (values[1] >> 2));
        output[1] = static_cast<uint8_t>(((values[1] & 0x03U) << 6)
            | (values[2] << 1) | (values[3] >> 4));
        output[2] = static_cast<uint8_t>(((values[3] & 0x0FU) << 4)
            | (values[4] >> 1));
        output[3] = static_cast<uint8_t>(((values[4] & 0x01U) << 7)
            | (values[5] << 2) | (values[6] >> 3));
        output[4] = static_cast<uint8_t>(((values[6] & 0x07U) << 5)
            | values[7]);
        *output_size = 5;
        return (FT_ERR_SUCCESS);
    }
    return (FT_ERR_INVALID_ARGUMENT);
}

uint8_t *encoding_base32_decode(const char *input, ft_size_t input_size,
    ft_size_t *output_size)
{
    uint8_t *output;
    ft_size_t output_capacity;
    ft_size_t input_index;
    ft_size_t output_index;
    ft_size_t block_index;
    ft_size_t output_block_size;
    uint8_t block_values[8];
    ft_bool padding_started;
    int32_t value;
    int32_t block_error;

    if (output_size != ft_nullptr)
        *output_size = 0;
    if (input == ft_nullptr && input_size != 0)
    {
        (void)link_shim_set_encoding_invalid_argument();
        return (ft_nullptr);
    }
    output_capacity = ((input_size + 7) / 8) * 5;
    output = static_cast<uint8_t *>(cma_malloc(output_capacity + 1));
    if (output == ft_nullptr)
    {
        (void)link_shim_set_encoding_invalid_argument();
        return (ft_nullptr);
    }
    input_index = 0;
    output_index = 0;
    while (input_index < input_size)
    {
        block_index = 0;
        padding_started = FT_FALSE;
        while (block_index < 8 && input_index < input_size)
        {
            if (input[input_index] == '=')
            {
                padding_started = FT_TRUE;
                input_index++;
                break ;
            }
            value = encoding_base32_value(input[input_index]);
            if (value < 0)
            {
                cma_free(output);
                (void)link_shim_set_encoding_invalid_argument();
                return (ft_nullptr);
            }
            block_values[block_index] = static_cast<uint8_t>(value);
            block_index++;
            input_index++;
        }
        if (padding_started == FT_TRUE)
        {
            while (input_index < input_size)
            {
                if (input[input_index] != '=')
                {
                    cma_free(output);
                    (void)link_shim_set_encoding_invalid_argument();
                    return (ft_nullptr);
                }
                input_index++;
            }
        }
        block_error = encoding_base32_decode_block(block_values, block_index,
                output + output_index, &output_block_size);
        if (block_error != FT_ERR_SUCCESS)
        {
            cma_free(output);
            (void)link_shim_set_encoding_invalid_argument();
            return (ft_nullptr);
        }
        output_index += output_block_size;
        if (padding_started == FT_TRUE)
            break ;
        if (block_index < 8 && input_index != input_size)
        {
            cma_free(output);
            (void)link_shim_set_encoding_invalid_argument();
            return (ft_nullptr);
        }
    }
    if (output_size != ft_nullptr)
        *output_size = output_index;
    output[output_index] = 0;
    (void)link_shim_set_encoding_success();
    return (output);
}
