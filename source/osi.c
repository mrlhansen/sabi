#include <sabi/types.h>
#include <sabi/osi.h>
#include <sabi/api.h>
#include <string.h>

static const char *os = "Microsoft Windows NT";
static const int rev = 2;
static const char *osi[] = {
	"Windows 2000",        // Windows 2000
	"Windows 2001",        // Windows XP
	"Windows 2001 SP1",    // Windows XP SP1
	"Windows 2001 SP2",    // Windows XP SP2
	"Windows 2006",        // Windows Vista
	"Windows 2006 SP1",    // Windows Vista SP1
	"Windows 2009",        // Windows 7
	"Windows 2012",        // Windows 8
	"Windows 2013",        // Windows 8.1
	"Windows 2015",        // Windows 10
};

void sabi_osi_parse(state_t *state)
{
	sabi_data_t *retv, data;
	uint64_t value;
	int count;

	count = sizeof(osi)/sizeof(*osi);
	sabi_data_object(state, &data);
	retv = state->retv;
	value = 0;

	for(int i = 0; i < count; i++)
	{
		if(strcmp(data.string.value, osi[i]) == 0)
		{
			value = 0xFFFFFFFF;
			break;
		}
	}

	sabi_clean_data(&data);
	sabi_clean_data(state->retv);

	retv->integer.type = SABI_DATA_INTEGER;
	retv->integer.type = value;
}

void sabi_osi_method(sabi_node_t *node)
{
	node->object.method.type = SABI_OBJECT_METHOD;
	node->object.method.start = 0;
	node->object.method.end = 0;
	node->object.method.flags = 1;
	node->object.method.argc = 1;
}

void sabi_osi_string(sabi_node_t *node)
{
	static sabi_data_t data;
	node->object.name.type = SABI_OBJECT_NAME;
	node->object.name.data = &data;
	data.string.type = SABI_DATA_STRING;
	strcpy(data.string.value, os);
}

void sabi_osi_revision(sabi_node_t *node)
{
	static sabi_data_t data;
	node->object.name.type = SABI_OBJECT_NAME;
	node->object.name.data = &data;
	data.integer.type = SABI_DATA_INTEGER;
	data.integer.value = rev;
}
