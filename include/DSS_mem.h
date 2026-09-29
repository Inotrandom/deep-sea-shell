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

DSS_mempair_t **DSS_mem_end(DSS_mem_t *this) { return (this->data + this->data_n); }

/// Returns a pointer to a pair for the corresponding `key`.
void *DSS_mem_get(DSS_mem_t *this, char *key)
{
	void *found = NULL;
	for (DSS_mempair_t **iter = this->data; iter < DSS_mem_end(this); ++iter)
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

bool DSS_mem_in_bounds(DSS_mem_t *this)
{
	if (this->data_n >= this->data_maxn)
	{
		return false;
	}

	return true;
}

void DSS_mem_expand(DSS_mem_t *this)
{
	this->data_maxn *= DSS_MEM_EXPANSION_COEFFICIENT;
	this->data = (DSS_mempair_t **)realloc(this->data, sizeof(DSS_mempair_t *) * this->data_maxn);
}

void DSS_mem_push_back(DSS_mem_t *this, DSS_mempair_t *pair)
{
	if (DSS_mem_in_bounds(this) == false)
	{
		DSS_mem_expand(this);
	}

	*DSS_mem_end(this) = pair;
	++(this->data_n);
}

DSS_mem_t *DSS_mem_create(void)
{
	DSS_mem_t *this = (DSS_mem_t *)malloc(sizeof(DSS_mempair_t));
	this->data_maxn = DSS_MEM_STARTING_SIZE;
	this->data_n = 0;
	this->data = (DSS_mempair_t **)malloc(sizeof(DSS_mempair_t *) * DSS_MEM_STARTING_SIZE);

	return this;
}

/// This should not be called directly, as it will leave memory dangling.
void DSS_mem_destroy(DSS_mem_t *this)
{
	for (DSS_mempair_t **iter = this->data; iter < DSS_mem_end(this); ++iter)
	{
		DSS_mempair_destroy(*iter);
	}
	free(this->data);
	this->data = NULL;
	free(this);
	this = NULL;
}

#endif // DSS_MEM_H
