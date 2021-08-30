#ifndef SABI_EC_H
#define SABI_EC_H

#include <sabi/global.h>
#include <stdint.h>

enum {
	SABI_EC_STATUS_OBF   = (1 << 0),
	SABI_EC_STATUS_IBF   = (1 << 1),
	SABI_EC_STATUS_BURST = (1 << 4),
	SABI_EC_STATUS_SCI   = (1 << 5),
	SABI_EC_STATUS_SMI   = (1 << 6),
};

enum {
	SABI_EC_READ          = 0x80,
	SABI_EC_WRITE         = 0x81,
	SABI_EC_BURST_ENABLE  = 0x82,
	SABI_EC_BURST_DISABLE = 0x83,
	SABI_EC_QUERY         = 0x84,
};

typedef struct sabi_ec {
	int space; // 0 = mem, 1 = io
	uint32_t base;
	uint32_t data;
	sabi_node_t *parent;
	struct sabi_ec *next;
} sabi_ec_t;

void sabi_ec_read(sabi_ec_t*, uint8_t, uint8_t*);
void sabi_ec_write(sabi_ec_t*, uint8_t, uint8_t);

sabi_ec_t *sabi_ec_resolve(sabi_node_t*);
void sabi_ec_call_reg(void);

#endif
