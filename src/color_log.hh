#ifndef COLORLOG_HH_
#define COLORLOG_HH_

#include "private_alloc.hh"
#include "util.hh"

#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define YELLOW "\033[1;33m"
#define BLUE "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN "\033[1;36m"
#define END "\033[0m\n"

using MyStringStream = std::basic_ostringstream<char, std::char_traits<char>,
                                                PrivateAllocator<char>>;

inline MyStringStream ProcessChooseColor() {
  pid_t pid = getpid();
  int color = pid % 6;
  MyStringStream stream;
  switch (color) {
    case 0:
      stream << RED;
      break;
    case 1:
      stream << GREEN;
      break;
    case 2:
      stream << YELLOW;
      break;
    case 3:
      stream << BLUE;
      break;
    case 4:
      stream << MAGENTA;
      break;
    case 5:
      stream << CYAN;
      break;
  }
  stream << getpid() << ": ";
  return stream;
}

#if defined(NDEBUG)
#define ColorLog(x)                                                            \
  std::cerr << (static_cast<MyStringStream&>(ProcessChooseColor() << x << END) \
                    .str())
#else
#define ColorLog(x)
#endif

#endif  // COLORLOG_HH_
