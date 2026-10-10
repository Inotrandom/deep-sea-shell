#ifndef DSS_MEM_H
#define DSS_MEM_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "DSS_mempair.h"

#define DSS_MEM_STARTING_SIZE 8
#define DSS_MEM_EXPANSION_COEFFICIENT 2

typedef struct
{
	DSS_mempair_t **data;
	unsigned int data_maxn;
	unsigned int data_n;
} DSS_mem_t;

DSS_mempair_t **DSS_mem_end(DSS_mem_t *self) { return (self->data + self->data_n); }

/// Returns a pointer to a pair for the corresponding `key`.
void *DSS_mem_get(DSS_mem_t *self, char *key)
{
	void *found = NULL;
	for (DSS_mempair_t **iter = self->data; iter < DSS_mem_end(self); ++iter)
	{
		if ((**iter).key == NULL)
		{
			continue;
		}

		if (strcmp((**iter).key, key) != 0)
		{
			continue;
		}

		found = (**iter).pair;
		break;
	}

	return found;
}

bool DSS_mem_in_bounds(DSS_mem_t *self)
{
	if (self->data_n >= self->data_maxn)
	{
		return false;
	}

	return true;
}

void DSS_mem_expand(DSS_mem_t *self)
{
	self->data_maxn *= DSS_MEM_EXPANSION_COEFFICIENT;
	self->data = (DSS_mempair_t **)realloc(self->data, sizeof(DSS_mempair_t *) * self->data_maxn);
}

void DSS_mem_push_back(DSS_mem_t *self, DSS_mempair_t *pair)
{
	if (DSS_mem_in_bounds(self) == false)
	{
		DSS_mem_expand(self);
	}

	*DSS_mem_end(self) = pair;
	++(self->data_n);
}

DSS_mem_t *DSS_mem_create(void)
{
	DSS_mem_t *self = (DSS_mem_t *)malloc(sizeof(DSS_mempair_t));
	self->data_maxn = DSS_MEM_STARTING_SIZE;
	self->data_n = 0;
	self->data = (DSS_mempair_t **)malloc(sizeof(DSS_mempair_t *) * DSS_MEM_STARTING_SIZE);

	return self;
}

/// This should not be called directly, as it will leave memory dangling.
void DSS_mem_destroy(DSS_mem_t *self)
{
	for (DSS_mempair_t **iter = self->data; iter < DSS_mem_end(self); ++iter)
	{
		DSS_mempair_destroy(*iter);
	}
	free(self->data);
	self->data = NULL;
	free(self);
	self = NULL;
}

#endif // DSS_MEM_H
