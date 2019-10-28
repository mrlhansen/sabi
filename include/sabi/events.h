#ifndef SABI_EVENTS_H
#define SABI_EVENTS_H

#include <stdint.h>

enum {
	ACPI_TIMER        = (1 << 0),
	ACPI_BUS_MASTER   = (1 << 4),
	ACPI_GLOBAL       = (1 << 5),
	ACPI_POWER_BUTTON = (1 << 8),
	ACPI_SLEEP_BUTTON = (1 << 9),
	ACPI_RTC_ALARM    = (1 << 10),
	ACPI_PCIE_WAKE    = (1 << 14),
	ACPI_WAKE         = (1 << 15),
};

uint16_t sabi_read_event();
void sabi_write_event(uint16_t);
int sabi_enable_acpi(int);

#endif
