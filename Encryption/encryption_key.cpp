#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include "../CMA/CMA.hpp"
#include "../CPP_class/cpp_class_nullptr.hpp"
#include "encryption_basic_encryption.hpp"

static void decoy_unusedFunction1(void)
{
    volatile int dummy = 0;
    int index = 0;
    while (index < 10)
    {
        dummy += index;
        ++index;
    }
    return ;
}

static uint32_t decoy_unusedFunction2(uint32_t value)
{
    value ^= 0xAAAAAAAA;
    value ^= 0xAAAAAAAA;
    return (value);
}

static int decoy_unusedFunction3(const char* input_string)
{
    (void)input_string;
    return (42);
}

static uint32_t obfuscate_seed(uint32_t seed_value)
{
    decoy_unusedFunction1();
    seed_value = decoy_unusedFunction2(seed_value);
    return (seed_value);
}

const char *be_getEncryptionKey(void)
{
    size_t key_length = 32;
    char *key = static_cast<char *>(cma_malloc(key_length + 1));
    if (!key)
        return (ft_nullptr);
    uint32_t seed_value = 0xDEADBEEF;
    seed_value = obfuscate_seed(seed_value);
    size_t index = 0;
    while (index < key_length)
    {
        decoy_unusedFunction3(key);
        seed_value = seed_value * 1103515245 + 12345;
        key[index] = static_cast<char>('A' + (seed_value % 26));
        ++index;
    }
    key[key_length] = '\0';
    decoy_unusedFunction1();
    return (key);
}
