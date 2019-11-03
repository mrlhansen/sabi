#ifndef SABI_CONV_H
#define SABI_CONV_H

#include <sabi/global.h>

int sabi_atoi(uint8_t, int);
int sabi_itoa(char*, uint64_t, int);

void sabi_conv_tobuffer(sabi_data_t*, sabi_data_t*);
void sabi_conv_tointeger(sabi_data_t*, sabi_data_t*);
void sabi_conv_tostring(sabi_data_t*, sabi_data_t*, int);
void sabi_conv_tobasestring(sabi_data_t*, sabi_data_t*, int);

#endif
