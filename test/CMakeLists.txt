include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

set(TEST_SOURCES
        cfsm_test_init.cpp
        cfsm_test_processing.cpp
        cfsm_test_state_actions.cpp
)

add_executable(cfsm_test_suite_GT ${TEST_SOURCES})
target_link_libraries(cfsm_test_suite_GT cfsm gmock gmock_main)
add_test(cfsm_test_suite_GT cfsm_test_suite_GT)
