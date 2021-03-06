/**
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "cfsm/cfsm.h"

static inline bool cfsm_is_started(struct cfsm_state *fsm) {
    return nullptr != fsm->current_state;
}

static inline bool cfsm_is_stopped(struct cfsm_state *fsm) {
    return nullptr == fsm->current_state;
}

struct cfsm_state *cfsm_init(struct cfsm_state *state, int num_states, struct cfsm_state *states, struct cfsm_state *initial_state) {
    state->num_states = num_states;
    state->states = states;
    state->initial_state = initial_state;
    state->current_state = nullptr;
    return state;
}

struct cfsm_state *cfsm_init_state(struct cfsm_state *state, const char *name) {
    state->name = name;
    state->num_transitions = 0;
    state->transitions = nullptr;
    state->entry_action = cfsm_null_state_action;
    state->exit_action = cfsm_null_state_action;

    // a state does not contain submachine by default, need cfsm_init_state_fsm call
    state->num_states = 0;
    state->states = nullptr;
    state->initial_state = nullptr;
    state->current_state = nullptr;
    return state;
}

void cfsm_state_destroy(struct cfsm_state *fsm) {
    if (cfsm_is_started(fsm)) {
        // ERROR: cfsm must be stopped in order to be destroyed!
        return;
    }

    // check for substates to destroy
    if (fsm->states != nullptr && fsm->num_states != 0) {
        for (int i = 0; i < fsm->num_states; ++i) {
            cfsm_state_destroy(&fsm->states[i]);
        }
    }

    // destroy current fsm. free container but transitions need to be released in place of creation
    // TODO: utilize concept of container to manage memory allocated for cfsm!
    while (fsm->transitions != nullptr) {
        struct cfsm_transition_list *head = fsm->transitions;
        fsm->transitions = head->next;
        if (head != nullptr) {
            free(head);
        }
    }

    fsm->num_transitions = 0;
}

void cfsm_null_state_action(struct cfsm_state *state, int event_id, void *event_data) {
    (void)state;
    (void)event_id;
    (void)event_data;
}

void cfsm_null_action(struct cfsm_state *source, struct cfsm_state *target, int event_id, void *event_data) {
    (void)source;
    (void)target;
    (void)event_id;
    (void)event_data;
}

bool cfsm_null_guard(struct cfsm_state *source, struct cfsm_state *target, int event_id, void *event_data) {
    (void)source;
    (void)target;
    (void)event_id;
    (void)event_data;
    return true;
}

struct cfsm_transition * cfsm_init_transition(struct cfsm_transition *t, struct cfsm_state *source, struct cfsm_state *target, int event_id) {
    return cfsm_init_transition_ag(t, source, target, event_id, cfsm_null_action, cfsm_null_guard);
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

void cfsm_add_transition(struct cfsm_state *fsm, struct cfsm_transition *t) {
    if (cfsm_is_started(fsm)) {
        // WARN: modification of already running state machine is prohibited!
        return;
    }

    // TODO: use container abstraction to keep track on transitions and traverse them efficiently.
    struct cfsm_transition_list *node = malloc(sizeof(struct cfsm_transition_list));
    ++t->source->num_transitions;
    node->next = t->source->transitions;
    node->transition = t;
    t->source->transitions = node;
}

void cfsm_start(struct cfsm_state *fsm, int event_id, void *event_data) {
    if (cfsm_is_started(fsm)) {
        // WARN: called cfsm_start over already started fsm. No effect, use restart instead!
        return;
    }
    struct cfsm_state * fsm_initial_state = fsm->initial_state;
    fsm->current_state = fsm_initial_state;
    fsm_initial_state->entry_action(fsm_initial_state, event_id, event_data);
    // TODO: cfsm_start should call start over all subfsms!
}

void cfsm_stop(struct cfsm_state *fsm, int event_id, void *event_data) {
    if (cfsm_is_stopped(fsm)) {
        // WARN: called cfsm_stop over already stopped fsm. No effect.
        return;
    }

    struct cfsm_state * current_state = fsm->current_state;
    current_state->exit_action(current_state, event_id, event_data);
    fsm->current_state = nullptr;
    // TODO: cfsm_stop should call stop over all subfsms!
}

void cfsm_restart(struct cfsm_state *fsm, int event_id, void *event_data) {
    cfsm_stop(fsm, event_id, event_data);
    cfsm_start(fsm, event_id, event_data);
}

enum cfsm_status cfsm_process_event(struct cfsm_state *fsm, int event_id, void *event_data) {
    enum cfsm_status result = cfsm_status_not_ok; // -> transition not found

    if (cfsm_is_stopped(fsm)) {
        cfsm_start(fsm, event_id, event_data);
    }

    struct cfsm_state *current_state = fsm->current_state; // get current state O(1);
    // find transition from current state on event_id O(s->num_transition)
    struct cfsm_transition_list *transition_node = current_state->transitions;
    while (nullptr != transition_node) {
        struct cfsm_transition *t = transition_node->transition;

        if (event_id == t->event_id) {
            if (t->guard(t->source, t->target, event_id, event_data)) {
                t->source->exit_action(t->source, event_id, event_data); //! FIXME: call exit only if target != source
                t->action(t->source, t->target, event_id, event_data);
                fsm->current_state = t->target;
                t->target->entry_action(t->target, event_id, event_data); //! FIXME: call entry only if target != source
                return cfsm_status_ok;
            } else {
                result = cfsm_status_guard_rejected;
            }
        }

        transition_node = transition_node->next;
    }

    return result;
}

bool cfsm_transition_is_internal(struct cfsm_transition *t) {
    return t->source == t->target;
}
