#ifndef SABI_PCI_H
#define SABI_PCI_H

#include <sabi/global.h>

enum {
	SABI_IRQ_MODE     = (1 << 0), // 0 = level, 1 = edge
	SABI_IRQ_POLARITY = (1 << 1), // 0 = high, 1 = low
	SABI_IRQ_SHARING  = (1 << 2), // 0 = exclusive, 1 = shared
	SABI_IRQ_WAKE     = (1 << 3), // 0 = no wake, 1 = wake
};

typedef struct {
	int slot;
	int pin;
	int gsi;
	int flags;
} sabi_prt_t;

typedef struct {
	int seg;
	int bus;
	int slot;
	int func;
} sabi_pci_t;

sabi_node_t *sabi_pci_next_node(sabi_node_t *node);
sabi_node_t *sabi_pci_find_device(sabi_node_t *node, int slot, int fn);
int sabi_pci_eval_prt(sabi_node_t *parent, sabi_prt_t **prt);
int sabi_pci_eval_bbn(sabi_node_t *parent);
sabi_pci_t *sabi_pci_resolve_address(sabi_node_t *region);

#endif
