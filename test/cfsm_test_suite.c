/**
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "cfsm_test_suite.h"

#include <time.h>

inline static unsigned time_seed(void) {
    time_t t;
    return (unsigned) time(&t);
}

int main(int argc, char *argv[]) {
    unsigned tseed = time_seed();
    if (argc > 1) {
        char* ptr;
        tseed = strtoul(argv[1], &ptr, 10);
    }

    cfsm_test_init(__FILENAME__, tseed);

    cfsm_test_suite_init();
    cfsm_test_suite_processing();
    cfsm_test_suite_state_actions();
    cfsm_test_suite_submachines();

    return cfsm_test_run_all();
}
