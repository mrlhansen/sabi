#include <sabi/namespace.h>
#include <sabi/operand.h>
#include <sabi/parser.h>
#include <sabi/fields.h>
#include <sabi/types.h>
#include <sabi/host.h>
#include <sabi/conv.h>
#include <sabi/api.h>
#include <string.h>

static void index_to_operand(sabi_data_t *sp, operand_t *op)
{
	sabi_data_t *dp;
	int index;

	op->flags = OPERAND_DATA;
	op->refof = &op->data;
	dp = op->refof;
	index = sp->reference.index;
	sp = sp->reference.refof;

	if(sp->type == SABI_DATA_BUFFER)
	{
		dp->field.type = SABI_DATA_BUFFER_FIELD;
		dp->field.size = 8;
		dp->field.offset = 0;
		dp->field.ptr = (sp->buffer.ptr + index);
	}
	else if(sp->type == SABI_DATA_STRING)
	{
		dp->field.type = SABI_DATA_BUFFER_FIELD;
		dp->field.size = 8;
		dp->field.offset = 0;
		dp->field.ptr = (uint8_t*)(sp->string.value + index);
	}
	else if(sp->type == SABI_DATA_PACKAGE)
	{
		op->refof = (sp->package.data + index);
	}
	else
	{
		sabi_fatal("invalid type %x for index", sp->type);
	}
}

void sabi_def_index(state_t *state, operand_t *op)
{
	operand_t src, dst;
	uint64_t index;
	sabi_data_t *data;
	int target;

	// Parse arguments
	sabi_parse_operand(state, &src);
	index = sabi_integer(state);

	if(*state->aml)
	{
		sabi_parse_operand(state, &dst);
		target = 1;
	}
	else
	{
		state->aml++;
		target = 0;
	}

	// Operand type
	if((src.flags & OPERAND_DATA) == 0)
	{
		sabi_fatal("Invalid operand for DefIndex");
	}

	// Construct reference
	op->flags = (OPERAND_DATA | OPERAND_INDEX);
	op->refof = &op->data;
	data = op->refof;

	data->reference.type = SABI_DATA_REFERENCE;
	data->reference.refof = src.refof;
	data->reference.index = index;
	data->reference.flags = op->flags;

	// Store in target
	if(target)
	{
		sabi_write_operand(&dst, data);
	}
}

void sabi_def_derefof(state_t *state, operand_t *op)
{
	sabi_object_t *object;
	sabi_data_t data;
	void *refof;
	int flags;

	// Parse operand (could also be a string)
	sabi_parse_operand(state, op);
	sabi_read_operand(op, &data);

	if(data.type == SABI_DATA_REFERENCE)
	{
		refof = data.reference.refof;
		flags = data.reference.flags;
	}
	else
	{
		sabi_fatal("Invalid argument for DerefOf");
	}

	// Dereference
	if(flags & OPERAND_INDEX)
	{
		index_to_operand(&data, op);
	}
	else if(flags & OPERAND_DATA)
	{
		op->flags = flags;
		op->refof = refof;
	}
	else if(flags & OPERAND_OBJECT)
	{
		object = refof;
		if(object->type == SABI_OBJECT_NAME)
		{
			op->flags = OPERAND_DATA;
			op->refof = object->name.data;
		}
		else
		{
			op->flags = OPERAND_OBJECT;
			op->refof = object;
		}
	}
	else
	{
		sabi_fatal("Unknown reference");
	}
}

void sabi_def_refof(state_t *state, operand_t *op)
{
	sabi_data_t *dp;
	void *refof;
	int flags;

	sabi_parse_operand(state, op);
	refof = op->refof;
	flags = op->flags;

	if(flags & (OPERAND_DEBUG | OPERAND_INDEX))
	{
		sabi_fatal("Invalid argument for RefOf");
	}

	op->flags = OPERAND_DATA;
	op->refof = &op->data;
	dp = op->refof;

	dp->reference.type = SABI_DATA_REFERENCE;
	dp->reference.refof = refof;
	dp->reference.flags = flags;
}

int sabi_parse_operand(state_t *state, operand_t *op)
{
	int opcode, num, type;
	sabi_object_t *object;
	sabi_node_t *node;
	uint8_t *aml;

	opcode = sabi_pop_opcode(state);
	num = (opcode & 0x07);

	// LocalX
	if((opcode >= 0x60) && (opcode <= 0x67))
	{
		op->flags = (OPERAND_DATA | OPERAND_LOCAL);
		op->refof = (state->local + num);
		return 1;
	}

	// ArgX
	if((opcode >= 0x68) && (opcode <= 0x6E))
	{
		op->flags = (OPERAND_DATA | OPERAND_ARG);
		op->refof = (state->argv + num);
		return 1;
	}

	// Debug
	if(opcode == 0x5B31)
	{
		op->flags = OPERAND_DEBUG;
		op->refof = 0;
		return 1;
	}

	// DefRefOf
	if(opcode == 0x71)
	{
		sabi_def_refof(state, op);
		return 1;
	}

	// DefDerefOf
	if(opcode == 0x83)
	{
		sabi_def_derefof(state, op);
		return 1;
	}

	// DefIndex
	if(opcode == 0x88)
	{
		sabi_def_index(state, op);
		return 1;
	}

	// Rewind
	sabi_push_opcode(state, opcode);
	aml = state->aml;

	// NameString
	node = sabi_parse_name(state);
	if(node == 0)
	{
		return 0;
	}

	object = &node->object;
	type = object->type;

	if(type == SABI_OBJECT_NAME)
	{
		op->flags = OPERAND_DATA;
		op->refof = object->name.data;
		return 1;
	}

	if((type == SABI_OBJECT_FIELD) || (type == SABI_OBJECT_INDEX_FIELD))
	{
		op->flags = OPERAND_OBJECT;
		op->refof = object;
		return 1;
	}

	state->aml = aml;
	return 0;
}

void sabi_read_operand(operand_t *op, sabi_data_t *dp)
{
	sabi_object_t *object;
	sabi_data_t *data;
	uint64_t value;
	int type;

	if(op->flags & OPERAND_DEBUG)
	{
		dp->type = SABI_DATA_NULL;
		return;
	}

	if(op->flags & OPERAND_DATA)
	{
		data = op->refof;
		type = data->type;
		object = 0;
	}
	else
	{
		object = op->refof;
		type = object->type;
		data = 0;
	}

	if(object)
	{
		if(type == SABI_OBJECT_FIELD)
		{
			sabi_access_field(object, &value, 0);
			dp->integer.type = SABI_DATA_INTEGER;
			dp->integer.value = value;
		}
		else if(type == SABI_OBJECT_INDEX_FIELD)
		{
			sabi_access_index_field(object, &value, 0);
			dp->integer.type = SABI_DATA_INTEGER;
			dp->integer.value = value;
		}
		else
		{
			sabi_debug("cannot read from object type %x", type);
		}
		return;
	}

	if(type == SABI_DATA_BUFFER_FIELD)
	{
		sabi_access_buffer_field(data, &value, 0);
		dp->integer.type = SABI_DATA_INTEGER;
		dp->integer.value = value;
	}
	else
	{
		*dp = *data;
	}
}

void sabi_write_operand(operand_t *op, sabi_data_t *dp)
{
	sabi_object_t *object;
	sabi_data_t *data;
	int otype, stype;
	operand_t otmp;
	uint64_t value;

	if(op->flags & OPERAND_DEBUG)
	{
		return;
	}

	if(op->flags & OPERAND_INDEX)
	{
		index_to_operand(op->refof, &otmp);
		op = &otmp;
	}

	stype = dp->type;

	if(op->flags & OPERAND_DATA)
	{
		data = op->refof;
		otype = data->type;
		object = 0;
	}
	else
	{
		object = op->refof;
		otype = object->type;
		data = 0;
	}

	if(object)
	{
		if(otype == SABI_OBJECT_FIELD)
		{
			if(stype == SABI_DATA_INTEGER)
			{
				value = dp->integer.value;
				sabi_access_field(object, &value, 1);
			}
			else
			{
				sabi_debug("write with non-integer data");
			}
		}
		else if(otype == SABI_OBJECT_INDEX_FIELD)
		{
			if(stype == SABI_DATA_INTEGER)
			{
				value = dp->integer.value;
				sabi_access_index_field(object, &value, 1);
			}
			else
			{
				sabi_debug("write with non-integer data");
			}
		}
		else
		{
			sabi_fatal("cannot write to object type %x", otype);
		}
		return;
	}

	if(op->flags & OPERAND_LOCAL)
	{
		sabi_clean_data(data);
		sabi_clone_data(data, dp);
		return;
	}

	if(op->flags & OPERAND_ARG)
	{
		sabi_clean_data(data);
		sabi_clone_data(data, dp);
		sabi_debug("Writing to ArgX - exception here if *dp contains a reference !");
		return;
	}

	if(otype == SABI_DATA_NULL)
	{
		sabi_clone_data(data, dp);
	}
	else if(otype == SABI_DATA_INTEGER)
	{
		data->integer.value = sabi_conv_tointeger(0, dp, 1);
	}
	else if(otype == SABI_DATA_BUFFER_FIELD)
	{
		value = sabi_conv_tointeger(0, dp, 1);
		sabi_access_buffer_field(data, &value, 1);
	}
	else if(otype == SABI_DATA_BUFFER)
	{
		sabi_conv_tobuffer(data, dp, 1);
	}
	else if(otype == SABI_DATA_STRING)
	{
		sabi_conv_tobasestring(data, dp, 16, 1);
	}
	else if(otype == SABI_DATA_PACKAGE)
	{
		if(stype == SABI_DATA_PACKAGE)
		{
			sabi_clean_data(data);
			sabi_clone_data(data, dp);
		}
		else
		{
			sabi_debug("unhandled conversion to package from type %x", stype);
		}
	}
	else
	{
		sabi_debug("cannot write to data type %x", otype);
	}
}
