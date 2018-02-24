#include <cfsm/cfsm.h>
#include <assert.h>

#include "cfsm_test_runner.h"

int call_sequence_number = 0;

struct state_action_data {
    struct cfsm_state * state;
    int event_id;
    void * event_data;

    int call_count;
    int sequence_number;
};

struct state_action_data s0_entry_action_data;
struct state_action_data s0_exit_action_data;
struct state_action_data s1_entry_action_data;
struct state_action_data s1_exit_action_data;

void init_state_action_data(struct state_action_data * data) {
    data->state = nullptr;
    data->event_id = 0;
    data->event_data = nullptr;

    data->call_count = 0;
    data->sequence_number = 0;
}

void setup() {
    init_state_action_data(&s0_entry_action_data);
    init_state_action_data(&s0_exit_action_data);
    init_state_action_data(&s1_entry_action_data);
    init_state_action_data(&s1_exit_action_data);
}

void s0_entry_action(struct cfsm_state * state, int event_id, void *event_data) {
    ++s0_entry_action_data.call_count;
    s0_entry_action_data.sequence_number = ++call_sequence_number;

    s0_entry_action_data.state = state;
    s0_entry_action_data.event_id = event_id;
    s0_entry_action_data.event_data = event_data;
}

void s0_exit_action(struct cfsm_state * state, int event_id, void *event_data) {
    ++s0_exit_action_data.call_count;
    s0_exit_action_data.sequence_number = ++call_sequence_number;

    s0_exit_action_data.state = state;
    s0_exit_action_data.event_id = event_id;
    s0_exit_action_data.event_data = event_data;
}

void s1_entry_action(struct cfsm_state * state, int event_id, void *event_data) {
    ++s1_entry_action_data.call_count;
    s1_entry_action_data.sequence_number = ++call_sequence_number;

    s1_entry_action_data.state = state;
    s1_entry_action_data.event_id = event_id;
    s1_entry_action_data.event_data = event_data;
}

void s1_exit_action(struct cfsm_state * state, int event_id, void *event_data) {
    ++s1_exit_action_data.call_count;
    s1_exit_action_data.sequence_number = ++call_sequence_number;

    s1_exit_action_data.state = state;
    s1_exit_action_data.event_id = event_id;
    s1_exit_action_data.event_data = event_data;
}

void cfsm_start(struct cfsm *fsm, int event_id, void *event_data) {
    struct cfsm_state * fsm_initial_state = fsm->initial_state;
    fsm->current_state = fsm_initial_state;
    fsm_initial_state->entry_action(fsm_initial_state, event_id, event_data);
    // TODO: cfsm_start should call start over all substates!
}

/**
 * TEST CASES
 */
void cfsm_test_start_calls_entry_action_over_initial_state(void) {
    setup();

    int initial_event_id = 8;
    int event_data = 523;

    struct cfsm_state state;
    cfsm_init_state(&state, "hakuna-matata");
    state.entry_action = s0_entry_action;

    struct cfsm c;
    cfsm_init(&c, 1, &state, &state);

    cfsm_start(&c, initial_event_id, &event_data);

    assert(1 == s0_entry_action_data.call_count && "entry action on initial state should be called");
    assert(&state == s0_entry_action_data.state);
    assert(initial_event_id == s0_entry_action_data.event_id);
    assert(&event_data == s0_entry_action_data.event_data && "no copying of event data");
}

void cfsm_stop(struct cfsm *fsm, int event_id, void *event_data) {
    struct cfsm_state * current_state = fsm->current_state;
    current_state->exit_action(current_state, event_id, event_data);
    fsm->current_state = nullptr;
    // TODO: cfsm_stop should call start over all substates!
}

void cfsm_test_stop_calls_exit_action_over_current_state(void) {
    setup();

    int stop_event_id = 9;
    int event_data = 523;

    struct cfsm_state state;
    cfsm_init_state(&state, "hakuna-matata");
    state.exit_action = s0_exit_action;

    struct cfsm c;
    cfsm_init(&c, 1, &state, &state);

    cfsm_stop(&c, stop_event_id, &event_data);

    assert(1 == s0_exit_action_data.call_count && "entry action on initial state should be called");
    assert(&state == s0_exit_action_data.state);
    assert(stop_event_id == s0_exit_action_data.event_id);
    assert(&event_data == s0_exit_action_data.event_data && "no copying of event data");
}


void cfsm_restart(struct cfsm *fsm, int event_id, void *event_data) {
    cfsm_stop(fsm, event_id, event_data);
    cfsm_start(fsm, event_id, event_data);
}

void cfsm_test_restart_calls_exit_action_over_current_state_then_transit_to_initial_state_then_call_entry_action_over_initial_state(void) {
    setup();

    int restart_event_id = 17;
    int event_data = 635;

    struct cfsm_state state;
    cfsm_init_state(&state, "hakuna-matata");
    state.entry_action = s0_entry_action;
    state.exit_action = s0_exit_action;

    struct cfsm c;
    cfsm_init(&c, 1, &state, &state);

    cfsm_restart(&c, restart_event_id, &event_data);

    assert(1 == s0_exit_action_data.call_count && "entry action on initial state should be called");
    assert(&state == s0_exit_action_data.state);
    assert(restart_event_id == s0_exit_action_data.event_id);
    assert(&event_data == s0_exit_action_data.event_data && "no copying of event data");

    assert(1 == s0_entry_action_data.call_count && "entry action on initial state should be called");

    assert(&state == s0_entry_action_data.state);
    assert(restart_event_id == s0_entry_action_data.event_id);
    assert(&event_data == s0_entry_action_data.event_data && "no copying of event data");

    assert(s0_exit_action_data.sequence_number < s0_entry_action_data.sequence_number && "entry over initial should be called after exit over current");
}

void cfsm_test_processing_event_calls_actions_over_endpoint_states(void) {
    setup();

    int event_id = 10;
    int event_data = 135;

    struct cfsm_state states[2];
    cfsm_init_state(&states[0], "hakuna-matata");
    cfsm_init_state(&states[1], "abra-cadabra");

    states[0].exit_action = s0_exit_action;
    states[1].entry_action = s1_entry_action;

    struct cfsm_transition t;
    cfsm_init_transition(&t, &states[0], &states[1], event_id);

    struct cfsm c;
    cfsm_init(&c, 2, states, &states[0]);

    cfsm_add_transition(&c, &t);

    assert(cfsm_status_ok == cfsm_process_event(&c, event_id, &event_data));

    assert(1 == s0_exit_action_data.call_count && "should call exit action on source state");
    assert(&states[0] == s0_exit_action_data.state);
    assert(event_id == s0_exit_action_data.event_id);
    assert(&event_data == s0_exit_action_data.event_data && "no copy allowed!");

    assert(1 == s1_entry_action_data.call_count && "should call entry action on target state");
    assert(&states[1] == s1_entry_action_data.state);
    assert(event_id == s1_entry_action_data.event_id);
    assert(&event_data == s1_entry_action_data.event_data && "no copy allowed!");
}

int main(int argc, char *argv[]) {
    cfsm_test_add("cfsm_test_processing_event_calls_actions_over_endpoint_states",
                  cfsm_test_processing_event_calls_actions_over_endpoint_states);
    cfsm_test_add("cfsm_test_start_calls_entry_action_over_initial_state",
                  cfsm_test_start_calls_entry_action_over_initial_state);
    cfsm_test_add("cfsm_test_stop_calls_exit_action_over_current_state",
                  cfsm_test_stop_calls_exit_action_over_current_state);
    cfsm_test_add("cfsm_test_restart_calls_exit_action_over_current_state_then_transit_to_initial_state_then_call_entry_action_over_initial_state",
                  cfsm_test_restart_calls_exit_action_over_current_state_then_transit_to_initial_state_then_call_entry_action_over_initial_state);


    cfsm_test_run_all();
    cfsm_test_destroy(testbench);
    return 0;
}