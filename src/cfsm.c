#include "cfsm/cfsm.h"

struct cfsm *cfsm_init(struct cfsm *fsm) {
    fsm->num_states = 0;
    fsm->states = nullptr;
    fsm->current_state = nullptr;

    return fsm;
}

struct cfsm_state *cfsm_init_state(struct cfsm_state *state, const char *name) {
    state->name = name;
    state->num_transitions = 0;
    state->transitions = nullptr;

    return state;
}

void cfsm_null_action(struct cfsm_state *origin, struct cfsm_state *next, int event_id, void *event_data) {
    return;
}

bool cfsm_null_guard(struct cfsm_state *origin, struct cfsm_state *next, int event_id, void *event_data) {
    return true;
}

struct cfsm_transition * cfsm_init_transition(struct cfsm_transition * t, struct cfsm_state* origin, struct cfsm_state * next, int event_id) {
    return cfsm_init_transition_ag(t, origin, next, event_id, cfsm_null_action, cfsm_null_guard);
}

struct cfsm_transition *cfsm_init_transition_a(struct cfsm_transition *t, struct cfsm_state *origin, struct cfsm_state *next, int event_id, cfsm_action_f action) {
    return cfsm_init_transition_ag(t, origin, next, event_id, action, cfsm_null_guard);
}

struct cfsm_transition *cfsm_init_transition_g(struct cfsm_transition *t, struct cfsm_state *origin, struct cfsm_state *next, int event_id, cfsm_guard_f guard) {
    return cfsm_init_transition_ag(t, origin, next, event_id, cfsm_null_action, guard);
}

struct cfsm_transition *cfsm_init_transition_ag(struct cfsm_transition *t, struct cfsm_state *origin, struct cfsm_state *next, int event_id, cfsm_action_f action, cfsm_guard_f guard) {
    t->origin = origin;
    t->next = next;
    t->event_id = event_id;

    cfsm_transition_set_action(t, action);
    cfsm_transition_set_guard(t, guard);

    return t;
}

void cfsm_transition_set_guard(struct cfsm_transition *t, cfsm_guard_f guard) {
    t->guard = guard;
}

void cfsm_transition_set_action(struct cfsm_transition *t, cfsm_action_f action) {
    t->action = action;
}

void cfsm_add_state(struct cfsm *fsm, struct cfsm_state *state) {
    // TODO: use container abstraction to keep track on states. Efficient traverse is not crucial here, but space occupancy is important.
    struct cfsm_state_list *l = malloc(sizeof(struct cfsm_state_list));
    l->next = fsm->states;
    l->state = state;
    fsm->states = l;
    ++fsm->num_states;
}

void cfsm_add_transition(struct cfsm *fsm, struct cfsm_transition *t) {
    // TODO: use container abstraction to keep track on transitions and traverse them efficiently.
    struct cfsm_transition_list *node = malloc(sizeof(struct cfsm_transition_list));
    ++t->origin->num_transitions;
    node->next = t->origin->transitions;
    node->transition = t;
    t->origin->transitions = node;
}

enum cfsm_status cfsm_process_event(struct cfsm *fsm, int event_id, void *event_data) {
    enum cfsm_status result = cfsm_status_not_ok; // -> no transition found

    struct cfsm_state *origin = fsm->current_state; // get current state O(1);

    // find transition from current state on event_id O(s->num_transition)
    struct cfsm_transition_list *ttt = origin->transitions;
    while (nullptr != ttt) {
        struct cfsm_transition *tt = ttt->transition;
        if (event_id == tt->event_id) {
            struct cfsm_transition *t = ttt->transition;
            if (t->guard(origin, t->next, event_id, event_data)) {
                t->action(origin, t->next, event_id, event_data);
                return cfsm_status_ok;
            } else {
                result = cfsm_status_guard_rejected;
            }
        }
        ttt = ttt->next;
    }

    return result;

}
