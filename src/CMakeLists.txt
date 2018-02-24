set(CFSM_HEADERS
        ../include/cfsm/cfsm.h
        )

add_library(cfsm cfsm.c ${CFSM_HEADERS})