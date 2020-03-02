#ifndef ORBIT_LINUX_TRACING_LOGGING_H_
#define ORBIT_LINUX_TRACING_LOGGING_H_

// TODO: Move logging to OrbitBase once we have clearer plans for such module.

#include <cstdio>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#endif

#define LOG(format, ...) fprintf(stderr, format "\n", ##__VA_ARGS__)

#define ERROR(format, ...) LOG("Error: " format, ##__VA_ARGS__)

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif  // ORBIT_LINUX_TRACING_LOGGING_H_
