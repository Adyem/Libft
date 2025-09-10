#ifndef FT_ISTRINGSTREAM_HPP
#define FT_ISTRINGSTREAM_HPP

#include <string>
#include "class_istream.hpp"
#include "class_stringbuf.hpp"

class ft_istringstream : public ft_istream
{
    private:
        ft_stringbuf _buffer;

    public:
        ft_istringstream(const std::string &string) noexcept;
        ~ft_istringstream() noexcept;

        ft_istringstream(const ft_istringstream &) = delete;
        ft_istringstream &operator=(const ft_istringstream &) = delete;

        std::string str() const;

    protected:
        std::size_t do_read(char *buffer, std::size_t count);
};

ft_istringstream &operator>>(ft_istringstream &input, int &value);
ft_istringstream &operator>>(ft_istringstream &input, std::string &value);

#endif
