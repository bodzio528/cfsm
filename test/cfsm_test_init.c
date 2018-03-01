/**
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include <cfsm/cfsm.h>

#include "cfsm_test/cfsm_test_runner.h"

void cfsm_test_fsm_init(void) {
    struct cfsm_state c;
    cfsm_init(&c, 0, nullptr, nullptr);

    CFSM_TEST_ASSERT(0 == c.num_states);
    CFSM_TEST_ASSERT(nullptr == c.states);
    CFSM_TEST_ASSERT(nullptr == c.current_state);
}

void cfsm_test_fsm_init_value_check(void) {
    void * states = (void *)12;
    void * initial_state = (void *)44;

    struct cfsm_state c;
    cfsm_init(&c, 5, states, initial_state);

    CFSM_TEST_ASSERT(5 == c.num_states);
    CFSM_TEST_ASSERT(states == c.states);
    CFSM_TEST_ASSERT(initial_state == c.initial_state);
    CFSM_TEST_ASSERT(nullptr == c.current_state);
}

void cfsm_test_state_init_static(void) {
    const char *name = "hakuna-matata";
    struct cfsm_state s;
    cfsm_init_state(&s, name);

    CFSM_TEST_ASSERT(0 == s.num_transitions);
    CFSM_TEST_ASSERT(nullptr == s.transitions);
    CFSM_TEST_ASSERT(name == s.name);
}

void cfsm_test_state_init_dynamic(void) {
    const char *name = "hakuna-matata";
    struct cfsm_state *s = cfsm_init_state(malloc(sizeof(struct cfsm_state)), name);

    CFSM_TEST_ASSERT(0 == s->num_transitions);
    CFSM_TEST_ASSERT(nullptr == s->transitions);
    CFSM_TEST_ASSERT(name == s->name);

    free(s);
}

void cfsm_test_init_transition(void) {
    struct cfsm_state states[2];

    struct cfsm_transition *t = cfsm_init_transition(malloc(sizeof(struct cfsm_transition)), &states[0], &states[1], 1);

    CFSM_TEST_ASSERT(&states[0] == t->source);
    CFSM_TEST_ASSERT(&states[1] == t->target);
    CFSM_TEST_ASSERT(1 == t->event_id);
    CFSM_TEST_ASSERT(cfsm_null_action == t->action);
    CFSM_TEST_ASSERT(cfsm_null_guard == t->guard);

    cfsm_action_f action = (void *) 10;
    cfsm_guard_f guard = (void *) 12;

    cfsm_transition_set_action(t, action);
    cfsm_transition_set_guard(t, guard);

    CFSM_TEST_ASSERT(action == t->action);
    CFSM_TEST_ASSERT(guard == t->guard);
    free(t);
}

void cfsm_test_add_single_transition_increment_transition_counter_for_state(void) {
    struct cfsm_state s;
    cfsm_init_state(&s, "hakuna-matata");

    struct cfsm_state c;
    cfsm_init(&c, 1, &s, &s);

    CFSM_TEST_ASSERT(1 == c.num_states && "state count should be equal to 1");

    struct cfsm_transition t;
    cfsm_init_transition(&t, &s, &s, 1);
    cfsm_add_transition(&c, &t);
    CFSM_TEST_ASSERT((1 == s.num_transitions) && "transition count should be equal to 1 for source state");

    CFSM_TEST_ASSERT(true == cfsm_transition_is_internal(&t) && "transition with source == next is internal transition");
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

    CFSM_TEST_ASSERT((1 == states[0].num_transitions) && "transition count should be equal to 1 for source state");
    CFSM_TEST_ASSERT((0 == states[1].num_transitions) && "transition count should be equal to 0 for target state");

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

    CFSM_TEST_ASSERT(0 == c.num_transitions && "no accessible transitions after destroy");
    CFSM_TEST_ASSERT(nullptr == c.transitions && "zero transition count after destroy");

    free(t);
    free(u);
}


int main(int argc, char *argv[]) {
    cfsm_test_init(__FILENAME__);

    CFSM_TEST_ADD(cfsm_test_fsm_init);
    CFSM_TEST_ADD(cfsm_test_fsm_init_value_check);
    CFSM_TEST_ADD(cfsm_test_state_init_static);
    CFSM_TEST_ADD(cfsm_test_state_init_dynamic);
    CFSM_TEST_ADD(cfsm_test_init_transition);
    CFSM_TEST_ADD(cfsm_test_add_single_transition_increment_transition_counter_for_state);
    CFSM_TEST_ADD(cfsm_test_add_transition_only_to_stopped_cfsm);
    CFSM_TEST_ADD(cfsm_test_stopped_cfsm_can_be_destroyed);

    cfsm_test_run_all();
    return cfsm_test_destroy();
}