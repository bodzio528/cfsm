#include <assert.h>
#include <cfsm/cfsm.h>

#include "cfsm_test_runner.h"

void cfsm_add_state(struct cfsm *_cfsm, struct cfsm_state *_state) {
    ++_cfsm->num_states;
}

void cfsm_add_transition(struct cfsm *_cfsm, struct cfsm_transition *_transition) {
    ++_transition->origin->num_transitions;
}

void cfsm_test_add_single_transition_increment_transition_counter_for_state(void) {
    struct cfsm c = {0};
    struct cfsm_state s = {"hakuna_matata", 0};
    cfsm_add_state(&c, &s);
    assert(1 == c.num_states && "state count should be equal to 1");

    struct cfsm_transition t = {&s, 1, &s, nullptr, nullptr};
    cfsm_add_transition(&c, &t);
    assert((1 == s.num_transitions) && "transition count should be equal to 1 for origin state");
}

int main(int argc, char *argv[]) {
    cfsm_test_add("cfsm_test_add_single_transition_increment_transition_counter_for_state", cfsm_test_add_single_transition_increment_transition_counter_for_state);

    cfsm_test_run_all();
    cfsm_test_destroy(testbench);

    return 0;
}