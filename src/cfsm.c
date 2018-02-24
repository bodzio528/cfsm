#include "cfsm/cfsm.h"
struct cfsm *cfsm_init(struct cfsm *fsm, int num_states, struct cfsm_state *states, struct cfsm_state * initial_state) {
    fsm->num_states = num_states;
    fsm->states = states;
    fsm->initial_state = initial_state;
    // TODO: fsm->current_state = nullptr; // explicit start call is required!
    fsm->current_state = initial_state;

    return fsm;
}

struct cfsm_state *cfsm_init_state(struct cfsm_state *state, const char *name) {
    state->name = name;
    state->num_transitions = 0;
    state->transitions = nullptr;
    state->entry_action = cfsm_null_state_action;
    state->exit_action = cfsm_null_state_action;

    return state;
}

void cfsm_null_state_action(struct cfsm_state * state, int event_id, void *event_data) {
    (void)state;
    (void)event_id;
    (void)event_data;
    return;
}

void cfsm_null_action(struct cfsm_state *source, struct cfsm_state *target, int event_id, void *event_data) {
    (void)source;
    (void)target;
    (void)event_id;
    (void)event_data;
    return;
}

bool cfsm_null_guard(struct cfsm_state *source, struct cfsm_state *target, int event_id, void *event_data) {
    (void)source;
    (void)target;
    (void)event_id;
    (void)event_data;
    return true;
}

struct cfsm_transition * cfsm_init_transition(struct cfsm_transition * t, struct cfsm_state* source, struct cfsm_state * target, int event_id) {
    return cfsm_init_transition_ag(t, source, target, event_id, cfsm_null_action, cfsm_null_guard);
}

struct cfsm_transition *cfsm_init_transition_a(struct cfsm_transition *t, struct cfsm_state *source, struct cfsm_state *target, int event_id, cfsm_action_f action) {
    return cfsm_init_transition_ag(t, source, target, event_id, action, cfsm_null_guard);
}

struct cfsm_transition *cfsm_init_transition_g(struct cfsm_transition *t, struct cfsm_state *source, struct cfsm_state *target, int event_id, cfsm_guard_f guard) {
    return cfsm_init_transition_ag(t, source, target, event_id, cfsm_null_action, guard);
}

struct cfsm_transition *cfsm_init_transition_ag(struct cfsm_transition *t, struct cfsm_state *source, struct cfsm_state *target, int event_id, cfsm_action_f action, cfsm_guard_f guard) {
    t->source = source;
    t->target = target;
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

void cfsm_add_transition(struct cfsm *fsm, struct cfsm_transition *t) {
    // TODO: use container abstraction to keep track on transitions and traverse them efficiently.
    struct cfsm_transition_list *node = malloc(sizeof(struct cfsm_transition_list));
    ++t->source->num_transitions;
    node->next = t->source->transitions;
    node->transition = t;
    t->source->transitions = node;
}

void cfsm_start(struct cfsm *fsm, int event_id, void *event_data) {
    struct cfsm_state * fsm_initial_state = fsm->initial_state;
    fsm->current_state = fsm_initial_state;
    fsm_initial_state->entry_action(fsm_initial_state, event_id, event_data);
    // TODO: cfsm_start should call start over all substates!
}

void cfsm_stop(struct cfsm *fsm, int event_id, void *event_data) {
    struct cfsm_state * current_state = fsm->current_state;
    current_state->exit_action(current_state, event_id, event_data);
    fsm->current_state = nullptr;
    // TODO: cfsm_stop should call start over all substates!
}

void cfsm_restart(struct cfsm *fsm, int event_id, void *event_data) {
    cfsm_stop(fsm, event_id, event_data);
    cfsm_start(fsm, event_id, event_data);
}

enum cfsm_status cfsm_process_event(struct cfsm *fsm, int event_id, void *event_data) {
    enum cfsm_status result = cfsm_status_not_ok; // -> transition not found

    struct cfsm_state *current_state = fsm->current_state; // get current state O(1);

    // find transition from current state on event_id O(s->num_transition)
    struct cfsm_transition_list *ttt = current_state->transitions;
    while (nullptr != ttt) {
        struct cfsm_transition *tt = ttt->transition;
        if (event_id == tt->event_id) {
            struct cfsm_transition *t = ttt->transition;
            if (t->guard(t->source, t->target, event_id, event_data)) {
                t->source->exit_action(t->source, event_id, event_data);
                t->action(t->source, t->target, event_id, event_data);
                fsm->current_state = t->target;
                t->target->entry_action(t->target, event_id, event_data);
                return cfsm_status_ok;
            } else {
                result = cfsm_status_guard_rejected;
            }
        }
        ttt = ttt->next;
    }

    return result;
}
