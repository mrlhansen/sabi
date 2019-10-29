#include <sabi/host.h>

void *sabi_host_alloc(int count, int size)
{
	// Allocate (count * size) bytes of zero-initialized memory
	return 0;
}

void sabi_host_free(void *ptr)
{
	// Free memory
}

void sabi_host_debug(const char *file, int line, const char *fmt, ...)
{
	// Called whenever SABI emits a debug message
}

void sabi_host_panic()
{
	// Called whenever SABI encounters a fatal error
	// This function must not return
	while(1);
}

uint64_t sabi_host_map(uint64_t address)
{
	// Map a physical address into a virtual address
	return 0;
}

void sabi_host_sleep(uint64_t usec)
{
	// Microsecond sleep
}

void sabi_host_pmio_read(uint16_t address, uint64_t *value, int width)
{
	// Read from an IO port using the specified access width
	*value = 0;
}

void sabi_host_pmio_write(uint16_t address, uint64_t value, int width)
{
	// Write to an IO port using the specified access width
}

void sabi_host_pci_read(sabi_pci_t *dev, uint32_t reg, uint64_t *value, int width)
{
	// Read from a PCI device register using the specified access width
	*value = 0;
}

void sabi_host_pci_write(sabi_pci_t *dev, uint32_t reg, uint64_t value, int width)
{
	// Write to a PCI device register using the specified access width
}
