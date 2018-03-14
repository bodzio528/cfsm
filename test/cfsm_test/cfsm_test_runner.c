/**
 * Licensed under the MIT License. See LICENSE file in the project head for full license information.
 */

#include "cfsm_test_runner.h"
#include <time.h>

struct cfsm_test_node *cfsm_test_g_testbench = nullptr;
enum cfsm_test_result cfsm_test_g_result = cfsm_test_result_no_run;
const char *cfsm_test_g_runner_name = nullptr;
unsigned cfsm_test_g_seed = 0;

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

static struct cfsm_counters {
    int success_count;
    int failed_count;
};

inline static struct cfsm_test_node *cfsm_test_i_shuffle_testbed(struct cfsm_test_node *head);
inline static struct cfsm_test_node *cfsm_test_i_run_testbed(struct cfsm_test_node *head);
inline static struct cfsm_test_node *cfsm_test_i_get_failed_list(struct cfsm_test_node *head, struct cfsm_counters *counters);
inline static struct cfsm_test_node *cfsm_test_i_remove_head(struct cfsm_test_node *head);
inline static void cfsm_test_i_print_stat_header(struct cfsm_counters *counters);
inline static bool cfsm_test_i_rnd_choice(void);
inline static void cfsm_test_i_run_single_case(struct cfsm_test_node *node);

int cfsm_test_run_all() {
    srand(cfsm_test_g_seed);
    printf("[START] %s seed = %d\n", cfsm_test_g_runner_name, cfsm_test_g_seed);

    struct cfsm_counters counters = {.failed_count = 0, .success_count = 0};

    struct cfsm_test_node *failed = cfsm_test_i_get_failed_list(cfsm_test_i_run_testbed(cfsm_test_i_shuffle_testbed(cfsm_test_g_testbench)), &counters);

    cfsm_test_i_print_stat_header(&counters);
    if (counters.failed_count) {
        while (nullptr != failed) {
            fprintf(stderr, "failure! %s %s:%d\n", failed->name, failed->location, failed->line);
            failed = cfsm_test_i_remove_head(failed);
        }
    }

    return counters.failed_count;
}


struct cfsm_test_node *cfsm_test_i_shuffle_testbed(struct cfsm_test_node *head) {
    // PHASE 1: random split
    struct cfsm_test_node *left = nullptr;
    struct cfsm_test_node *right = nullptr;

    struct cfsm_test_node *ctest = head;
    while (nullptr != ctest) {
        struct cfsm_test_node * next = ctest->next;
        if (cfsm_test_i_rnd_choice()) {
            ctest->next = left;
            left = ctest;
        } else {
            ctest->next = right;
            right = ctest;
        }
        ctest = next;
    }

    // PHASE 2: random swap and merge
    struct cfsm_test_node *source = right;
    struct cfsm_test_node *target = left;

    if (cfsm_test_i_rnd_choice()) {
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


struct cfsm_test_node *cfsm_test_i_run_testbed(struct cfsm_test_node *head) {
    struct cfsm_test_node *ctest = head;
    while (nullptr != ctest) {
        cfsm_test_i_run_single_case(ctest);
        ctest = ctest->next;
    }
    return head;
}


struct cfsm_test_node *cfsm_test_i_get_failed_list(struct cfsm_test_node *ctest, struct cfsm_counters *counters) {
    struct cfsm_test_node * failed = nullptr;

    while (nullptr != ctest) {
        if (cfsm_test_result_success == ctest->result) {
            ++counters->success_count;
            ctest = cfsm_test_i_remove_head(ctest);
        } else {
            ++counters->failed_count;
            struct cfsm_test_node *next = ctest->next;
            ctest->next = failed;
            failed = ctest;
            ctest = next;
        }
    }

    return failed;
}

struct cfsm_test_node *cfsm_test_i_remove_head(struct cfsm_test_node *target) {
    if (nullptr == target) {
        return target;
    }

    struct cfsm_test_node *next = target->next;
    free(target);
    return next;
}

void cfsm_test_i_print_stat_header(struct cfsm_counters *counters) {
    printf("[FINISHED] TEST RUN = %d OK = %d FAILED = %d seed = %d\n",
           counters->success_count + counters->failed_count,
           counters->success_count,
           counters->failed_count,
           cfsm_test_g_seed);
}

bool cfsm_test_i_rnd_choice(void) {
    return (rand() & 1u) == true;
}

void cfsm_test_i_run_single_case(struct cfsm_test_node *ctest) {
    cfsm_test_g_result = cfsm_test_result_success;
    printf("%s\n", ctest->name);
    ctest->testcase();
    ctest->result = cfsm_test_g_result;
    printf("%s %s\n", ctest->name, (cfsm_test_g_result ? "ok" : "failed"));
}