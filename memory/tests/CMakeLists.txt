aux_source_directory(.. native_source)
set(SRCS
    test_mem.cpp
    ../../log/pr.cpp
    ../../log/log.cpp
)
list(APPEND SRCS ${native_source})

set(INCS
    ../
    ../../log
)
include_directories(${INCS})

add_executable(mem_test ${SRCS})
target_link_libraries(mem_test pthread)

list(REMOVE_ITEM SRCS test_mem.cpp)
list(APPEND SRCS test_buf.cpp)
add_executable(buf_test ${SRCS})
target_link_libraries(buf_test pthread)

