#include <assert.h>
#include <cfsm/cfsm.h>

#include "cfsm_test_runner.h"

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

bool guard(struct cfsm_state *origin, struct cfsm_state *next, int event_id, void *event_data) {
    guard_data.origin = origin;
    guard_data.next = next;
    guard_data.event_id = event_id;
    guard_data.event_data = event_data;
    ++guard_data.count;
    return true;
}

bool negative_guard(struct cfsm_state *origin, struct cfsm_state *next, int event_id, void *event_data) {
    guard(origin, next, event_id, event_data);
    return false;
}

void action(struct cfsm_state *origin, struct cfsm_state *next, int event_id, void *event_data) {
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
    cfsm_init_state(&states[0], "hakuna_matata");
    cfsm_init_state(&states[1], "abra-cadabra");

    struct cfsm_state c;
    cfsm_init(&c, 2, states, &states[0]);

    assert(cfsm_status_not_ok == cfsm_process_event(&c, event_id, &event_data) && "processing status is not ok");
    assert(c.current_state == &states[0] && "current state is not allowed to change without transition");
}

void cfsm_test_process_event_returns_not_ok_if_no_transition_for_event_is_found(void) {
    setup();

    int event_id = 33;
    int event_data = 123;

    struct cfsm_state states[2];
    cfsm_init_state(&states[0], "hakuna_matata");
    cfsm_init_state(&states[1], "abra-cadabra");

    struct cfsm_state c;
    cfsm_init(&c, 2, states, &states[0]);

    struct cfsm_transition t;
    cfsm_init_transition(&t, &states[0], &states[1], event_id);
    cfsm_transition_set_action(&t, action);
    cfsm_transition_set_guard(&t, guard);

    cfsm_add_transition(&c, &t);

    assert(cfsm_status_not_ok == cfsm_process_event(&c, 335, &event_data) && "processing status is not ok");
    assert(c.current_state == &states[0] && "current state is not allowed to change without transition");
}

void cfsm_test_process_event_null_guard_means_transition_is_enabled(void) {
    setup();

    int event_id = 33;
    int event_data = 123;

    struct cfsm_state states[2];
    cfsm_init_state(&states[0], "hakuna_matata");
    cfsm_init_state(&states[1], "abra-cadabra");

    struct cfsm_state c;
    cfsm_init(&c, 2, states, &states[0]);

    struct cfsm_transition t;
    cfsm_init_transition(&t, &states[0], &states[1], event_id);
    cfsm_transition_set_action(&t, action);

    cfsm_add_transition(&c, &t);

    assert(cfsm_status_ok == cfsm_process_event(&c, event_id, &event_data) && "processing status is not ok");
    assert(0 == guard_data.count && "null guard should've never been called!");
    assert(1 == action_data.count && "action should've been called once");
}

void cfsm_test_process_event_returns_status_guard_rejected(void) {
    setup();

    int event_id = 33;
    int event_data = 123;

    struct cfsm_state states[2];
    cfsm_init_state(&states[0], "hakuna_matata");
    cfsm_init_state(&states[1], "abra-cadabra");

    struct cfsm_state c;
    cfsm_init(&c, 2, states, &states[0]);

    struct cfsm_transition t;
    cfsm_init_transition(&t, &states[0], &states[1], event_id);
    cfsm_transition_set_action(&t, action);
    cfsm_transition_set_guard(&t, negative_guard);

    cfsm_add_transition(&c, &t);

    assert(cfsm_status_guard_rejected == cfsm_process_event(&c, event_id, &event_data) && "processing status is ok");
    assert(1 == guard_data.count && "guard should've been called once");
    assert(&states[0] == guard_data.origin);
    assert(&states[1] == guard_data.next);
    assert(event_id == guard_data.event_id);
    assert(&event_data == guard_data.event_data);

    assert(0 == action_data.count && "action should've been never called!");

    assert(c.current_state == &states[0] && "guard rejected transition should not allow current state to change");
}

void cfsm_test_process_event_calls_action_when_transition_enabled(void) {
    setup();

    int event_id = 33;
    int event_data = 123;

    struct cfsm_state states[2];
    cfsm_init_state(&states[0], "hakuna_matata");
    cfsm_init_state(&states[1], "abra-cadabra");

    struct cfsm_state c;
    cfsm_init(&c, 2, states, &states[0]);

    struct cfsm_transition t;
    cfsm_init_transition(&t, &states[0], &states[1], event_id);
    cfsm_transition_set_action(&t, action);
    cfsm_transition_set_guard(&t, guard);

    cfsm_add_transition(&c, &t);

    assert(cfsm_status_ok == cfsm_process_event(&c, event_id, &event_data) && "processing status is ok");
    assert(1 == guard_data.count && "guard should've been be called once");
    assert(&states[0] == guard_data.origin);
    assert(&states[1] == guard_data.next);
    assert(event_id == guard_data.event_id);
    assert(&event_data == guard_data.event_data);

    assert(1 == action_data.count && "action should've been called once");
    assert(&states[0] == action_data.origin);
    assert(&states[1] == action_data.next);
    assert(event_id == action_data.event_id);
    assert(&event_data == action_data.event_data);

    assert(c.current_state == &states[1] && "successfull transition leads to state change");
}

void cfsm_test_process_event_guard_rejected_leads_to_transition_search_continuation(void) {
    setup();

    int event_id = 33;
    int event_data = 123;

    struct cfsm_state states[2];
    cfsm_init_state(&states[0], "hakuna_matata");
    cfsm_init_state(&states[1], "abra-cadabra");

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

    assert(cfsm_status_guard_rejected == cfsm_process_event(&c, event_id, &event_data) && "processing status is ok");
    assert(2 == guard_data.count && "guard should've been called twice");

    assert(0 == action_data.count && "action should've been never called!");
}

void cfsm_test_process_event_guard_rejected_leads_to_transition_search_continuation_stop_on_first_accept(void) {
    setup();

    int event_id = 33;
    int event_data = 123;

    struct cfsm_state states[3];
    cfsm_init_state(&states[0], "hakuna_matata");
    cfsm_init_state(&states[1], "abra-cadabra");
    cfsm_init_state(&states[2], "hokus-pokus");

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

    assert(cfsm_status_ok == cfsm_process_event(&c, event_id, &event_data) && "processing status is ok");
    assert(1 < guard_data.count && "guard should've been called more than once!");

    assert(1 == action_data.count && "action should've been called exactly once!");
    assert(&states[0] == action_data.origin);
    assert(&states[2] == action_data.next);
    assert(event_id == action_data.event_id);
    assert(&event_data == action_data.event_data);
}


int main(int argc, char *argv[]) {
    cfsm_test_add("cfsm_test_process_event_calls_action_when_transition_enabled",
                  cfsm_test_process_event_calls_action_when_transition_enabled);
    cfsm_test_add("cfsm_test_process_event_returns_status_guard_rejected",
                  cfsm_test_process_event_returns_status_guard_rejected);
    cfsm_test_add("cfsm_test_process_event_guard_rejected_leads_to_transition_search_continuation",
                  cfsm_test_process_event_guard_rejected_leads_to_transition_search_continuation);
    cfsm_test_add("cfsm_test_process_event_returns_not_ok_if_no_transition_exists",
                  cfsm_test_process_event_returns_not_ok_if_no_transition_exists);
    cfsm_test_add("cfsm_test_process_event_returns_not_ok_if_no_transition_for_event_is_found",
                  cfsm_test_process_event_returns_not_ok_if_no_transition_for_event_is_found);
    cfsm_test_add("cfsm_test_process_event_null_guard_means_transition_is_enabled",
                  cfsm_test_process_event_null_guard_means_transition_is_enabled);
    cfsm_test_add("cfsm_test_process_event_guard_rejected_leads_to_transition_search_continuation_stop_on_first_accept",
                  cfsm_test_process_event_guard_rejected_leads_to_transition_search_continuation_stop_on_first_accept);

    cfsm_test_run_all();
    cfsm_test_destroy(testbench);
    return 0;
}