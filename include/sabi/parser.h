#ifndef SABI_PARSER_H
#define SABI_PARSER_H

#include <sabi/global.h>
#include <sabi/state.h>
#include <stdint.h>

int sabi_pop_opcode(state_t*);
void sabi_push_opcode(state_t*, int);

int sabi_parse_method(state_t*);
int sabi_parse_math(state_t*, uint64_t*);
void sabi_parse_next(state_t*);

void sabi_parse_termlist(state_t*, uint8_t*);

#endif
