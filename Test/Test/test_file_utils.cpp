#include "../../File/file_utils.hpp"
#include "../../File/open_dir.hpp"
#include "../../Basic/basic.hpp"
#include "../../CPP_class/class_file.hpp"
#include "../../CPP_class/class_file_stream.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Compatebility/compatebility_internal.hpp"
#include "../Compatebility/compatebility_system_test_hooks.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include <cstddef>
#include <cstdio>
#include <cstring>

#if defined(_WIN32) || defined(_WIN64)
# include <fcntl.h>
# define TEST_FILE_BINARY_FLAG O_BINARY
#else
# include <fcntl.h>
# define TEST_FILE_BINARY_FLAG 0
#endif

#if defined(_WIN32) || defined(_WIN64)
# include <windows.h>
#else
# include <unistd.h>
#endif

static void remove_directory_if_present(const char *directory_path)
{
    int error_code = FT_ERR_SUCCESSS;

    if (cmp_directory_exists(directory_path, &error_code) != 1)
        return ;
#if defined(_WIN32) || defined(_WIN64)
    RemoveDirectoryA(directory_path);
#else
    rmdir(directory_path);
#endif
    return ;
}

static void create_cross_device_test_file(const char *path)
{
    FILE *file_pointer;

    file_pointer = ft_fopen(path, "w");
    if (file_pointer != ft_nullptr)
    {
        std::fputs("cross-device move payload", file_pointer);
        ft_fclose(file_pointer);
    }
    return ;
}

static void write_payload(const char *path, const unsigned char *payload, size_t payload_size)
{
    FILE *file_pointer;
    size_t total_written;
    size_t chunk_written;

    file_pointer = ft_fopen(path, "wb");
    if (file_pointer == ft_nullptr)
        return ;
    total_written = 0;
    while (total_written < payload_size)
    {
        chunk_written = std::fwrite(payload + total_written, 1, payload_size - total_written, file_pointer);
        if (chunk_written == 0)
            break ;
        total_written += chunk_written;
    }
    ft_fclose(file_pointer);
    return ;
}

static size_t read_payload(const char *path, unsigned char *buffer, size_t buffer_capacity)
{
    FILE *file_pointer;
    size_t total_read;
    size_t chunk_read;

    if (buffer == ft_nullptr || buffer_capacity == 0)
        return (0);
    file_pointer = ft_fopen(path, "rb");
    if (file_pointer == ft_nullptr)
        return (0);
    total_read = 0;
    while (total_read < buffer_capacity)
    {
        chunk_read = std::fread(buffer + total_read, 1, buffer_capacity - total_read, file_pointer);
        if (chunk_read == 0)
            break ;
        total_read += chunk_read;
    }
    ft_fclose(file_pointer);
    return (total_read);
}

static void build_path_from_template(char *destination, const char *path_template, char separator)
{
    size_t index;

    index = 0;
    while (path_template[index] != '\0')
    {
        if (path_template[index] == '/')
            destination[index] = separator;
        else
            destination[index] = path_template[index];
        ++index;
    }
    destination[index] = '\0';
    return ;
}

static void build_native_path(char *destination, const char *path_template)
{
    build_path_from_template(destination, path_template, cmp_path_separator());
    return ;
}

static void build_alternate_separator_path(char *destination, const char *path_template)
{
    char path_separator;
    char alternate_separator;

    path_separator = cmp_path_separator();
    if (path_separator == '/')
        alternate_separator = '\\';
    else
        alternate_separator = '/';
    build_path_from_template(destination, path_template, alternate_separator);
    return ;
}

FT_TEST(test_file_path_join_prefers_absolute_right, "file_path_join returns absolute right operand")
{
    ft_string result = file_path_join("/etc", "/var/log");

    FT_ASSERT_EQ(FT_ERR_SUCCESSS, result.get_error());
    FT_ASSERT_EQ(0, ft_strcmp(result.c_str(), "/var/log"));
    return (1);
}

FT_TEST(test_file_path_join_keeps_drive_letter, "file_path_join keeps Windows drive absolute path")
{
    ft_string result = file_path_join("/left", "C:/temp");

    FT_ASSERT_EQ(FT_ERR_SUCCESSS, result.get_error());
    FT_ASSERT_EQ(0, ft_strcmp(result.c_str(), "C:/temp"));
    return (1);
}

FT_TEST(test_file_path_normalize_collapses_duplicates, "file_path_normalize collapses duplicate separators")
{
    char path_buffer[64];
    char expected_buffer[64];
    ft_string normalized;

    build_alternate_separator_path(path_buffer, "/folder///sub///file.txt");
    normalized = file_path_normalize(path_buffer);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, normalized.get_error());
    build_native_path(expected_buffer, "/folder/sub/file.txt");
    FT_ASSERT_EQ(0, ft_strcmp(normalized.c_str(), expected_buffer));
    return (1);
}

FT_TEST(test_file_path_normalize_preserves_trailing_separator, "file_path_normalize preserves trailing separators")
{
    char path_buffer[64];
    char expected_buffer[64];
    ft_string normalized;

    build_alternate_separator_path(path_buffer, "/folder/subdir///");
    normalized = file_path_normalize(path_buffer);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, normalized.get_error());
    build_native_path(expected_buffer, "/folder/subdir/");
    FT_ASSERT_EQ(0, ft_strcmp(normalized.c_str(), expected_buffer));
    return (1);
}

FT_TEST(test_file_path_normalize_handles_null_input, "file_path_normalize gracefully handles null input")
{
    ft_string normalized;

    normalized = file_path_normalize(ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, normalized.get_error());
    FT_ASSERT_EQ(0, ft_strcmp(normalized.c_str(), ""));
    return (1);
}

FT_TEST(test_file_path_join_appends_missing_separator, "file_path_join inserts missing separator between components")
{
    char left_buffer[64];
    char right_buffer[64];
    char expected_buffer[128];
    ft_string result;

    build_alternate_separator_path(left_buffer, "/var//log");
    build_alternate_separator_path(right_buffer, "nginx///access.log");
    result = file_path_join(left_buffer, right_buffer);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, result.get_error());
    build_native_path(expected_buffer, "/var/log/nginx/access.log");
    FT_ASSERT_EQ(0, ft_strcmp(result.c_str(), expected_buffer));
    return (1);
}

FT_TEST(test_file_path_join_with_empty_left, "file_path_join handles empty left operand")
{
    char right_buffer[64];
    char expected_buffer[64];
    ft_string result;

    build_alternate_separator_path(right_buffer, "folder//file.txt");
    result = file_path_join("", right_buffer);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, result.get_error());
    build_native_path(expected_buffer, "folder/file.txt");
    FT_ASSERT_EQ(0, ft_strcmp(result.c_str(), expected_buffer));
    return (1);
}

FT_TEST(test_file_move_cross_device_fallback, "file_move falls back to copy and delete when forced cross-device")
{
    const char *source_path = "test_file_move_cross_device_source.txt";
    const char *destination_path = "test_file_move_cross_device_destination.txt";
    FILE *destination_file;
    char destination_buffer[64];

    file_delete(source_path);
    file_delete(destination_path);
    create_cross_device_test_file(source_path);
    cmp_set_force_cross_device_move(1);
    FT_ASSERT_EQ(0, file_move(source_path, destination_path));
    cmp_set_force_cross_device_move(0);
    FT_ASSERT_EQ(1, file_exists(destination_path));
    FT_ASSERT_EQ(0, file_exists(source_path));
    destination_file = ft_fopen(destination_path, "r");
    FT_ASSERT(destination_file != ft_nullptr);
    if (destination_file == ft_nullptr)
        return (0);
    FT_ASSERT(std::fgets(destination_buffer, 64, destination_file) != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp(destination_buffer, "cross-device move payload"));
    FT_ASSERT_EQ(FT_SUCCESS, ft_fclose(destination_file));
    FT_ASSERT_EQ(0, file_delete(destination_path));
    return (1);
}

FT_TEST(test_file_dir_exists_matches_file_exists_semantics, "file_dir_exists returns 1 when directory exists")
{
    const char *directory_path = "test_file_dir_exists_matches_file_exists_semantics";

    remove_directory_if_present(directory_path);
    FT_ASSERT_EQ(0, file_dir_exists(directory_path));
    FT_ASSERT_EQ(0, file_create_directory(directory_path, 0777));
    FT_ASSERT_EQ(1, file_dir_exists(directory_path));
    remove_directory_if_present(directory_path);
    FT_ASSERT_EQ(0, file_dir_exists(directory_path));
    return (1);
}

FT_TEST(test_file_copy_buffer_size_matches_ft_file, "file_default_copy_buffer_size mirrors ft_file default")
{
    FT_ASSERT_EQ(ft_file_default_buffer_size(), file_default_copy_buffer_size());
    return (1);
}

FT_TEST(test_file_copy_with_buffer_small_chunks, "file_copy_with_buffer streams data through ft_file")
{
    const char *source_path = "test_file_copy_with_buffer_small_chunks_source.bin";
    const char *destination_path = "test_file_copy_with_buffer_small_chunks_destination.bin";
    unsigned char payload[19] = {0};
    unsigned char read_source[32];
    unsigned char read_destination[32];
    size_t index;
    size_t source_size;
    size_t destination_size;

    index = 0;
    while (index < sizeof(payload))
    {
        payload[index] = static_cast<unsigned char>(index * 7 + 3);
        index += 1;
    }
    file_delete(source_path);
    file_delete(destination_path);
    write_payload(source_path, payload, sizeof(payload));
    FT_ASSERT_EQ(0, file_copy_with_buffer(source_path, destination_path, 5));
    source_size = read_payload(source_path, read_source, sizeof(read_source));
    destination_size = read_payload(destination_path, read_destination, sizeof(read_destination));
    FT_ASSERT_EQ(source_size, destination_size);
    FT_ASSERT_EQ(0, ft_memcmp(read_source, read_destination, source_size));
    file_delete(source_path);
    file_delete(destination_path);
    return (1);
}

FT_TEST(test_file_copy_with_buffer_rejects_null_paths, "file_copy_with_buffer validates inputs")
{
    int error_code = FT_ERR_SUCCESSS;

    FT_ASSERT_EQ(-1, file_copy_with_buffer(ft_nullptr, "ignored.bin", 4));
    error_code = ft_global_error_stack_drop_last_error();
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, error_code);
    FT_ASSERT_EQ(-1, file_copy_with_buffer("ignored.bin", ft_nullptr, 4));
    error_code = ft_global_error_stack_drop_last_error();
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, error_code);
    return (1);
}

FT_TEST(test_ft_file_copy_to_propagates_errors, "ft_file copy_to reports invalid handles and arguments")
{
    ft_file source_file;
    int error_code;

    FT_ASSERT_EQ(-1, source_file.copy_to("ft_file_copy_to_destination.bin"));
    error_code = ft_global_error_stack_drop_last_error();
    FT_ASSERT_EQ(FT_ERR_INVALID_HANDLE, error_code);
    FT_ASSERT_EQ(-1, source_file.copy_to(ft_nullptr));
    error_code = ft_global_error_stack_drop_last_error();
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, error_code);
    return (1);
}

FT_TEST(test_ft_file_copy_to_streams_payload, "ft_file copy_to streams using shared buffer helper")
{
    const char *source_path = "test_ft_file_copy_to_streams_payload_source.bin";
    const char *destination_path = "test_ft_file_copy_to_streams_payload_destination.bin";
    unsigned char payload[23] = {0};
    unsigned char destination_payload[32];
    unsigned char source_payload[32];
    ft_file source_file;
    size_t index;
    size_t source_size;
    size_t destination_size;
    int open_flags;

    index = 0;
    while (index < sizeof(payload))
    {
        payload[index] = static_cast<unsigned char>(index * 5 + 11);
        index += 1;
    }
    file_delete(source_path);
    file_delete(destination_path);
    write_payload(source_path, payload, sizeof(payload));
    open_flags = O_RDONLY | TEST_FILE_BINARY_FLAG;
    FT_ASSERT_EQ(0, source_file.open(source_path, open_flags));
    FT_ASSERT_EQ(0, source_file.copy_to_with_buffer(destination_path, 8));
    source_size = read_payload(source_path, source_payload, sizeof(source_payload));
    destination_size = read_payload(destination_path, destination_payload, sizeof(destination_payload));
    FT_ASSERT_EQ(source_size, destination_size);
    FT_ASSERT_EQ(0, ft_memcmp(source_payload, destination_payload, source_size));
    file_delete(source_path);
    file_delete(destination_path);
    return (1);
}

FT_TEST(test_file_readdir_handles_null_stream, "file_readdir sets FT_ERR_INVALID_ARGUMENT when stream is null")
{
    int error_code = FT_ERR_SUCCESSS;

    FT_ASSERT_EQ(ft_nullptr, file_readdir(ft_nullptr));
    error_code = ft_global_error_stack_drop_last_error();
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, error_code);
    return (1);
}

FT_TEST(test_file_readdir_clears_error_on_success, "file_readdir sets FT_ERR_SUCCESSS after reading entry")
{
    file_dir *directory_stream;
    file_dirent *directory_entry;
    int error_code;

    directory_stream = file_opendir(".");
    FT_ASSERT(directory_stream != ft_nullptr);
    if (directory_stream == ft_nullptr)
        return (0);
    directory_entry = file_readdir(directory_stream);
    FT_ASSERT(directory_entry != ft_nullptr);
    error_code = ft_global_error_stack_drop_last_error();
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, error_code);
    FT_ASSERT_EQ(0, file_closedir(directory_stream));
    ft_global_error_stack_drop_last_error();
    return (1);
}

FT_TEST(test_file_closedir_handles_null_stream, "file_closedir sets FT_ERR_INVALID_ARGUMENT when stream is null")
{
    int error_code = FT_ERR_SUCCESSS;

    FT_ASSERT_EQ(-1, file_closedir(ft_nullptr));
    error_code = ft_global_error_stack_drop_last_error();
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, error_code);
    return (1);
}

FT_TEST(test_file_closedir_clears_error_on_success, "file_closedir sets FT_ERR_SUCCESSS after close")
{
    file_dir *directory_stream;
    int error_code;

    directory_stream = file_opendir(".");
    FT_ASSERT(directory_stream != ft_nullptr);
    if (directory_stream == ft_nullptr)
        return (0);
    FT_ASSERT_EQ(0, file_closedir(directory_stream));
    error_code = ft_global_error_stack_drop_last_error();
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, error_code);
    return (1);
}

FT_TEST(test_file_copy_with_buffer_zero_size_uses_default,
        "file_copy_with_buffer accepts zero buffer size and still copies data")
{
    const char  *source_path;
    const char  *destination_path;
    const char  *payload_string;
    FILE        *file_stream;
    char        read_buffer[32];
    size_t      payload_length;
    size_t      bytes_written;
    size_t      bytes_read;

    source_path = "test_file_copy_with_buffer_zero_size_source.txt";
    destination_path = "test_file_copy_with_buffer_zero_size_destination.txt";
    payload_string = "bufferless copy payload";
    payload_length = ft_strlen_size_t(payload_string);
    file_delete(source_path);
    file_delete(destination_path);
    file_stream = std::fopen(source_path, "wb");
    if (file_stream == ft_nullptr)
        return (0);
    bytes_written = std::fwrite(payload_string, 1, payload_length, file_stream);
    std::fclose(file_stream);
    if (bytes_written != payload_length)
    {
        file_delete(source_path);
        file_delete(destination_path);
        return (0);
    }
    if (file_copy_with_buffer(source_path, destination_path, 0) != 0)
    {
        file_delete(source_path);
        file_delete(destination_path);
        return (0);
    }
    if (ft_global_error_stack_drop_last_error() != FT_ERR_SUCCESSS)
    {
        file_delete(source_path);
        file_delete(destination_path);
        return (0);
    }
    file_stream = std::fopen(destination_path, "rb");
    if (file_stream == ft_nullptr)
    {
        file_delete(source_path);
        file_delete(destination_path);
        return (0);
    }
    std::memset(read_buffer, 0, sizeof(read_buffer));
    bytes_read = std::fread(read_buffer, 1, payload_length, file_stream);
    std::fclose(file_stream);
    if (bytes_read != payload_length)
    {
        file_delete(source_path);
        file_delete(destination_path);
        return (0);
    }
    if (std::memcmp(read_buffer, payload_string, payload_length) != 0)
    {
        file_delete(source_path);
        file_delete(destination_path);
        return (0);
    }
    file_delete(source_path);
    file_delete(destination_path);
    return (1);
}
