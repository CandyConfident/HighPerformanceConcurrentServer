set(SRCS
    test_threadpool.cpp
    ../../log/pr.cpp
    ../../log/log.cpp
)
set(INCS
    ../
    ../../log
)
include_directories(${INCS})
add_executable(threadpool_test ${SRCS})
target_link_libraries(threadpool_test pthread)