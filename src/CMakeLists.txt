# Licensed under the MIT License. See LICENSE file in the project root for full license information.

set(CFSM_HEADERS
        ../include/cfsm/cfsm.h
        ../include/cfsm/cfsm_nullptr.h)

add_library(cfsm cfsm.c ${CFSM_HEADERS})
set_target_properties(cfsm PROPERTIES LINKER_LANGUAGE C)