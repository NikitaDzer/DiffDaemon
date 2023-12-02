cmake_minimum_required(VERSION 3.12)
project(DiffDaemon)

file(GLOB DDAEMON_SOURCES CONFIGURE_DEPENDS 
    "common/*.c"
    "daemon/*.c"
    "io/*.c"
    "ipc/*.c"
    "service/*.c"
    "utils/*.c"
    main.c)

add_executable(ddaemon ${DDAEMON_SOURCES})
target_include_directories(ddaemon PUBLIC .)
