#include <cstdarg>
#include <cstddef>
#include "printf.hpp"

int pf_vsnprintf(char *buf, size_t size, const char *fmt, ...) {
    size_t total = 0;
    char *dest = buf;
    size_t remaining = size;

   	va_list args;
    va_start(args, fmt);
    while (*fmt) {
        if (*fmt != '%') {
            if (remaining > 1) {
                *dest++ = *fmt;
                remaining--;
            }
            total++;
            fmt++;
            continue;
        }

        // Found '%', check for literal '%' or a format specifier.
        fmt++;  // Skip the '%'
        if (*fmt == '%') {
            if (remaining > 1) {
                *dest++ = '%';
                remaining--;
            }
            total++;
            fmt++;
            continue;
        }

        // Process a basic conversion specifier.
        char spec = *fmt++;
        switch (spec) {
            case 's': {
                // String conversion.
                const char *s = va_arg(args, const char*);
                if (!s)
                    s = "(null)";
                while (*s) {
                    if (remaining > 1) {
                        *dest++ = *s;
                        remaining--;
                    }
                    total++;
                    s++;
                }
                break;
            }
            case 'c': {
                // Single character.
                int ch = va_arg(args, int);
                if (remaining > 1) {
                    *dest++ = static_cast<char>(ch);
                    remaining--;
                }
                total++;
                break;
            }
            case 'd':
            case 'i': {
                // Signed integer.
                int val = va_arg(args, int);
                char num_buffer[32]; // Buffer for number conversion.
                int i = 0;
                bool negative = false;
                unsigned int uval;
                if (val < 0) {
                    negative = true;
                    uval = static_cast<unsigned int>(-val);
                } else {
                    uval = static_cast<unsigned int>(val);
                }
                // Convert the integer to string (in reverse order).
                do {
                    num_buffer[i++] = '0' + (uval % 10);
                    uval /= 10;
                } while (uval > 0);
                if (negative) {
                    num_buffer[i++] = '-';
                }
                // Output the number in correct order.
                for (int j = i - 1; j >= 0; j--) {
                    if (remaining > 1) {
                        *dest++ = num_buffer[j];
                        remaining--;
                    }
                    total++;
                }
                break;
            }
            case 'u': {
                // Unsigned integer.
                unsigned int uval = va_arg(args, unsigned int);
                char num_buffer[32];
                int i = 0;
                do {
                    num_buffer[i++] = '0' + (uval % 10);
                    uval /= 10;
                } while (uval > 0);
                for (int j = i - 1; j >= 0; j--) {
                    if (remaining > 1) {
                        *dest++ = num_buffer[j];
                        remaining--;
                    }
                    total++;
                }
                break;
            }
            case 'x':
            case 'X': {
                // Unsigned integer in hexadecimal.
                unsigned int uval = va_arg(args, unsigned int);
                char num_buffer[32];
                int i = 0;
                do {
                    int digit = uval % 16;
                    if (digit < 10)
                        num_buffer[i++] = '0' + digit;
                    else {
                        // Choose lowercase or uppercase based on specifier.
                        num_buffer[i++] = (spec == 'x') ? ('a' + digit - 10) : ('A' + digit - 10);
                    }
                    uval /= 16;
                } while (uval > 0);
                for (int j = i - 1; j >= 0; j--) {
                    if (remaining > 1) {
                        *dest++ = num_buffer[j];
                        remaining--;
                    }
                    total++;
                }
                break;
            }
            case 'p': {
                // Pointer value: output as "0x<hex>".
                void *ptr = va_arg(args, void*);
                unsigned long addr = reinterpret_cast<unsigned long>(ptr);
                const char *prefix = "0x";
                for (int k = 0; prefix[k] != '\0'; k++) {
                    if (remaining > 1) {
                        *dest++ = prefix[k];
                        remaining--;
                    }
                    total++;
                }
                char num_buffer[32];
                int i = 0;
                do {
                    int digit = addr % 16;
                    if (digit < 10)
                        num_buffer[i++] = '0' + digit;
                    else
                        num_buffer[i++] = 'a' + (digit - 10);
                    addr /= 16;
                } while (addr > 0);
                for (int j = i - 1; j >= 0; j--) {
                    if (remaining > 1) {
                        *dest++ = num_buffer[j];
                        remaining--;
                    }
                    total++;
                }
                break;
            }
            default: {
                // For unsupported specifiers, output the '%' and the specifier as literals.
                if (remaining > 1) {
                    *dest++ = '%';
                    remaining--;
                }
                total++;
                if (remaining > 1) {
                    *dest++ = spec;
                    remaining--;
                }
                total++;
                break;
            }
        }
    }

    // Always null-terminate the result if size > 0.
    if (size > 0) {
        if (remaining > 0)
            *dest = '\0';
        else
            buf[size - 1] = '\0';
    }
    return static_cast<int>(total);
}

// Wrapper function that mimics snprintf using our custom implementation.
int pf_snprintf(char *buf, size_t size, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int ret = pf_vsnprintf(buf, size, fmt, args);
    va_end(args);
    return ret;
}
