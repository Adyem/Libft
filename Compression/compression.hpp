#ifndef COMPRESSION_HPP
# define COMPRESSION_HPP

#include <cstddef>
#include <cstdint>
#include "../CPP_class/class_string_class.hpp"
#include "../PThread/mutex.hpp"
#include "../Template/vector.hpp"

/*
** Maximum number of bytes supported by the compression helpers. The original
** size is stored alongside the payload using a 32-bit prefix, so values
** greater than UINT32_MAX cannot be represented.
*/
static const std::size_t   compression_max_size = static_cast<std::size_t>(UINT32_MAX);

unsigned char    *compress_buffer(const unsigned char *input_buffer, std::size_t input_size, std::size_t *compressed_size);
unsigned char    *decompress_buffer(const unsigned char *input_buffer, std::size_t input_size, std::size_t *decompressed_size);

unsigned char    *ft_compress(const unsigned char *input_buffer, std::size_t input_size, std::size_t *compressed_size);
unsigned char    *ft_decompress(const unsigned char *input_buffer, std::size_t input_size, std::size_t *decompressed_size);

typedef struct s_compress_stream_progress
{
    std::size_t    total_bytes_read;
    std::size_t    total_bytes_written;
}   t_compress_stream_progress;

typedef int (*t_compress_stream_progress_callback)(const t_compress_stream_progress *progress, void *user_data);
typedef int (*t_compress_stream_cancel_callback)(const t_compress_stream_progress *progress, void *user_data);

struct s_compress_stream_options_snapshot;

class t_compress_stream_options
{
    private:
        std::size_t                            _input_buffer_size;
        std::size_t                            _output_buffer_size;
        t_compress_stream_progress_callback    _progress_callback;
        t_compress_stream_cancel_callback      _cancel_callback;
        void                                   *_callback_user_data;
        int                                     _compression_level;
        int                                     _window_bits;
        int                                     _memory_level;
        int                                     _strategy;
        mutable int                             _error_code;
        mutable pt_mutex                        _mutex;

        void    set_error(int error_code) const;

    public:
        t_compress_stream_options(void);
        ~t_compress_stream_options(void);

        int         reset(void);
        int         set_input_buffer_size(std::size_t input_buffer_size);
        int         set_output_buffer_size(std::size_t output_buffer_size);
        int         set_progress_callback(t_compress_stream_progress_callback callback, void *user_data);
        int         set_cancel_callback(t_compress_stream_cancel_callback callback, void *user_data);
        int         set_callbacks(t_compress_stream_progress_callback progress_callback,
                        t_compress_stream_cancel_callback cancel_callback,
                        void *user_data);
        int         set_compression_level(int compression_level);
        int         set_window_bits(int window_bits);
        int         set_memory_level(int memory_level);
        int         set_strategy(int strategy);
        std::size_t get_input_buffer_size() const;
        std::size_t get_output_buffer_size() const;
        t_compress_stream_progress_callback    get_progress_callback() const;
        t_compress_stream_cancel_callback      get_cancel_callback() const;
        void        *get_callback_user_data() const;
        int         get_compression_level() const;
        int         get_window_bits() const;
        int         get_memory_level() const;
        int         get_strategy() const;
        int         snapshot(struct s_compress_stream_options_snapshot *snapshot) const;
        int         get_error() const;
        const char  *get_error_str() const;
};

int              ft_compress_stream_with_options(int input_fd, int output_fd, const t_compress_stream_options *options);
int              ft_decompress_stream_with_options(int input_fd, int output_fd, const t_compress_stream_options *options);
int              ft_compress_stream(int input_fd, int output_fd);
int              ft_decompress_stream(int input_fd, int output_fd);
void             ft_compress_stream_apply_speed_preset(t_compress_stream_options *options);
void             ft_compress_stream_apply_ratio_preset(t_compress_stream_options *options);
unsigned char    *ft_base64_encode(const unsigned char *input_buffer, std::size_t input_size, std::size_t *encoded_size);
unsigned char    *ft_base64_decode(const unsigned char *input_buffer, std::size_t input_size, std::size_t *decoded_size);
int              ft_compress_string_to_vector(const ft_string &input, ft_vector<unsigned char> &output);
int              ft_compress_vector_to_vector(const ft_vector<unsigned char> &input, ft_vector<unsigned char> &output);
int              ft_decompress_vector_to_string(const ft_vector<unsigned char> &input, ft_string &output);
int              ft_decompress_vector_to_vector(const ft_vector<unsigned char> &input, ft_vector<unsigned char> &output);

#endif
