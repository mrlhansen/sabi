#include <sabi/namespace.h>
#include <sabi/resources.h>
#include <sabi/host.h>
#include <sabi/api.h>
#include <sabi/ec.h>

static sabi_ec_t *list = 0;

static uint8_t read_status(sabi_ec_t *ec)
{
	uint64_t status;

	if(ec->space)
	{
		sabi_host_pmio_read(ec->base, &status, 8);
	}
	else
	{
		// missing
		status = 0;
	}

	return status;
}

static uint8_t read_data(sabi_ec_t *ec)
{
	uint64_t value;
	uint8_t status;

	do
	{
		status = read_status(ec);
		status = (status & SABI_EC_STATUS_OBF);
	}
	while(status == 0);

	if(ec->space)
	{
		sabi_host_pmio_read(ec->data, &value, 8);
	}
	else
	{
		// missing
		value = 0;
	}

	return value;
}

static void write_command(sabi_ec_t *ec, uint8_t value)
{
	uint8_t status;

	do
	{
		status = read_status(ec);
		status = (status & SABI_EC_STATUS_IBF);
	}
	while(status);

	if(ec->space)
	{
		sabi_host_pmio_write(ec->base, value, 8);
	}
	else
	{
		// missing
	}
}

static void write_data(sabi_ec_t *ec, uint8_t value)
{
	uint8_t status;

	do
	{
		status = read_status(ec);
		status = (status & SABI_EC_STATUS_IBF);
	}
	while(status);

	if(ec->space)
	{
		sabi_host_pmio_write(ec->data, value, 8);
	}
	else
	{
		// missing
	}
}

static void enable_burst(sabi_ec_t *ec)
{
	int status;

	write_command(ec, SABI_EC_BURST_ENABLE);
	status = read_data(ec);
	if(status != 0x90)
	{
		// error
	}

	status = read_status(ec);
	if((status & SABI_EC_STATUS_BURST) == 0)
	{
		// error
	}
}

static void disable_burst(sabi_ec_t *ec)
{
	write_command(ec, SABI_EC_BURST_DISABLE);
	while(read_status(ec) & SABI_EC_STATUS_BURST);
}

void sabi_ec_write(sabi_ec_t *ec, uint8_t offset, uint8_t value)
{
	enable_burst(ec);

	write_command(ec, SABI_EC_WRITE);
	write_data(ec, offset);
	write_data(ec, value);

	disable_burst(ec);
}

void sabi_ec_read(sabi_ec_t *ec, uint8_t offset, uint8_t *value)
{
	enable_burst(ec);

	write_command(ec, SABI_EC_READ);
	write_data(ec, offset);
	value[0] = read_data(ec);

	disable_burst(ec);
}

sabi_ec_t *sabi_ec_resolve(sabi_node_t *region)
{
	int type, space, status;
	sabi_descriptor_t desc;
	sabi_resource_t res;
	uint64_t base, data;
	sabi_node_t *node;
	sabi_ec_t *ec;
	uint32_t hid;

	node = sabi_ns_search(region, "_HID");
	if(node == 0)
	{
		return 0;
	}

	node = node->parent;
	ec = node->object.device.adr;
	if(ec)
	{
		return ec;
	}

	hid = sabi_eisaid("PNP0C09");
	status = sabi_check_pnp_id(node, hid);
	if(status == 0)
	{
		return 0;
	}

	status = sabi_eval_resource(node, &res);
	if(status < 0)
	{
		return 0;
	}

	type = sabi_next_resource(&res, &desc);
	if(type == 0x08)
	{
		space = 0x01;
		data = desc.u.res08->min;
	}
	else
	{
		sabi_fatal("unhandled resource type %x", type);
	}

	type = sabi_next_resource(&res, &desc);
	if(type == 0x08)
	{
		base = desc.u.res08->min;
	}
	else
	{
		sabi_fatal("unhandled resource type %x", type);
	}

	sabi_free_resource(&res);
	ec = sabi_host_alloc(1, sizeof(sabi_ec_t));
	node->object.device.adr = ec;

	ec->space = space;
	ec->base = base;
	ec->data = data;
	ec->parent = node;
	ec->next = list;
	list = ec;

	return ec;
}

void sabi_ec_call_reg()
{
	sabi_data_t args[2];
	sabi_node_t *node;
	sabi_ec_t *ec;

	ec = list;
	args[0].integer.type = SABI_DATA_INTEGER;
	args[0].integer.value = 0x03;
	args[1].integer.type = SABI_DATA_INTEGER;
	args[1].integer.value = 0x01;

	while(ec)
	{
		node = sabi_ns_exists(ec->parent, "_REG");
		if(node)
		{
			sabi_eval_method(node, 2, args, 0);
		}
		ec = ec->next;
	}
}
