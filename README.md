# cfsm
pure C implementation of finite state machine

### Features

    [x] create FSM with states and transitions
    [x] register actions on state entry and exit
    [x] register action on transitions
    [x] register guards for transition
    [x] call process event to utilise abovementioned features
    [x] start, stop or restart your machine on demand with consistency kept

### Open Issues

    [ ] make substates a reality
        [ ] this requires struct cfsm and struct cfsm_state become one
    [ ] action return codes should be propagated to event_process caller somehow
    [ ] optimisations for internal transitions
    [ ] e-Transitions (the weirdy ones without an event)
    [ ] handle history of submachines (discard_on_exit, reset_on_entry, keep_state)
        [ ] preferably for each machine independently
    [ ] print fsm as graphviz description
        [x] state names
        [ ] action and description generation
            [ ] without custom macro definitions? See Ultimate goals. 

### Ultimate goal(s)

#### No behaviour depending on macro redefinition
Simply `#inlude <cfsm/cfsm.h>` and define a state machine of your own!
#### NFA support (Non-deterministic Finite Automaton)
Imagine being in more than one state simultaneously! Gain advantage from superposition just like subatomic particle. Now you can express heavy computational problems in simple terms - let the machine do the work for you!
