#include <sabi/namespace.h>
#include <sabi/host.h>
#include <sabi/osi.h>
#include <string.h>

static sabi_node_t *root = 0;

static sabi_node_t *new_node(const char *name)
{
	sabi_node_t *node;
	node = sabi_host_alloc(1, sizeof(sabi_node_t));
	node->name = *(uint32_t*)name;
	return node;
}

static void append_node(sabi_node_t *parent, sabi_node_t *node)
{
	node->parent = parent;
	node->next = parent->child;
	parent->child = node;
}

sabi_node_t *sabi_ns_root(void)
{
	return root;
}

void sabi_ns_unlink(sabi_node_t *node)
{
	sabi_node_t *curr, *prev;

	prev = 0;
	curr = node->parent->child;

	while(curr)
	{
		if(curr == node)
		{
			if(prev == 0)
			{
				node->parent->child = curr->next;
			}
			else
			{
				prev->next = curr->next;
			}
			break;
		}

		prev = curr;
		curr = curr->next;
	}
}

sabi_node_t *sabi_ns_exists(sabi_node_t *node, const char *name)
{
	uint32_t value;

	node = node->child;
	value = *(uint32_t*)name;

	while(node)
	{
		if(node->name == value)
		{
			return node;
		}
		node = node->next;
	}

	return 0;
}

sabi_node_t *sabi_ns_search(sabi_node_t *node, const char *name)
{
	sabi_node_t *child;

	while(node)
	{
		child = sabi_ns_exists(node, name);
		if(child)
		{
			if(child->object.type == SABI_OBJECT_ALIAS)
			{
				return child->object.alias.node;
			}
			else
			{
				return child;
			}
		}
		else
		{
			node = node->parent;
		}
	}

	return 0;
}

sabi_node_t *sabi_ns_path(sabi_node_t *node, const char *path)
{
	int length;

	length = strlen(path);
	if(node == 0)
	{
		node = root;
	}

	for(int n = 0; n < length; n += 4)
	{
		node = sabi_ns_exists(node, path);

		if(node == 0)
		{
			return 0;
		}
		else if(node->object.type == SABI_OBJECT_ALIAS)
		{
			node = node->object.alias.node;
		}

		path += 4;
	}

	return node;
}

sabi_node_t *sabi_ns_find(sabi_node_t *node, sabi_name_t *name)
{
	char *prefix, *path;
	char search;
	int length;

	search = 0;
	prefix = name->prefix;
	path = name->value;
	length = strlen(path);

	if(*prefix == '\0' && length == 4)
	{
		search = 1;
	}

	if(*prefix == '\\')
	{
		node = root;
	}

	while(*prefix == '^')
	{
		node = node->parent;
		if(node == 0)
		{
			return 0;
		}
		prefix++;
	}

	if(search)
	{
		node = sabi_ns_search(node, path);
	}
	else
	{
		node = sabi_ns_path(node, path);
	}

	return node;
}

void sabi_ns_add_scope(state_t *state, sabi_name_t *name, sabi_object_t *object)
{
	sabi_node_t *node, *current;
	char *prefix, *path;
	int length;

	prefix = name->prefix;
	path = name->value;
	length = strlen(path);
	current = state->scope;

	if(*prefix == '\\')
	{
		current = root;
	}

	while(*prefix == '^')
	{
		current = current->parent;
		prefix++;
	}

	for(int n = 0; n < length; n += 4)
	{
		node = sabi_ns_exists(current, path);

		if(node == 0)
		{
			node = new_node(path);
			node->parent = current;
			node->object = *object;

			node->stack = state->stack;
			state->stack = node;
			state->count++;

			node->next = current->child;
			current->child = node;
		}

		current = node;
		path += 4;
	}

	state->scope = current;
}

sabi_node_t *sabi_ns_add_object(state_t *state, sabi_name_t *name, sabi_object_t *object)
{
	sabi_node_t *scope;
	sabi_node_t *node;

	scope = state->scope;
	sabi_ns_add_scope(state, name, object);
	node = state->scope;
	state->scope = scope;

	return node;
}

void sabi_ns_init(void)
{
	sabi_node_t *node;
	sabi_object_t object;

	if(root)
	{
		return;
	}

	root = new_node("ROOT");
	object.device.type = SABI_OBJECT_DEVICE;
	object.device.adr = 0;

	// Add _SB scope
	node = new_node("_SB_");
	node->object = object;
	append_node(root, node);

	// Add _TZ scope
	node = new_node("_TZ_");
	node->object = object;
	append_node(root, node);

	// Add _PR scope
	node = new_node("_PR_");
	node->object = object;
	append_node(root, node);

	// Add _GPE scope
	node = new_node("_GPE");
	node->object = object;
	append_node(root, node);

	// Add _OS object
	node = new_node("_OS_");
	sabi_osi_string(node);
	append_node(root, node);

	// Add _REV object
	node = new_node("_REV");
	sabi_osi_revision(node);
	append_node(root, node);

	// Add _OSI object
	node = new_node("_OSI");
	sabi_osi_method(node);
	append_node(root, node);
}
