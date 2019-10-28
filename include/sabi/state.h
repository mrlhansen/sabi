#ifndef SABI_STATE_H
#define SABI_STATE_H

#include <sabi/global.h>
#include <stdint.h>

enum {
	INTR_ERROR     = 1,
	INTR_RETURN    = 2,
	INTR_BREAK     = 3,
	INTR_CONTINUE  = 4
};

typedef struct {
	sabi_node_t *scope;
	sabi_node_t *stack;
	sabi_data_t *argv;
	sabi_data_t *local;
	sabi_data_t *retv;
	uint8_t *aml;
	int intr;
	int argc;
	int count;
} state_t;

state_t *sabi_new_state();
void sabi_free_state(state_t*);

#endif
