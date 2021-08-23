#include <sabi/namespace.h>
#include <sabi/state.h>
#include <sabi/host.h>
#include <sabi/api.h>

state_t *sabi_new_state(void)
{
	sabi_data_t *data;
	state_t *state;
	int memsz;

	memsz = sizeof(state_t) + 16*sizeof(sabi_data_t);
	state = sabi_host_alloc(1, memsz);
	data = (sabi_data_t*)(state+1);

	state->scope = sabi_ns_root();
	state->stack = 0;
	state->aml = 0;
	state->intr = 0;
	state->argc = 0;
	state->count = 0;
	state->retv = data;
	state->argv = data+1;
	state->local = data+8;

	for(int i = 0; i < 16; i++)
	{
		data[i].type = SABI_DATA_NULL;
	}

	return state;
}

void sabi_free_state(state_t *state)
{
	sabi_node_t *node, *next;
	sabi_object_t *object;
	sabi_data_t *data;

	data = state->retv;
	node = state->stack;

	for(int i = 0; i < 16; i++)
	{
		sabi_clean_data(data+i);
	}

	while(node)
	{
		sabi_ns_unlink(node);
		next = node->stack;
		object = &node->object;

		if(object->type == SABI_OBJECT_NAME)
		{
			data = object->name.data;
			sabi_clean_data(data);
			sabi_host_free(data);
		}
		else if(object->type == SABI_OBJECT_DEVICE)
		{
			if(object->device.adr)
			{
				sabi_host_free(object->device.adr);
			}
		}

		sabi_host_free(node);
		node = next;
	}

	sabi_host_free(state);
}
