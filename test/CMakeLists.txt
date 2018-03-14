# Licensed under the MIT License. See LICENSE file in the project root for full license information.

# The additional / is important to remove the last character from the path.
# Note that it does not matter if the OS uses / or \, because we are only
# saving the path size.
string(LENGTH "${CMAKE_SOURCE_DIR}/" CFSM_TEST_SOURCE_PATH_LENGTH)
add_definitions("-DCFSM_TEST_SOURCE_PATH_LENGTH=${CFSM_TEST_SOURCE_PATH_LENGTH}")

set(UT_SOURCES
        cfsm_test_init.c
        cfsm_test_processing.c
        cfsm_test_state_actions.c
        cfsm_test_submachines.c
)

set(UT_FRAMEWORK
        cfsm_test/cfsm_test_runner.h
        cfsm_test/cfsm_test_runner.c
)

set(UT_MAIN
        cfsm_test_suite.c)

add_executable(cfsm_test_suite ${UT_SOURCES} ${UT_MAIN} ${UT_FRAMEWORK})
target_link_libraries(cfsm_test_suite cfsm)
add_test(NAME cfsm_test_suite COMMAND cfsm_test_suite)
