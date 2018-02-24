set(HEADERS
    cfsm_test_runner.h
)

add_executable(cfsm_test_init cfsm_test_init.c ${HEADERS})
target_link_libraries(cfsm_test_init cfsm)
add_test(NAME cfsm_test_init COMMAND cfsm_test_init)

add_executable(cfsm_test_processing cfsm_test_processing.c)
target_link_libraries(cfsm_test_processing cfsm)
add_test(NAME cfsm_test_processing COMMAND cfsm_test_processing)

add_executable(cfsm_test_state_actions cfsm_test_state_actions.c)
target_link_libraries(cfsm_test_state_actions cfsm)
add_test(NAME cfsm_test_state_actions COMMAND cfsm_test_state_actions)