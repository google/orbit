#ifndef ORBIT_LINUX_TRACING_LOGGING_H_
#define ORBIT_LINUX_TRACING_LOGGING_H_

// TODO: Move logging to OrbitBase once we have clearer plans for such module.

#include <cstdio>

#define LOG(format, ...) fprintf(stderr, format "\n", ##__VA_ARGS__)

#define ERROR(format, ...) LOG("Error: " format, ##__VA_ARGS__)

#endif  // ORBIT_LINUX_TRACING_LOGGING_H_
