#ifndef HTTP2_CLIENT_HPP
#define HTTP2_CLIENT_HPP

#include "../CPP_class/class_string_class.hpp"
#include "../Template/vector.hpp"
#include "../Template/map.hpp"
#include "ssl_wrapper.hpp"
#include <cstdint>
#include <cstddef>

struct http2_header_field
{
    ft_string   name;
    ft_string   value;
};

struct http2_frame
{
    uint8_t     type;
    uint8_t     flags;
    uint32_t    stream_id;
    ft_string   payload;
};

class http2_stream_manager
{
    private:
        void        set_error(int error_code) const noexcept;

        ft_map<uint32_t, ft_string>  _streams;
        mutable int                  _error_code;

    public:
        http2_stream_manager() noexcept;
        ~http2_stream_manager() noexcept;

        bool        open_stream(uint32_t stream_identifier) noexcept;
        bool        append_data(uint32_t stream_identifier, const char *data,
                        size_t length) noexcept;
        bool        close_stream(uint32_t stream_identifier) noexcept;
        bool        get_stream_buffer(uint32_t stream_identifier,
                        ft_string &out_buffer) const noexcept;
        int         get_error() const noexcept;
        const char  *get_error_str() const noexcept;
};

bool    http2_encode_frame(const http2_frame &frame, ft_string &out_buffer,
            int &error_code) noexcept;
bool    http2_decode_frame(const unsigned char *buffer, size_t buffer_size,
            size_t &offset, http2_frame &out_frame, int &error_code) noexcept;
bool    http2_compress_headers(const ft_vector<http2_header_field> &headers,
            ft_string &out_block, int &error_code) noexcept;
bool    http2_decompress_headers(const ft_string &block,
            ft_vector<http2_header_field> &out_headers, int &error_code) noexcept;
bool    http2_select_alpn_protocol(SSL *ssl_session, bool &selected_http2,
            int &error_code) noexcept;

#endif
