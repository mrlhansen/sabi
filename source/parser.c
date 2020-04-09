#include <sabi/namespace.h>
#include <sabi/operand.h>
#include <sabi/parser.h>
#include <sabi/types.h>
#include <sabi/defs.h>
#include <sabi/host.h>
#include <sabi/osi.h>
#include <sabi/api.h>

int sabi_pop_opcode(state_t *state)
{
	int opcode, next;

	opcode = *state->aml++;
	next = *state->aml;

	if(opcode == 0x5B)
	{
		opcode = (0x5B00 | *state->aml++);
	}
	else if(opcode == 0x92)
	{
		if((next >= 0x93) && (next <= 0x95))
		{
			opcode = (0x9200 | *state->aml++);
		}
	}

	return opcode;
}

void sabi_push_opcode(state_t *state, int opcode)
{
	state->aml--;
	if(opcode & 0xFF00)
	{
		state->aml--;
	}
}

int sabi_parse_method(state_t *state)
{
	sabi_object_t *object;
	sabi_node_t *node;
	sabi_data_t *argv;
	state_t *child;
	uint8_t *aml;
	int count;

	// Check for method
	aml = state->aml;
	node = sabi_parse_name(state);

	if(node == 0)
	{
		return 0;
	}
	else
	{
		object = &node->object;
	}

	if(object->type != SABI_OBJECT_METHOD)
	{
		state->aml = aml;
		return 0;
	}

	// OSI method
	if(object->method.start == 0)
	{
		sabi_osi_parse(state);
		return 1;
	}

	// Allocate memory
	child = sabi_new_state();
	child->scope = node;
	child->aml = object->method.start;
	argv = child->argv;

	// Parse arguments
	count = object->method.argc;
	child->argc = count;

	for(int i = 0; i < count; i++)
	{
		sabi_data_object(state, argv+i);
	}

	// Execute function
	sabi_parse_termlist(child, object->method.end);

	// Finalize
	sabi_clean_data(state->retv);
	sabi_clone_data(state->retv, child->retv);
	sabi_free_state(child);

	return 1;
}

int sabi_parse_math(state_t *state, uint64_t *value)
{
	sabi_data_t data;
	operand_t operand;
	int opcode, found;
	uint64_t x, y, z;

	z = 0;
	found = 1;
	opcode = sabi_pop_opcode(state);

	switch(opcode)
	{
		case 0x0072: // Add
			x = sabi_integer(state);
			y = sabi_integer(state);
			z = (x + y);
			break;
		case 0x0074: // Subtract
			x = sabi_integer(state);
			y = sabi_integer(state);
			z = (x - y);
			break;
		case 0x0077: // Multiply
			x = sabi_integer(state);
			y = sabi_integer(state);
			z = (x * y);
			break;
		case 0x0079: // Shift left
			x = sabi_integer(state);
			y = sabi_integer(state);
			z = (x << y);
			break;
		case 0x007A: // Shift right
			x = sabi_integer(state);
			y = sabi_integer(state);
			z = (x >> y);
			break;
		case 0x007B: // Bitwise AND
			x = sabi_integer(state);
			y = sabi_integer(state);
			z = (x & y);
			break;
		case 0x007C: // Bitwise NOT AND
			x = sabi_integer(state);
			y = sabi_integer(state);
			z = ~(x & y);
			break;
		case 0x007D: // Bitwise OR
			x = sabi_integer(state);
			y = sabi_integer(state);
			z = (x | y);
			break;
		case 0x007E: // Bitwise NOT OR
			x = sabi_integer(state);
			y = sabi_integer(state);
			z = ~(x | y);
			break;
		case 0x007F: // Bitwise XOR
			x = sabi_integer(state);
			y = sabi_integer(state);
			z = (x ^ y);
			break;
		case 0x0080: // Bitwise NOT
			x = sabi_integer(state);
			z = ~x;
			break;
		case 0x5B28: // FromBCD
			sabi_def_convertbcd(state, &z, 1);
			break;
		case 0x5B29: // ToBCD
			sabi_def_convertbcd(state, &z, 0);
			break;
		case 0x0081: // FindSetLeftBit
			sabi_def_findsetbit(state, &z, 1);
			break;
		case 0x0082: // FindSetRightBit
			sabi_def_findsetbit(state, &z, 0);
			break;
		case 0x0085: // Modulo
			x = sabi_integer(state);
			y = sabi_integer(state);
			z = (x % y);
			break;
		case 0x0099: // ToInteger
			sabi_def_tointeger(state, &z);
			break;
		default:
			found = 0;
			break;
	}

	if(found)
	{
		*value = z;

		if(*state->aml)
		{
			data.integer.type = SABI_DATA_INTEGER;
			data.integer.value = z;
			sabi_parse_operand(state, &operand);
			sabi_write_operand(&operand, &data);
		}
		else
		{
			state->aml++;
		}

		return 1;
	}
	else
	{
		found = 1;
	}

	switch(opcode)
	{
		case 0x0075: // DefIncrement
			sabi_def_increment(state, &z, +1);
			break;
		case 0x0076: // DefDecrement
			sabi_def_increment(state, &z, -1);
			break;
		case 0x0078: // DefDivide
			sabi_def_divide(state, &z);
			break;
		case 0x0087: // DefSizeOf
			sabi_def_sizeof(state, &z);
			break;
		case 0x5B12: // DefCondRefOf
			sabi_def_condrefof(state, &z);
			break;
		case 0x5B23: // DefAcquire
			sabi_def_acquire(state, &z);
			break;
		case 0x5B25: // DefWait
			sabi_def_wait(state, &z);
			break;
		case 0x008E: // ObjectType
			sabi_def_objecttype(state, &z);
			break;
		case 0x0090: // Logical AND
			x = sabi_integer(state);
			y = sabi_integer(state);
			z = (x && y);
			break;
		case 0x0091: // Logical OR
			x = sabi_integer(state);
			y = sabi_integer(state);
			z = (x || y);
			break;
		case 0x0092: // Logical NOT
			x = sabi_integer(state);
			z = (!x);
			break;
		case 0x0093: // Logical equal
			x = sabi_integer(state);
			y = sabi_integer(state);
			z = (x == y);
			break;
		case 0x9293: // Logical not equal
			x = sabi_integer(state);
			y = sabi_integer(state);
			z = (x != y);
			break;
		case 0x0094: // Logical greater
			x = sabi_integer(state);
			y = sabi_integer(state);
			z = (x > y);
			break;
		case 0x9295: // Logical greater or equal
			x = sabi_integer(state);
			y = sabi_integer(state);
			z = (x >= y);
			break;
		case 0x0095: // Logical less
			x = sabi_integer(state);
			y = sabi_integer(state);
			z = (x < y);
			break;
		case 0x9294: // Logical less or equal
			x = sabi_integer(state);
			y = sabi_integer(state);
			z = (x <= y);
			break;
		default:
			found = 0;
			break;
	}

	if(found)
	{
		*value = z;
	}
	else
	{
		sabi_push_opcode(state, opcode);
	}

	return found;
}

void sabi_parse_next(state_t *state)
{
	int opcode, found;
	operand_t operand;
	uint64_t value;

	found = 1;
	opcode = sabi_pop_opcode(state);

	switch(opcode)
	{
		case 0x0006: // DefAlias
			sabi_def_alias(state);
			break;
		case 0x0008: // DefName
			sabi_def_name(state);
			break;
		case 0x0010: // DefScope
			sabi_def_scope(state);
			break;
		case 0x5B13: // DefCreateField
			sabi_def_create_field(state, 1, 0);
			break;
		case 0x0070: // DefStore
			sabi_def_store(state);
			break;
		case 0x008A: // DefCreateDWordField
			sabi_def_create_field(state, 8, 32);
			break;
		case 0x008B: // DefCreateWordField
			sabi_def_create_field(state, 8, 16);
			break;
		case 0x008C: // DefCreateByteField
			sabi_def_create_field(state, 8, 8);
			break;
		case 0x008D: // DefCreateBitField
			sabi_def_create_field(state, 1, 1);
			break;
		case 0x008F: // DefCreateQWordField
			sabi_def_create_field(state, 8, 64);
			break;
		case 0x5B82: // DefDevice
			sabi_def_device(state);
			break;
		case 0x5B02: // DefEvent
			sabi_def_event(state);
			break;
		case 0x5B24: // DefSignal
			sabi_def_signal(state);
			break;
		case 0x5B26: // DefReset
			sabi_def_reset(state);
			break;
		case 0x0015: // DefExternal
			sabi_def_external(state);
			break;
		case 0x5B81: // DefField
			sabi_def_field(state);
			break;
		case 0x5B86: // DefIndexField
			sabi_def_index_field(state);
			break;
		case 0x5B87: // DefBankField
			sabi_def_bank_field(state);
			break;
		case 0x0014: // DefMethod
			sabi_def_method(state);
			break;
		case 0x5B21: // DefStall
			sabi_def_stall(state);
			break;
		case 0x5B22: // DefSleep
			sabi_def_sleep(state);
			break;
		case 0x5B01: // DefMutex
			sabi_def_mutex(state);
			break;
		case 0x5B80: // DefOpRegion
			sabi_def_op_region(state);
			break;
		case 0x5B84: // DefPowerRes
			sabi_def_power_res(state);
			break;
		case 0x5B83: // DefProcessor
			sabi_def_processor(state);
			break;
		case 0x5B85: // DefThermalZone
			sabi_def_thermal_zone(state);
			break;
		case 0x0086: // DefNotify
			sabi_def_notify(state);
			break;
		case 0x009F: // DefContinue
			state->intr = INTR_CONTINUE;
			break;
		case 0x00A0: // DefIfElse
			sabi_def_if_else(state);
			break;
		case 0x00A2: // DefWhile
			sabi_def_while(state);
			break;
		case 0x5B27: // DefRelease
			sabi_def_release(state);
			break;
		case 0x00A3: // DefNoop
			break;
		case 0x00A4: // DefReturn
			sabi_def_return(state);
			break;
		case 0x00A5: // DefBreak
			state->intr = INTR_BREAK;
			break;
		case 0x0088: // DefIndex
			sabi_def_index(state, &operand);
			break;
		default:
			found = 0;
			break;
	}

	if(found)
	{
		return;
	}
	else
	{
		sabi_push_opcode(state, opcode);
	}

	if(sabi_parse_math(state, &value))
	{
		return;
	}

	if(sabi_parse_method(state))
	{
		return;
	}

	sabi_fatal("unknown opcode %x", opcode);
}

void sabi_parse_termlist(state_t *state, uint8_t *end)
{
	while(state->aml < end)
	{
		sabi_parse_next(state);
		if(state->intr)
		{
			break;
		}
	}
	state->aml = end;
}
