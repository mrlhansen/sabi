#ifndef SABI_OSI_H
#define SABI_OSI_H

#include <sabi/global.h>
#include <sabi/state.h>

void sabi_osi_parse(state_t*);
void sabi_osi_method(sabi_node_t*);
void sabi_osi_string(sabi_node_t*);
void sabi_osi_revision(sabi_node_t*);

#endif
