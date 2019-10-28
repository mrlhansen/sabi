#include <sabi/fields.h>
#include <sabi/host.h>
#include <sabi/ec.h>

static uint64_t system_read(uint64_t address, void *devptr, int space, int width)
{
	uint64_t value;
	uint8_t *u8ptr;

	value = 0;
	u8ptr = (uint8_t*)&value;

	if(space == 0x00)
	{
		switch(width)
		{
			case 64:
				value = *(volatile uint64_t*)address;
				break;
			case 32:
				value = *(volatile uint32_t*)address;
				break;
			case 16:
				value = *(volatile uint16_t*)address;
				break;
			case 8:
				value = *(volatile uint8_t*)address;
				break;
		}
	}
	else if(space == 0x01)
	{
		sabi_host_pmio_read(address, &value, width);
	}
	else if(space == 0x02)
	{
		sabi_host_pci_read(devptr, address, &value, width);
	}
	else if(space == 0x03)
	{
		sabi_ec_read(devptr, address, u8ptr);
	}
	else
	{
		sabi_fatal("Unimplemented IO space");
	}

	return value;
}

static void system_write(uint64_t address, void *devptr, int space, int width, uint64_t value)
{
	if(space == 0x00)
	{
		switch(width)
		{
			case 64:
				*(volatile uint64_t*)address = value;
				break;
			case 32:
				*(volatile uint32_t*)address = value;
				break;
			case 16:
				*(volatile uint16_t*)address = value;
				break;
			case 8:
				*(volatile uint8_t*)address = value;
				break;
		}
	}
	else if(space == 0x01)
	{
		sabi_host_pmio_write(address, value, width);
	}
	else if(space == 0x02)
	{
		sabi_host_pci_write(devptr, address, value, width);
	}
	else if(space == 0x03)
	{
		sabi_ec_write(devptr, address, value);
	}
	else
	{
		sabi_fatal("Unimplemented IO space");
	}
}

static int access_width(int access, int width)
{
	switch(access)
	{
		case 0:
			break;
		case 1:
			width = 8;
			break;
		case 2:
			width = 16;
			break;
		case 3:
			width = 32;
			break;
		case 4:
			width = 64;
			break;
		default:
			width = 0;
			break;
	}
	return width;
}

static void access_align(align_t *rs, int size, int offset, int width, int update, int rw)
{
	int address, count, bytes;
	int read, partial;

	// Parameters
	address = ((offset/8) & ~((width/8)-1));
	offset = (offset % width);
	count = (offset + size + width - 1) / width;
	bytes = (count * width) / 8;

	// Invalid
	if(bytes > 8)
	{
		sabi_fatal("unable to align access (count=%d, width=%d, offset=%d)", count, width, offset);
	}

	// Partial value needed
	if(offset || (size % width))
	{
		partial = 1;
	}
	else
	{
		partial = 0;
	}

	// Reading necessary
	if(rw)
	{
		if((update == 0) && partial)
		{
			read = 1;
		}
		else
		{
			read = 0;
		}
	}
	else
	{
		read = 1;
	}

	// Store values
	rs->address = address;
	rs->offset = offset;
	rs->size = size;
	rs->count = count;
	rs->read = read;
	rs->partial = partial;
	rs->update = update;
}

static uint64_t access_mask(align_t *rs, uint64_t value)
{
	uint8_t size, offset;
	uint64_t mask;

	if(rs->partial)
	{
		size = rs->size;
		offset = rs->offset;
		mask = ((1UL << size) - 1);
		value = ((value >> offset) & mask);
	}

	return value;
}

static uint64_t access_preserve(align_t *rs, uint64_t old, uint64_t new)
{
	uint8_t size, offset;
	uint64_t mask;

	if(rs->partial == 0)
	{
		return new;
	}

	if(rs->update == 1)
	{
		old = ~0UL;
	}
	else if(rs->update == 2)
	{
		old = 0;
	}

	size = rs->size;
	offset = rs->offset;

	mask = ((1UL << size) - 1);
	new = ((new & mask) << offset);
	mask = ~(mask << offset);
	new = ((old & mask) | new);

	return new;
}

void sabi_access_field(sabi_object_t *object, uint64_t *ioval, int rw)
{
	uint64_t address, offset, input, value, size;
	uint8_t access, update, space, width, bw;
	sabi_object_t *parent;
	void *devptr;
	align_t rs;

	// Arguments
	parent = object->field.parent;
	space = parent->region.space;
	address = parent->region.offset;
	offset = object->field.offset;
	size = object->field.length;
	access = (object->field.flags & 0x0F);
	update = ((object->field.flags >> 5) & 0x03);
	devptr = parent->region.adr;
	input = *ioval;
	value = 0;

	// Access width
	width = access_width(access, 8);
	if(width == 0)
	{
		sabi_fatal("unhandled access type %d", access);
	}

	// Align access
	access_align(&rs, size, offset, width, update, rw);
	size = (rs.count - 1);
	address = (address + rs.address);
	bw = (width / 8);

	// Read data
	if(rs.read)
	{
		for(int i = size; i >= 0; i--)
		{
			offset = address + (i * bw);
			value = (value << width);
			value |= system_read(offset, devptr, space, width);
		}
	}

	// Return if reading
	if(rw == 0)
	{
		*ioval = access_mask(&rs, value);
		return;
	}

	// Preserve
	value = access_preserve(&rs, value, input);

	// Write
	for(int i = 0; i <= size; i++)
	{
		offset = address + (i * bw);
		system_write(offset, devptr, space, width, value);
		value = (value >> width);
	}
}

void sabi_access_index_field(sabi_object_t *object, uint64_t *ioval, int rw)
{
	uint64_t address, offset, input, value, temp, size;
	uint8_t access, update, width, bw;
	sabi_object_t *index, *data;
	align_t rs;

	// Arguments
	index = object->indexfield.index;
	data = object->indexfield.data;
	offset = object->indexfield.offset;
	size = object->indexfield.length;
	access = (object->indexfield.flags & 0x0F);
	update = ((object->indexfield.flags >> 5) & 0x03);
	input = *ioval;
	value = 0;

	// Access width
	width = access_width(access, 8);
	if(width == 0)
	{
		sabi_fatal("unhandled access type %d", access);
	}

	// Align access
	access_align(&rs, size, offset, width, update, rw);
	size = (rs.count - 1);
	address = rs.address;
	bw = (width / 8);

	// Read data
	if(rs.read)
	{
		for(int i = size; i >= 0; i--)
		{
			offset = address + (i * bw);
			value = (value << width);
			sabi_access_field(index, &offset, 1);
			sabi_access_field(data, &temp, 0);
			value |= temp;
		}
	}

	// Return if reading
	if(rw == 0)
	{
		*ioval = access_mask(&rs, value);
		return;
	}

	// Preserve
	value = access_preserve(&rs, value, input);

	// Write
	for(int i = 0; i <= size; i++)
	{
		offset = address + (i * bw);
		sabi_access_field(index, &offset, 1);
		sabi_access_field(data, &value, 1);
		value = (value >> width);
	}
}

void sabi_access_buffer_field(sabi_data_t *data, uint64_t *ioval, int rw)
{
	uint64_t value, input;
	int offset, size;
	align_t rs;

	// Arguments
	size = data->field.size;
	offset = data->field.offset;
	input = *ioval;
	value = 0;

	// Align access
	access_align(&rs, size, offset, 8, 0, rw);
	size = (rs.count - 1);

	// Read
	if(rs.read)
	{
		for(int i = size; i >= 0; i--)
		{
			value = (value << 8);
			value |= data->field.ptr[i];
		}
	}

	// Return if reading
	if(rw == 0)
	{
		*ioval = access_mask(&rs, value);
		return;
	}

	// Preserve
	value = access_preserve(&rs, value, input);

	// Write
	for(int i = 0; i <= size; i++)
	{
		data->field.ptr[i] = (value & 0xFF);
		value = (value >> 8);
	}
}
