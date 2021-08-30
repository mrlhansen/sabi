#include <sabi/namespace.h>
#include <sabi/events.h>
#include <sabi/tables.h>
#include <sabi/host.h>
#include <sabi/api.h>
#include <sabi/ec.h>

static void sabi_call_init(void)
{
	sabi_node_t *node, *snode, *inode;
	int sta, type, child;
	sabi_data_t data;

	node = 0;
	child = 0;

	while((node = sabi_next_node(node, child)))
	{
		type = node->object.type;
		sta = 0x0F;
		child = 0;

		if(type & SABI_OBJECT_SCOPE)
		{
			inode = sabi_ns_exists(node, "_INI");
			if(inode)
			{
				snode = sabi_ns_exists(node, "_STA");
				if(snode)
				{
					sabi_eval_node(snode, &data);
					sta = data.integer.value;
				}
				if(sta & 0x01)
				{
					sabi_eval_node(inode, &data);
				}
			}
			if(sta & 0x09)
			{
				child = 1;
			}
		}
	}
}

static void sabi_call_pic(int mode)
{
	sabi_node_t *node;
	sabi_data_t argv;

	node = sabi_resolve_path(0, "\\_PIC");
	if(node == 0)
	{
		return;
	}

	argv.integer.type = SABI_DATA_INTEGER;
	argv.integer.value = mode;

	sabi_eval_method(node, 1, &argv, 0);
}

uint16_t sabi_read_event(void)
{
	uint32_t pm1a, pm1b;
	uint64_t a, b;
	fadt_t *fadt;

	fadt = sabi_fadt_table();
	if(fadt == 0)
	{
		return 0;
	}

	a = 0;
	b = 0;
	pm1a = fadt->pm1a_evt_blk;
	pm1b = fadt->pm1b_evt_blk;

	if(pm1a)
	{
		sabi_host_pmio_read(pm1a, &a, 16);
		sabi_host_pmio_write(pm1a, a, 16);
	}

	if(pm1b)
	{
		sabi_host_pmio_read(pm1b, &b, 16);
		sabi_host_pmio_write(pm1b, b, 16);
	}

	return (a | b);
}

void sabi_write_event(uint16_t value)
{
	uint32_t pm1a, pm1b;
	fadt_t *fadt;

	fadt = sabi_fadt_table();
	if(fadt == 0)
	{
		return;
	}

	pm1a = fadt->pm1a_evt_blk + (fadt->pm1_evt_len / 2);
	pm1b = fadt->pm1b_evt_blk + (fadt->pm1_evt_len / 2);

	if(fadt->pm1a_evt_blk)
	{
		sabi_host_pmio_write(pm1a, value, 16);
	}

	if(fadt->pm1b_evt_blk)
	{
		sabi_host_pmio_write(pm1b, value, 16);
	}
}

int sabi_enable_acpi(int mode)
{
	fadt_t *fadt;
	uint64_t reg;

	fadt = sabi_fadt_table();
	if(fadt == 0)
	{
		return -1;
	}

	sabi_call_init();
	sabi_call_pic(mode);
	sabi_ec_call_reg();

	sabi_host_pmio_read(fadt->pm1a_cnt_blk, &reg, 16);
	if(reg & 0x01)
	{
		return 0;
	}

	sabi_host_pmio_write(fadt->smi_cmd, fadt->acpi_enable, 8);
	sabi_host_sleep(10000);

	for(int i = 0; i < 100; i++)
	{
		sabi_host_pmio_read(fadt->pm1a_cnt_blk, &reg, 16);
		if(reg & 0x01)
		{
			break;
		}
		sabi_host_sleep(10000);
	}

	if((reg & 0x01) == 0)
	{
		return -1;
	}

	sabi_write_event(ACPI_POWER_BUTTON | ACPI_SLEEP_BUTTON);
	sabi_read_event();

	return 0;
}
