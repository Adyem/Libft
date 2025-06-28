#ifndef LIBFT_HPP
# define LIBFT_HPP

#define SUCCES 0
#define FAILURE 1

#include <cstddef>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

size_t 		ft_strlen_size_t(const char *string);
int			ft_strlen(const char *string);
char		*ft_strchr(const char *string, int char_to_find);
int			ft_atoi(const char *string);
void		ft_bzero(void *string, size_t size);
void		*ft_memchr(const void *pointer, int character, size_t size);
void		*ft_memcpy(void* destination, const void* source, size_t num);
void		*ft_memmove(void *destination, const void *source, size_t size);
char		*ft_strchr(const char *string, int char_to_find);
size_t		ft_strlcat(char *destination, const char *source, size_t bufferSize);
size_t		ft_strlcpy(char *destination, const char *source, size_t bufferSize);
char		*ft_strrchr(const char *string, int char_to_find);
char		*ft_strnstr(const char *haystack, const char *needle, size_t length);
int			ft_strncmp(const char *string_1, const char *string_2, size_t max_len);
int			ft_memcmp(const void *pointer1, const void *pointer2, size_t size);
int			ft_isdigit(int character);
int         ft_isalpha(int character);
int         ft_isalnum(int character);
long		ft_atol(const char *string);
int			ft_strcmp(const char *string1, const char *string2);
void		ft_to_lower(char *string);
void		ft_to_upper(char *string);
char 		*ft_strncpy(char *destination, const char *source, size_t number_of_characters);
void 		*ft_memset(void *destination, int value, size_t number_of_bytes);
int 		ft_isspace(int character);
int 		ft_abs(int number);

#endif
