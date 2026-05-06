#ifndef SQLITE_SUPPORT_HPP
#define SQLITE_SUPPORT_HPP

#if !defined(SQLITE3_AVAILABLE)
# if defined(__has_include)
#  if __has_include(<sqlite3.h>)
#   include <sqlite3.h>
#   define SQLITE3_AVAILABLE 1
#  else
#   define SQLITE3_AVAILABLE 0
#  endif
# else
#  define SQLITE3_AVAILABLE 0
# endif
#endif

#endif
