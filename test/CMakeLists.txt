add_executable(cfsm_UT cfsm_ut.c cfsm_test_runner.h)
add_test(NAME cfsm_UT COMMAND cfsm_UT)
target_link_libraries(cfsm_UT cfsm)