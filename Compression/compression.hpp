#ifndef COMPRESSION_HPP
# define COMPRESSION_HPP

#include <cstddef>
#include <cstdint>
#include "../CPP_class/class_string_class.hpp"
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

typedef struct s_compress_stream_options
{
    std::size_t    input_buffer_size;
    std::size_t    output_buffer_size;
}   t_compress_stream_options;

int              ft_compress_stream_with_options(int input_fd, int output_fd, const t_compress_stream_options *options);
int              ft_decompress_stream_with_options(int input_fd, int output_fd, const t_compress_stream_options *options);
int              ft_compress_stream(int input_fd, int output_fd);
int              ft_decompress_stream(int input_fd, int output_fd);
unsigned char    *ft_base64_encode(const unsigned char *input_buffer, std::size_t input_size, std::size_t *encoded_size);
unsigned char    *ft_base64_decode(const unsigned char *input_buffer, std::size_t input_size, std::size_t *decoded_size);
int              ft_compress_string_to_vector(const ft_string &input, ft_vector<unsigned char> &output);
int              ft_compress_vector_to_vector(const ft_vector<unsigned char> &input, ft_vector<unsigned char> &output);
int              ft_decompress_vector_to_string(const ft_vector<unsigned char> &input, ft_string &output);
int              ft_decompress_vector_to_vector(const ft_vector<unsigned char> &input, ft_vector<unsigned char> &output);

#endif
