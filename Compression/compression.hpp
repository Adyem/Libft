#ifndef COMPRESSION_HPP
# define COMPRESSION_HPP

#include <cstddef>
#include <cstdint>

unsigned char    *compress_buffer(const unsigned char *input_buffer, std::size_t input_size, std::size_t *compressed_size);
unsigned char    *decompress_buffer(const unsigned char *input_buffer, std::size_t input_size, std::size_t *decompressed_size);

unsigned char    *ft_compress(const unsigned char *input_buffer, std::size_t input_size, std::size_t *compressed_size);
unsigned char    *ft_decompress(const unsigned char *input_buffer, std::size_t input_size, std::size_t *decompressed_size);
int              ft_compress_stream(int input_fd, int output_fd);
int              ft_decompress_stream(int input_fd, int output_fd);
unsigned char    *ft_base64_encode(const unsigned char *input_buffer, std::size_t input_size, std::size_t *encoded_size);
unsigned char    *ft_base64_decode(const unsigned char *input_buffer, std::size_t input_size, std::size_t *decoded_size);

#endif
