#ifndef COMPRESSION_HPP
# define COMPRESSION_HPP

#include <cstddef>
#include <cstdint>

unsigned char    *compress_buffer(const unsigned char *input_buffer, std::size_t input_size, std::size_t *compressed_size);
unsigned char    *decompress_buffer(const unsigned char *input_buffer, std::size_t input_size, std::size_t *decompressed_size);

#endif
