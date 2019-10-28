#include <sabi/resources.h>
#include <sabi/namespace.h>
#include <sabi/host.h>
#include <sabi/api.h>

int sabi_next_resource(sabi_resource_t *res, sabi_descriptor_t *desc)
{
	int type, length;
	uint8_t *ptr;

	if(res->pos)
	{
		ptr = (res->pos + res->skip);
	}
	else
	{
		ptr = res->buf;
	}

	type = *ptr;

	if(type & 0x80)
	{
		length = *(uint16_t*)(ptr+1) + 3;
	}
	else
	{
		length = (type & 0x07) + 1;
		type = (type >> 3);
	}

	if(type == 0x0F)
	{
		ptr = 0;
		length = 0;
		type = 0;
	}

	res->pos = ptr;
	res->skip = length;
	desc->u.data = ptr;
	desc->type = type;
	desc->length = length;

	return type;
}

int sabi_eval_resource(sabi_node_t *parent, sabi_resource_t *res)
{
	sabi_node_t *node;
	sabi_data_t data;

	node = sabi_ns_exists(parent, "_CRS");
	if(node)
	{
		sabi_eval_node(node, &data);
		res->buf = data.buffer.ptr;
		res->length = data.buffer.size;
		res->pos = 0;
		res->skip = 0;
		return 0;
	}

	return -SABI_ERROR_LOOKUP;
}

void sabi_free_resource(sabi_resource_t *res)
{
	sabi_host_free(res->buf);
}
