aux_source_directory(.. native_source)
aux_source_directory(../../memory memory_source)
set(SRCS
    echo_server.cpp
    ../../log/pr.cpp
    ../../log/log.cpp
)
list(APPEND SRCS ${native_source})
list(APPEND SRCS ${memory_source})

set(INCS
    ../
    ../../log
    ../../threadpool
)
include_directories(${INCS})
add_executable(echo_server ${SRCS})
target_link_libraries(echo_server pthread)

execute_process(COMMAND sh install_client.sh WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

list(REMOVE_ITEM SRCS echo_server.cpp)
list(APPEND SRCS http_for_bench.cpp)
add_executable(http_for_bench ${SRCS})
target_link_libraries(http_for_bench pthread)