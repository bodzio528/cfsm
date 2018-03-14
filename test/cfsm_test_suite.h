/**
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#ifndef CFSM_TEST_SUITE_H_
#define CFSM_TEST_SUITE_H_

#include "cfsm_test/cfsm_test_runner.h"

void cfsm_test_suite_init(void);
void cfsm_test_suite_processing(void);
void cfsm_test_suite_state_actions(void);
void cfsm_test_suite_submachines(void);

#endif /* CFSM_TEST_SUITE_H_ */
