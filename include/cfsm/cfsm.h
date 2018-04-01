/**
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#ifndef LIBCFSM_CFSM_H_
#define LIBCFSM_CFSM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "cfsm_nullptr.h"

/**
 * CFSM STATE
 */
struct cfsm_state;

typedef void (*cfsm_state_action_f)(struct cfsm_state *state, int event_id, void *event_data);

struct cfsm_state {
    const char *name;
    int num_transitions;
    struct cfsm_transition_list *transitions;

    cfsm_state_action_f entry_action;
    cfsm_state_action_f exit_action;

    // sub-fsm
    int num_states;
    struct cfsm_state *states;
    struct cfsm_state *initial_state;
    struct cfsm_state *current_state;
};

/**
 * install fsm within a state, effectively prepare state to contain substates amd accept process event calls
 * @param state state to be promoted
 * @param num_states number of substates
 * @param states substates array
 * @param initial_state chosen state from substates array
 * @return pointer to promoted state, usually the same as input
 */
struct cfsm_state *
cfsm_init(struct cfsm_state *state, int num_states, struct cfsm_state *states, struct cfsm_state *initial_state);
struct cfsm_state *cfsm_init_state(struct cfsm_state *state, const char *name);

void cfsm_state_destroy(struct cfsm_state *fsm);

void cfsm_null_state_action(struct cfsm_state *state, int event_id, void *event_data);

/**
 * CFSM TRANSITION
 */
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

bool cfsm_null_guard(struct cfsm_state *source, struct cfsm_state *target, int event_id, void *event_data);
void cfsm_null_action(struct cfsm_state *source, struct cfsm_state *target, int event_id, void *event_data);

void cfsm_add_transition(struct cfsm_state *fsm, struct cfsm_transition *t);

struct cfsm_transition *
cfsm_init_transition(struct cfsm_transition *t, struct cfsm_state *source, struct cfsm_state *target, int event_id);
struct cfsm_transition *
cfsm_init_transition_ag(struct cfsm_transition *t, struct cfsm_state *source, struct cfsm_state *target, int event_id,
                        cfsm_action_f action, cfsm_guard_f guard);

void cfsm_transition_set_action(struct cfsm_transition *t, cfsm_action_f action);
void cfsm_transition_set_guard(struct cfsm_transition *t, cfsm_guard_f guard);

bool cfsm_transition_is_internal(struct cfsm_transition *t);

/**
 * CFSM EVENT PROCESSING FUNCTIONS
 */
void cfsm_start(struct cfsm_state *fsm, int event_id, void *event_data);

void cfsm_stop(struct cfsm_state *fsm, int event_id, void *event_data);

void cfsm_restart(struct cfsm_state *fsm, int event_id, void *event_data);

enum cfsm_status {
    cfsm_status_ok,
    cfsm_status_not_ok,
    cfsm_status_guard_rejected,
    cfsm_status_deffered
};

enum cfsm_status cfsm_process_event(struct cfsm_state *fsm, int event_id, void *event_data);

#ifdef __cplusplus
}
#endif

#endif /* LIBCFSM_CFSM_H_ */
