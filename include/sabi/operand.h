#ifndef SABI_OPERAND_H
#define SABI_OPERAND_H

#include <sabi/global.h>
#include <sabi/state.h>

enum {
	OPERAND_NULL   = 0x00,
	OPERAND_DEBUG  = 0x01,
	OPERAND_OBJECT = 0x02,
	OPERAND_DATA   = 0x04,
	OPERAND_INDEX  = 0x10,
	OPERAND_LOCAL  = 0x20,
	OPERAND_ARG    = 0x40,
};

typedef struct {
	int flags;
	void *refof;
	sabi_data_t data;
} operand_t;

void sabi_def_index(state_t*, operand_t*);
void sabi_def_derefof(state_t*, operand_t*);
void sabi_def_refof(state_t*, operand_t*);

int sabi_parse_operand(state_t*, operand_t*);
void sabi_read_operand(operand_t*, sabi_data_t*);
void sabi_write_operand(operand_t*, sabi_data_t*);

#endif
