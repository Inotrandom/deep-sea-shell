#ifndef DSS_ENV_H
#define DSS_ENV_H

#include <stdbool.h>

#include "DSS_mem.h"
#include "DSS_command.h"
#include "DSS_event.h"
#include "DSS_parser.h"

#define DSS_MAX_DEFINED_COMMANDS 1024

typedef struct
{
	DSS_mem_t *mem;

	DSS_command_t **defined_commands;
	unsigned int defined_commands_n;

	DSS_event_t *event_destroying;
} DSS_env_t;

bool DSS_env_commands_in_bounds(DSS_env_t *this)
{
	if (this->defined_commands_n >= DSS_MAX_DEFINED_COMMANDS)
	{
		return false;
	}

	return true;
}

void DSS_env_exec(DSS_env_t *this, char *what)
{
	char **lines = DSS_string_parse(what, "\n", false);

	for (char **line = lines; *line != NULL; ++line)
	{
		char **tokens = DSS_string_parse(*line, " ", true);
		unsigned int n_tokens = DSS_string_array_len(tokens);

		if (n_tokens == 0)
		{
			continue;
		}

		char *id = strdup(tokens[0]);

		for (DSS_command_t **iter = this->defined_commands; iter < (this->defined_commands + this->defined_commands_n); ++iter)
		{
			DSS_command_match_call(*iter, id, this->mem, n_tokens - 1, tokens + 1);

			// TODO: error handling function
		}

		free(id);
		id = NULL;

		if (tokens != NULL)
		{
			DSS_string_array_destroy(tokens);
			tokens = NULL;
		}
	}
}

void DSS_env_define_command(DSS_env_t *this, char *id, DSS_command_funcptr_t func)
{
	if (DSS_env_commands_in_bounds(this) == false)
	{
		perror("DSS_env_define_command() commands out of bounds; too many commands defined.\n");
		return;
	}

	DSS_command_t *new_command = DSS_command_create(id, func);
	this->defined_commands[this->defined_commands_n] = new_command;
	++this->defined_commands_n;
}

DSS_env_t *DSS_env_create(void)
{
	DSS_env_t *this = (DSS_env_t *)malloc(sizeof(DSS_env_t));
	this->defined_commands = (DSS_command_t **)malloc(sizeof(DSS_command_t) * DSS_MAX_DEFINED_COMMANDS);
	this->defined_commands_n = 0;

	this->event_destroying = DSS_event_create();

	return this;
}

void DSS_env_destroy(DSS_env_t *this)
{
	// (Handled on the same thread and procedurally, thus memory-safe)
	DSS_event_fire(this->event_destroying, this);

	for (DSS_command_t **iter = this->defined_commands; iter < (this->defined_commands + this->defined_commands_n); ++iter)
	{
		DSS_command_destroy(*iter);
	}

	free(this->defined_commands);
	this->defined_commands = NULL;

	DSS_event_destroy(this->event_destroying);
	free(this);
	this = NULL;
}

#endif // DSS_ENV_H
