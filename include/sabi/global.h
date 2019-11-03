#ifndef SABI_GLOBAL_H
#define SABI_GLOBAL_H

#include <stdint.h>

enum {
	SABI_OBJECT_NULL         = 0x0001,
	SABI_OBJECT_ALIAS        = 0x0002,
	SABI_OBJECT_EVENT        = 0x0003,
	SABI_OBJECT_FIELD        = 0x0004,
	SABI_OBJECT_INDEX_FIELD  = 0x0005,
	SABI_OBJECT_BANK_FIELD   = 0x0006,
	SABI_OBJECT_METHOD       = 0x0007,
	SABI_OBJECT_MUTEX        = 0x0008,
	SABI_OBJECT_NAME         = 0x0009,
	SABI_OBJECT_OP_REGION    = 0x000A,
	SABI_OBJECT_DEVICE       = 0x00B0,
	SABI_OBJECT_POWER_RES    = 0x00C0,
	SABI_OBJECT_PROCESSOR    = 0x00D0,
	SABI_OBJECT_THERMAL_ZONE = 0x00E0,
	SABI_OBJECT_SCOPE        = 0x00F0,
};

enum {
	SABI_DATA_NULL         = 0x0100,
	SABI_DATA_BUFFER       = 0x0200,
	SABI_DATA_BUFFER_FIELD = 0x0300,
	SABI_DATA_PACKAGE      = 0x0400,
	SABI_DATA_STRING       = 0x0500,
	SABI_DATA_INTEGER      = 0x0600,
	SABI_DATA_NAME         = 0x0700,
	SABI_DATA_REFERENCE    = 0x0800,
};

enum {
	SABI_ERROR_NONE   = 0, // No error
	SABI_ERROR_LOOKUP = 1, // Namespace lookup failed
	SABI_ERROR_TYPE   = 2, // Type mismatch
	SABI_ERROR_RANGE  = 3, // Out of range
	SABI_ERROR_EXEC   = 4, // Execution failed
	SABI_ERROR_MEMORY = 5, // Memory allocation failed
	SABI_ERROR_ARGS   = 6, // Invalid arguments
};

typedef struct sabi_node sabi_node_t;
typedef union sabi_data sabi_data_t;
typedef union sabi_object sabi_object_t;

typedef struct {
	char prefix[16];
	char value[64];
} sabi_name_t;

union sabi_data {
	uint16_t type;
	struct {
		uint16_t type;
		uint32_t size;
		uint8_t *ptr;
	} buffer;
	struct {
		uint16_t type;
		uint32_t size;
		uint8_t offset;
		uint8_t *ptr;
	} field;
	struct {
		uint16_t type;
		uint64_t value;
	} integer;
	struct {
		uint16_t type;
		char *value;
	} string;
	struct {
		uint16_t type;
		sabi_name_t path;
	} name;
	struct {
		uint16_t type;
		uint32_t count;
		sabi_data_t *data;
	} package;
	struct {
		uint16_t type;
		void *refof;
		int index;
		int flags;
	} reference;
};

union sabi_object {
	uint16_t type;
	struct {
		uint16_t type;
		sabi_node_t *node;
	} alias;
	struct {
		uint16_t type;
		void *adr;
	} device;
	struct {
		uint16_t type;
		uint8_t *start;
		uint8_t *end;
		uint8_t flags;
		uint8_t argc;
	} method;
	struct {
		uint16_t type;
		uint8_t space;
		uint64_t offset;
		uint64_t length;
		void *adr;
	} region;
	struct {
		uint16_t type;
		uint8_t system_level;
		uint32_t resource_order;
	} power;
	struct {
		uint16_t type;
		uint8_t proc_id;
		uint32_t pblk_addr;
		uint8_t pblk_len;
	} cpu;
	struct {
		uint16_t type;
		uint8_t syncflags;
	} mutex;
	struct {
		uint16_t type;
		sabi_data_t *data;
	} name;
	struct {
		uint16_t type;
		uint8_t flags;
		uint8_t attrib;
		uint32_t offset;
		uint32_t length;
		sabi_object_t *parent;
	} field;
	struct {
		uint16_t type;
		uint8_t flags;
		uint8_t attrib;
		uint32_t offset;
		uint32_t length;
		sabi_object_t *index;
		sabi_object_t *data;
	} indexfield;
	struct {
		uint16_t type;
		uint8_t flags;
		uint8_t attrib;
		uint32_t offset;
		uint32_t length;
		uint64_t value;
		sabi_object_t *region;
		sabi_object_t *bank;
	} bankfield;
};

struct sabi_node {
	uint32_t name;
	sabi_node_t *next;
	sabi_node_t *child;
	sabi_node_t *parent;
	sabi_node_t *stack;
	sabi_object_t object;
};

#endif
