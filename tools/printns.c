#include <sabi/namespace.h>
#include <sabi/api.h>
#include <stdlib.h>
#include <stdio.h>

static void print_child(sabi_node_t *node, int level, uint32_t bits)
{
	while(node)
	{
		for(int i = 0; i < level; i++)
		{
			if(bits & (1u << i))
			{
				printf("|    ");
			}
			else
			{
				printf("     ");
			}
		}

		if(node->next)
		{
			printf("|--- %.4s\n", (char*)&node->name);
		}
		else
		{
			printf("+--- %.4s\n", (char*)&node->name);
			bits &= ~(1u << level);
		}

		if(node->child)
		{
			print_child(node->child, level+1, bits);
		}

		node = node->next;
	}
}

static void sabi_ns_print()
{
	sabi_node_t *root = sabi_ns_root();
	print_child(root, 0, 0xFFFFFFFF);
}

int main(int argc, char *argv[])
{
	uint8_t *aml;
	FILE *fp;
	int fsz;

	if(argc != 2)
	{
		printf("Usage: %s <file.aml>\n", argv[0]);
		return 0;
	}

	fp = fopen(argv[1], "r");
	if(fp == 0)
	{
		perror(argv[1]);
		return -1;
	}

	fseek(fp, 0, SEEK_END);
	fsz = ftell(fp);
	rewind(fp);

	aml = malloc(fsz);
	fread(aml, 1, fsz, fp);
	fclose(fp);

	sabi_register_table((uint64_t)aml);
	sabi_ns_print();

	free(aml);
	return 0;
}
