/**
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#ifndef CFSM_TEST_RUNNER_H_
#define CFSM_TEST_RUNNER_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "cfsm/cfsm_nullptr.h"

#ifndef CFSM_TEST_SOURCE_PATH_LENGTH
#define CFSM_TEST_SOURCE_PATH_LENGTH 0
#endif
#define __FILENAME__ (__FILE__ + CFSM_TEST_SOURCE_PATH_LENGTH)

#define CFSM_TEST_LOG_FP(fp, fmt, ...) do { fprintf(fp, "[UT] %s:%d: " fmt "\n", __FILENAME__, __LINE__, __VA_ARGS__); fflush(fp); } while (0)
#define CFSM_TEST_LOG(fmt, ...) do { printf("[UT] %s:%d: " fmt "\n", __FILENAME__, __LINE__, __VA_ARGS__); fflush(stdout); } while (0)

enum cfsm_test_result {
    cfsm_test_result_failure = 0,
    cfsm_test_result_success = 1,
    cfsm_test_result_no_run = 2
};

struct cfsm_test_node {
    const char *name;
    struct cfsm_test_node *next;

    void (*testcase)(void);
    enum cfsm_test_result result;
    const char * location;
    int line;
};

struct cfsm_test_node *cfsm_test_g_testbench;
enum cfsm_test_result cfsm_test_g_result;
const char *cfsm_test_g_runner_name;
unsigned cfsm_test_g_seed;

void cfsm_test_init(const char * name, unsigned tseed);
void cfsm_test_add(const char *name, void(*testcase)(void), const char * location, int line);
int cfsm_test_run_all();

#define CFSM_TEST_ADD(testfunc) cfsm_test_add(#testfunc, testfunc, __FILENAME__, __LINE__)

#define CFSM_TEST_ASSERT_MSG(condition, message) if (! (condition)) { CFSM_TEST_LOG_FP(stderr, "condition failed: %s", #message); cfsm_test_g_result = cfsm_test_result_failure; }
#define CFSM_TEST_ASSERT(condition) CFSM_TEST_ASSERT_MSG(condition, #condition)

#endif /* CFSM_TEST_RUNNER_H_ */
