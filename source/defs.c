#include <sabi/namespace.h>
#include <sabi/operand.h>
#include <sabi/parser.h>
#include <sabi/types.h>
#include <sabi/defs.h>
#include <sabi/host.h>
#include <sabi/conv.h>
#include <sabi/api.h>
#include <sabi/ec.h>
#include <string.h>

void sabi_def_acquire(state_t *state, uint64_t *value)
{
	sabi_name_t name;
	sabi_name_string(state, &name);
	sabi_word(state);
	*value = 0;
}

void sabi_def_alias(state_t *state)
{
	sabi_name_t name, alias;
	sabi_object_t object;

	// Parse arguments
	sabi_name_string(state, &name);
	sabi_name_string(state, &alias);

	// Construct object
	object.alias.type = SABI_OBJECT_ALIAS;
	object.alias.node = sabi_ns_find(state->scope, &name);

	// Append to namespace
	sabi_ns_add_object(state, &alias, &object);
}

void sabi_def_bank_field(state_t *state)
{
	sabi_node_t *rnode, *bnode;
	sabi_name_t region, bank;
	sabi_object_t object;
	uint64_t value;
	uint8_t flags;
	field_t field;
	pkg_t pkg;

	// Parse arguments
	sabi_pkg_length(state, &pkg);
	sabi_name_string(state, &region);
	sabi_name_string(state, &bank);
	value = sabi_integer(state);
	flags = sabi_byte(state);

	// Default values
	field.offset = 0;
	field.length = 0;
	field.flags = flags;
	field.attrib = 0;

	// Parent nodes
	rnode = sabi_ns_find(state->scope, &region);
	bnode = sabi_ns_find(state->scope, &bank);

	// Construct object
	object.bankfield.type = SABI_OBJECT_BANK_FIELD;
	object.bankfield.region = &(rnode->object);
	object.bankfield.bank = &(bnode->object);
	object.bankfield.value = value;

	// Iterate all elements
	while(state->aml < pkg.end)
	{
		sabi_field_element(state, &field);
		if(field.with_name)
		{
			object.bankfield.flags = field.flags;
			object.bankfield.attrib = field.attrib;
			object.bankfield.offset = field.offset;
			object.bankfield.length = field.length;
			sabi_ns_add_object(state, &field.name, &object);
		}
	}
}

void sabi_def_buffer(state_t *state, sabi_data_t *data)
{
	uint32_t size;
	uint8_t *ptr;
	pkg_t pkg;

	// Parse arguments
	sabi_pkg_length(state, &pkg);
	size = sabi_integer(state);

	// Allocate memory
	ptr = sabi_host_alloc(1, size);

	// Construct object
	data->buffer.type = SABI_DATA_BUFFER;
	data->buffer.size = size;
	data->buffer.ptr = ptr;

	// Fill buffer
	while(state->aml < pkg.end)
	{
		*ptr++ = sabi_byte(state);
	}
}

void sabi_def_condrefof(state_t *state, uint64_t *value)
{
	operand_t operand;
	sabi_node_t *node;
	sabi_data_t data;
	sabi_name_t name;
	int target;

	// Parse arguments
	sabi_name_string(state, &name);
	node = sabi_ns_find(state->scope, &name);

	if(*state->aml)
	{
		sabi_parse_operand(state, &operand);
		target = 1;
	}
	else
	{
		target = 0;
		state->aml++;
	}

	// Source status
	if(node)
	{
		*value = 1;
	}
	else
	{
		*value = 0;
		return;
	}

	// Store reference
	if(target)
	{
		data.reference.type = SABI_DATA_REFERENCE;
		data.reference.refof = &node->object;
		data.reference.flags = OPERAND_OBJECT;
		sabi_write_operand(&operand, &data);
	}
}

void sabi_def_convertbcd(state_t *state, uint64_t *value, int from)
{
	uint64_t x, z, p;

	// Parse argument (target is handled by caller)
	x = sabi_integer(state);
	z = 0;
	p = 1;

	// Convert BCD
	if(from)
	{
		while(x)
		{
			z = ((x % 16) * p) + z;
			p = (p * 10);
			x = (x / 16);
		}
	}
	else
	{
		while(x)
		{
			z = ((x % 10) * p) + z;
			p = (p * 16);
			x = (x / 10);
		}
	}

	// Store result
	*value = z;
}

void sabi_def_create_field(state_t *state, int bits, int length)
{
	sabi_data_t *data, parent;
	sabi_object_t object;
	operand_t operand;
	sabi_name_t name;
	uint64_t index;

	// Parse arguments
	sabi_parse_operand(state, &operand);
	index = sabi_integer(state);
	if(length == 0)
	{
		length = sabi_integer(state);
	}
	sabi_name_string(state, &name);

	// Type check
	sabi_read_operand(&operand, &parent);
	if(parent.type != SABI_DATA_BUFFER)
	{
		sabi_fatal("wrong data type %x", parent.type);
	}

	// Convert index to bits
	index = (bits * index);

	// Allocate data
	data = sabi_host_alloc(1, sizeof(sabi_data_t));

	// Construct field
	data->field.type = SABI_DATA_BUFFER_FIELD;
	data->field.offset = (index % 8);
	data->field.size = length;
	data->field.ptr = parent.buffer.ptr + (index / 8);

	// Construct object
	object.name.type = SABI_OBJECT_NAME;
	object.name.data = data;

	// Append to namespace
	sabi_ns_add_object(state, &name, &object);
}

void sabi_def_device(state_t *state)
{
	sabi_object_t object;
	sabi_node_t *scope;
	sabi_name_t name;
	pkg_t pkg;

	// Parse arguments
	sabi_pkg_length(state, &pkg);
	sabi_name_string(state, &name);

	// Construct object
	object.device.type = SABI_OBJECT_DEVICE;
	object.device.adr = 0;

	// Append to namespace
	scope = state->scope;
	sabi_ns_add_scope(state, &name, &object);
	sabi_parse_termlist(state, pkg.end);
	state->scope = scope;
}

void sabi_def_divide(state_t *state, uint64_t *value)
{
	uint64_t x, y, rs, rm;
	operand_t operand;
	sabi_data_t data;

	// Parse arguments
	x = sabi_integer(state);
	y = sabi_integer(state);

	// Calculate results
	rs = (x / y);
	rm = (x % y);
	*value = rs;

	// Store reminder
	if(*state->aml)
	{
		data.integer.type = SABI_DATA_INTEGER;
		data.integer.value = rm;
		sabi_parse_operand(state, &operand);
		sabi_write_operand(&operand, &data);
	}
	else
	{
		state->aml++;
	}

	// Store quotient
	if(*state->aml)
	{
		data.integer.type = SABI_DATA_INTEGER;
		data.integer.value = rs;
		sabi_parse_operand(state, &operand);
		sabi_write_operand(&operand, &data);
	}
	else
	{
		state->aml++;
	}
}

void sabi_def_event(state_t *state)
{
	sabi_object_t object;
	sabi_name_t name;

	// Parse arguments
	sabi_name_string(state, &name);

	// Construct object
	object.type = SABI_OBJECT_EVENT;

	// Append to namespace
	sabi_ns_add_object(state, &name, &object);
}

void sabi_def_external(state_t *state)
{
	sabi_name_t name;
	sabi_name_string(state, &name);
	sabi_word(state);
}

void sabi_def_field(state_t *state)
{
	sabi_object_t object;
	sabi_node_t *parent;
	sabi_name_t name;
	field_t field;
	uint8_t flags;
	pkg_t pkg;

	// Parse arguments
	sabi_pkg_length(state, &pkg);
	sabi_name_string(state, &name);
	flags = sabi_byte(state);

	// Default values
	field.offset = 0;
	field.length = 0;
	field.flags = flags;
	field.attrib = 0;

	// Parent region node
	parent = sabi_ns_find(state->scope, &name);

	// Construct object
	object.field.type = SABI_OBJECT_FIELD;
	object.field.parent = &(parent->object);

	// Iterate all elements
	while(state->aml < pkg.end)
	{
		sabi_field_element(state, &field);
		if(field.with_name)
		{
			object.field.flags = field.flags;
			object.field.attrib = field.attrib;
			object.field.offset = field.offset;
			object.field.length = field.length;
			sabi_ns_add_object(state, &field.name, &object);
		}
	}
}

void sabi_def_findsetbit(state_t *state, uint64_t *value, int left)
{
	uint64_t x;
	int pos;

	// Parse argument (target is handled by caller)
	x = sabi_integer(state);

	// No bits set
	if(x == 0)
	{
		*value = 0;
		return;
	}

	// Find first set bit
	if(left)
	{
		pos = 0; // Left = MSB
		while(x)
		{
			x >>= 1;
			pos++;
		}
	}
	else
	{
		pos = 1; // Right = LSB
		while((x & 1) == 0)
		{
			x >>= 1;
			pos++;
		}
	}

	// Store result
	*value = pos;
}

void sabi_def_if_else(state_t *state)
{
	uint64_t cond;
	pkg_t pkg;

	// Parse arguments
	sabi_pkg_length(state, &pkg);
	cond = sabi_integer(state);

	// If statement
	if(cond)
	{
		sabi_parse_termlist(state, pkg.end);
	}
	else
	{
		state->aml = pkg.end;
	}

	// Else statement
	if(*state->aml == 0xA1)
	{
		state->aml++;
		sabi_pkg_length(state, &pkg);

		if(cond == 0)
		{
			sabi_parse_termlist(state, pkg.end);
		}

		state->aml = pkg.end;
	}
}

void sabi_def_increment(state_t *state, uint64_t *value, int sign)
{
	operand_t operand;
	sabi_data_t data;

	sabi_parse_operand(state, &operand);
	sabi_read_operand(&operand, &data);

	if(data.type != SABI_DATA_INTEGER)
	{
		sabi_debug("not an integer, type %x", data.type);
	}

	if(sign > 0)
	{
		data.integer.value++;
	}
	else
	{
		data.integer.value--;
	}

	*value = data.integer.value;
	sabi_write_operand(&operand, &data);
}

void sabi_def_index_field(state_t *state)
{
	sabi_node_t *dnode, *inode;
	sabi_name_t data, index;
	sabi_object_t object;
	uint8_t flags;
	field_t field;
	pkg_t pkg;

	// Parse arguments
	sabi_pkg_length(state, &pkg);
	sabi_name_string(state, &index);
	sabi_name_string(state, &data);
	flags = sabi_byte(state);

	// Default values
	field.offset = 0;
	field.length = 0;
	field.flags = flags;
	field.attrib = 0;

	// Parent field nodes
	inode = sabi_ns_find(state->scope, &index);
	dnode = sabi_ns_find(state->scope, &data);

	// Construct object
	object.indexfield.type = SABI_OBJECT_INDEX_FIELD;
	object.indexfield.index = &(inode->object);
	object.indexfield.data = &(dnode->object);

	// Iterate all elements
	while(state->aml < pkg.end)
	{
		sabi_field_element(state, &field);
		if(field.with_name)
		{
			object.indexfield.flags = field.flags;
			object.indexfield.attrib = field.attrib;
			object.indexfield.offset = field.offset;
			object.indexfield.length = field.length;
			sabi_ns_add_object(state, &field.name, &object);
		}
	}
}

void sabi_def_method(state_t *state)
{
	sabi_object_t object;
	sabi_name_t name;
	uint8_t flags;
	pkg_t pkg;

	// Parse arguments
	sabi_pkg_length(state, &pkg);
	sabi_name_string(state, &name);
	flags = sabi_byte(state);

	// Construct object
	object.method.type = SABI_OBJECT_METHOD;
	object.method.flags = flags;
	object.method.argc = (flags & 0x07);
	object.method.start = state->aml;
	object.method.end = pkg.end;

	// Skip over region
	state->aml = pkg.end;

	// Append to namespace
	sabi_ns_add_object(state, &name, &object);
}

void sabi_def_mutex(state_t *state)
{
	sabi_object_t object;
	sabi_name_t name;
	uint8_t flags;

	// Parse arguments
	sabi_name_string(state, &name);
	flags = sabi_byte(state);

	// Construct object
	object.mutex.type = SABI_OBJECT_MUTEX;
	object.mutex.syncflags = flags;

	// Append to namespace
	sabi_ns_add_object(state, &name, &object);
}

void sabi_def_name(state_t *state)
{
	sabi_object_t object;
	sabi_name_t name;
	sabi_data_t *dp;

	// Allocate data
	dp = sabi_host_alloc(1, sizeof(sabi_data_t));

	// Parse arguments
	sabi_name_string(state, &name);
	sabi_data_object(state, dp);

	// Construct object
	object.name.type = SABI_OBJECT_NAME;
	object.name.data = dp;

	// Append to namespace
	sabi_ns_add_object(state, &name, &object);
}

void sabi_def_notify(state_t *state)
{
	sabi_name_t name;
	sabi_name_string(state, &name);
	sabi_integer(state);
}

void sabi_def_objecttype(state_t *state, uint64_t *value)
{
	sabi_data_t *data;
	operand_t operand;
	int type;

	// Parse arguments
	sabi_parse_operand(state, &operand);
	data = operand.refof;

	// Handle references
	if(data->type == SABI_DATA_REFERENCE)
	{
		data = data->reference.refof;
	}

	// Find object type
	switch(data->type)
	{
		case SABI_DATA_NULL:
			type = 0;
			break;
		case SABI_DATA_INTEGER:
			type = 1;
			break;
		case SABI_DATA_STRING:
			type = 2;
			break;
		case SABI_DATA_BUFFER:
			type = 3;
			break;
		case SABI_DATA_PACKAGE:
			type = 4;
			break;
		case SABI_OBJECT_FIELD:
		case SABI_OBJECT_INDEX_FIELD:
		case SABI_OBJECT_BANK_FIELD:
			type = 5;
			break;
		case SABI_OBJECT_DEVICE:
			type = 6;
			break;
		case SABI_OBJECT_EVENT:
			type = 7;
			break;
		case SABI_OBJECT_METHOD:
			type = 8;
			break;
		case SABI_OBJECT_MUTEX:
			type = 9;
			break;
		case SABI_OBJECT_OP_REGION:
			type = 10;
			break;
		case SABI_OBJECT_POWER_RES:
			type = 11;
			break;
		case SABI_OBJECT_PROCESSOR:
			type = 12;
			break;
		case SABI_OBJECT_THERMAL_ZONE:
			type = 13;
			break;
		case SABI_DATA_BUFFER_FIELD:
			type = 14;
			break;
		default:
			type = -1;
			break;
	}

	// Unhandled type
	if(type < 0)
	{
		sabi_fatal("unhandled object type");
	}

	// Return value
	*value = type;
}

void sabi_def_op_region(state_t *state)
{
	uint64_t offset, length;
	sabi_object_t object;
	sabi_node_t *node;
	sabi_name_t name;
	uint8_t space;
	void *adr;

	// Parse arguments
	sabi_name_string(state, &name);
	space = sabi_byte(state);
	offset = sabi_integer(state);
	length = sabi_integer(state);

	// System memory
	if(space == 0x00)
	{
		offset = sabi_host_map(offset);
	}

	// Construct object
	object.region.type = SABI_OBJECT_OP_REGION;
	object.region.space = space;
	object.region.offset = offset;
	object.region.length = length;
	object.region.adr = 0;

	// Append to namespace
	node = sabi_ns_add_object(state, &name, &object);

	// Extended address
	if(space == 0x02)
	{
		adr = sabi_pci_resolve_address(node);
	}
	else if(space == 0x03)
	{
		adr = sabi_ec_resolve(node);
	}
	else
	{
		adr = 0;
	}

	// Store
	node->object.region.adr = adr;
}

void sabi_def_package(state_t *state, sabi_data_t *data, int var)
{
	sabi_data_t *dp;
	pkg_t pkg;
	int count;

	// Parse arguments
	sabi_pkg_length(state, &pkg);
	if(var)
	{
		count = sabi_integer(state);
	}
	else
	{
		count = sabi_byte(state);
	}

	// Allocate data
	dp = sabi_host_alloc(count, sizeof(sabi_data_t));

	// Construct object
	data->package.type = SABI_DATA_PACKAGE;
	data->package.count = count;
	data->package.data = dp;

	// Fill data array
	while(state->aml < pkg.end)
	{
		sabi_data_object(state, dp);
		dp++;
		count--;
	}

	// Uninitialized elements
	while(count--)
	{
		dp->type = SABI_DATA_NULL;
		dp++;
	}
}

void sabi_def_power_res(state_t *state)
{
	sabi_object_t object;
	sabi_node_t *scope;
	sabi_name_t name;
	uint16_t order;
	uint8_t level;
	pkg_t pkg;

	// Parse arguments
	sabi_pkg_length(state, &pkg);
	sabi_name_string(state, &name);
	level = sabi_byte(state);
	order = sabi_word(state);

	// Construct object
	object.power.type = SABI_OBJECT_POWER_RES;
	object.power.system_level = level;
	object.power.resource_order = order;

	// Append to namespace
	scope = state->scope;
	sabi_ns_add_scope(state, &name, &object);
	sabi_parse_termlist(state, pkg.end);
	state->scope = scope;
}

void sabi_def_processor(state_t *state)
{
	sabi_object_t object;
	sabi_node_t *scope;
	sabi_name_t name;
	uint8_t id, len;
	uint32_t addr;
	pkg_t pkg;

	// Parse arguments
	sabi_pkg_length(state, &pkg);
	sabi_name_string(state, &name);
	id = sabi_byte(state);
	addr = sabi_dword(state);
	len = sabi_byte(state);

	// Construct object
	object.cpu.type = SABI_OBJECT_PROCESSOR;
	object.cpu.proc_id = id;
	object.cpu.pblk_addr = addr;
	object.cpu.pblk_len = len;

	// Append to namespace
	scope = state->scope;
	sabi_ns_add_scope(state, &name, &object);
	sabi_parse_termlist(state, pkg.end);
	state->scope = scope;
}

void sabi_def_release(state_t *state)
{
	sabi_name_t name;
	sabi_name_string(state, &name);
}

void sabi_def_reset(state_t *state)
{
	sabi_name_t name;
	sabi_name_string(state, &name);
}

void sabi_def_return(state_t *state)
{
	state->intr = INTR_RETURN;
	sabi_clean_data(state->retv);
	sabi_data_object(state, state->retv);
}

void sabi_def_scope(state_t *state)
{
	sabi_node_t *scope, *node;
	sabi_name_t name;
	pkg_t pkg;

	// Parse arguments
	sabi_pkg_length(state, &pkg);
	sabi_name_string(state, &name);

	// Find scope
	node = sabi_ns_find(state->scope, &name);
	if(node == 0)
	{
		sabi_fatal("namespace lookup failed: %s", name.value);
	}

	// Change scope
	scope = state->scope;
	state->scope = node;
	sabi_parse_termlist(state, pkg.end);
	state->scope = scope;
}

void sabi_def_signal(state_t *state)
{
	sabi_name_t name;
	sabi_name_string(state, &name);
}

void sabi_def_sizeof(state_t *state, uint64_t *value)
{
	operand_t operand;
	sabi_data_t data;

	// Parse argument
	sabi_parse_operand(state, &operand);
	sabi_read_operand(&operand, &data);

	// Object size
	if(data.type == SABI_DATA_BUFFER)
	{
		*value = data.buffer.size;
	}
	else if(data.type == SABI_DATA_STRING)
	{
		*value = strlen(data.string.value);
	}
	else if(data.type == SABI_DATA_PACKAGE)
	{
		*value = data.package.count;
	}
	else
	{
		*value = 0;
	}
}

void sabi_def_sleep(state_t *state)
{
	uint64_t msec;
	msec = sabi_integer(state);
	sabi_host_sleep(1000*msec);
}

void sabi_def_stall(state_t *state)
{
	uint64_t usec;
	usec = sabi_integer(state);
	sabi_host_sleep(usec);
}

void sabi_def_store(state_t *state)
{
	operand_t operand;
	sabi_data_t data;

	// Parse arguments
	sabi_data_object(state, &data);
	sabi_parse_operand(state, &operand);

	// Store data
	sabi_write_operand(&operand, &data);
	sabi_clean_data(&data);
}

void sabi_def_thermal_zone(state_t *state)
{
	sabi_object_t object;
	sabi_node_t *scope;
	sabi_name_t name;
	pkg_t pkg;

	// Parse arguments
	sabi_pkg_length(state, &pkg);
	sabi_name_string(state, &name);

	// Construct object
	object.type = SABI_OBJECT_THERMAL_ZONE;

	// Append to namespace
	scope = state->scope;
	sabi_ns_add_scope(state, &name, &object);
	sabi_parse_termlist(state, pkg.end);
	state->scope = scope;
}

void sabi_def_tobuffer(state_t *state, sabi_data_t *dp)
{
	operand_t operand;
	sabi_data_t data;
	int target;

	// Parse arguments
	sabi_data_object(state, &data);
	if(*state->aml)
	{
		sabi_parse_operand(state, &operand);
		target = 1;
	}
	else
	{
		target = 0;
		state->aml++;
	}

	// Convert
	sabi_conv_tobuffer(dp, &data, 0);

	// Store result
	if(target)
	{
		sabi_write_operand(&operand, dp);
	}

	// Clean
	sabi_clean_data(&data);
}

void sabi_def_tointeger(state_t *state, uint64_t *value)
{
	sabi_data_t data, dp;

	// Parse argument (target is handled by caller)
	sabi_data_object(state, &data);

	// Convert
	sabi_conv_tointeger(&dp, &data, 0);
	*value = dp.integer.value;

	// Clean
	sabi_clean_data(&data);
}

void sabi_def_tostring(state_t *state, sabi_data_t *dp)
{
	operand_t operand;
	sabi_data_t data;
	int target, len;

	// Parse arguments
	sabi_data_object(state, &data);
	len = sabi_integer(state);
	if(*state->aml)
	{
		sabi_parse_operand(state, &operand);
		target = 1;
	}
	else
	{
		target = 0;
		state->aml++;
	}

	// Convert
	sabi_conv_tostring(dp, &data, len);

	// Store result
	if(target)
	{
		sabi_write_operand(&operand, dp);
	}

	// Clean
	sabi_clean_data(&data);
}

void sabi_def_tobasestring(state_t *state, sabi_data_t *dp, int base)
{
	operand_t operand;
	sabi_data_t data;
	int target;

	// Parse arguments
	sabi_data_object(state, &data);
	if(*state->aml)
	{
		sabi_parse_operand(state, &operand);
		target = 1;
	}
	else
	{
		target = 0;
		state->aml++;
	}

	// Convert
	sabi_conv_tobasestring(dp, &data, base, 0);

	// Store result
	if(target)
	{
		sabi_write_operand(&operand, dp);
	}

	// Clean
	sabi_clean_data(&data);
}

void sabi_def_wait(state_t *state, uint64_t *value)
{
	sabi_name_t name;
	sabi_name_string(state, &name);
	sabi_integer(state);
	*value = 0;
}

void sabi_def_while(state_t *state)
{
	uint8_t *start;
	uint64_t cond;
	pkg_t pkg;

	// Parse arguments
	sabi_pkg_length(state, &pkg);
	start = state->aml;
	cond = sabi_integer(state);

	// Iterate
	while(cond)
	{
		sabi_parse_termlist(state, pkg.end);

		if(state->intr == INTR_BREAK)
		{
			state->intr = 0;
			break;
		}
		else if(state->intr == INTR_CONTINUE)
		{
			state->intr = 0;
		}
		else if(state->intr)
		{
			break;
		}

		state->aml = start;
		cond = sabi_integer(state);
	}

	// Jump to end
	state->aml = pkg.end;
}
