# TODO

Remaining unchecked items: 17

## Mutex reinitialization audit

All mutex reinitialization audit items have been completed.

## Basic module AGENTS.md compliance

- [ ] Remove ternary operator from `Basic/basic_utf8_decode.cpp` (`ft_utf8_is_trailing_byte`, line 7).
- [ ] Remove ternary operator from `Basic/basic_utf8_grapheme.cpp` (`ft_utf8_code_point_in_range`, line 10).
- [ ] Change `break;` to `break ;` in `Basic/basic_utf8_grapheme.cpp` (line 44).
- [ ] Change `break;` to `break ;` in `Basic/basic_utf8_grapheme.cpp` (line 50).
- [ ] Change `break;` to `break ;` in `Basic/basic_utf8_grapheme.cpp` (line 52).
- [ ] Change `return;` to `return ;` in `Basic/basic_toupper.cpp` (line 6).
- [ ] Add final `return ;` at end of `zero_buffer` in `Basic/basic_memmove_s.cpp`.
- [ ] Add final `return ;` at end of `zero_buffer` in `Basic/basic_memcpy_s.cpp`.
- [ ] Add final `return ;` at end of `zero_buffer` in `Basic/basic_strcpy_s.cpp`.
- [ ] Add final `return ;` at end of `zero_buffer` in `Basic/basic_strncpy_s.cpp`.
- [ ] Add final `return ;` at end of `zero_buffer` in `Basic/basic_strcat_s.cpp`.
- [ ] Add final `return ;` at end of `zero_buffer` in `Basic/basic_strncat_s.cpp`.
- [ ] Add final `return ;` at end of `ft_to_upper` in `Basic/basic_toupper.cpp`.

## Advanced module AGENTS.md compliance

- [ ] Remove all `for` loops in `Advanced` module source files and replace them with compliant control flow (`while`), including `advanced_split.cpp`, `advanced_strjoin.cpp`, `advanced_strjoin_multiple.cpp`, `advanced_memdup.cpp`, `advanced_strndup.cpp`, and `advanced_strtrim.cpp`.
- [ ] Remove all ternary operators from `Advanced/advanced_itoa.cpp` and `Advanced/advanced_itoa_base.cpp`.
- [ ] Add final `return ;` to void helper `free_strings` in `Advanced/advanced_split.cpp`.
- [ ] Rename short variable names in `Advanced` module to full descriptive snake_case names (for example `len1`, `len2`, `i`, `j`, `src`, `dest`).
