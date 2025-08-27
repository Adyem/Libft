#include "../../Printf/printf.hpp"
#include "efficiency_utils.hpp"

#include <cstdio>

int test_efficiency_printf(void) {
  const size_t iterations = 1000;
  const char *fmt = "Hello %s %d %x";
  const char *str = "world";
  int num = 42;
  unsigned hex = 0x2a;
  volatile int sink = 0;

  FILE *devnull = std::fopen("/dev/null", "w");
  if (!devnull)
    return (0);

  auto start_std = clock_type::now();
  for (size_t i = 0; i < iterations; ++i) {
    prevent_optimization((void *)fmt);
    prevent_optimization((void *)str);
    sink += std::fprintf(devnull, fmt, str, num, hex);
    prevent_optimization((void *)&sink);
  }
  std::fflush(devnull);
  auto end_std = clock_type::now();

  auto start_ft = clock_type::now();
  for (size_t i = 0; i < iterations; ++i) {
    prevent_optimization((void *)fmt);
    prevent_optimization((void *)str);
    sink += ft_fprintf(devnull, fmt, str, num, hex);
    prevent_optimization((void *)&sink);
  }
  std::fflush(devnull);
  auto end_ft = clock_type::now();

  std::fclose(devnull);
  print_comparison("printf", elapsed_us(start_std, end_std),
                   elapsed_us(start_ft, end_ft));
  return (1);
}
