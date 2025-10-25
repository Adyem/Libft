#ifndef HTTP2_CLIENT_HPP
#define HTTP2_CLIENT_HPP

#include "../CPP_class/class_string_class.hpp"
#include "../Template/vector.hpp"
#include "../Template/map.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "ssl_wrapper.hpp"
#include <cstdint>
#include <cstddef>

class http2_header_field
{
    private:
        ft_string           _name;
        ft_string           _value;
        mutable int         _error_code;
        mutable bool        _thread_safe_enabled;
        mutable pt_mutex   *_mutex;

        void    set_error(int error_code) const noexcept;
        int     lock(bool *lock_acquired) const noexcept;
        void    unlock(bool lock_acquired) const noexcept;
        void    teardown_thread_safety() noexcept;

    public:
        http2_header_field() noexcept;
        ~http2_header_field() noexcept;

        http2_header_field(const http2_header_field &other) noexcept;
        http2_header_field(http2_header_field &&other) noexcept;
        http2_header_field &operator=(const http2_header_field &other) noexcept;
        http2_header_field &operator=(http2_header_field &&other) noexcept;

        int     enable_thread_safety() noexcept;
        void    disable_thread_safety() noexcept;
        bool    is_thread_safe() const noexcept;

        bool    set_name(const ft_string &name_value) noexcept;
        bool    set_name_from_cstr(const char *name_value) noexcept;
        bool    set_name_from_buffer(const char *buffer, size_t length) noexcept;

        bool    set_value(const ft_string &value_value) noexcept;
        bool    set_value_from_cstr(const char *value_value) noexcept;
        bool    set_value_from_buffer(const char *buffer, size_t length) noexcept;

        bool    assign(const ft_string &name_value, const ft_string &value_value) noexcept;
        bool    assign_from_cstr(const char *name_value, const char *value_value) noexcept;
        bool    assign_from_buffers(const char *name_buffer, size_t name_length,
                    const char *value_buffer, size_t value_length) noexcept;

        bool    copy_name(ft_string &out_name) const noexcept;
        bool    copy_value(ft_string &out_value) const noexcept;

        void    clear() noexcept;

        int         get_error() const noexcept;
        const char  *get_error_str() const noexcept;
};

class http2_frame
{
    private:
        uint8_t             _type;
        uint8_t             _flags;
        uint32_t            _stream_identifier;
        ft_string           _payload;
        mutable int         _error_code;
        mutable bool        _thread_safe_enabled;
        mutable pt_mutex   *_mutex;

        void    set_error(int error_code) const noexcept;
        int     lock(bool *lock_acquired) const noexcept;
        void    unlock(bool lock_acquired) const noexcept;
        void    teardown_thread_safety() noexcept;

    public:
        http2_frame() noexcept;
        ~http2_frame() noexcept;

        http2_frame(const http2_frame &other) noexcept;
        http2_frame(http2_frame &&other) noexcept;
        http2_frame &operator=(const http2_frame &other) noexcept;
        http2_frame &operator=(http2_frame &&other) noexcept;

        int     enable_thread_safety() noexcept;
        void    disable_thread_safety() noexcept;
        bool    is_thread_safe() const noexcept;

        bool    set_type(uint8_t type_value) noexcept;
        bool    get_type(uint8_t &out_type) const noexcept;

        bool    set_flags(uint8_t flags_value) noexcept;
        bool    get_flags(uint8_t &out_flags) const noexcept;

        bool    set_stream_identifier(uint32_t stream_identifier_value) noexcept;
        bool    get_stream_identifier(uint32_t &out_stream_identifier) const noexcept;

        bool    set_payload(const ft_string &payload_value) noexcept;
        bool    set_payload_from_buffer(const char *buffer, size_t length) noexcept;
        bool    copy_payload(ft_string &out_payload) const noexcept;
        void    clear_payload() noexcept;

        int         get_error() const noexcept;
        const char  *get_error_str() const noexcept;
};

struct http2_stream_state
{
    ft_string   buffer;
    uint32_t    dependency_identifier;
    uint8_t     weight;
    bool        exclusive_dependency;
    uint32_t    remote_window;
    uint32_t    local_window;

    http2_stream_state() noexcept;
    ~http2_stream_state() noexcept;
};

class http2_stream_manager
{
    private:
        void        set_error(int error_code) const noexcept;
        bool        validate_receive_window(uint32_t stream_identifier,
                        uint32_t length) noexcept;
        bool        record_received_data(uint32_t stream_identifier,
                        uint32_t length) noexcept;
        bool        reserve_remote_connection_window(uint32_t length) noexcept;
        bool        record_connection_send(uint32_t length) noexcept;
        void        remove_stream_identifier(uint32_t stream_identifier) noexcept;

        ft_map<uint32_t, http2_stream_state>    _streams;
        ft_vector<uint32_t>                     _stream_identifiers;
        uint32_t                                _initial_remote_window;
        uint32_t                                _initial_local_window;
        uint32_t                                _connection_remote_window;
        uint32_t                                _connection_local_window;
        mutable int                              _error_code;

    public:
        http2_stream_manager() noexcept;
        ~http2_stream_manager() noexcept;

        bool        open_stream(uint32_t stream_identifier) noexcept;
        bool        append_data(uint32_t stream_identifier, const char *data,
                        size_t length) noexcept;
        bool        close_stream(uint32_t stream_identifier) noexcept;
        bool        get_stream_buffer(uint32_t stream_identifier,
                        ft_string &out_buffer) const noexcept;
        bool        update_priority(uint32_t stream_identifier,
                        uint32_t dependency_identifier, uint8_t weight,
                        bool exclusive) noexcept;
        bool        get_priority(uint32_t stream_identifier,
                        uint32_t &dependency_identifier, uint8_t &weight,
                        bool &exclusive) const noexcept;
        bool        update_remote_initial_window(uint32_t new_window) noexcept;
        bool        update_local_initial_window(uint32_t new_window) noexcept;
        uint32_t    get_local_window(uint32_t stream_identifier) const noexcept;
        uint32_t    get_remote_window(uint32_t stream_identifier) const noexcept;
        bool        increase_local_window(uint32_t stream_identifier,
                        uint32_t increment) noexcept;
        bool        increase_remote_window(uint32_t stream_identifier,
                        uint32_t increment) noexcept;
        bool        reserve_send_window(uint32_t stream_identifier,
                        uint32_t length) noexcept;
        bool        update_connection_local_window(uint32_t increment) noexcept;
        bool        update_connection_remote_window(uint32_t increment) noexcept;
        uint32_t    get_connection_local_window() const noexcept;
        uint32_t    get_connection_remote_window() const noexcept;
        int         get_error() const noexcept;
        const char  *get_error_str() const noexcept;
};

class http2_settings_state
{
    private:
        void        set_error(int error_code) const noexcept;
        bool        apply_single_setting(uint16_t identifier, uint32_t value,
                        http2_stream_manager &streams) noexcept;

        uint32_t    _header_table_size;
        bool        _enable_push;
        uint32_t    _max_concurrent_streams;
        uint32_t    _initial_local_window;
        uint32_t    _initial_remote_window;
        uint32_t    _max_frame_size;
        uint32_t    _max_header_list_size;
        mutable int _error_code;

    public:
        http2_settings_state() noexcept;
        ~http2_settings_state() noexcept;

        bool        apply_remote_settings(const http2_frame &frame,
                        http2_stream_manager &streams) noexcept;
        bool        update_local_initial_window(uint32_t new_window,
                        http2_stream_manager &streams) noexcept;
        bool        update_remote_initial_window(uint32_t new_window,
                        http2_stream_manager &streams) noexcept;
        uint32_t    get_header_table_size() const noexcept;
        bool        get_enable_push() const noexcept;
        uint32_t    get_max_concurrent_streams() const noexcept;
        uint32_t    get_initial_local_window() const noexcept;
        uint32_t    get_initial_remote_window() const noexcept;
        uint32_t    get_max_frame_size() const noexcept;
        uint32_t    get_max_header_list_size() const noexcept;
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
