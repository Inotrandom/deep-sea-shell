#ifndef DSS_COMMAND_H
#define DSS_COMMAND_H

#include <stdbool.h>
#include "DSS_mem.h"

typedef int (*DSS_command_funcptr_t)(DSS_mem_t *env_mem, int argc, char **argv);

typedef struct
{
	char *id;
	DSS_command_funcptr_t command;
} DSS_command_t;

int DSS_command_call(DSS_command_t *this, DSS_mem_t *env_mem, int argc, char **argv) { return this->command(env_mem, argc, argv); }

/// Check identifiers before calling
int DSS_command_match_call(DSS_command_t *this, char *id, DSS_mem_t *env_mem, int argc, char **argv)
{
	if (strcmp(this->id, id) != 0)
	{
		return -1;
	}

	return DSS_command_call(this, env_mem, argc, argv);
}

DSS_command_t *DSS_command_create(const char *id, DSS_command_funcptr_t command)
{
	DSS_command_t *this = (DSS_command_t *)malloc(sizeof(DSS_command_t));
	this->command = command;
	this->id = strdup(id);
	return this;
}

void DSS_command_destroy(DSS_command_t *this)
{
	free(this->id);
	this->id = NULL;
	free(this);
	this = NULL;
}

#endif // DSS_COMMAND_H
