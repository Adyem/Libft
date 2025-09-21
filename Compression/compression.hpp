#ifndef COMPRESSION_HPP
# define COMPRESSION_HPP

#include <cstddef>
#include <cstdint>

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
int              ft_compress_stream(int input_fd, int output_fd);
int              ft_decompress_stream(int input_fd, int output_fd);
unsigned char    *ft_base64_encode(const unsigned char *input_buffer, std::size_t input_size, std::size_t *encoded_size);
unsigned char    *ft_base64_decode(const unsigned char *input_buffer, std::size_t input_size, std::size_t *decoded_size);

#endif
