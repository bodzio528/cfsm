/**
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include <cfsm/cfsm.h>

#include "cfsm_test_suite.h"

void cfsm_test_submachines_21(void) {
    CFSM_TEST_ASSERT(true);
}


void cfsm_test_suite_submachines(void) {
    CFSM_TEST_ADD(cfsm_test_submachines_21);
}