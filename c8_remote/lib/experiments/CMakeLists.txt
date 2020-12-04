
include_directories(include)
add_library(experiments crypto.c system.c power.c util/thread_list.c)
add_dependencies(experiments payload)