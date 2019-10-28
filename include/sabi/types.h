#ifndef SABI_TYPES_H
#define SABI_TYPES_H

#include <sabi/global.h>
#include <sabi/state.h>

typedef struct {
	uint32_t length;
	uint8_t *end;
} pkg_t;

typedef struct {
	sabi_name_t name;
	uint8_t flags;
	uint8_t attrib;
	uint32_t offset;
	uint32_t length;
	uint8_t with_name;
} field_t;

void sabi_name_string(state_t*, sabi_name_t*);
sabi_node_t *sabi_parse_name(state_t*);

uint8_t sabi_byte(state_t*);
uint16_t sabi_word(state_t*);
uint32_t sabi_dword(state_t*);
uint64_t sabi_qword(state_t*);
uint64_t sabi_integer(state_t*);

void sabi_field_element(state_t*, field_t*);
void sabi_pkg_length(state_t*, pkg_t*);
void sabi_data_object(state_t*, sabi_data_t*);

#endif
