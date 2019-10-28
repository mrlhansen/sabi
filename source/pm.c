#include <sabi/namespace.h>
#include <sabi/tables.h>
#include <sabi/host.h>
#include <sabi/api.h>
#include <sabi/pm.h>

static void slp_package(sabi_data_t *pkg, int *sa, int *sb)
{
	sabi_data_t *data;

	data = pkg->package.data+0;
	*sa = (data->integer.value & 0x07);

	data = pkg->package.data+1;
	*sb = (data->integer.value & 0x07);
}

int sabi_pm_sleep(int state)
{
	// not yet implemented
	return -1;
}

void sabi_pm_soft_off()
{
	sabi_node_t *root, *node;
	sabi_data_t data;
	uint32_t pm1a, pm1b;
	int slpa, slpb;
	uint64_t reg;
	fadt_t *fadt;

	root = sabi_ns_root();
	fadt = sabi_fadt_table();

	if(fadt == 0)
	{
		return;
	}

	pm1a = fadt->pm1a_cnt_blk;
	pm1b = fadt->pm1b_cnt_blk;

	node = sabi_ns_exists(root, "_S5_");
	if(node == 0)
	{
		return;
	}

	sabi_eval_node(node, &data);
	slp_package(&data, &slpa, &slpb);
	sabi_clean_data(&data);

	node = sabi_ns_exists(root, "_PTS");
	if(node == 0)
	{
		return;
	}

	data.integer.type = SABI_DATA_INTEGER;
	data.integer.value = 5;
	sabi_eval_method(node, 1, &data, 0);

	sabi_host_pmio_read(pm1a, &reg, 16);
	reg = ((reg & 0xC3FF) | (slpa << 10) | (1 << 13));
	sabi_host_pmio_write(pm1a, reg, 16);

	if(pm1b)
	{
		sabi_host_pmio_read(pm1b, &reg, 16);
		reg = ((reg & 0xC3FF) | (slpb << 10) | (1 << 13));
		sabi_host_pmio_write(pm1b, reg, 16);
	}
}
