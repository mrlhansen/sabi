#ifndef SABI_RESOURCES_H
#define SABI_RESOURCES_H

#include <sabi/global.h>
#include <stdint.h>

typedef struct {
	uint8_t name; // 0x04 = IRQ Descriptor
	uint16_t mask;
	uint8_t flags;
} __attribute__((packed)) res04_t;

typedef struct {
	uint8_t name; // 0x08 = I/O Port Descriptor
	uint8_t info;
	uint16_t min;
	uint16_t max;
	uint8_t aln;
	uint8_t len;
} __attribute__((packed)) res08_t;

typedef struct {
	uint8_t name; // 0x81 = 24-Bit Memory Range Descriptor
	uint16_t length;
	uint8_t info;
	uint16_t min;
	uint16_t max;
	uint16_t aln;
	uint16_t len;
} __attribute__((packed)) res81_t;

typedef struct {
	uint8_t name; // 0x82 = Generic Register Descriptor
	uint16_t length;
	uint8_t asi;
	uint8_t rbw;
	uint8_t rbo;
	uint8_t asz;
	uint64_t adr;
} __attribute__((packed)) res82_t;

typedef struct {
	uint8_t name; // 0x84 = Vendor-Defined Descriptor
	uint16_t length;
	uint8_t type;
	uint8_t uuid[16];
	uint8_t data[0];
} __attribute__((packed)) res84_t;

typedef struct {
	uint8_t name; // 0x85 = 32-Bit Memory Range Descriptor
	uint16_t length;
	uint8_t info;
	uint32_t min;
	uint32_t max;
	uint32_t aln;
	uint32_t len;
} __attribute__((packed)) res85_t;

typedef struct {
	uint8_t name; // 0x86 = 32-Bit Fixed Memory Range Descriptor
	uint16_t length;
	uint8_t info;
	uint32_t bas;
	uint32_t len;
} __attribute__((packed)) res86_t;

typedef struct {
	uint8_t name; // 0x87 - DWord Address Space Descriptor
	uint16_t length;
	uint8_t type;
	uint8_t gflags;
	uint8_t sflags;
	uint32_t gra;
	uint32_t min;
	uint32_t max;
	uint32_t tra;
	uint32_t len;
} __attribute__((packed)) res87_t;

typedef struct {
	uint8_t name; // 0x88 - Word Address Space Descriptor
	uint16_t length;
	uint8_t type;
	uint8_t gflags;
	uint8_t sflags;
	uint16_t gra;
	uint16_t min;
	uint16_t max;
	uint16_t tra;
	uint16_t len;
} __attribute__((packed)) res88_t;

typedef struct {
	uint8_t name; // 0x89 - Extended Interrupt Descriptor
	uint16_t length;
	uint8_t flags;
	uint8_t count;
	uint32_t intr[0];
} __attribute__((packed)) res89_t;

typedef struct {
	uint8_t name; // 0x8A - QWord Address Space Descriptor
	uint16_t length;
	uint8_t type;
	uint8_t gflags;
	uint8_t sflags;
	uint64_t gra;
	uint64_t min;
	uint64_t max;
	uint64_t tra;
	uint64_t len;
} __attribute__((packed)) res8A_t;

typedef struct {
	uint8_t name; // 0x8B - Extended Address Space Descriptor
	uint16_t length;
	uint8_t type;
	uint8_t gflags;
	uint8_t sflags;
	uint16_t revision;
	uint64_t gra;
	uint64_t min;
	uint64_t max;
	uint64_t tra;
	uint64_t len;
	uint64_t att;
} __attribute__((packed)) res8B_t;

typedef struct {
	uint8_t *buf;
	uint8_t *pos;
	int length;
	int skip;
} sabi_resource_t;

typedef struct {
	int length;
	int type;
	union {
		void *data;
		res04_t *res04;
		res08_t *res08;
		res81_t *res81;
		res82_t *res82;
		res84_t *res84;
		res85_t *res85;
		res86_t *res86;
		res87_t *res87;
		res88_t *res88;
		res89_t *res89;
		res8A_t *res8A;
		res8B_t *res8B;
	} u;
} sabi_descriptor_t;

int sabi_next_resource(sabi_resource_t *res, sabi_descriptor_t *desc);
int sabi_eval_resource(sabi_node_t *parent, sabi_resource_t *res);
void sabi_free_resource(sabi_resource_t *res);

#endif
