/**
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include <cfsm/cfsm.h>

#include "cfsm_test/cfsm_test_runner.h"

/**
 * TEST FIXTURE
 */
struct transition_call {
    int count;
    struct cfsm_state *origin;
    struct cfsm_state *next;
    int event_id;
    void *event_data;
};

struct transition_call action_data;
struct transition_call guard_data;

static void reset_transition_call(struct transition_call * transition_call_data) {
    transition_call_data->origin = nullptr;
    transition_call_data->next = nullptr;
    transition_call_data->event_id = 0;
    transition_call_data->event_data = nullptr;
    transition_call_data->count = 0;
}

static void setup() {
    reset_transition_call(&action_data);
    reset_transition_call(&guard_data);
}

void guard_impl(struct cfsm_state *origin, struct cfsm_state *next, int event_id, void *event_data) {
    guard_data.origin = origin;
    guard_data.next = next;
    guard_data.event_id = event_id;
    guard_data.event_data = event_data;

    ++guard_data.count;
}

bool guard(struct cfsm_state *origin, struct cfsm_state *next, int event_id, void *event_data) {
    CFSM_TEST_LOG("%s called! %s(=>)%s [id=%d/accepted] ", __FUNCTION__, origin->name, next->name, event_id);

    guard_impl(origin, next, event_id, event_data);
    return true;
}

bool negative_guard(struct cfsm_state *origin, struct cfsm_state *next, int event_id, void *event_data) {
    CFSM_TEST_LOG("%s called! %s(=>)%s [id=%d/rejected] ", __FUNCTION__, origin->name, next->name, event_id);

    guard_impl(origin, next, event_id, event_data);
    return false;
}

void action(struct cfsm_state *origin, struct cfsm_state *next, int event_id, void *event_data) {
    CFSM_TEST_LOG("%s called! %s(=>)%s [id=%d]", __FUNCTION__ , origin->name, next->name, event_id);
    action_data.origin = origin;
    action_data.next = next;
    action_data.event_id = event_id;
    action_data.event_data = event_data;
    ++action_data.count;
}

/**
 * TEST CASES
 */
void cfsm_test_process_event_returns_not_ok_if_no_transition_exists(void) {
    setup();

    int event_id = 33;
    int event_data = 123;

    struct cfsm_state states[2];
    cfsm_init_state(&states[0], "state_0");
    cfsm_init_state(&states[1], "state_1");

    struct cfsm_state c;
    cfsm_init(&c, 2, states, &states[0]);

    CFSM_TEST_ASSERT(cfsm_status_not_ok == cfsm_process_event(&c, event_id, &event_data) && "processing status is not ok");
    CFSM_TEST_ASSERT(c.current_state == &states[0] && "current state is not allowed to change without transition");
}

void cfsm_test_process_event_returns_not_ok_if_no_transition_for_event_is_found(void) {
    setup();

    int event_id = 33;
    int event_data = 123;

    struct cfsm_state states[2];
    cfsm_init_state(&states[0], "state_0");
    cfsm_init_state(&states[1], "state_1");

    struct cfsm_state c;
    cfsm_init(&c, 2, states, &states[0]);

    struct cfsm_transition t;
    cfsm_init_transition(&t, &states[0], &states[1], event_id);
    cfsm_transition_set_action(&t, action);
    cfsm_transition_set_guard(&t, guard);

    cfsm_add_transition(&c, &t);

    CFSM_TEST_ASSERT(cfsm_status_not_ok == cfsm_process_event(&c, 335, &event_data) && "processing status is not ok");
    CFSM_TEST_ASSERT(c.current_state == &states[0] && "current state is not allowed to change without transition");
}

void cfsm_test_process_event_null_guard_means_transition_is_enabled(void) {
    setup();

    int event_id = 33;
    int event_data = 123;

    struct cfsm_state states[2];
    cfsm_init_state(&states[0], "state_0");
    cfsm_init_state(&states[1], "state_1");

    struct cfsm_state c;
    cfsm_init(&c, 2, states, &states[0]);

    struct cfsm_transition t;
    cfsm_init_transition(&t, &states[0], &states[1], event_id);
    cfsm_transition_set_action(&t, action);

    cfsm_add_transition(&c, &t);

    CFSM_TEST_ASSERT(cfsm_status_ok == cfsm_process_event(&c, event_id, &event_data) && "processing status is not ok");
    CFSM_TEST_ASSERT(0 == guard_data.count && "null guard should've never been called!");
    CFSM_TEST_ASSERT(1 == action_data.count && "action should've been called once");
}

void cfsm_test_process_event_returns_status_guard_rejected(void) {
    setup();

    int event_id = 33;
    int event_data = 123;

    struct cfsm_state states[2];
    cfsm_init_state(&states[0], "state_0");
    cfsm_init_state(&states[1], "state_1");

    struct cfsm_state c;
    cfsm_init(&c, 2, states, &states[0]);

    struct cfsm_transition t;
    cfsm_init_transition(&t, &states[0], &states[1], event_id);
    cfsm_transition_set_action(&t, action);
    cfsm_transition_set_guard(&t, negative_guard);

    cfsm_add_transition(&c, &t);

    CFSM_TEST_ASSERT(cfsm_status_guard_rejected == cfsm_process_event(&c, event_id, &event_data) && "processing status is ok");
    CFSM_TEST_ASSERT(1 == guard_data.count && "guard should've been called once");
    CFSM_TEST_ASSERT(&states[0] == guard_data.origin);
    CFSM_TEST_ASSERT(&states[1] == guard_data.next);
    CFSM_TEST_ASSERT(event_id == guard_data.event_id);
    CFSM_TEST_ASSERT(&event_data == guard_data.event_data);

    CFSM_TEST_ASSERT(0 == action_data.count && "action should've been never called!");

    CFSM_TEST_ASSERT(c.current_state == &states[0] && "guard rejected transition should not allow current state to change");
}


void cfsm_test_process_event_calls_action_when_transition_enabled(void) {
    setup();

    int event_id = 33;
    int event_data = 123;

    struct cfsm_state states[2];
    cfsm_init_state(&states[0], "state_0");
    cfsm_init_state(&states[1], "state_1");

    struct cfsm_state c;
    cfsm_init(&c, 2, states, &states[0]);

    struct cfsm_transition t;
    cfsm_init_transition(&t, &states[0], &states[1], event_id);
    cfsm_transition_set_action(&t, action);
    cfsm_transition_set_guard(&t, guard);

    cfsm_add_transition(&c, &t);

    CFSM_TEST_ASSERT_MSG(cfsm_status_ok == cfsm_process_event(&c, event_id, &event_data), "processing status should be ok");

    CFSM_TEST_ASSERT_MSG(1 == guard_data.count, "guard should've been be called once");
    CFSM_TEST_ASSERT(&states[0] == guard_data.origin);
    CFSM_TEST_ASSERT(&states[1] == guard_data.next);
    CFSM_TEST_ASSERT(event_id == guard_data.event_id);
    CFSM_TEST_ASSERT(&event_data == guard_data.event_data);

    CFSM_TEST_ASSERT_MSG(1 == action_data.count, "action should've been called once");
    CFSM_TEST_ASSERT(&states[0] == action_data.origin);
    CFSM_TEST_ASSERT(&states[1] == action_data.next);
    CFSM_TEST_ASSERT(event_id == action_data.event_id);
    CFSM_TEST_ASSERT(&event_data == action_data.event_data);

    CFSM_TEST_ASSERT_MSG(c.current_state == &states[1], "successfull transition leads to state change");
}

void cfsm_test_process_event_guard_rejected_leads_to_transition_search_continuation(void) {
    setup();

    int event_id = 33;
    int event_data = 123;

    struct cfsm_state states[2];
    cfsm_init_state(&states[0], "state_0");
    cfsm_init_state(&states[1], "state_1");

    struct cfsm_state c;
    cfsm_init(&c, 2, states, &states[0]);

    struct cfsm_transition t1;
    cfsm_init_transition(&t1, &states[0], &states[1], event_id);
    cfsm_transition_set_action(&t1, action);
    cfsm_transition_set_guard(&t1, negative_guard);

    cfsm_add_transition(&c, &t1);

    struct cfsm_transition t2;
    cfsm_init_transition(&t2, &states[0], &states[1], event_id);
    cfsm_transition_set_action(&t2, action);
    cfsm_transition_set_guard(&t2, negative_guard);

    cfsm_add_transition(&c, &t2);

    CFSM_TEST_ASSERT(cfsm_status_guard_rejected == cfsm_process_event(&c, event_id, &event_data) && "processing status is ok");
    CFSM_TEST_ASSERT(2 == guard_data.count && "guard should've been called twice");

    CFSM_TEST_ASSERT(0 == action_data.count && "action should've been never called!");
}

void cfsm_test_process_event_guard_rejected_leads_to_transition_search_continuation_stop_on_first_accept(void) {
    setup();

    int event_id = 33;
    int event_data = 123;

    struct cfsm_state states[3];
    cfsm_init_state(&states[0], "state_0");
    cfsm_init_state(&states[1], "state_1");
    cfsm_init_state(&states[2], "state_2");

    struct cfsm_state c;
    cfsm_init(&c, 2, states, &states[0]);

    struct cfsm_transition t1;
    cfsm_init_transition(&t1, &states[0], &states[1], event_id);
    cfsm_transition_set_action(&t1, action);
    cfsm_transition_set_guard(&t1, negative_guard);

    cfsm_add_transition(&c, &t1);

    struct cfsm_transition t2;
    cfsm_init_transition(&t2, &states[0], &states[2], event_id);
    cfsm_transition_set_action(&t2, action);
    cfsm_transition_set_guard(&t2, guard);

    cfsm_add_transition(&c, &t2);

    struct cfsm_transition t3;
    cfsm_init_transition(&t3, &states[0], &states[1], event_id);
    cfsm_transition_set_action(&t3, action);
    cfsm_transition_set_guard(&t3, negative_guard);

    cfsm_add_transition(&c, &t3);

    CFSM_TEST_ASSERT(cfsm_status_ok == cfsm_process_event(&c, event_id, &event_data) && "processing status is ok");
    CFSM_TEST_ASSERT(1 < guard_data.count && "guard should've been called more than once!");

    CFSM_TEST_ASSERT(1 == action_data.count && "action should've been called exactly once!");
    CFSM_TEST_ASSERT(&states[0] == action_data.origin);
    CFSM_TEST_ASSERT(&states[2] == action_data.next);
    CFSM_TEST_ASSERT(event_id == action_data.event_id);
    CFSM_TEST_ASSERT(&event_data == action_data.event_data);
}


int main(int argc, char *argv[]) {
    cfsm_test_init(__FILENAME__);

    CFSM_TEST_ADD(cfsm_test_process_event_calls_action_when_transition_enabled);
    CFSM_TEST_ADD(cfsm_test_process_event_returns_status_guard_rejected);
    CFSM_TEST_ADD(cfsm_test_process_event_guard_rejected_leads_to_transition_search_continuation);
    CFSM_TEST_ADD(cfsm_test_process_event_returns_not_ok_if_no_transition_exists);
    CFSM_TEST_ADD(cfsm_test_process_event_returns_not_ok_if_no_transition_for_event_is_found);
    CFSM_TEST_ADD(cfsm_test_process_event_null_guard_means_transition_is_enabled);
    CFSM_TEST_ADD(cfsm_test_process_event_guard_rejected_leads_to_transition_search_continuation_stop_on_first_accept);

    cfsm_test_run_all();
    return cfsm_test_destroy();
}