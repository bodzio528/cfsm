#ifndef CFSM_CFSM_TESTRUNNER_H
#define CFSM_CFSM_TESTRUNNER_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define nullptr NULL

struct cfsm_test_node {
    const char *name;
    struct cfsm_test_node *next;

    void (*testcase)(void);
};

struct cfsm_test_node *testbench = nullptr;

void cfsm_test_add(const char *name, void(*testcase)(void));

void cfsm_test_run_all();

void cfsm_test_destroy(struct cfsm_test_node *root);

void cfsm_test_run_single_case(const struct cfsm_test_node *ctest);

void cfsm_test_add(const char *name, void (*testcase)(void)) {
    struct cfsm_test_node *new_test = malloc(sizeof(struct cfsm_test_node));
    new_test->name = name;
    new_test->next = testbench;
    new_test->testcase = testcase;

    testbench = new_test;
}

void cfsm_test_run_all() {
    // TODO: implement test shuffle!
    struct cfsm_test_node *root = testbench;
    while (nullptr != root) {
        cfsm_test_run_single_case(root);
        root = root->next;
    }
    printf("%s\n", "[PASSED]");
}

void cfsm_test_run_single_case(const struct cfsm_test_node *ctest) {
    printf("run %s: ", ctest->name);
    ctest->testcase();
    printf("%s\n", "ok");
}

void cfsm_test_destroy(struct cfsm_test_node *root) {
    if (nullptr != root->next)
        cfsm_test_destroy(root->next);
    free(root);
}

#endif //CFSM_CFSM_TESTRUNNER_H
