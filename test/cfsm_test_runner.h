/**
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#ifndef CFSM_CFSM_TESTRUNNER_H
#define CFSM_CFSM_TESTRUNNER_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define cfsm_test_log(location, fmt, ...) do { fprintf(stdout, "[UT] %s: " fmt "\n", (location), __VA_ARGS__); fflush(stderr); } while (0)

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
    cfsm_test_log(ctest->name, "%s", " start");
    ctest->testcase();
    cfsm_test_log(ctest->name, "%s", " passed!");
    cfsm_test_log("", "%s", "");
}

void cfsm_test_destroy(struct cfsm_test_node *root) {
    if (nullptr != root->next)
        cfsm_test_destroy(root->next);
    free(root);
}

#define CFSM_TEST_ADD(testfunc) cfsm_test_add(#testfunc, testfunc)

#endif //CFSM_CFSM_TESTRUNNER_H
