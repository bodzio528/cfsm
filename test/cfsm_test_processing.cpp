/**
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include <cfsm/cfsm.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace ::testing;

struct GuardMock {
    MOCK_CONST_METHOD4(call, bool(struct cfsm_state * source, struct cfsm_state* target, int event_id, void * event_data));
};

struct ActionMock {
    MOCK_CONST_METHOD4(call, void(struct cfsm_state * source, struct cfsm_state* target, int event_id, void * event_data));
};

std::unique_ptr<ActionMock> g_action;
void callAction(struct cfsm_state * source, struct cfsm_state* target, int event_id, void * event_data){
    return g_action->call(source, target, event_id, event_data);
};

std::unique_ptr<GuardMock> g_guard;
bool callGuard(struct cfsm_state * source, struct cfsm_state* target, int event_id, void * event_data){
    return g_guard->call(source, target, event_id, event_data);
};

struct ProcessingTestCase : Test {
    ProcessingTestCase() {
        g_action = std::make_unique<ActionMock>();
        g_guard = std::make_unique<GuardMock>();
    }

    ~ProcessingTestCase() override {
        g_action.reset(nullptr);
        g_guard.reset(nullptr);
    }

    int event_id = 33;
    int event_data = 123;
};

TEST_F(ProcessingTestCase, cfsm_test_process_event_returns_not_ok_if_no_transition_exists) {
    struct cfsm_state states[2];
    cfsm_init_state(&states[0], "state_0");
    cfsm_init_state(&states[1], "state_1");

    cfsm_state c{};
    cfsm_init(&c, 2, states, &states[0]);

    ASSERT_TRUE(cfsm_status_not_ok == cfsm_process_event(&c, event_id, &event_data)) << "processing status is not ok";
    ASSERT_TRUE(c.current_state == &states[0]) << "current state is not allowed to change without transition";
}

TEST_F(ProcessingTestCase, cfsm_test_process_event_returns_not_ok_if_no_transition_for_event_is_found) {
    struct cfsm_state states[2];
    cfsm_init_state(&states[0], "state_0");
    cfsm_init_state(&states[1], "state_1");

    cfsm_state c{};
    cfsm_init(&c, 2, states, &states[0]);

    cfsm_transition t{};
    cfsm_init_transition(&t, &states[0], &states[1], event_id);

    cfsm_transition_set_action(&t, callAction);
    cfsm_transition_set_guard(&t, callGuard);

    cfsm_add_transition(&c, &t);

    EXPECT_CALL(*g_guard, call(_,_,_,_)).Times(0);
    EXPECT_CALL(*g_action, call(_,_,_,_)).Times(0);

    ASSERT_TRUE(cfsm_status_not_ok == cfsm_process_event(&c, 335, &event_data)) << "processing status is not ok";
    ASSERT_TRUE(c.current_state == &states[0]) << "current state is not allowed to change without transition";
}

TEST_F(ProcessingTestCase, cfsm_test_process_event_null_guard_means_transition_is_enabled) {
    struct cfsm_state states[2];
    cfsm_init_state(&states[0], "state_0");
    cfsm_init_state(&states[1], "state_1");

    cfsm_state c{};
    cfsm_init(&c, 2, states, &states[0]);

    cfsm_transition t{};
    cfsm_init_transition(&t, &states[0], &states[1], event_id);
    cfsm_transition_set_action(&t, callAction);

    cfsm_add_transition(&c, &t);

    EXPECT_CALL(*g_guard, call(_, _, _, _)).Times(0);
    EXPECT_CALL(*g_action, call(_,_,_,_)).Times(1);

    ASSERT_TRUE(cfsm_status_ok == cfsm_process_event(&c, event_id, &event_data)) << "processing status is not ok";
}

TEST_F(ProcessingTestCase, cfsm_test_process_event_returns_status_guard_rejected) {
    struct cfsm_state states[2];
    cfsm_init_state(&states[0], "state_0");
    cfsm_init_state(&states[1], "state_1");

    cfsm_state c{};
    cfsm_init(&c, 2, states, &states[0]);

    cfsm_transition t{};
    cfsm_init_transition(&t, &states[0], &states[1], event_id);
    cfsm_transition_set_action(&t, callAction);
    cfsm_transition_set_guard(&t, callGuard);

    cfsm_add_transition(&c, &t);

    EXPECT_CALL(*g_guard, call(&states[0], &states[1], event_id, &event_data)).WillOnce(Return(false));
    EXPECT_CALL(*g_action, call(_,_,_,_)).Times(0);

    ASSERT_TRUE(cfsm_status_guard_rejected == cfsm_process_event(&c, event_id, &event_data)) << "processing status is ok";

    ASSERT_TRUE(c.current_state == &states[0]) << "guard rejected transition should not allow current state to change";
}

TEST_F(ProcessingTestCase, cfsm_test_process_event_calls_action_when_transition_enabled) {
    struct cfsm_state states[2];
    cfsm_init_state(&states[0], "state_0");
    cfsm_init_state(&states[1], "state_1");

    struct cfsm_state c;
    cfsm_init(&c, 2, states, &states[0]);

    struct cfsm_transition t;
    cfsm_init_transition(&t, &states[0], &states[1], event_id);
    cfsm_transition_set_action(&t, callAction);
    cfsm_transition_set_guard(&t, callGuard);

    cfsm_add_transition(&c, &t);

    EXPECT_CALL(*g_guard, call(&states[0], &states[1], event_id, &event_data)).WillOnce(Return(true));
    EXPECT_CALL(*g_action, call(&states[0], &states[1], event_id, &event_data)).Times(1);

    ASSERT_TRUE(cfsm_status_ok == cfsm_process_event(&c, event_id, &event_data)) << "processing status should be ok";

    ASSERT_TRUE(c.current_state == &states[1]) << "successfull transition leads to state change";
}


TEST_F(ProcessingTestCase, cfsm_test_process_event_guard_rejected_leads_to_transition_search_continuation) {
    struct cfsm_state states[2];
    cfsm_init_state(&states[0], "state_0");
    cfsm_init_state(&states[1], "state_1");

    struct cfsm_state c;
    cfsm_init(&c, 2, states, &states[0]);

    struct cfsm_transition t1;
    cfsm_init_transition(&t1, &states[0], &states[1], event_id);
    cfsm_transition_set_action(&t1, callAction);
    cfsm_transition_set_guard(&t1, callGuard);

    cfsm_add_transition(&c, &t1);

    struct cfsm_transition t2;
    cfsm_init_transition(&t2, &states[0], &states[1], event_id);
    cfsm_transition_set_action(&t2, callAction);
    cfsm_transition_set_guard(&t2, callGuard);

    cfsm_add_transition(&c, &t2);

    EXPECT_CALL(*g_guard, call(&states[0], &states[1], event_id, &event_data)).Times(2).WillRepeatedly(Return(false));
    EXPECT_CALL(*g_action, call(_,_,_,_)).Times(0);

    ASSERT_TRUE(cfsm_status_guard_rejected == cfsm_process_event(&c, event_id, &event_data)) << "processing status is ok";
}

TEST_F(ProcessingTestCase, cfsm_test_process_event_guard_rejected_leads_to_transition_search_continuation_stop_on_first_accept) {
    struct cfsm_state states[3];
    cfsm_init_state(&states[0], "state_0");
    cfsm_init_state(&states[1], "state_1");
    cfsm_init_state(&states[2], "state_2");

    cfsm_state c{};
    cfsm_init(&c, 2, states, &states[0]);

    cfsm_transition t3{};
    cfsm_init_transition(&t3, &states[0], &states[2], event_id);
    cfsm_transition_set_action(&t3, callAction);
    cfsm_transition_set_guard(&t3, callGuard);

    cfsm_add_transition(&c, &t3);

    cfsm_transition t2{};
    cfsm_init_transition(&t2, &states[0], &states[1], event_id);
    cfsm_transition_set_action(&t2, callAction);
    cfsm_transition_set_guard(&t2, callGuard);

    cfsm_add_transition(&c, &t2);

    cfsm_transition t1{};
    cfsm_init_transition(&t1, &states[0], &states[0], event_id);
    cfsm_transition_set_action(&t1, callAction);
    cfsm_transition_set_guard(&t1, callGuard);

    cfsm_add_transition(&c, &t1);

    EXPECT_CALL(*g_guard, call(&states[0], &states[0], event_id, &event_data)).Times(1).WillRepeatedly(Return(false));
    EXPECT_CALL(*g_guard, call(&states[0], &states[1], event_id, &event_data)).Times(1).WillRepeatedly(Return(false));
    EXPECT_CALL(*g_guard, call(&states[0], &states[2], event_id, &event_data)).Times(1).WillRepeatedly(Return(true));

    EXPECT_CALL(*g_action, call(&states[0], &states[2], event_id, &event_data)).Times(1);

    /*
     * t1: s0 -> s0 : g(false) |
     * t2: s0 -> s1 : g(false) |
     * t3: s0 -> s2 : g(true) \|/
     */

    ASSERT_TRUE(cfsm_status_ok == cfsm_process_event(&c, event_id, &event_data) && "processing status is ok");
}
