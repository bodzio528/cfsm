/**
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "cfsm_test_runner.h"
#include <stdlib.h>
#include <time.h>

struct cfsm_test_node *cfsm_test_g_testbench = nullptr;
enum cfsm_test_result cfsm_test_g_result = cfsm_test_result_no_run;
const char *cfsm_test_g_runner_name = nullptr;

void cfsm_test_init(const char * name, unsigned tseed) {
    cfsm_test_g_runner_name = name;
    cfsm_test_g_testbench = nullptr;
    cfsm_test_g_seed = tseed;
}

void cfsm_test_add(const char *name, void (*testcase)(void), const char * location, int line) {
    struct cfsm_test_node *new_test = malloc(sizeof(struct cfsm_test_node));
    new_test->name = name;
    new_test->next = cfsm_test_g_testbench;
    new_test->testcase = testcase;
    new_test->result = cfsm_test_result_success;
    new_test->location = location;
    new_test->line = line;

    cfsm_test_g_testbench = new_test;
}

struct cfsm_test_node *cfsm_test_remove(struct cfsm_test_node *target) {
    if (nullptr == target) {
        return target;
    }

    struct cfsm_test_node *next = target->next;
    free(target);
    return next;
}

inline static bool rnd_choice(void) {
    return (rand() & 1u) == true;
}

inline static struct cfsm_test_node *shuffle(struct cfsm_test_node *root) {
    struct cfsm_test_node *left = nullptr;
    struct cfsm_test_node *right = nullptr;

    struct cfsm_test_node *ctest = root;
    while (nullptr != ctest) {
        struct cfsm_test_node * next = ctest->next;
        if (rnd_choice()) {
            ctest->next = left;
            left = ctest;
        } else {
            ctest->next = right;
            right = ctest;
        }
        ctest = next;
    }

    struct cfsm_test_node *source = right;
    struct cfsm_test_node *target = left;

    if (rnd_choice()) {
        struct cfsm_test_node *tmp = target;
        target = source;
        source = tmp;
    }

    while (nullptr != source) {
        struct cfsm_test_node * next = source->next;
        source->next = target;
        target = source;
        source = next;
    }

    return target;
}

inline static void cfsm_test_run_single_case(struct cfsm_test_node *ctest) {
    cfsm_test_g_result = cfsm_test_result_success;
    printf("%s\n", ctest->name);
    ctest->testcase();
    ctest->result = cfsm_test_g_result;
    printf("%s %s\n", ctest->name, (cfsm_test_g_result ? "ok" : "failed"));
}

inline static struct cfsm_test_node *run_testbench(struct cfsm_test_node *root) {
    struct cfsm_test_node *ctest = root;
    while (nullptr != ctest) {
        cfsm_test_run_single_case(ctest);
        ctest = ctest->next;
    }
    return root;
}

int cfsm_test_run_all() {
    srand(cfsm_test_g_seed);
    printf("[START] %s seed = %d\n", cfsm_test_g_runner_name, cfsm_test_g_seed);

    int success_count = 0;
    int failed_count = 0;
    struct cfsm_test_node * failed = nullptr;

    struct cfsm_test_node * ctest = run_testbench(shuffle(cfsm_test_g_testbench));
    while (nullptr != ctest) {
        if (cfsm_test_result_success == ctest->result) {
            ++success_count;
            ctest = cfsm_test_remove(ctest);
        } else {
            ++failed_count;
            struct cfsm_test_node *next = ctest->next;
            ctest->next = failed;
            failed = ctest;
            ctest = next;
        }
    }

    printf("[FINISHED] TEST RUN = %d OK = %d FAILED = %d seed = %d\n", success_count + failed_count, success_count, failed_count, cfsm_test_g_seed);
    if (failed_count) {
        while (nullptr != failed) {
            fprintf(stderr, "failure! %s %s:%d\n", failed->name, failed->location, failed->line);
            failed = cfsm_test_remove(failed);
        }
    }

    return failed_count;
}