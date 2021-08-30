#include <sabi/api.h>
#include <sabi/namespace.h>
#include <sabi/tables.h>
#include <sabi/parser.h>
#include <sabi/state.h>
#include <sabi/host.h>
#include <string.h>

static fadt_t *fadt = 0;

static void parse_table(uint64_t address, uint32_t size)
{
	uint8_t *end;
	state_t *state;

	sabi_ns_init();
	state = sabi_new_state();

	state->aml = (uint8_t*)address;
	end = (uint8_t*)(address + size);
	sabi_parse_termlist(state, end);

	state->stack = 0;
	sabi_free_state(state);
}

fadt_t *sabi_fadt_table(void)
{
	return fadt;
}

void sabi_register_table(uint64_t address)
{
	int valid, size;
	hdr_t *hdr;

	hdr = (hdr_t*)address;
	valid = 0;

	if(strncmp(hdr->signature, "FACP", 4) == 0)
	{
		fadt = (fadt_t*)hdr;

		if(fadt->dsdt)
		{
			address = fadt->dsdt;
		}
		else
		{
			address = fadt->x_dsdt;
		}

		address = sabi_host_map(address);
		hdr = (hdr_t*)address;
		valid = 1;
	}
	else if(strncmp(hdr->signature, "DSDT", 4) == 0)
	{
		valid = 1;
	}
	else if(strncmp(hdr->signature, "SSDT", 4) == 0)
	{
		valid = 1;
	}

	if(valid)
	{
		size = (hdr->length - 36);
		address = (address + 36);
		parse_table(address, size);
	}
}
