# cfsm
pure C implementation of finite state machine

## License
Licensed under the MIT License. See LICENSE file in the project root for full license information.

### Features

    [x] create FSM with states and transitions
    [x] register actions on state entry and exit
    [x] register action on transitions
    [x] register guards for transition
    [x] call process event to utilise abovementioned features
    [x] start, stop or restart your machine on demand with consistency kept
    [x] unit tests powered by googletest

### Open Issues

    [ ] make substates a reality
        [x] this requires struct cfsm and struct cfsm_state become one
        [ ] take recursive approach in process_event
        [ ] handle history of submachines (discard_on_exit, reset_on_entry, keep_state)
            [ ] for each machine set the policy independently
    [ ] advanced trace
        [ ] install logger handlers
        [ ] State History Buffer handlers
    [ ] action return codes should be propagated to event_process caller somehow
    [ ] optimisations for internal transitions
    [ ] e-Transitions (the weirdy ones without an event)
    [ ] print fsm as graphviz description
        [x] state names
        [ ] state indices
        [ ] action and description generation
            [ ] without custom macro definitions? See Ultimate goals. 
    [ ] memory management
        [ ] handle deffered events
            [ ] use unsorted queue for deffered events
            [ ] after transition to new state, get first event that match to any transmission
            [ ] transition not found leaves event on deffered queue
            [ ] transition guard_reject removes event from deffered queue
            [ ] deferred queue is LILO container but with fast forwarding available
                [ ] this means cfsm accept deferred events in order of apperance 
        [ ] define cfsm_process_event as sink for incomming event
            [ ] perhabs deleter function is required as part of event
                [ ] should be easy and compatible with std::unique_ptr
        [ ] use data structure to manage outcomming transitions for state
            [ ] make it customizable - user should be able to operate on arrays
            [ ] iterate over all transitions is nice to have
            [ ] get list of transitions for certain event_id is key feature
        [ ] use data structure to manage substates (iterating)
            [ ] make it customizable - user should be able to operate on arrays
            [ ] iterate over all substates is nice to have
            [ ] random access to state via pointer or index is key feature 

### Ultimate goal(s)

#### No behaviour depending on macro redefinition
Simply `#inlude <cfsm/cfsm.h>` and define a state machine of your own!

#### NFA support (Non-deterministic Finite Automaton)
Imagine being in more than one state simultaneously! Gain advantage from superposition just like subatomic particle. Now you can express heavy computational problems in simple terms - let the machine do the work for you!
