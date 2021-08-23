#ifndef SABI_TABLES_H
#define SABI_TABLES_H

#include <stdint.h>

typedef struct {
	uint8_t address_space_id;
	uint8_t register_bit_width;
	uint8_t register_bit_offset;
	uint8_t access_size;
	uint64_t address;
} __attribute__((packed)) gas_t;

typedef struct {
	char signature[4];
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;
	char oem_id[6];
	char oem_table[8];
	uint32_t oem_revision;
	char creator_id[4];
	uint32_t creator_revision;
} __attribute__((packed)) hdr_t;

typedef struct {
	hdr_t hdr;
	uint32_t firmware_ctrl;
	uint32_t dsdt;
	uint8_t reserved;
	uint8_t preferred_pm_profile;
	uint16_t sci_int;
	uint32_t smi_cmd;
	uint8_t acpi_enable;
	uint8_t acpi_disable;
	uint8_t s4bios_req;
	uint8_t pstate_cnt;
	uint32_t pm1a_evt_blk;
	uint32_t pm1b_evt_blk;
	uint32_t pm1a_cnt_blk;
	uint32_t pm1b_cnt_blk;
	uint32_t pm2_cnt_blk;
	uint32_t pm_tmr_blk;
	uint32_t gpe0_blk;
	uint32_t gpe1_blk;
	uint8_t pm1_evt_len;
	uint8_t pm1_cnt_len;
	uint8_t pm2_cnt_len;
	uint8_t pm_tmr_len;
	uint8_t gpe0_blk_len;
	uint8_t gpe1_blk_len;
	uint8_t gpe1_base;
	uint8_t cst_cnt;
	uint16_t p_lvl2_lat;
	uint16_t p_lvl3_lat;
	uint16_t flush_size;
	uint16_t flush_stride;
	uint8_t duty_offset;
	uint8_t duty_width;
	uint8_t day_alarm;
	uint8_t mon_alarm;
	uint8_t century;
	uint16_t iapc_boot_arch;
	uint8_t reserved2;
	uint32_t flags;
	gas_t reset_reg;
	uint8_t reset_value;
	uint8_t reserved3[3];
	uint64_t x_firmware_control;
	uint64_t x_dsdt;
	gas_t x_pm1a_evt_blk;
	gas_t x_pm1b_evt_blk;
	gas_t x_pm1a_cnt_blk;
	gas_t x_pm1b_cnt_blk;
	gas_t x_pm2_cnt_blk;
	gas_t x_pm_tmr_blk;
	gas_t x_gpe0_blk;
	gas_t x_gpe1_blk;
	gas_t sleep_control_reg;
	gas_t sleep_status_reg;
} __attribute__((packed)) fadt_t;

fadt_t *sabi_fadt_table(void);

#endif
