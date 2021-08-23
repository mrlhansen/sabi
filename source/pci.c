#include <sabi/resources.h>
#include <sabi/namespace.h>
#include <sabi/host.h>
#include <sabi/api.h>
#include <sabi/pci.h>

static void sabi_pci_package(sabi_node_t *parent, sabi_data_t *pkg, sabi_prt_t *prt)
{
	int type, status, index;
	sabi_descriptor_t desc;
	sabi_resource_t res;
	sabi_name_t *name;
	sabi_node_t *node;

	pkg = pkg->package.data;
	type = 0;

	prt->slot = (pkg[0].integer.value >> 16);
	prt->pin = pkg[1].integer.value;
	prt->flags = 0x06;
	prt->gsi = -1;

	if(pkg[2].type == SABI_DATA_INTEGER)
	{
		prt->gsi = pkg[3].integer.value;
		return;
	}

	name = &(pkg[2].name.path);
	index = pkg[3].integer.value + 1;

	node = sabi_ns_find(parent, name);
	if(node == 0)
	{
		return;
	}

	status = sabi_eval_resource(node, &res);
	if(status < 0)
	{
		return;
	}

	while(index--)
	{
		type = sabi_next_resource(&res, &desc);
	}

	if(type == 0x04)
	{
		status = desc.u.res04->mask;
		while(status)
		{
			prt->gsi++;
			status >>= 1;
		}
		if(desc.length == 3)
		{
			status = desc.u.res04->flags;
			prt->flags = (((status & 0x38) >> 2) | (status & 0x01));
		}

	}
	else if(type == 0x89)
	{
		prt->gsi = desc.u.res89->intr[0];
		prt->flags = (desc.u.res89->flags >> 1);
	}

	sabi_free_resource(&res);
}

sabi_pci_t *sabi_pci_resolve_address(sabi_node_t *region)
{
	sabi_node_t *node, *parent;
	uint64_t seg, bus, dev;
	sabi_data_t data;
	sabi_pci_t *adr;

	seg = 0;
	bus = 0;
	dev = 0;

	node = sabi_ns_search(region, "_ADR");
	if(node == 0)
	{
		return 0;
	}

	parent = node->parent;
	adr = parent->object.device.adr;

	if(adr)
	{
		return adr;
	}
	else
	{
		sabi_eval_node(node, &data);
		dev = data.integer.value;
	}

	node = sabi_ns_search(region, "_BBN");
	if(node)
	{
		sabi_eval_node(node, &data);
		bus = data.integer.value;
	}

	node = sabi_ns_search(region, "_SEG");
	if(node)
	{
		sabi_eval_node(node, &data);
		seg = data.integer.value;
	}

	adr = sabi_host_alloc(1, sizeof(sabi_pci_t));
	parent->object.device.adr = adr;

	adr->seg = seg;
	adr->bus = bus;
	adr->slot = ((dev >> 16) & 0xFF);
	adr->func = (dev & 0xFF);

	return adr;
}

sabi_node_t *sabi_pci_next_node(sabi_node_t *node)
{
	int type, search;

	search = 1;

	while((node = sabi_next_node(node, search)))
	{
		type = node->object.type;
		search = 0;

		if(type == SABI_OBJECT_DEVICE)
		{
			if(sabi_check_pnp_id(node, "PNP0A03"))
			{
				return node;
			}
			search = 1;
		}
	}

	return 0;
}

sabi_node_t *sabi_pci_find_device(sabi_node_t *node, int slot, int fn)
{
	uint32_t type, adr;
	sabi_node_t *child;
	sabi_data_t data;

	adr = ((slot << 16) | fn);
	node = node->child;

	while(node)
	{
		type = node->object.type;

		if(type == SABI_OBJECT_DEVICE)
		{
			child = sabi_ns_exists(node, "_ADR");
			if(child)
			{
				sabi_eval_node(child, &data);
				if(data.integer.value == adr)
				{
					return node;
				}
			}
		}

		node = node->next;
	}

	return 0;
}

int sabi_pci_eval_prt(sabi_node_t *parent, sabi_prt_t **prt)
{
	sabi_node_t *node;
	sabi_data_t data;
	sabi_prt_t *list;
	int retv;

	node = sabi_ns_exists(parent, "_PRT");
	retv = 0;
	list = 0;

	if(node)
	{
		sabi_eval_node(node, &data);

		if(data.type == SABI_DATA_PACKAGE)
		{
			retv = data.package.count;
			list = sabi_host_alloc(retv, sizeof(sabi_prt_t));
			for(int i = 0; i < retv; i++)
			{
				sabi_pci_package(parent, data.package.data+i, list+i);
			}
		}
		else
		{
			retv = -SABI_ERROR_TYPE;
		}

		sabi_clean_data(&data);
	}
	else
	{
		retv = -SABI_ERROR_LOOKUP;
	}

	*prt = list;
	return retv;
}

int sabi_pci_eval_bbn(sabi_node_t *parent)
{
	sabi_descriptor_t desc;
	sabi_resource_t res;
	sabi_node_t *node;
	sabi_data_t data;
	int status, retv;

	// Try BBN
	node = sabi_ns_exists(parent, "_BBN");
	if(node)
	{
		sabi_eval_node(node, &data);
		return data.integer.value;
	}

	// Try CRS
	status = sabi_eval_resource(parent, &res);
	if(status < 0)
	{
		return 0;
	}

	sabi_next_resource(&res, &desc);
	if(desc.type == 0x88)
	{
		retv = desc.u.res88->min;
	}
	else
	{
		retv = 0;
	}
	sabi_free_resource(&res);

	// Return bus number
	return retv;
}
