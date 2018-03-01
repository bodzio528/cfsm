/**
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#ifndef CFSM_CFSM_TESTRUNNER_H
#define CFSM_CFSM_TESTRUNNER_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#ifndef CFSM_TEST_SOURCE_PATH_LENGTH
#define CFSM_TEST_SOURCE_PATH_LENGTH 0
#endif
#define __FILENAME__ (__FILE__ + CFSM_TEST_SOURCE_PATH_LENGTH)

#define CFSM_TEST_LOG_FP(fp, fmt, ...) do { fprintf(fp, "[UT] %s:%d: " fmt "\n", __FILENAME__, __LINE__, __VA_ARGS__); fflush(fp); } while (0)
#define CFSM_TEST_LOG(fmt, ...) do { printf("[UT] %s:%d: " fmt "\n", __FILENAME__, __LINE__, __VA_ARGS__); fflush(stdout); } while (0)

#define CFSM_TEST_SUCCESS 1
#define CFSM_TEST_FAILURE 0

struct cfsm_test_node {
    const char *name;
    struct cfsm_test_node *next;

    void (*testcase)(void);
    int result;
};

struct cfsm_test_node *testbench = nullptr;
const char *testbench_name = nullptr;
int testresult = CFSM_TEST_SUCCESS;

void cfsm_test_add(const char *name, void(*testcase)(void));
void cfsm_test_run_all();
int cfsm_test_destroy();
void cfsm_test_run_single_case(struct cfsm_test_node *ctest);

#define CFSM_TEST_ADD(testfunc) cfsm_test_add(#testfunc, testfunc)

#define CFSM_TEST_ASSERT_MSG(condition, message) if (! (condition)) { CFSM_TEST_LOG_FP(stderr, "condition failed: %s", #message); testresult = CFSM_TEST_FAILURE; }
#define CFSM_TEST_ASSERT(condition) CFSM_TEST_ASSERT_MSG(condition, #condition)

void cfsm_test_init(const char * name) {
    testbench_name = name;
    testbench = nullptr;
}

void cfsm_test_add(const char *name, void (*testcase)(void)) {
    struct cfsm_test_node *new_test = malloc(sizeof(struct cfsm_test_node));
    new_test->name = name;
    new_test->next = testbench;
    new_test->testcase = testcase;

    testbench = new_test;
}

void cfsm_test_run_all() {
    printf("[START] %s\n", testbench_name);
    int testbench_result = CFSM_TEST_SUCCESS;
    // TODO: implement test shuffle!
    struct cfsm_test_node *root = testbench;
    while (nullptr != root) {
        cfsm_test_run_single_case(root);
        if (root->result != CFSM_TEST_SUCCESS) {
            testbench_result = CFSM_TEST_FAILURE;
        }
        root = root->next;
    }
    printf("[%s] %s\n",(CFSM_TEST_SUCCESS == testbench_result ? "PASSED" : "FAILED") , testbench_name);
}

void cfsm_test_run_single_case(struct cfsm_test_node *ctest) {
    testresult = CFSM_TEST_SUCCESS;
    printf("%s\n", ctest->name);
    ctest->testcase();
    ctest->result = testresult;
    printf("%s %s\n", ctest->name, (testresult ? "passed!" : "failed!"));
}

int cfsm_test_destroy(void) {
    bool result = true;

    struct cfsm_test_node * current = testbench;
    while (nullptr != current->next) {
        struct cfsm_test_node * next = current->next;
        result = result && (current->result == CFSM_TEST_SUCCESS);
        free(current);
        current = next;
    }

    return !result;
}

#endif /* CFSM_CFSM_TESTRUNNER_H */
