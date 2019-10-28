#ifndef SABI_HOST_H
#define SABI_HOST_H

#include <sabi/pci.h>
#include <stdint.h>

#ifdef NDEBUG
#define sabi_debug(fmt...) ((void)0)
#define sabi_fatal(fmt...) sabi_host_panic()
#else
#define sabi_debug(fmt...) sabi_host_debug(__FILE__, __LINE__, fmt)
#define sabi_fatal(fmt...) sabi_host_debug(__FILE__, __LINE__, fmt); sabi_host_panic()
#endif

void *sabi_host_alloc(int, int);
void sabi_host_free(void*);

void sabi_host_debug(const char*, int, const char*, ...);
void sabi_host_panic() __attribute__((noreturn));

uint64_t sabi_host_map(uint64_t);
void sabi_host_sleep(uint64_t);

void sabi_host_pmio_read(uint16_t, uint64_t*, int);
void sabi_host_pmio_write(uint16_t, uint64_t, int);

void sabi_host_pci_read(sabi_pci_t*, uint32_t, uint64_t*, int);
void sabi_host_pci_write(sabi_pci_t*, uint32_t, uint64_t, int);

#endif
