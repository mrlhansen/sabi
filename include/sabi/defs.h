#ifndef SABI_DEFS_H
#define SABI_DEFS_H

#include <sabi/global.h>
#include <sabi/state.h>

void sabi_def_acquire(state_t*, uint64_t*);
void sabi_def_alias(state_t*);
void sabi_def_bank_field(state_t*);
void sabi_def_buffer(state_t*, sabi_data_t*);
void sabi_def_condrefof(state_t*, uint64_t*);
void sabi_def_convertbcd(state_t*, uint64_t*, int);
void sabi_def_create_field(state_t*, int, int);
void sabi_def_device(state_t*);
void sabi_def_divide(state_t*, uint64_t*);
void sabi_def_event(state_t*);
void sabi_def_external(state_t*);
void sabi_def_field(state_t*);
void sabi_def_findsetbit(state_t*, uint64_t*, int);
void sabi_def_if_else(state_t*);
void sabi_def_increment(state_t*, uint64_t*, int);
void sabi_def_index_field(state_t*);
void sabi_def_method(state_t*);
void sabi_def_mutex(state_t*);
void sabi_def_name(state_t*);
void sabi_def_notify(state_t*);
void sabi_def_objecttype(state_t*, uint64_t*);
void sabi_def_op_region(state_t*);
void sabi_def_package(state_t*, sabi_data_t*, int);
void sabi_def_power_res(state_t*);
void sabi_def_processor(state_t*);
void sabi_def_release(state_t*);
void sabi_def_reset(state_t*);
void sabi_def_return(state_t*);
void sabi_def_scope(state_t*);
void sabi_def_signal(state_t*);
void sabi_def_sizeof(state_t*, uint64_t*);
void sabi_def_sleep(state_t*);
void sabi_def_stall(state_t*);
void sabi_def_store(state_t*);
void sabi_def_thermal_zone(state_t*);
void sabi_def_tobuffer(state_t*, sabi_data_t*);
void sabi_def_tointeger(state_t*, uint64_t*);
void sabi_def_tostring(state_t*, sabi_data_t*);
void sabi_def_tobasestring(state_t*, sabi_data_t*, int);
void sabi_def_wait(state_t*, uint64_t*);
void sabi_def_while(state_t*);

#endif
