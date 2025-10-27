#ifndef PRINTF_ENGINE_HPP
#define PRINTF_ENGINE_HPP

#include <cstddef>
#include <cstdarg>

struct pf_engine_format_spec;

typedef int (*t_pf_engine_write_callback)(const char *data_pointer, size_t data_length, void *context, size_t *written_count);

int pf_engine_format(const char *format, va_list args, t_pf_engine_write_callback writer, void *context, size_t *written_count);

#endif
