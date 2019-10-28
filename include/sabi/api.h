#ifndef SABI_API_H
#define SABI_API_H

#include <sabi/global.h>

// Evaluate nodes
int sabi_eval_method(sabi_node_t *node, int argc, sabi_data_t *argv, sabi_data_t *retv);
int sabi_eval_node(sabi_node_t *node, sabi_data_t *retv);

// Namespace iterator
sabi_node_t *sabi_next_node(sabi_node_t *node, int child);

// Path resolution
sabi_node_t *sabi_resolve_child(sabi_node_t *parent, const char *name);
sabi_node_t *sabi_resolve_search(sabi_node_t *parent, const char *name);
sabi_node_t *sabi_resolve_path(sabi_node_t *parent, const char *path);

// Data manipulation
void sabi_clean_data(sabi_data_t *sptr);
void sabi_clone_data(sabi_data_t *dptr, sabi_data_t *sptr);

// Hardware identification
uint32_t sabi_eisaid(const char *name);
int sabi_check_pnp_id(sabi_node_t *parent, uint32_t id);

// Tables
void sabi_register_table(uint64_t);

#endif
