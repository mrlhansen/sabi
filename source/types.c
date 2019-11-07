#include <sabi/namespace.h>
#include <sabi/operand.h>
#include <sabi/parser.h>
#include <sabi/types.h>
#include <sabi/host.h>
#include <sabi/conv.h>
#include <sabi/defs.h>
#include <sabi/api.h>
#include <string.h>

static int test_name_char(uint8_t c, int lead)
{
	if(lead)
	{
		return ((c >= 'A' && c <= 'Z') || c == '_');
	}
	else
	{
		return ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_');
	}
}

static void sabi_string(state_t *state, sabi_data_t *data)
{
	char *str;
	int len;

	str = (char*)state->aml;
	len = 1 + strlen(str);

	data->string.type = SABI_DATA_STRING;
	data->string.value = sabi_host_alloc(1, len);
	strcpy(data->string.value, str);
	state->aml += len;
}

static int test_name_string(state_t *state)
{
	uint8_t *ptr;
	ptr = state->aml;

	// PrefixChar or RootChar
	while(*ptr == '\\' || *ptr == '^')
	{
		ptr++;
	}

	// DualNamePrefix
	if(*ptr == 0x2E)
	{
		ptr++;
	}

	// MultiNamePrefix
	if(*ptr == 0x2F)
	{
		ptr += 2;
	}

	// Check for valid NameSeg
	if(test_name_char(ptr[0],1) == 0)
	{
		return 0;
	}

	if(test_name_char(ptr[1],0) == 0)
	{
		return 0;
	}

	if(test_name_char(ptr[2],0) == 0)
	{
		return 0;
	}

	if(test_name_char(ptr[3],0) == 0)
	{
		return 0;
	}

	return 1;
}

sabi_node_t *sabi_parse_name(state_t *state)
{
	sabi_node_t *node;
	sabi_name_t name;
	uint8_t *aml;

	if(test_name_string(state) == 0)
	{
		return 0;
	}

	aml = state->aml;
	sabi_name_string(state, &name);
	node = sabi_ns_find(state->scope, &name);

	if(node == 0)
	{
		state->aml = aml;
		return 0;
	}

	return node;
}

void sabi_pkg_length(state_t *state, pkg_t *pkg)
{
	uint32_t length, temp;
	uint8_t bytes, *aml;

	aml = state->aml;
	pkg->end = aml;
	bytes = (*aml & 0xC0) >> 6;

	if(bytes == 0)
	{
		length = (*aml++ & 0x3F);
	}
	else
	{
		length = (*aml++ & 0x0F);
	}

	for(int n = 0; n < bytes; n++)
	{
		temp = *aml++;
		length |= (temp << (4+8*n));
	}

	pkg->length = length;
	pkg->end += length;
	state->aml = aml;
}

void sabi_name_string(state_t *state, sabi_name_t *str)
{
	uint8_t count, opcode;
	uint8_t *ptr, *aml;

	aml = state->aml;
	ptr = (uint8_t*)str->prefix;

	while(*aml == '\\' || *aml == '^')
	{
		*ptr++ = *aml++;
	}

	*ptr = '\0';
	opcode = *aml++;

	if(opcode == 0x2E) // DualNamePrefix
	{
		count = 2;
	}
	else if(opcode == 0x2F) // MultiNamePrefix
	{
		count = *aml++;
	}
	else if(opcode == 0x00) // NullName
	{
		count = 0;
	}
	else // NameSeg
	{
		aml--;
		count = 1;
	}

	count = (4 * count);
	ptr = (uint8_t*)str->value;

	while(count--)
	{
		*ptr++ = *aml++;
	}

	*ptr = '\0';
	state->aml = aml;
}

uint64_t sabi_integer(state_t *state)
{
	operand_t operand;
	sabi_data_t data;
	uint64_t value;
	uint8_t prefix;
	int found;

	found = 1;
	prefix = *state->aml++;
	value = 0;

	switch(prefix)
	{
		case 0x00: // ZeroOp
			value = 0;
			break;
		case 0x01: // OneOp
			value = 1;
			break;
		case 0xFF: // OnesOp
			value = ~0UL;
			break;
		case 0x0A: // BytePrefix
			value = sabi_byte(state);
			break;
		case 0x0B: // WordPrefix
			value = sabi_word(state);
			break;
		case 0x0C: // DWordPrefix
			value = sabi_dword(state);
			break;
		case 0x0E: // QWordPrefix
			value = sabi_qword(state);
			break;
		default:
			found = 0;
			state->aml--;
			break;
	}

	if(found)
	{
		return value;
	}

	if(sabi_parse_math(state, &value))
	{
		return value;
	}

	if(sabi_parse_method(state))
	{
		value = sabi_conv_tointeger(0, &data, 1);
		return value;
	}

	found = sabi_parse_operand(state, &operand);
	if(found)
	{
		sabi_read_operand(&operand, &data);
		value = sabi_conv_tointeger(0, &data, 1);
		return value;
	}

	sabi_fatal("integer not found, opcode %x", prefix);
	return 0;
}

uint8_t sabi_byte(state_t *state)
{
	uint8_t ret;
	ret = *(uint8_t*)state->aml;
	state->aml += sizeof(uint8_t);
	return ret;
}

uint16_t sabi_word(state_t *state)
{
	uint16_t ret;
	ret = *(uint16_t*)state->aml;
	state->aml += sizeof(uint16_t);
	return ret;
}

uint32_t sabi_dword(state_t *state)
{
	uint32_t ret;
	ret = *(uint32_t*)state->aml;
	state->aml += sizeof(uint32_t);
	return ret;
}

uint64_t sabi_qword(state_t *state)
{
	uint64_t ret;
	ret = *(uint64_t*)state->aml;
	state->aml += sizeof(uint64_t);
	return ret;
}

void sabi_field_element(state_t *state, field_t *field)
{
	sabi_name_t name;
	uint8_t prefix;
	pkg_t pkg;

	prefix = *state->aml++;
	field->offset += field->length;
	field->length = 0;
	field->with_name = 0;

	if(prefix == 0x00) // ReservedField
	{
		sabi_pkg_length(state, &pkg);
		field->length = pkg.length;
	}
	else if(prefix == 0x01) // AccessField
	{
		field->flags = sabi_byte(state);
		field->attrib = sabi_byte(state);
	}
	else if(prefix == 0x02) // ConnectField
	{
		sabi_fatal("ConnectField not implemented");
	}
	else if(prefix == 0x03) // ExtendedAccessField
	{
		sabi_fatal("ExtendedAccessField not implemented");
	}
	else // NamedField
	{
		state->aml--;
		sabi_name_string(state, &name);
		sabi_pkg_length(state, &pkg);
		field->name = name;
		field->length = pkg.length;
		field->with_name = 1;
	}
}

void sabi_data_object(state_t *state, sabi_data_t *data)
{
	operand_t operand;
	sabi_name_t name;
	sabi_data_t temp;
	uint8_t opcode;
	int found;

	// Method call
	if(sabi_parse_method(state))
	{
		sabi_clone_data(data, state->retv);
		return;
	}

	// Data reference
	if(sabi_parse_operand(state, &operand))
	{
		sabi_read_operand(&operand, &temp);
		sabi_clone_data(data, &temp);
		return;
	}

	// Namespace object
	if(test_name_string(state))
	{
		sabi_name_string(state, &name);
		data->name.type = SABI_DATA_NAME;
		data->name.path = name;
		return;
	}

	// Data objects
	opcode = *state->aml++;
	found = 1;

	switch(opcode)
	{
		case 0x11:
			sabi_def_buffer(state, data);
			break;
		case 0x12:
			sabi_def_package(state, data, 0);
			break;
		case 0x13:
			sabi_def_package(state, data, 1);
			break;
		case 0x0D:
			sabi_string(state, data);
			break;
		case 0x96:
			sabi_def_tobuffer(state, data);
			break;
		case 0x97:
			sabi_def_tobasestring(state, data, 10);
			break;
		case 0x98:
			sabi_def_tobasestring(state, data, 16);
			break;
		case 0x9C:
			sabi_def_tostring(state, data);
			break;
		default:
			found = 0;
			state->aml--;
			break;
	}

	if(found)
	{
		return;
	}

	// Integer
	data->integer.type = SABI_DATA_INTEGER;
	data->integer.value = sabi_integer(state);
}
