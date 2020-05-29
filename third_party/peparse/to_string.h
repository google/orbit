#ifndef _TO_STRING_H
#define _TO_STRING_H
#include <sstream>

template <class T>
static
std::string to_string(T t, std::ios_base & (*f)(std::ios_base&)) {
    std::ostringstream oss;
    oss << f << t;
    return oss.str();
}
#endif
