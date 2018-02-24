#ifndef LIBCFSM_CFSM_H
#define LIBCFSM_CFSM_H

#pragma once

#include <stdbool.h>
#include <stdlib.h>

#define nullptr NULL

struct cfsm_state;

typedef void (*cfsm_state_action_f)(struct cfsm_state * state, int event_id, void *event_data);

struct cfsm_state {
    const char *name;
    int num_transitions;
    struct cfsm_transition_list *transitions;

    cfsm_state_action_f entry_action;
    cfsm_state_action_f exit_action;
};

struct cfsm_state *cfsm_init_state(struct cfsm_state *state, const char *name);
void cfsm_null_state_action(struct cfsm_state * state, int event_id, void *event_data);


struct cfsm {
    struct cfsm_state *initial_state;
    struct cfsm_state *current_state;

    int num_states;
};

struct cfsm *cfsm_init(struct cfsm *fsm, int num_states, struct cfsm_state *states, struct cfsm_state *initial_state);

struct cfsm_transition;

struct cfsm_transition_list {
    struct cfsm_transition *transition;
    struct cfsm_transition_list *next;
};

typedef void (*cfsm_action_f)(struct cfsm_state *origin, struct cfsm_state *next, int event_id, void *event_data);
typedef bool (*cfsm_guard_f)(struct cfsm_state *origin, struct cfsm_state *next, int event_id, void *event_data);

struct cfsm_transition {
    struct cfsm_state *source;
    int event_id;
    struct cfsm_state *target;
    cfsm_action_f action;
    cfsm_guard_f guard;
};

bool cfsm_null_guard(struct cfsm_state *origin, struct cfsm_state *next, int event_id, void *event_data);
void cfsm_null_action(struct cfsm_state *origin, struct cfsm_state *next, int event_id, void *event_data);

void cfsm_add_transition(struct cfsm *fsm, struct cfsm_transition *t);

struct cfsm_transition *cfsm_init_transition(struct cfsm_transition *t, struct cfsm_state *source, struct cfsm_state *target, int event_id);
struct cfsm_transition *cfsm_init_transition_a(struct cfsm_transition *t, struct cfsm_state *source, struct cfsm_state *target, int event_id, cfsm_action_f action);
struct cfsm_transition *cfsm_init_transition_g(struct cfsm_transition *t, struct cfsm_state *source, struct cfsm_state *target, int event_id, cfsm_guard_f guard);
struct cfsm_transition *cfsm_init_transition_ag(struct cfsm_transition *t, struct cfsm_state *source, struct cfsm_state *target, int event_id, cfsm_action_f action, cfsm_guard_f guard);

void cfsm_transition_set_action(struct cfsm_transition *t, cfsm_action_f action);
void cfsm_transition_set_guard(struct cfsm_transition *t, cfsm_guard_f guard);

inline bool cfsm_transition_is_internal(struct cfsm_transition * t) {
    return t->source == t->target;
}

enum cfsm_status {
    cfsm_status_ok,
    cfsm_status_not_ok,
    cfsm_status_guard_rejected,
    cfsm_status_deffered
};

enum cfsm_status cfsm_process_event(struct cfsm *fsm, int event_id, void *event_data);

#endif /* LIBCFSM_CFSM_H */
