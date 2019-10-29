#include <sabi/host.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

void *sabi_host_alloc(int count, int size)
{
	return calloc(count, size);
}

void sabi_host_free(void *ptr)
{
	free(ptr);
}

void sabi_host_debug(const char *file, int line, const char *fmt, ...)
{
	char obuf[256];
	va_list args;

	va_start(args, fmt);
	vsprintf(obuf, fmt, args);
	va_end(args);

	printf("%s:%d: %s\n", file, line, obuf);
}

void sabi_host_panic()
{
	exit(1);
}

uint64_t sabi_host_map(uint64_t address)
{
	static uint64_t test;
	return (uint64_t)&test;
}

void sabi_host_sleep(uint64_t usec)
{

}

void sabi_host_pmio_read(uint16_t address, uint64_t *value, int width)
{
	*value = 0;
}

void sabi_host_pmio_write(uint16_t address, uint64_t value, int width)
{

}

void sabi_host_pci_read(sabi_pci_t *dev, uint32_t reg, uint64_t *value, int width)
{
	*value = 0;
}

void sabi_host_pci_write(sabi_pci_t *dev, uint32_t reg, uint64_t value, int width)
{

}
