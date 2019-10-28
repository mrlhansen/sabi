#ifndef SABI_FIELDS_H
#define SABI_FIELDS_H

#include <sabi/global.h>
#include <stdint.h>

typedef struct {
	uint32_t address;
	uint8_t offset;
	uint8_t size;
	uint8_t count;
	uint8_t read;
	uint8_t partial;
	uint8_t update;
} align_t;

void sabi_access_field(sabi_object_t*, uint64_t*, int);
void sabi_access_index_field(sabi_object_t*, uint64_t*, int);
void sabi_access_buffer_field(sabi_data_t*, uint64_t*, int);

#endif
