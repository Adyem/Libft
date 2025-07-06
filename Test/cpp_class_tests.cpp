#include "../CPP_class/string_class.hpp"
#include "../CPP_class/data_buffer.hpp"
#include "../CPP_class/file.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <string>

int test_ft_string_append(void)
{
    ft_string s;
    s.append('h');
    s.append('i');
    return (s.size() == 2 && std::strcmp(s.c_str(), "hi") == 0);
}

int test_ft_string_concat(void)
{
    ft_string a("Hello");
    ft_string b("World");
    ft_string c = a + " " + b;
    return (c == "Hello World");
}

int test_data_buffer_io(void)
{
    DataBuffer buf;
    buf << 42 << std::string("abc");
    int i = 0;
    std::string s;
    buf >> i >> s;
    return (i == 42 && s == "abc" && buf.good());
}

int test_ft_file_write_read(void)
{
    const char *fname = "tmp_cpp_file.txt";
    {
        ft_file file(fname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (file.get_fd() < 0)
            return 0;
        if (file.write("hello") <= 0)
            return 0;
    }
    {
        ft_file file(fname, O_RDONLY);
        if (file.get_fd() < 0)
            return 0;
        char buf[6] = {0};
        if (file.read(buf, 5) != 5)
            return 0;
        file.close();
        ::unlink(fname);
        return (std::strcmp(buf, "hello") == 0);
    }
}
