/**
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include <cfsm/cfsm.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace ::testing;

struct StateActionMock {
    MOCK_CONST_METHOD3(call, void(struct cfsm_state* state, int event_id, void* event_data));
};

std::unique_ptr<StateActionMock> s0_entry_action_mock;
void s0_entry_action(struct cfsm_state* state, int event_id, void* event_data) {
    s0_entry_action_mock->call(state, event_id, event_data);
}

std::unique_ptr<StateActionMock> s0_exit_action_mock;
void s0_exit_action(struct cfsm_state* state, int event_id, void* event_data) {
    s0_exit_action_mock->call(state, event_id, event_data);
}

std::unique_ptr<StateActionMock> s1_entry_action_mock;
void s1_entry_action(struct cfsm_state* state, int event_id, void* event_data) {
    s1_entry_action_mock->call(state, event_id, event_data);
}

std::unique_ptr<StateActionMock> s1_exit_action_mock;
void s1_exit_action(struct cfsm_state* state, int event_id, void* event_data) {
    s1_exit_action_mock->call(state, event_id, event_data);
}

struct cfsm_test_state_actions : Test {
    cfsm_test_state_actions() {
        s0_entry_action_mock = std::make_unique<StateActionMock>();
        s0_exit_action_mock = std::make_unique<StateActionMock>();
        s1_entry_action_mock = std::make_unique<StateActionMock>();
        s1_exit_action_mock = std::make_unique<StateActionMock>();
    }

    ~cfsm_test_state_actions() override {
        s0_entry_action_mock.reset(nullptr);
        s0_exit_action_mock.reset(nullptr);
        s1_entry_action_mock.reset(nullptr);
        s1_exit_action_mock.reset(nullptr);
    }

    int event_data = 523;
};

TEST_F(cfsm_test_state_actions, cfsm_test_start_calls_entry_action_over_initial_state) {
    int initial_event_id = 8;

    cfsm_state state{};
    cfsm_init_state(&state, "state_0");
    state.entry_action = s0_entry_action;

    cfsm_state c{};
    cfsm_init(&c, 1, &state, &state);

    EXPECT_CALL(*s0_entry_action_mock, call(&state, initial_event_id, &event_data)).Times(1);

    cfsm_start(&c, initial_event_id, &event_data);
}

TEST_F(cfsm_test_state_actions, cfsm_test_stop_calls_exit_action_over_current_state) {
    int stop_event_id = 9;

    cfsm_state state{};
    cfsm_init_state(&state, "state_0");
    state.exit_action = s0_exit_action;

    cfsm_state c{};
    cfsm_init(&c, 1, &state, &state);
    cfsm_start(&c, 0, nullptr);

    EXPECT_CALL(*s0_exit_action_mock, call(&state, stop_event_id, &event_data)).Times(1);

    cfsm_stop(&c, stop_event_id, &event_data);
}

TEST_F(cfsm_test_state_actions, cfsm_test_restart_calls_exit_action_over_current_state_then_transit_to_initial_state_then_call_entry_action_over_initial_state) {
    int restart_event_id = 17;

    struct cfsm_state state[1];
    cfsm_init_state(&state[0], "state_0");
    state[0].entry_action = s0_entry_action;
    state[0].exit_action = s0_exit_action;

    cfsm_state c{};
    cfsm_init(&c, 1, &state[0], &state[0]);

    c.current_state = &state[0]; // simulate already running machine

    InSequence seq;

    EXPECT_CALL(*s0_exit_action_mock, call(&state[0], restart_event_id, &event_data)).Times(1);
    EXPECT_CALL(*s0_entry_action_mock, call(&state[0], restart_event_id, &event_data)).Times(1);

    cfsm_restart(&c, restart_event_id, &event_data);
}

TEST_F(cfsm_test_state_actions, cfsm_test_processing_event_calls_actions_over_endpoint_states) {
    int event_id = 10;

    struct cfsm_state states[2];
    cfsm_init_state(&states[0], "state_0");
    cfsm_init_state(&states[1], "state_1");

    states[0].exit_action = s0_exit_action;
    states[1].entry_action = s1_entry_action;

    cfsm_transition t{};
    cfsm_init_transition(&t, &states[0], &states[1], event_id);

    cfsm_state c{};
    cfsm_init(&c, 2, states, &states[0]);
    c.current_state = nullptr;

    cfsm_add_transition(&c, &t);

    EXPECT_CALL(*s0_exit_action_mock, call(&states[0], event_id, &event_data)).Times(1);
    EXPECT_CALL(*s1_entry_action_mock, call(&states[1], event_id, &event_data)).Times(1);

    ASSERT_TRUE(cfsm_status_ok == cfsm_process_event(&c, event_id, &event_data));
}
