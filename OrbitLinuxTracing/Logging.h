#ifndef ORBIT_LINUX_TRACING_LOGGING_H_
#define ORBIT_LINUX_TRACING_LOGGING_H_

// TODO: Move logging to OrbitBase once we have clearer plans for such module.

#include <cstdio>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"

#define LOG(format, ...) fprintf(stderr, format "\n", ##__VA_ARGS__)

#define ERROR(format, ...) LOG("Error: " format, ##__VA_ARGS__)

#pragma clang diagnostic pop

#endif  // ORBIT_LINUX_TRACING_LOGGING_H_
