#ifndef DSS_MEMPAIR_H
#define DSS_MEMPAIR_H

#include <stdlib.h>
#include <string.h>

typedef struct
{
	char *key;
	void *pair;
	unsigned int pair_len;
} DSS_mempair_t;

/// Will allocate a copy of the `key` argument.
void DSS_mempair_fill_key(DSS_mempair_t *this, const char *key) { this->key = strdup(key); }

DSS_mempair_t *DSS_mempair_create(char *key, void *pair, int pair_len)
{
	DSS_mempair_t *this = (DSS_mempair_t *)malloc(sizeof(DSS_mempair_t));
	this->key = key;
	this->pair = pair;
	this->pair_len = pair_len;

	return this;
}

/// Does not free the pair data. This should not be called directly.
void DSS_mempair_destroy(DSS_mempair_t *this)
{
	free(this->key);
	free(this);
}

#endif // DSS_MEMPAIR_H
