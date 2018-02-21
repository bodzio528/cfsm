#ifndef LIBCFSM_CFSM_H
#define LIBCFSM_CFSM_H

#pragma once

#include <stdbool.h>

struct cfsm {
    int num_states;
};

struct cfsm_state {
    const char *name;
    int num_transitions;
};

struct cfsm_transition {
    struct cfsm_state * origin;
    int event_id;
    struct cfsm_state * next;
    void (*action)(struct cfsm_state* origin, struct cfsm_state * next, int event_id, void* event_data);
    bool (*guard)(struct cfsm_state* origin, struct cfsm_state * next, int event_id, void* event_data);
};

void cfsm_add_state(struct cfsm *_cfsm, struct cfsm_state *_state);
void cfsm_add_transition(struct cfsm *_cfsm, struct cfsm_transition *_transition);

#endif /* LIBCFSM_CFSM_H */
