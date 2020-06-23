set(SRCS
    test_pr.cpp
    ../pr.cpp
)
include_directories(${PROJECT_SOURCE_DIR}/..)

add_executable(pr_test ${SRCS})

set(SRCS
    test_q.cpp
    ../pr.cpp
)
add_executable(q_test ${SRCS})
target_link_libraries(q_test pthread)

set(SRCS
    test_log_level.cpp
    ../pr.cpp
    ../log.cpp
)
add_executable(log_level_test ${SRCS})
target_link_libraries(log_level_test pthread)

set(SRCS
    test_log_sync.cpp
    ../pr.cpp
    ../log.cpp
)
add_executable(log_sync_test ${SRCS})
target_link_libraries(log_sync_test pthread)

set(SRCS
    test_log_async.cpp
    ../pr.cpp
    ../log.cpp
)
add_executable(log_async_test ${SRCS})
target_link_libraries(log_async_test pthread)