/**
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include <cfsm/cfsm.h>

#include <gtest/gtest.h>

using namespace ::testing;

TEST(cfsm_test_init, cfsm_test_fsm_init) {
    auto c = std::make_unique<cfsm_state>();
    cfsm_init(c.get(), 0, nullptr, nullptr);

    ASSERT_EQ(0, c->num_states);
    ASSERT_EQ(nullptr, c->states);
    ASSERT_EQ(nullptr, c->current_state);
}

TEST(cfsm_test_init, cfsm_test_fsm_init_value_check) {
    auto states = (void *)12;
    auto initial_state = (void *)44;

    cfsm_state c;
    cfsm_init(&c, 5, static_cast<cfsm_state *>(states), static_cast<cfsm_state *>(initial_state));

    ASSERT_TRUE(5 == c.num_states);
    ASSERT_TRUE(states == c.states);
    ASSERT_TRUE(initial_state == c.initial_state);
    ASSERT_TRUE(nullptr == c.current_state);
}

TEST(cfsm_test_init, cfsm_test_state_init_static) {
    const char *name = "hakuna-matata";
    cfsm_state s;
    cfsm_init_state(&s, name);

    ASSERT_TRUE(0 == s.num_transitions);
    ASSERT_TRUE(nullptr == s.transitions);
    ASSERT_TRUE(name == s.name);
}

TEST(cfsm_test_init, cfsm_test_state_init_dynamic) {
    const char *name = "hakuna-matata";
    struct cfsm_state *s = cfsm_init_state(new struct cfsm_state(), name);

    ASSERT_TRUE(0 == s->num_transitions);
    ASSERT_TRUE(nullptr == s->transitions);
    ASSERT_TRUE(name == s->name);

    delete s;
}

TEST(cfsm_test_init, cfsm_test_init_transition) {
    struct cfsm_state states[2];

    struct cfsm_transition *t = cfsm_init_transition(new struct cfsm_transition, &states[0], &states[1], 1);

    ASSERT_TRUE(&states[0] == t->source);
    ASSERT_TRUE(&states[1] == t->target);
    ASSERT_TRUE(1 == t->event_id);
    ASSERT_TRUE(cfsm_null_action == t->action);
    ASSERT_TRUE(cfsm_null_guard == t->guard);

    auto action = (cfsm_action_f) 10;
    auto guard = (cfsm_guard_f) 12;

    cfsm_transition_set_action(t, action);
    cfsm_transition_set_guard(t, guard);

    ASSERT_TRUE(action == t->action);
    ASSERT_TRUE(guard == t->guard);

    delete t;
}

TEST(cfsm_test_init, cfsm_test_add_single_transition_increment_transition_counter_for_state) {
    struct cfsm_state s;
    cfsm_init_state(&s, "hakuna-matata");

    struct cfsm_state c;
    cfsm_init(&c, 1, &s, &s);

    ASSERT_TRUE(1 == c.num_states && "state count should be equal to 1");

    struct cfsm_transition t;
    cfsm_init_transition(&t, &s, &s, 1);
    cfsm_add_transition(&c, &t);
    ASSERT_TRUE((1 == s.num_transitions) && "transition count should be equal to 1 for source state");

    ASSERT_TRUE(cfsm_transition_is_internal(&t)) << "transition with source == next is internal transition";
}

TEST(cfsm_test_init, cfsm_test_add_transition_only_to_stopped_cfsm) {
    struct cfsm_state states[2];
    cfsm_init_state(&states[0], "hakuna");
    cfsm_init_state(&states[1], "matata");

    struct cfsm_transition *t = cfsm_init_transition(new struct cfsm_transition, &states[0], &states[1], 1);
    struct cfsm_transition *u = cfsm_init_transition(new struct cfsm_transition, &states[0], &states[1], 74);

    struct cfsm_state c;
    cfsm_init(&c, 2, states, &states[0]);
    cfsm_add_transition(&c, t);

    c.current_state = (struct cfsm_state *)55; /// simulate already started fsm

    cfsm_add_transition(&c, u);

    ASSERT_TRUE((1 == states[0].num_transitions)) << "transition count should be equal to 1 for source state";
    ASSERT_TRUE((0 == states[1].num_transitions)) << "transition count should be equal to 0 for target state";

    delete t;
    delete u;
}

TEST(cfsm_test_init, cfsm_test_stopped_cfsm_can_be_destroyed) {
    struct cfsm_state states[2];
    cfsm_init_state(&states[0], "hakuna");
    cfsm_init_state(&states[1], "matata");

    struct cfsm_transition *t = cfsm_init_transition(new struct cfsm_transition, &states[0], &states[1], 1);
    struct cfsm_transition *u = cfsm_init_transition(new struct cfsm_transition, &states[1], &states[0], 2);

    struct cfsm_state c;
    cfsm_init_state(&c, "FSM");
    cfsm_init(&c, 2, states, &states[0]);

    cfsm_add_transition(&c, t);
    cfsm_add_transition(&c, u);

    cfsm_state_destroy(&c);

    ASSERT_TRUE(0 == c.num_transitions) << "no accessible transitions after destroy";
    ASSERT_TRUE(nullptr == c.transitions) << "zero transition count after destroy";

    delete t;
    delete u;
}
