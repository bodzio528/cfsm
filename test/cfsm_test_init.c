#include <assert.h>
#include <cfsm/cfsm.h>

#include "cfsm_test_runner.h"

void cfsm_state_destroy(struct cfsm_state *fsm);

void cfsm_test_fsm_init(void) {
    struct cfsm_state c;
    cfsm_init(&c, 0, nullptr, nullptr);

    assert(0 == c.num_states);
    assert(nullptr == c.states);
    assert(nullptr == c.current_state);
}

void cfsm_test_fsm_init_value_check(void) {
    void * states = (void *)12;
    void * initial_state = (void *)44;

    struct cfsm_state c;
    cfsm_init(&c, 5, states, initial_state);

    assert(5 == c.num_states);
    assert(states == c.states);
    assert(initial_state == c.initial_state);
    assert(nullptr == c.current_state);
}

void cfsm_test_state_init_static(void) {
    const char *name = "hakuna-matata";
    struct cfsm_state s;
    cfsm_init_state(&s, name);

    assert(0 == s.num_transitions);
    assert(nullptr == s.transitions);
    assert(name == s.name);
}

void cfsm_test_state_init_dynamic(void) {
    const char *name = "hakuna-matata";
    struct cfsm_state *s = cfsm_init_state(malloc(sizeof(struct cfsm_state)), name);

    assert(0 == s->num_transitions);
    assert(nullptr == s->transitions);
    assert(name == s->name);

    free(s);
}

void cfsm_test_init_transition(void) {
    struct cfsm_state states[2];

    struct cfsm_transition *t = cfsm_init_transition(malloc(sizeof(struct cfsm_transition)), &states[0], &states[1], 1);

    assert(&states[0] == t->source);
    assert(&states[1] == t->target);
    assert(1 == t->event_id);
    assert(cfsm_null_action == t->action);
    assert(cfsm_null_guard == t->guard);

    cfsm_action_f action = (void *) 10;
    cfsm_guard_f guard = (void *) 12;

    cfsm_transition_set_action(t, action);
    cfsm_transition_set_guard(t, guard);

    assert(action == t->action);
    assert(guard == t->guard);
    free(t);
}

void cfsm_test_add_single_transition_increment_transition_counter_for_state(void) {
    struct cfsm_state s;
    cfsm_init_state(&s, "hakuna-matata");

    struct cfsm_state c;
    cfsm_init(&c, 1, &s, &s);

    assert(1 == c.num_states && "state count should be equal to 1");

    struct cfsm_transition t;
    cfsm_init_transition(&t, &s, &s, 1);
    cfsm_add_transition(&c, &t);
    assert((1 == s.num_transitions) && "transition count should be equal to 1 for source state");

    assert(true == cfsm_transition_is_internal(&t) && "transition with source == next is internal transition");
}

void cfsm_test_add_transition_only_to_stopped_cfsm(void) {
    struct cfsm_state states[2];
    cfsm_init_state(&states[0], "hakuna");
    cfsm_init_state(&states[1], "matata");

    struct cfsm_transition *t = cfsm_init_transition(malloc(sizeof(struct cfsm_transition)), &states[0], &states[1], 1);
    struct cfsm_transition *u = cfsm_init_transition(malloc(sizeof(struct cfsm_transition)), &states[0], &states[1], 74);

    struct cfsm_state c;
    cfsm_init(&c, 2, states, &states[0]);
    cfsm_add_transition(&c, t);

    c.current_state = (struct cfsm_state *)55; /// simulate already started fsm

    cfsm_add_transition(&c, u);

    assert((1 == states[0].num_transitions) && "transition count should be equal to 1 for source state");
    assert((0 == states[1].num_transitions) && "transition count should be equal to 0 for target state");

    free(t);
    free(u);
}

void cfsm_test_stopped_cfsm_can_be_destroyed(void) {
    struct cfsm_state states[2];
    cfsm_init_state(&states[0], "hakuna");
    cfsm_init_state(&states[1], "matata");

    struct cfsm_transition *t = cfsm_init_transition(malloc(sizeof(struct cfsm_transition)), &states[0], &states[1], 1);
    struct cfsm_transition *u = cfsm_init_transition(malloc(sizeof(struct cfsm_transition)), &states[1], &states[0], 2);

    struct cfsm_state c;
    cfsm_init_state(&c, "FSM");
    cfsm_init(&c, 2, states, &states[0]);

    cfsm_add_transition(&c, t);
    cfsm_add_transition(&c, u);

    cfsm_state_destroy(&c);

    assert(0 == c.num_transitions && "no accessible transitions after destroy");
    assert(nullptr == c.transitions && "zero transition count after destroy");

    free(t);
    free(u);
}


int main(int argc, char *argv[]) {
    cfsm_test_add("cfsm_test_fsm_init", cfsm_test_fsm_init);
    cfsm_test_add("cfsm_test_fsm_init_value_check", cfsm_test_fsm_init_value_check);
    cfsm_test_add("cfsm_test_state_init_static", cfsm_test_state_init_static);
    cfsm_test_add("cfsm_test_state_init_dynamic", cfsm_test_state_init_dynamic);
    cfsm_test_add("cfsm_test_init_transition", cfsm_test_init_transition);
    cfsm_test_add("cfsm_test_add_single_transition_increment_transition_counter_for_state",
                  cfsm_test_add_single_transition_increment_transition_counter_for_state);
    cfsm_test_add("cfsm_test_add_transition_only_to_stopped_cfsm", cfsm_test_add_transition_only_to_stopped_cfsm);
    cfsm_test_add("cfsm_test_stopped_cfsm_can_be_destroyed", cfsm_test_stopped_cfsm_can_be_destroyed);


    cfsm_test_run_all();
    cfsm_test_destroy(testbench);

    return 0;
}