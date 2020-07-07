set(SRCS
    test_timer.cpp
    ../../log/pr.cpp
    ../../log/log.cpp
)
set(INCS
    ../
    ../../log
    ../../threadpool
)
include_directories(${INCS})
add_executable(timer_test ${SRCS})
target_link_libraries(timer_test pthread)