#include "../CPP_class/class_string_class.hpp"
#include "../CPP_class/class_data_buffer.hpp"
#include "../CPP_class/class_file.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

int test_ft_string_append(void)
{
    ft_string s;
    s.append('h');
    s.append('i');
    return (s.size() == 2 && std::strcmp(s.c_str(), "hi") == 0);
}

int test_ft_string_concat(void)
{
    ft_string first_string("Hello");
    ft_string second_string("World");
    ft_string combined_string = first_string + " " + second_string;
    ft_string copy_string(combined_string);
    ft_string assigned_string;
    assigned_string = combined_string;
    combined_string.append('!');
    ft_string moved_string(std::move(copy_string));
    ft_string move_assigned_string;
    move_assigned_string = std::move(assigned_string);
    bool status = (combined_string == "Hello World!" &&
        moved_string == "Hello World" &&
        move_assigned_string == "Hello World" &&
        copy_string.size() == 0 &&
        assigned_string.size() == 0);
    return (status);
}

int test_data_buffer_io(void)
{
    DataBuffer source_buffer;
    source_buffer << 42 << ft_string("abc");
    DataBuffer copy_buffer(source_buffer);
    DataBuffer assigned_buffer;
    assigned_buffer = source_buffer;
    DataBuffer moved_buffer(std::move(source_buffer));
    DataBuffer move_assigned_buffer;
    move_assigned_buffer = std::move(assigned_buffer);
    int number_copy = 0;
    ft_string string_copy;
    int number_move = 0;
    ft_string string_move;
    int number_move_assign = 0;
    ft_string string_move_assign;
    copy_buffer >> number_copy >> string_copy;
    moved_buffer >> number_move >> string_move;
    move_assigned_buffer >> number_move_assign >> string_move_assign;
    bool status = (number_copy == 42 && string_copy == "abc" &&
        number_move == 42 && string_move == "abc" &&
        number_move_assign == 42 && string_move_assign == "abc" &&
        source_buffer.size() == 0 && assigned_buffer.size() == 0 &&
        copy_buffer.good() && moved_buffer.good() && move_assigned_buffer.good());
    return (status);
}

int test_ft_file_write_read(void)
{
    const char *fname = "tmp_cpp_file.txt";
    {
        ft_file file(fname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (file.get_fd() < 0)
            return (0);
        if (file.write("hello") <= 0)
            return (0);
    }
    {
        ft_file file(fname, O_RDONLY);
        if (file.get_fd() < 0)
            return (0);
        char buf[6] = {0};
        if (file.read(buf, 5) != 5)
            return (0);
        file.close();
        ::unlink(fname);
        return (std::strcmp(buf, "hello") == 0);
    }
}
