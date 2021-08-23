#ifndef SABI_NAMESPACE_H
#define SABI_NAMESPACE_H

#include <sabi/global.h>
#include <sabi/state.h>
#include <stdint.h>

sabi_node_t *sabi_ns_exists(sabi_node_t*, const char*);
sabi_node_t *sabi_ns_search(sabi_node_t*, const char*);
sabi_node_t *sabi_ns_path(sabi_node_t*, const char*);
sabi_node_t *sabi_ns_find(sabi_node_t*, sabi_name_t*);

void sabi_ns_add_scope(state_t*, sabi_name_t*, sabi_object_t*);
sabi_node_t *sabi_ns_add_object(state_t*, sabi_name_t*, sabi_object_t*);

sabi_node_t *sabi_ns_root(void);
void sabi_ns_unlink(sabi_node_t*);
void sabi_ns_init(void);

#endif
